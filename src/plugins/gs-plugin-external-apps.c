/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2016 Endless Mobile, Inc
 *
 * Author: Joaquim Rocha <jrocha@endlessm.com>
 *
 * Licensed under the GNU General Public License Version 2
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <config.h>
#include <flatpak.h>
#include <glib.h>
#include <gnome-software.h>
#include <json-glib/json-glib.h>
#include <string.h>

#include "gs-flatpak.h"

#define EXTERNAL_ASSETS_SPEC_VERSION 1
#define JSON_SPEC_KEY "spec"
#define JSON_RUNTIME_KEY "runtime"
#define JSON_RUNTIME_NAME_KEY "name"
#define JSON_RUNTIME_URL_KEY "url"
#define JSON_RUNTIME_TYPE_KEY "type"

#define METADATA_URL "GnomeSoftware::external-app::url"
#define METADATA_TYPE "GnomeSoftware::external-app::type"
#define METADATA_HEADLESS_APP "GnomeSoftware::external-app::headless-app"
#define METADATA_BUILD_DIR "GnomeSoftware::external-app::build-dir"
#define METADATA_EXTERNAL_ASSETS "flatpak-3rdparty::external-assets"

#define TMP_ASSSETS_PREFIX "gs-external-apps"

#define EXT_APPS_SYSTEM_REPO_NAME "eos-external-apps"

typedef enum {
	GS_PLUGIN_EXTERNAL_TYPE_UNKNOWN = 0,
	GS_PLUGIN_EXTERNAL_TYPE_DEB,
	GS_PLUGIN_EXTERNAL_TYPE_TAR
} GsPluginExternalType;

struct GsPluginData {
	GsFlatpak	*usr_flatpak;
	GsFlatpak	*sys_flatpak;
	char		*runtimes_build_dir;
};

void
gs_plugin_initialize (GsPlugin *plugin)
{
	GsPluginData *priv = gs_plugin_alloc_data (plugin, sizeof(GsPluginData));
	const char *ext_apps_build_dir = g_get_user_cache_dir ();

	priv->usr_flatpak = gs_flatpak_new (plugin, GS_FLATPAK_SCOPE_USER);
	priv->sys_flatpak = gs_flatpak_new (plugin, GS_FLATPAK_SCOPE_SYSTEM);
	priv->runtimes_build_dir = g_build_filename (ext_apps_build_dir,
						     TMP_ASSSETS_PREFIX,
						     NULL);

	/* Run plugin before the flatpak ones because we need them to install
	 * the app's headless part first */
	gs_plugin_add_rule (plugin, GS_PLUGIN_RULE_RUN_BEFORE, "flatpak-system");
	gs_plugin_add_rule (plugin, GS_PLUGIN_RULE_RUN_BEFORE, "flatpak-user");
}

void
gs_plugin_destroy (GsPlugin *plugin)
{
	GsPluginData *priv = gs_plugin_get_data (plugin);

	g_clear_object (&priv->usr_flatpak);
	g_clear_object (&priv->sys_flatpak);
	g_free (priv->runtimes_build_dir);
}

static gboolean
app_is_flatpak (GsApp *app)
{
	const gchar *id = gs_app_get_unique_id (app);
	return id && (g_str_has_prefix (id, GS_FLATPAK_USER_PREFIX) ||
		      g_str_has_prefix (id, GS_FLATPAK_SYSTEM_PREFIX));
}

void
gs_plugin_adopt_app (GsPlugin *plugin,
		     GsApp *app)
{
	if (!app_is_flatpak (app) ||
	    !gs_app_get_metadata_item (app, METADATA_EXTERNAL_ASSETS))
		return;

	g_debug ("Adopt '%s' as an external app", gs_app_get_id (app));

	gs_app_set_management_plugin (app, gs_plugin_get_name (plugin));
}

gboolean
gs_plugin_setup (GsPlugin *plugin,
		 GCancellable *cancellable,
		 GError **error)
{
	GsPluginData *priv = gs_plugin_get_data (plugin);

	if (!gs_flatpak_setup (priv->usr_flatpak, cancellable, error) ||
	    !gs_flatpak_setup (priv->sys_flatpak, cancellable, error))
		return FALSE;

	return TRUE;
}

