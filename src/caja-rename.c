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

#include <glib/gstdio.h>
#include <glib/gi18n-lib.h>
#include <libcaja-extension/caja-menu-provider.h>
#include "caja-rename.h"
#include "titlecase.h"

static GType m_cCajaRenameType = 0;
static GdkPixbuf *m_pPixBufFolder = NULL;
static GdkPixbuf *m_pPixBufFile = NULL;
static GtkListStore *m_pListStore = NULL;
static GtkBuilder *m_pBuilder = NULL;

static void updateList (GtkWidget *pWidget, gpointer pData)
{
    GtkTreeIter cIter;
    gboolean bValid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (m_pListStore), &cIter);

    while (bValid)
    {
        gchar *sName = NULL;
        gtk_tree_model_get (GTK_TREE_MODEL (m_pListStore), &cIter, 2, &sName, -1);

        // Case
        GtkRadioButton *pUpper = GTK_RADIO_BUTTON (gtk_builder_get_object (m_pBuilder, "radiobuttonCaseUpper"));
        gboolean bUpperActive = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pUpper));

        if (bUpperActive)
        {
            gchar *sNameNew = g_utf8_strup (sName, -1);
            g_free (sName);
            sName = g_strdup (sNameNew);
            g_free (sNameNew);
        }

        GtkRadioButton *pLower = GTK_RADIO_BUTTON (gtk_builder_get_object (m_pBuilder, "radiobuttonCaseLower"));
        gboolean bLowerActive = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pLower));

        if (bLowerActive)
        {
            gchar *sNameNew = g_utf8_strdown (sName, -1);
            g_free (sName);
            sName = g_strdup (sNameNew);
            g_free (sNameNew);
        }

        GtkRadioButton *pTitle = GTK_RADIO_BUTTON (gtk_builder_get_object (m_pBuilder, "radiobuttonCaseTitle"));
        gboolean bTitleActive = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pTitle));

        if (bTitleActive)
        {
            gchar *sNameNew = titlecase_do (sName, TRUE);
            g_free (sName);
            sName = g_strdup (sNameNew);
            g_free (sNameNew);
        }

        // Insert
        GtkEntry *pInsertEntry = GTK_ENTRY (gtk_builder_get_object (m_pBuilder, "entryInsertText"));
        const gchar *sInsert = gtk_entry_get_text (pInsertEntry);

        if (sInsert)
        {
            GtkSpinButton *pInsertButton = GTK_SPIN_BUTTON (gtk_builder_get_object (m_pBuilder, "spinbuttonInsertPosition"));
            gint nInsert = gtk_spin_button_get_value_as_int (pInsertButton);
            glong nLength = g_utf8_strlen (sName, -1);
            gchar *sBefore = g_utf8_substring (sName, 0, nInsert);
            gchar *sAfter = g_utf8_substring (sName, nInsert, nLength);
            gchar *sNameNew = g_strconcat (sBefore, sInsert, sAfter, NULL);
            g_free (sName);
            sName = g_strdup (sNameNew);
            g_free (sNameNew);
            g_free (sBefore);
            g_free (sAfter);
        }

        // Remove
        GtkSpinButton *pButtonLength = GTK_SPIN_BUTTON (gtk_builder_get_object (m_pBuilder, "spinbuttonRemoveLength"));
        gint nRemoveLength = gtk_spin_button_get_value_as_int (pButtonLength);

        if (nRemoveLength)
        {
            GtkSpinButton *pButtonFrom = GTK_SPIN_BUTTON (gtk_builder_get_object (m_pBuilder, "spinbuttonRemoveFrom"));
            gint nRemoveFrom = gtk_spin_button_get_value_as_int (pButtonFrom);
            glong nLength = g_utf8_strlen (sName, -1);
            gint nKeepFrom = 0;

            if (nRemoveFrom > -1)
            {
                nKeepFrom = nRemoveFrom + nRemoveLength;
            }
            else
            {
                nKeepFrom = MAX (0, nLength - ABS (nRemoveFrom) + nRemoveLength);
                nRemoveFrom = MAX (0, nLength - ABS (nRemoveFrom));
            }

            gchar *sBefore = g_utf8_substring (sName, 0, nRemoveFrom);
            gchar *sAfter = g_utf8_substring (sName, nKeepFrom, nLength);
            gchar *sNameNew = g_strconcat (sBefore, sAfter, NULL);
            g_free (sName);
            sName = g_strdup (sNameNew);
            g_free (sNameNew);
            g_free (sBefore);
            g_free (sAfter);
        }

        // Replace
        GtkEntry *pTargetEntry = GTK_ENTRY (gtk_builder_get_object (m_pBuilder, "entryReplaceTarget"));
        const gchar *sSearch = gtk_entry_get_text (pTargetEntry);

        if (sSearch)
        {
            GtkEntry *pWithEntry = GTK_ENTRY (gtk_builder_get_object (m_pBuilder, "entryReplaceWith"));
            const gchar *sWith = gtk_entry_get_text (pWithEntry);
            GString *sString = g_string_new (sName);
            g_free (sName);
            g_string_replace (sString, sSearch, sWith, 0);

            #if !GLIB_CHECK_VERSION (2, 76, 0)
                sName = g_string_free (sString, FALSE);
            #else
                sName = g_string_free_and_steal (sString);
            #endif
        }

        if (sName)
        {
            gtk_list_store_set (m_pListStore, &cIter, 3, sName, -1);
            g_free (sName);
        }

        bValid = gtk_tree_model_iter_next (GTK_TREE_MODEL (m_pListStore), &cIter);
    }

    // Enumerate
    GtkSpinButton *pButtonDigits = GTK_SPIN_BUTTON (gtk_builder_get_object (m_pBuilder, "spinbuttonEnumerateDigits"));
    gint nDigits = gtk_spin_button_get_value_as_int (pButtonDigits);

    if (nDigits)
    {
        GtkSpinButton *pButtonStart = GTK_SPIN_BUTTON (gtk_builder_get_object (m_pBuilder, "spinbuttonEnumerateStart"));
        gint nRow = gtk_spin_button_get_value_as_int (pButtonStart);
        bValid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (m_pListStore), &cIter);

        while (bValid)
        {
            gchar *sName = NULL;
            gtk_tree_model_get (GTK_TREE_MODEL (m_pListStore), &cIter, 3, &sName, -1);
            gchar *sNameNew = g_strdup_printf ("%0*d%s", nDigits, nRow, sName);
            g_free (sName);
            gtk_list_store_set (m_pListStore, &cIter, 3, sNameNew, -1);
            g_free (sNameNew);
            nRow += 1;
            bValid = gtk_tree_model_iter_next (GTK_TREE_MODEL (m_pListStore), &cIter);
        }
    }
}

