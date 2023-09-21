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

#ifndef CAJA_RENAME_H
#define CAJA_RENAME_H

#include <glib-object.h>

G_BEGIN_DECLS

#define CAJA_TYPE_RENAME (caja_rename_get_type ())
#define CAJA_RENAME(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), CAJA_TYPE_RENAME, CajaRename))
#define CAJA_IS_RENAME(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), CAJA_TYPE_RENAME))
typedef struct _CajaRename CajaRename;
typedef struct _CajaRenameClass CajaRenameClass;

struct _CajaRename
{
    GObject parent;
};

struct _CajaRenameClass
{
    GObjectClass parent;
};

GType caja_rename_get_type ();
void caja_rename_register_type (GTypeModule *pModule);

G_END_DECLS

#endif