static void
command_cancelled_cb (GCancellable *cancellable,
		      GSubprocess *subprocess)
{
	g_debug ("Killing process '%s' after a cancellation!",
		 g_subprocess_get_identifier (subprocess));
	g_subprocess_force_exit (subprocess);
}

static gboolean
run_command (const char **argv,
	     GCancellable *cancellable,
	     GError **error)
{
	g_autofree char *cmd = NULL;
	int exit_val = -1;
	g_autoptr(GSubprocess) subprocess = NULL;
	gboolean result = FALSE;

	subprocess = g_subprocess_newv (argv,
					G_SUBPROCESS_FLAGS_STDOUT_PIPE |
					G_SUBPROCESS_FLAGS_STDIN_PIPE,
					error);

	if (!subprocess)
		return FALSE;

	if (cancellable)
		g_cancellable_connect (cancellable,
				       G_CALLBACK (command_cancelled_cb),
				       subprocess, NULL);

	result = g_subprocess_wait_check (subprocess, NULL, error);

	exit_val = g_subprocess_get_exit_status (subprocess);
	cmd = g_strjoinv (" ", (char **) argv);

	g_debug ("Result of running '%s': retcode=%d", cmd, exit_val);

	return result;
}

static gboolean
build_and_install_external_runtime (GsApp *runtime,
				    GCancellable *cancellable,
				    GError **error)
{
	const char *runtime_url = gs_app_get_metadata_item (runtime,
							    METADATA_URL);
	const char *runtime_type = gs_app_get_metadata_item (runtime,
							     METADATA_TYPE);

	/* run the external apps builder script as the configured helper user */
	const char *argv[] = {"pkexec", "--user", EXT_APPS_HELPER_USER,
			      LIBEXECDIR "/eos-external-apps-build-install",
			      EXT_APPS_SYSTEM_REPO_NAME,
			      gs_app_get_id (runtime), runtime_url,
			      runtime_type,
			      NULL};

	g_debug ("Building and installing runtime extension '%s'...",
		 gs_app_get_unique_id (runtime));

	return run_command (argv, cancellable, error);
}

static inline GsPluginExternalType
get_type_from_string (const char *type)
{
	if (g_strcmp0 (type, "deb") == 0)
		return GS_PLUGIN_EXTERNAL_TYPE_DEB;
	else if (g_strcmp0 (type, "tar") == 0)
		return  GS_PLUGIN_EXTERNAL_TYPE_TAR;

	return GS_PLUGIN_EXTERNAL_TYPE_UNKNOWN;
}