static void onInsertText (GtkEditable *pWidget, gchar *sText, gint nLength, gint *nPosition, gpointer pData)
{
    GString *sString = g_string_new (sText);
    g_string_replace (sString, "/", "", 0);
    g_string_replace (sString, "\\", "", 0);

    #if !GLIB_CHECK_VERSION (2, 76, 0)
        gchar *sNewText = g_string_free (sString, FALSE);
    #else
        gchar *sNewText = g_string_free_and_steal (sString);
    #endif

    if (sNewText)
    {
        glong nNewLength = g_utf8_strlen (sNewText, -1);

        if (nNewLength != nLength)
        {
            g_signal_handlers_block_by_func (pWidget, onInsertText, pData);
            gtk_editable_insert_text (pWidget, sNewText, -1, nPosition);
            g_signal_handlers_unblock_by_func (pWidget, onInsertText, pData);
            g_free (sNewText);
            g_signal_stop_emission_by_name (pWidget, "insert_text");
        }
    }
}

static void onApply (GtkWidget *pWidget, gpointer pData)
{
    // Check for invalid names
    gchar *sInvalid = NULL;
    GtkTreeIter cIter;
    gboolean bValid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (m_pListStore), &cIter);

    while (bValid)
    {
        gchar *sNameNew = NULL;
        gtk_tree_model_get (GTK_TREE_MODEL (m_pListStore), &cIter, 3, &sNameNew, -1);
        gboolean bEmpty = g_str_equal (sNameNew, "");
        gboolean bDot = g_str_equal (sNameNew, ".");
        gboolean bTwoDots = g_str_equal (sNameNew, "..");

        if (bEmpty || bDot || bTwoDots)
        {
            gchar *sNameOld = NULL;
            gtk_tree_model_get (GTK_TREE_MODEL (m_pListStore), &cIter, 2, &sNameOld, -1);
            gchar *sInvalidNew = NULL;

            if (sInvalid)
            {
                sInvalidNew = g_strconcat (sInvalid, "\n", sNameOld, " -> ", sNameNew, NULL);
                g_free (sInvalid);
            }
            else
            {
                sInvalidNew = g_strconcat (sNameOld, " -> ", sNameNew, NULL);
            }

            g_free (sNameOld);
            sInvalid = g_strdup (sInvalidNew);
            g_free (sInvalidNew);
        }

        g_free (sNameNew);
        bValid = gtk_tree_model_iter_next (GTK_TREE_MODEL (m_pListStore), &cIter);
    }

    if (sInvalid)
    {
        GtkWidget *pDlg = gtk_message_dialog_new (GTK_WINDOW (pData), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_CLOSE, "%s\n\n%s", _("The following names are not acceptable:"), sInvalid);
        g_free (sInvalid);
        gtk_window_set_title (GTK_WINDOW (pDlg), _("Invalid names"));
        gtk_dialog_run (GTK_DIALOG (pDlg));
        gtk_widget_destroy (pDlg);
    }
    else
    {
        // Check for overwrite
        gchar *sOverwrite = NULL;
        bValid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (m_pListStore), &cIter);
        GList *lPaths = NULL;

        while (bValid)
        {
            gchar *sFolder = NULL;
            gchar *sNameOld = NULL;
            gchar *sNameNew = NULL;
            gtk_tree_model_get (GTK_TREE_MODEL (m_pListStore), &cIter, 0, &sFolder, 2, &sNameOld, 3, &sNameNew, -1);
            gboolean bEqual = g_str_equal (sNameOld, sNameNew);

            if (!bEqual)
            {
                gchar *sPath = g_build_filename (sFolder, sNameNew, NULL);
                GList* pPath = g_list_find (lPaths, sPath);
                gboolean bExists = g_file_test (sPath, G_FILE_TEST_EXISTS);

                if (pPath || bExists)
                {
                    gchar *sOverwriteNew = NULL;

                    if (sOverwrite)
                    {
                        sOverwriteNew = g_strconcat (sOverwrite, "\n", sNameOld, " -> ", sNameNew, NULL);
                        g_free (sOverwrite);
                    }
                    else
                    {
                        sOverwriteNew = g_strconcat (sNameOld, " -> ", sNameNew, NULL);
                    }

                    sOverwrite = g_strdup (sOverwriteNew);
                    g_free (sOverwriteNew);
                }

                lPaths = g_list_append (lPaths, sPath);
            }

            g_free (sFolder);
            g_free (sNameOld);
            g_free (sNameNew);
            bValid = gtk_tree_model_iter_next (GTK_TREE_MODEL (m_pListStore), &cIter);
        }

        g_list_free_full (lPaths, g_free);

        if (sOverwrite)
        {
            GtkWidget *pDlg = gtk_message_dialog_new (GTK_WINDOW (pData), GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "%s\n\n%s\n\n%s", _("The following will be overwritten:"), sOverwrite, _("Do you wish to continue?"));
            g_free (sOverwrite);
            gtk_window_set_title (GTK_WINDOW (pDlg), _("Confirm overwrite"));
            gint nResponse = gtk_dialog_run (GTK_DIALOG (pDlg));
            gtk_widget_destroy (pDlg);

            if (nResponse == GTK_RESPONSE_NO)
            {
                return;
            }
        }

        // Rename
        gchar *sFailed = NULL;
        bValid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (m_pListStore), &cIter);

        while (bValid)
        {
            gchar *sFolder = NULL;
            gchar *sNameOld = NULL;
            gchar *sNameNew = NULL;
            gtk_tree_model_get (GTK_TREE_MODEL (m_pListStore), &cIter, 0, &sFolder, 2, &sNameOld, 3, &sNameNew, -1);
            gboolean bEqual = g_str_equal (sNameOld, sNameNew);

            if (!bEqual)
            {
                gchar *sPathOld = g_build_filename (sFolder, sNameOld, NULL);
                gchar *sPathNew = g_build_filename (sFolder, sNameNew, NULL);
                int nResult = g_rename (sPathOld, sPathNew);
                g_free (sPathOld);
                g_free (sPathNew);

                if (nResult == -1)
                {
                    gchar *sFailedNew = NULL;

                    if (sFailed)
                    {
                        sFailedNew = g_strconcat (sFailed, "\n", sNameOld, " -> ", sNameNew, NULL);
                        g_free (sFailed);
                    }
                    else
                    {
                        sFailedNew = g_strconcat (sNameOld, " -> ", sNameNew, NULL);
                    }

                    sFailed = g_strdup (sFailedNew);
                    g_free (sFailedNew);
                }
                else
                {
                    gtk_list_store_set (m_pListStore, &cIter, 2, sNameNew, -1);
                }
            }

            g_free (sFolder);
            g_free (sNameOld);
            g_free (sNameNew);
            bValid = gtk_tree_model_iter_next (GTK_TREE_MODEL (m_pListStore), &cIter);
        }

        if (sFailed)
        {
            GtkWidget *pDlg = gtk_message_dialog_new (GTK_WINDOW (pData), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s\n\n%s", _("There were errors while renaming the following:"), sFailed);
            g_free (sFailed);
            gtk_window_set_title (GTK_WINDOW (pDlg), _("Rename error"));
            gtk_dialog_run (GTK_DIALOG (pDlg));
            gtk_widget_destroy (pDlg);
        }

        // Reset everything
        GtkRadioButton *pUnchanged = GTK_RADIO_BUTTON (gtk_builder_get_object (m_pBuilder, "radiobuttonCaseUnchanged"));
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pUnchanged), TRUE);
        GtkEntry *pInsert = GTK_ENTRY (gtk_builder_get_object (m_pBuilder, "entryInsertText"));
        gtk_entry_set_text (pInsert, "");
        GtkSpinButton *pPosition = GTK_SPIN_BUTTON (gtk_builder_get_object (m_pBuilder, "spinbuttonInsertPosition"));
        gtk_spin_button_set_value (pPosition, 0);
        GtkSpinButton *pFrom = GTK_SPIN_BUTTON (gtk_builder_get_object (m_pBuilder, "spinbuttonRemoveFrom"));
        gtk_spin_button_set_value (pFrom, 0);
        GtkSpinButton *pLength = GTK_SPIN_BUTTON (gtk_builder_get_object (m_pBuilder, "spinbuttonRemoveLength"));
        gtk_spin_button_set_value (pLength, 0);
        GtkEntry *pTarget = GTK_ENTRY (gtk_builder_get_object (m_pBuilder, "entryReplaceTarget"));
        gtk_entry_set_text (pTarget, "");
        GtkEntry *pWidth = GTK_ENTRY (gtk_builder_get_object (m_pBuilder, "entryReplaceWith"));
        gtk_entry_set_text (pWidth, "");
        GtkSpinButton *pDigits = GTK_SPIN_BUTTON (gtk_builder_get_object (m_pBuilder, "spinbuttonEnumerateDigits"));
        gtk_spin_button_set_value (pDigits, 0);
        GtkSpinButton *pStart = GTK_SPIN_BUTTON (gtk_builder_get_object (m_pBuilder, "spinbuttonEnumerateStart"));
        gtk_spin_button_set_value (pStart, 0);
    }
}

