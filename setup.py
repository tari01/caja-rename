#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from setuptools import setup
from cajarename.appdata import *
import os, polib

m_lstDataFiles = []

for strRoot, lstDirnames, lstFilenames in os.walk('po'):

    for strFilename in lstFilenames:

        strLocale = os.path.splitext(strFilename)[0]

        if strLocale != APPNAME:

            strLocaleDir = 'data/usr/share/locale/' + strLocale + '/LC_MESSAGES/'

            if not os.path.isdir(strLocaleDir):
                os.makedirs(strLocaleDir)

            polib.pofile('po/' + strFilename).save_as_mofile(strLocaleDir + APPNAME + '.mo')

for strRoot, lstDirnames, lstFilenames in os.walk('data'):

    for strFilename in lstFilenames:

        if strFilename == '.gitkeep':

            continue

        strPath = os.path.join(strRoot, strFilename)
        m_lstDataFiles.append((os.path.dirname(strPath).lstrip('data'), [strPath]))

setup(
    name = APPNAME,
    version = APPVERSION,
    description = APPDESCRIPTION,
    long_description = APPLONGDESCRIPTION,
    url = APPURL,
    author = APPAUTHOR,
    author_email = APPMAIL,
    maintainer = APPAUTHOR,
    maintainer_email = APPMAIL,
    license = 'GPL-3',
    classifiers = [
        'Development Status :: 5 - Production/Stable',
        'Environment :: X11 Applications :: GTK',
        'Intended Audience :: End Users/Desktop',
        'License :: OSI Approved :: GNU General Public License v3 or later (GPLv3+)',
        'Natural Language :: English',
        'Operating System :: POSIX :: Linux',
        'Programming Language :: Python :: 3 :: Only',
        'Topic :: Utilities'
        ],
    keywords = APPKEYWORDS,
    packages = [APPNAME],
    data_files = m_lstDataFiles,
    platforms = 'UNIX'
    )