static char *
extract_runtime_info_from_json_data (const char *data,
				     char **url,
				     char **type,
				     GError **error)
{
	gboolean ret;
	g_autoptr(JsonParser) parser = NULL;
	JsonObject *root, *runtime;
	JsonNode *node;
	g_autoptr(GList) members = NULL;
	const char *runtime_name = NULL;
	const char *json_url = NULL;
	const char *type_str = NULL;
	guint spec = 0;

	parser = json_parser_new ();

	ret = json_parser_load_from_data (parser, data, -1, error);
	if (!ret)
		return NULL;

	root = json_node_get_object (json_parser_get_root (parser));
	if (!root) {
		g_set_error (error,
		             GS_PLUGIN_ERROR,
		             GS_PLUGIN_ERROR_FAILED,
		             "no root object");
		return NULL;
	}

	node = json_object_get_member (root, JSON_SPEC_KEY);
	if (node)
		spec = json_node_get_int (node);
	if (spec != EXTERNAL_ASSETS_SPEC_VERSION) {
		g_set_error (error,
		             GS_PLUGIN_ERROR,
		             GS_PLUGIN_ERROR_FAILED,
		             "External asset's json spec version '%u' does "
			     "not match the plugin. Expected '%u'", spec,
			     EXTERNAL_ASSETS_SPEC_VERSION);
		return NULL;
	}

	node = json_object_get_member (root, JSON_RUNTIME_KEY);
	if (!node) {
		g_set_error (error,
		             GS_PLUGIN_ERROR,
		             GS_PLUGIN_ERROR_FAILED,
		             "External asset's json has no '%s' member set",
			     JSON_RUNTIME_KEY);
		return NULL;
	}

	runtime = json_node_get_object (node);

	node = json_object_get_member (runtime, JSON_RUNTIME_NAME_KEY);
	if (!node) {
		g_set_error (error,
		             GS_PLUGIN_ERROR,
		             GS_PLUGIN_ERROR_FAILED,
		             "External asset's runtime member has no '%s' key "
			     "set", JSON_RUNTIME_NAME_KEY);
		return NULL;
	}

	runtime_name = json_node_get_string (node);

	node = json_object_get_member (runtime, JSON_RUNTIME_URL_KEY);
	if (!node) {
		g_set_error (error,
		             GS_PLUGIN_ERROR,
		             GS_PLUGIN_ERROR_FAILED,
		             "External asset's runtime member has no '%s' key "
			     "set", JSON_RUNTIME_URL_KEY);
		return NULL;
	}

	json_url = json_node_get_string (node);

	/* optional elements */
	node = json_object_get_member (runtime, JSON_RUNTIME_TYPE_KEY);
	if (node)
		type_str = json_node_get_string (node);

	*url = g_strdup (json_url);
	*type = g_strdup (type_str);

	return g_strdup (runtime_name);
}

static GsApp *
gs_plugin_get_app_external_runtime (GsPlugin *plugin,
				    GsApp *headless_app)
{
	GsApp *runtime = NULL;
	GsPluginData *priv;
	g_autofree char *id = NULL;
	g_autofree char *full_id = NULL;
	g_autofree char *url = NULL;
	g_autofree char *type = NULL;
	g_autofree char *json_data = NULL;
	g_autoptr (GError) error = NULL;
	const char *metadata;

	metadata = gs_app_get_metadata_item (headless_app,
					     METADATA_EXTERNAL_ASSETS);

	if (!metadata)
		return NULL;

	json_data = g_uri_unescape_string (metadata, NULL);
	id = extract_runtime_info_from_json_data (json_data, &url, &type, &error);

	if (!id) {
		g_debug ("Error getting external runtime from "
			 "metadata: %s", error->message);
		return NULL;
	}

	priv = gs_plugin_get_data (plugin);

	full_id = g_strdup_printf ("%s%s",
				   gs_flatpak_get_prefix (priv->sys_flatpak),
				   id);

	runtime = gs_plugin_cache_lookup (plugin, full_id);
	if (runtime) {
		g_debug ("Found cached '%s'", full_id);
		gs_app_set_management_plugin (runtime,
					      gs_plugin_get_name (plugin));

		return runtime;
	}

	runtime = gs_app_new (id);
	gs_app_set_metadata (runtime, METADATA_HEADLESS_APP,
			     gs_app_get_unique_id (headless_app));
	gs_app_set_metadata (runtime, METADATA_URL, url);
	gs_app_set_metadata (runtime, METADATA_TYPE, type);
	gs_app_set_metadata (runtime, "flatpak::kind", "runtime");
	gs_app_set_kind (runtime, AS_APP_KIND_RUNTIME);
	gs_app_set_flatpak_name (runtime, id);
	gs_app_set_flatpak_arch (runtime, flatpak_get_default_arch ());
	gs_app_set_management_plugin (runtime, gs_plugin_get_name (plugin));

	gs_plugin_cache_add (plugin, full_id, runtime);

	if (gs_flatpak_is_installed (priv->sys_flatpak, runtime, NULL, NULL)) {
		gs_app_set_state (runtime, AS_APP_STATE_INSTALLED);
	}

	return runtime;
}

