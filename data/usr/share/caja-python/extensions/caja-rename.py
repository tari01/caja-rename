#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
#  caja-rename.py
#
#  Copyright 2017-2020 Robert Tari <robert.tari@gmail.com>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA 02110-1301, USA.

import gi

gi.require_version('Caja', '2.0')
gi.require_version('Gtk', '3.0')

import urllib.parse as urlparse
import os
import gettext
import locale
from gi.repository import Caja, Gtk, GObject, Gio
from cajarename.titlecase import titlecase

locale.setlocale(locale.LC_ALL, '')
gettext.bindtextdomain('cajarename', '/usr/share/locale')
gettext.textdomain('cajarename')
_ = gettext.gettext

class RenameMenu(GObject.GObject, Caja.MenuProvider):

    oWindow = None
    oPixBufFolder = None
    oPixBufFile = None
    oListStore = None
    oBuilder = None

    def __init__(self):

        self.oPixBufFolder = Gtk.IconTheme().get_default().load_icon('gtk-directory', 22, 0)
        self.oPixBufFile = Gtk.IconTheme().get_default().load_icon('gtk-file', 22, 0)

    def get_file_items(self, oWindow, lstItems):

        if len(lstItems) > 1:

            oMenuItem = Caja.MenuItem(name='cajarename', label=_('Rename...'), icon='font')
            oMenuItem.connect('activate', self.onActivate, lstItems)

            self.oWindow = oWindow

            return [oMenuItem]

    def updateList(self, oWidget, *args):

        for lstRow in self.oListStore:

            strName = lstRow[2]

            # Case
            if self.oBuilder.get_object('radiobuttonCaseUpper').get_active():
                strName = strName.upper()
            elif self.oBuilder.get_object('radiobuttonCaseLower').get_active():
                strName = strName.lower()
            elif self.oBuilder.get_object('radiobuttonCaseTitle').get_active():
                strName = titlecase(strName)

            # Insert
            strInsert = self.oBuilder.get_object('entryInsertText').get_text()

            if strInsert:

                nInsert = self.oBuilder.get_object('spinbuttonInsertPosition').get_value_as_int()
                strName = strName[0:nInsert] + strInsert + strName[nInsert:]

            # Remove
            nRemoveLength = self.oBuilder.get_object('spinbuttonRemoveLength').get_value_as_int()

            if nRemoveLength:

                nRemoveFrom = self.oBuilder.get_object('spinbuttonRemoveFrom').get_value_as_int()
                strName = strName[0:nRemoveFrom] + strName[nRemoveFrom + nRemoveLength:]

            # Replace
            strSearch = self.oBuilder.get_object('entryReplaceTarget').get_text()

            if strSearch:
                strName = strName.replace(strSearch, self.oBuilder.get_object('entryReplaceWith').get_text())

            lstRow[3] = strName

        # Enumerate
        nDigits = self.oBuilder.get_object('spinbuttonEnumerateDigits').get_value_as_int()

        if nDigits:

            nRow = self.oBuilder.get_object('spinbuttonEnumerateStart').get_value_as_int()

            for lstRow in self.oListStore:

                lstRow[3] = format(nRow, '0' + str(nDigits) + 'd') + lstRow[3]
                nRow += 1

    def onInsertText(self, oWidget, strText, nLength, nPosition):

        strText = ''.join([s for s in strText if s not in '/\\'])
        nId, nDetail = GObject.signal_parse_name('insert-text', oWidget, True)

        if strText:

            nHandler = GObject.signal_handler_find(oWidget, GObject.SignalMatchType.ID, nId, nDetail, None, 0, 0)
            nPosition = oWidget.get_position()

            GObject.signal_handler_block(oWidget, nHandler)
            oWidget.insert_text(strText, nPosition);
            GObject.signal_handler_unblock(oWidget, nHandler)
            GObject.idle_add(oWidget.set_position, nPosition + len(strText))

        GObject.signal_stop_emission(oWidget, nId, nDetail)

    def onApply(self, oDialog):

        # Check for invalid names
        strInvalid = ''

        for lstRow in self.oListStore:

            if lstRow[3] in ['', '.', '..']:
                strInvalid += '\n' + lstRow[2] + ' -> ' + lstRow[3]

        if strInvalid:

            oDlg = Gtk.MessageDialog(oDialog, Gtk.DialogFlags.MODAL, Gtk.MessageType.WARNING, Gtk.ButtonsType.CLOSE, _('The following names are not acceptable:') + '\n' + strInvalid)
            oDlg.set_title(_('Invalid names'))
            oDlg.run()
            oDlg.destroy()

        else:

            # Check for overwrite
            strOverwrite = ''
            lstPaths = []

            for lstRow in self.oListStore:

                if lstRow[2] != lstRow[3]:

                    strPath = os.path.join(lstRow[0], lstRow[3])

                    if strPath in lstPaths or os.path.exists(strPath):
                        strOverwrite += '\n' + lstRow[2] + ' -> ' + lstRow[3]

                    lstPaths.append(strPath)

            if strOverwrite:

                oDlg = Gtk.MessageDialog(oDialog, Gtk.DialogFlags.MODAL, Gtk.MessageType.QUESTION, Gtk.ButtonsType.YES_NO, _('The following will be overwritten:') + '\n' + strOverwrite + '\n\n' + _('Do you wish to continue?'))
                oDlg.set_title(_('Confirm overwrite'))
                nResult = oDlg.run()
                oDlg.destroy()

                if nResult == Gtk.ResponseType.NO:
                    return

            # Rename
            strFailed = ''

            for lstRow in self.oListStore:

                if lstRow[2] != lstRow[3]:

                    try:
                        os.rename(os.path.join(lstRow[0], lstRow[2]), os.path.join(lstRow[0], lstRow[3]))
                    except:
                        strFailed += '\n' + lstRow[2] + ' -> ' + lstRow[3]

            if strFailed:

                oDlg = Gtk.MessageDialog(oDialog, Gtk.DialogFlags.MODAL, Gtk.MessageType.ERROR, Gtk.ButtonsType.CLOSE, _('There were errors while renaming the following:') + '\n' + strFailed)
                oDlg.set_title(_('Rename error'))
                oDlg.run()
                oDlg.destroy()

            # Swap file names and reset everything
            for lstRow in self.oListStore:
                lstRow[2] = lstRow[3]

            self.oBuilder.get_object('radiobuttonCaseUnchanged').set_active(True)
            self.oBuilder.get_object('entryInsertText').set_text('')
            self.oBuilder.get_object('spinbuttonInsertPosition').set_value(0)
            self.oBuilder.get_object('spinbuttonRemoveFrom').set_value(0)
            self.oBuilder.get_object('spinbuttonRemoveLength').set_value(0)
            self.oBuilder.get_object('entryReplaceTarget').set_text('')
            self.oBuilder.get_object('entryReplaceWith').set_text('')
            self.oBuilder.get_object('spinbuttonEnumerateDigits').set_value(0)
            self.oBuilder.get_object('spinbuttonEnumerateStart').set_value(0)

            #oDialog.destroy()

    def onActivate(self, oMenuItem, lstItems):

        self.oBuilder = Gtk.Builder()
        self.oBuilder.add_from_file('/usr/share/cajarename/cajarename.glade')
        self.oBuilder.connect_signals(self)
        self.oListStore = self.oBuilder.get_object('liststore')
        oDialog = self.oBuilder.get_object('dialog')
        #oDialog.set_transient_for(self.oWindow) # Bug in Caja - hides dialogue in pager and dock
        #oDialog.set_icon_name('caja') # Bug in Caja - shows Nautilus' icon

        for oItem in lstItems:

            oPixBuf = self.oPixBufFolder if oItem.is_directory() else self.oPixBufFile
            strURI = oItem.get_uri()
            strPath = strURI

            if strURI.lower().startswith('file://'):
                strPath = strURI[7:]
            elif strURI.lower().startswith(('smb://', 'ftp://', 'sftp://')):
                strPath = Gio.Vfs.get_default().get_file_for_uri(strURI).get_path()

            strFolder, strName = os.path.split(urlparse.unquote(strPath))
            self.oListStore.append([strFolder, oPixBuf, strName, strName])

        oDialog.run()
        oDialog.destroy()

