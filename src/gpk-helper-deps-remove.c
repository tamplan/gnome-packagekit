/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2009 Richard Hughes <richard@hughsie.com>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "gpk-helper-deps-remove.h"
#include "gpk-marshal.h"
#include "gpk-gnome.h"
#include "gpk-common.h"
#include "gpk-dialog.h"

#include "egg-debug.h"

static void     gpk_helper_deps_remove_finalize	(GObject	  *object);

#define GPK_HELPER_DEPS_REMOVE_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), GPK_TYPE_HELPER_DEPS_REMOVE, GpkHelperDepsRemovePrivate))

struct GpkHelperDepsRemovePrivate
{
	GtkWindow		*window;
};

enum {
	GPK_HELPER_DEPS_REMOVE_EVENT,
	GPK_HELPER_DEPS_REMOVE_LAST_SIGNAL
};

static guint signals [GPK_HELPER_DEPS_REMOVE_LAST_SIGNAL] = { 0 };
G_DEFINE_TYPE (GpkHelperDepsRemove, gpk_helper_deps_remove, G_TYPE_OBJECT)

/**
 * gpk_helper_deps_remove_show:
 *
 * Return value: if we agreed
 **/
gboolean
gpk_helper_deps_remove_show (GpkHelperDepsRemove *helper, GPtrArray *packages, PkPackageList *deps_list)
{
	gchar *name = NULL;
	gchar *title = NULL;
	gchar *message = NULL;
	gchar **package_ids = NULL;
	guint length;
	GtkWidget *dialog;
	GtkResponseType response;

	/* empty list */
	length = pk_package_list_get_size (deps_list);
	if (length == 0) {
		g_signal_emit (helper, signals [GPK_HELPER_DEPS_REMOVE_EVENT], 0, GTK_RESPONSE_YES);
		goto out;
	}

	/* TRANSLATORS: title: show the number of other packages we depend on */
	title = g_strdup_printf (ngettext ("%i additional package also has to be removed",
					   "%i additional packages also have to be removed",
					   length), length);

	package_ids = pk_ptr_array_to_strv (packages);
	name = gpk_dialog_package_id_name_join_locale (package_ids);

	/* TRANSLATORS: message: describe in detail why it must happen */
	message = g_strdup_printf (ngettext ("To remove %s other packages that depend on it must also be removed.",
					     "To remove %s other packages that depend on them must also be removed.",
					     packages->len), name);

	dialog = gtk_message_dialog_new (helper->priv->window, GTK_DIALOG_DESTROY_WITH_PARENT,
					 GTK_MESSAGE_INFO, GTK_BUTTONS_CANCEL, "%s", title);
	gtk_message_dialog_format_secondary_markup (GTK_MESSAGE_DIALOG (dialog), "%s", message);
	gpk_dialog_embed_package_list_widget (GTK_DIALOG (dialog), deps_list);

//	gtk_dialog_add_button (GTK_DIALOG (dialog), "help", GTK_RESPONSE_HELP);
	/* TRANSLATORS: this is button text */
	gtk_dialog_add_button (GTK_DIALOG (dialog), _("Remove"), GTK_RESPONSE_YES);

	/* set icon name */
	gtk_window_set_icon_name (GTK_WINDOW (dialog), GPK_ICON_SOFTWARE_INSTALLER);

	response = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (GTK_WIDGET (dialog));

	/* yes / no */
	if (response == GTK_RESPONSE_YES) {
		g_signal_emit (helper, signals [GPK_HELPER_DEPS_REMOVE_EVENT], 0, response);
//	} else if (response == GTK_RESPONSE_HELP) {
//		gpk_gnome_help ("dialog-remove-other-packages");
	} else {
		g_signal_emit (helper, signals [GPK_HELPER_DEPS_REMOVE_EVENT], 0, GTK_RESPONSE_NO);
	}
out:
	g_strfreev (package_ids);
	g_free (name);
	g_free (title);
	g_free (message);
	return TRUE;
}

/**
 * gpk_helper_deps_remove_set_parent:
 **/
gboolean
gpk_helper_deps_remove_set_parent (GpkHelperDepsRemove *helper, GtkWindow *window)
{
	g_return_val_if_fail (GPK_IS_HELPER_DEPS_REMOVE (helper), FALSE);
	g_return_val_if_fail (window != NULL, FALSE);

	helper->priv->window = window;
	return TRUE;
}

/**
 * gpk_helper_deps_remove_class_init:
 * @klass: The GpkHelperDepsRemoveClass
 **/
static void
gpk_helper_deps_remove_class_init (GpkHelperDepsRemoveClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = gpk_helper_deps_remove_finalize;
	g_type_class_add_private (klass, sizeof (GpkHelperDepsRemovePrivate));
	signals [GPK_HELPER_DEPS_REMOVE_EVENT] =
		g_signal_new ("event",
			      G_TYPE_FROM_CLASS (object_class), G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GpkHelperDepsRemoveClass, event),
			      NULL, NULL, g_cclosure_marshal_VOID__UINT,
			      G_TYPE_NONE, 1, G_TYPE_UINT);
}

/**
 * gpk_helper_deps_remove_init:
 **/
static void
gpk_helper_deps_remove_init (GpkHelperDepsRemove *helper)
{
	helper->priv = GPK_HELPER_DEPS_REMOVE_GET_PRIVATE (helper);
	helper->priv->window = NULL;
}

/**
 * gpk_helper_deps_remove_finalize:
 **/
static void
gpk_helper_deps_remove_finalize (GObject *object)
{
	GpkHelperDepsRemove *helper;

	g_return_if_fail (GPK_IS_HELPER_DEPS_REMOVE (object));

	helper = GPK_HELPER_DEPS_REMOVE (object);

	G_OBJECT_CLASS (gpk_helper_deps_remove_parent_class)->finalize (object);
}

/**
 * gpk_helper_deps_remove_new:
 **/
GpkHelperDepsRemove *
gpk_helper_deps_remove_new (void)
{
	GpkHelperDepsRemove *helper;
	helper = g_object_new (GPK_TYPE_HELPER_DEPS_REMOVE, NULL);
	return GPK_HELPER_DEPS_REMOVE (helper);
}