static GsFlatpak *
gs_plugin_get_gs_flatpak_for_app (GsPlugin *plugin,
				  GsApp *app)
{
	GsPluginData *priv = gs_plugin_get_data (plugin);
	const char *app_unique_id = gs_app_get_unique_id (app);

	if (g_str_has_prefix (app_unique_id, GS_FLATPAK_SYSTEM_PREFIX))
		return priv->sys_flatpak;

	return priv->usr_flatpak;
}

gboolean
gs_plugin_refine_app (GsPlugin *plugin,
		      GsApp *app,
		      GsPluginRefineFlags flags,
		      GCancellable *cancellable,
		      GError **error)
{
	GsApp *ext_runtime;
	const char *metadata = NULL;
	GsFlatpak *flatpak = NULL;

	/* We cache all runtimes because an external runtime may have been
	 * adopted by the flatpak plugins */
	if (app_is_flatpak (app) && gs_flatpak_app_is_runtime (app)) {
		gs_plugin_cache_add (plugin, gs_app_get_id (app), app);
		g_debug ("Caching remote '%s'", gs_app_get_id (app));
	}

	/* only process this app if was created by this plugin */
	if (g_strcmp0 (gs_app_get_management_plugin (app),
		       gs_plugin_get_name (plugin)) != 0)
		return TRUE;

	g_debug ("Refining external app %s", gs_app_get_id (app));

	ext_runtime = gs_plugin_get_app_external_runtime (plugin, app);

	if (!ext_runtime) {
		g_debug ("Could not understand the asset from the "
			 "metadata in "
			 "app %s: %s",
			 gs_app_get_id (app), metadata);
		return TRUE;
	}

	flatpak = gs_plugin_get_gs_flatpak_for_app (plugin, app);

	if (!gs_flatpak_refine_app (flatpak, app, flags, cancellable, error)) {
		g_debug ("Refining app %s failed!", gs_app_get_unique_id (app));
		return FALSE;
	}

	/* We set the state to available because we assume that we can build
	 * the runtime */
	if (gs_app_get_state (ext_runtime) != AS_APP_STATE_INSTALLED) {
		gs_app_set_state (app, AS_APP_STATE_UNKNOWN);
		gs_app_set_state (app, AS_APP_STATE_AVAILABLE);
	}

	return TRUE;
}