static void onActivate (CajaMenuItem *pItem, GList *lFiles)
{
    titlecase_init ();
    m_pBuilder = gtk_builder_new_from_resource ("/in/tari/caja-rename/caja-rename.ui");
    gtk_builder_add_callback_symbol (m_pBuilder, "updateList", G_CALLBACK (updateList));
    gtk_builder_add_callback_symbol (m_pBuilder, "onInsertText", G_CALLBACK (onInsertText));
    gtk_builder_add_callback_symbol (m_pBuilder, "onApply", G_CALLBACK (onApply));
    gtk_builder_connect_signals (m_pBuilder, NULL);
    m_pListStore = GTK_LIST_STORE (gtk_builder_get_object (m_pBuilder, "liststore"));
    GtkDialog *pDialog = GTK_DIALOG (gtk_builder_get_object (m_pBuilder, "dialog"));
    GList *pPath = NULL;

    for (pPath = lFiles; pPath != NULL; pPath = pPath->next)
    {
        GdkPixbuf *pPixBuf = NULL;
        gboolean bDirectory = caja_file_info_is_directory (pPath->data);

        if (bDirectory)
        {
            pPixBuf = m_pPixBufFolder;
        }
        else
        {
            pPixBuf = m_pPixBufFile;
        }

        gchar *sUri = caja_file_info_get_uri (pPath->data);
        gchar *sScheme = g_uri_parse_scheme (sUri);
        gint nFileComp = strcmp (sScheme, "file");
        gchar *sPath = NULL;

        if (nFileComp == 0)
        {
            sPath = g_filename_from_uri (sUri, NULL, NULL);
        }
        else
        {
            GVfs *pVfs = g_vfs_get_default ();
            GFile *pFile = g_vfs_get_file_for_uri (pVfs, sUri);
            sPath = g_file_get_path (pFile);
            g_object_unref (pFile);
        }

        gchar *sDirName = g_path_get_dirname (sPath);
        gchar *sBaseName = g_path_get_basename (sPath);
        g_free (sPath);
        g_free (sScheme);
        g_free (sUri);
        GtkTreeIter cIter;
        gtk_list_store_append (m_pListStore, &cIter);
        gtk_list_store_set (m_pListStore, &cIter, 0, sDirName, 1, pPixBuf, 2, sBaseName, 3, sBaseName, -1);
        g_free (sDirName);
        g_free (sBaseName);
    }

    gtk_dialog_run (pDialog);
    gtk_widget_destroy (GTK_WIDGET (pDialog));
    titlecase_finish ();
}

