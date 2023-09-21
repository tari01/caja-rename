/*
 * Copyright 2017-2023 Robert Tari <robert@tari.in>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib/gi18n-lib.h>
#include <libcaja-extension/caja-extension-types.h>
#include "caja-rename.h"

static GType m_lTypes[1];

void caja_module_initialize (GTypeModule *pModule)
{
    g_print ("Initializing caja-rename extension\n");

    caja_rename_register_type (pModule);
    m_lTypes[0] = CAJA_TYPE_RENAME;
    bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
}

void caja_module_shutdown ()
{
    g_print ("Shutting down caja-rename extension\n");
}

void caja_module_list_types (const GType **lTypes, int *nTypes)
{
    *lTypes = m_lTypes;
    *nTypes = 1;
}