gboolean
gs_plugin_app_install (GsPlugin *plugin,
		       GsApp *app,
		       GCancellable *cancellable,
		       GError **error)
{
	g_autofree char *runtime = NULL;
	g_autofree char *url = NULL;
	GsApp *ext_runtime;
	GsPluginData *priv;
	GsFlatpak *flatpak = NULL;
	gboolean ret = FALSE;

	/* only process this app if was created by this plugin */
	if (g_strcmp0 (gs_app_get_management_plugin (app),
		       gs_plugin_get_name (plugin)) != 0)
		return TRUE;

	priv = gs_plugin_get_data (plugin);

	ext_runtime = gs_plugin_get_app_external_runtime (plugin, app);

	if (!ext_runtime) {
		g_debug ("External app '%s' didn't have any asset! "
			 "Not installing and marking as state unknown!",
			 gs_app_get_id (app));
		gs_app_set_state (app, AS_APP_STATE_UNKNOWN);

		return TRUE;
	}

	gs_app_set_state (app, AS_APP_STATE_INSTALLING);

	if (gs_app_get_state (ext_runtime) == AS_APP_STATE_UNKNOWN) {
		gs_app_set_progress (app, 50);

		if (!build_and_install_external_runtime (ext_runtime,
							 cancellable, error)) {
			g_debug ("Failed to build and install external "
				 "runtime '%s'", runtime);
			return FALSE;
		}

		gs_app_set_progress (app, 75);

		gs_app_set_origin (ext_runtime, EXT_APPS_SYSTEM_REPO_NAME);

		if (!gs_flatpak_refine_app (priv->sys_flatpak, ext_runtime,
					    GS_PLUGIN_REFINE_FLAGS_DEFAULT,
					    cancellable, error)) {
			g_debug ("Failed to refine '%s'",
				 gs_app_get_id (ext_runtime));
			return FALSE;
		}
	}

	switch (gs_app_get_state (ext_runtime)) {
	case AS_APP_STATE_INSTALLED:
		g_debug ("App asset '%s' is already installed",
			 gs_app_get_id (ext_runtime));
		break;

	case AS_APP_STATE_UPDATABLE:
		g_debug ("Updating '%s'", gs_app_get_id (ext_runtime));

		if (!gs_flatpak_update_app (priv->usr_flatpak, ext_runtime,
					    cancellable, error)) {
			g_debug ("Failed to update '%s'",
				 gs_app_get_id (ext_runtime));
			return FALSE;
		}
		break;

	case AS_APP_STATE_AVAILABLE:
		g_debug ("Installing '%s'", gs_app_get_id (ext_runtime));
		ret = gs_flatpak_app_install (priv->sys_flatpak, ext_runtime,
					      cancellable, error);

		if (!ret) {
			g_debug ("Failed to install '%s'",
				 gs_app_get_id (ext_runtime));
			return FALSE;
		}

		break;

	case AS_APP_STATE_UNKNOWN:
	default:
		/* In case we may end up here somehow, just let the situation
		 * be dealt with in the check for the 'installed' state below */
		break;
	}

	if (g_cancellable_is_cancelled (cancellable)) {
		g_debug ("Installation of '%s' was cancelled",
			 gs_app_get_unique_id (ext_runtime));

		return TRUE;
	}

	if (gs_app_get_state (ext_runtime) != AS_APP_STATE_INSTALLED) {
		gs_app_set_state (app, gs_app_get_state (ext_runtime));
		g_set_error (error, GS_PLUGIN_ERROR,
			     GS_PLUGIN_ERROR_NOT_SUPPORTED,
			     "Could not install external app '%s' because its "
			     "extension runtime '%s' is not installed",
			     gs_app_get_id (app),
			     gs_app_get_id (ext_runtime));
		return FALSE;
	}

	flatpak = gs_plugin_get_gs_flatpak_for_app (plugin, app);
	if (!gs_flatpak_app_install (flatpak, app, cancellable, error)) {
		g_debug ("Failed to install '%s'", gs_app_get_id (app));
		return FALSE;
	}

	return TRUE;
}

gboolean
gs_plugin_launch (GsPlugin *plugin,
		  GsApp *app,
		  GCancellable *cancellable,
		  GError **error)
{
	GsFlatpak *flatpak = gs_plugin_get_gs_flatpak_for_app (plugin, app);
	return gs_flatpak_launch (flatpak, app, cancellable, error);
}

gboolean
gs_plugin_app_remove (GsPlugin *plugin,
		      GsApp *app,
		      GCancellable *cancellable,
		      GError **error)
{
	GsApp *ext_runtime;
	GsPluginData *priv;
	GsFlatpak *flatpak = NULL;

	/* only process this app if was created by this plugin */
	if (g_strcmp0 (gs_app_get_management_plugin (app),
		       gs_plugin_get_name (plugin)) != 0)
		return TRUE;

	gs_app_set_state (app, AS_APP_STATE_REMOVING);

	priv = gs_plugin_get_data (plugin);

	ext_runtime = gs_plugin_get_app_external_runtime (plugin, app);

	if (!ext_runtime) {
		g_debug ("External app '%s' has no external runtime to be"
			 "removed", gs_app_get_id (app));
	} else if (gs_app_get_state (ext_runtime) == AS_APP_STATE_INSTALLED ||
		   gs_app_get_state (ext_runtime) == AS_APP_STATE_UPDATABLE) {
		g_autoptr(GError) local_error = NULL;

		if (!gs_flatpak_app_remove (priv->sys_flatpak, ext_runtime,
					    cancellable, &local_error)) {
			g_debug ("Cannot remove '%s': %s. Will try to "
				 "remove app '%s'.",
				 gs_app_get_id (ext_runtime),
				 local_error->message,
				 gs_app_get_id (app));
		}
	}

	flatpak = gs_plugin_get_gs_flatpak_for_app (plugin, app);
	return gs_flatpak_app_remove (flatpak, app, cancellable, error);
}