static GList *getFileItems (CajaMenuProvider *pProvider, GtkWidget *pWindow, GList *lFiles)
{
    guint nFiles = g_list_length (lFiles);

    if (nFiles > 1)
    {
        CajaMenuItem *pItem = caja_menu_item_new ("CajaRename::rename", _("Rename All…"), _("Rename selected items"), "caja-rename");
        GList *lFilesCopy = caja_file_info_list_copy (lFiles);
        g_signal_connect (pItem, "activate", G_CALLBACK (onActivate), lFilesCopy);

        return g_list_append (NULL, pItem);
    }
    else
    {
        return NULL;
    }
}

static void onInterfaceInit (CajaMenuProviderIface *pInterface)
{
    pInterface->get_file_items = getFileItems;
}

static void onInstanceInit (CajaRename *self)
{
    GtkIconTheme *pTheme = gtk_icon_theme_get_default ();
    gboolean bFolder = gtk_icon_theme_has_icon (pTheme, "folder");

    if (bFolder)
    {
        m_pPixBufFolder = gtk_icon_theme_load_icon (pTheme, "folder", 22, 0, NULL);
    }
    else
    {
        m_pPixBufFolder = gtk_icon_theme_load_icon (pTheme, "image-missing", 22, 0, NULL);
    }

    gboolean bFile = gtk_icon_theme_has_icon (pTheme, "text-x-generic");

    if (bFile)
    {
        m_pPixBufFile = gtk_icon_theme_load_icon (pTheme, "text-x-generic", 22, 0, NULL);
    }
    else
    {
        m_pPixBufFile = gtk_icon_theme_load_icon (pTheme, "image-missing", 22, 0, NULL);
    }
}

static void onClassInit (CajaRenameClass *klass)
{
}

GType caja_rename_get_type ()
{
    return m_cCajaRenameType;
}

void caja_rename_register_type (GTypeModule *pModule)
{
    static const GTypeInfo pInfo = {sizeof (CajaRenameClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) onClassInit, NULL, NULL, sizeof (CajaRename), 0, (GInstanceInitFunc) onInstanceInit, NULL};
    static const GInterfaceInfo cMenuProviderInfo = {(GInterfaceInitFunc) onInterfaceInit, NULL, NULL};
    m_cCajaRenameType = g_type_module_register_type (pModule, G_TYPE_OBJECT, "CajaRename", &pInfo, 0);
    g_type_module_add_interface (pModule, m_cCajaRenameType, CAJA_TYPE_MENU_PROVIDER, &cMenuProviderInfo);
}
