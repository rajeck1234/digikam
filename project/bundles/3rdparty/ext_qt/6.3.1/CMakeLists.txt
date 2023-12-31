# Script to build Qt for digiKam bundle.
#
# SPDX-FileCopyrightText: 2015-2022 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
#

SET(EXTPREFIX_qt "${EXTPREFIX}")

IF(NOT ENABLE_QTWEBENGINE)
    SET(DROP_QTWEBENGINE_DEPS
        -skip qtwebengine                 # No need Chromium browser support (QtWebkit instead)
        -skip qtwebchannel                # QtWebChannel support ==> QWebEngine dependency
        -skip qtquickcontrols             # QtQuick support      ==> QWebEngine dependency
        -skip qtshadertools               # QWebEngine dependency
        )
ENDIF()

# NOTES: even if OpenSSL libs paths are passed in bootstrap-qt6.sh, patch qt-appimage-openssl-linked.patch is necessary else linking fail in openssl plugin.
#        qt-appimage-support.patch allow to call QProcess from AppImage with restore native LDD envirronement.
#        qt-base-cmake.patch call up-to-date cmake binary compiled for AppImage instead of native one.

ExternalProject_Add(ext_qt

    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

    # Source code tarball generated by kde-qt6.sh script using KDE git repository.
    #URL file://${EXTERNALS_DOWNLOAD_DIR}/kde-qt6.tar

    URL https://download.qt.io/official_releases/qt/6.3/6.3.1/single/qt-everywhere-src-6.3.1.tar.xz
    URL_MD5 957a304773b281a4584f4c0254773456

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/qt-appimage-support.patch        &&
                  ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/qt-appimage-openssl-linked.patch &&
                  ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/qt-base-cmake.patch

#                  ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/qt-appimage-icu-linked.patch     &&

    # Script to configure Qt6 with all components required by digiKAm AppImage.
    CONFIGURE_COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/bootstrap-qt6.sh <SOURCE_DIR>/. &&
                      <SOURCE_DIR>/bootstrap-qt6.sh ${EXTPREFIX} ${DROP_QTWEBENGINE_DEPS}

    # Compile with Ninja build framework.
    BUILD_COMMAND ninja

    INSTALL_COMMAND ninja install

    UPDATE_COMMAND ""

    # We need to compile in source code else configuration fail with QtQuick.
    BUILD_IN_SOURCE 1
    ALWAYS 0
)

#ExternalProject_Add_step(ext_qt before_download
#
#   # Script to checkout Qt6 code from KDE repository https://invent.kde.org/qt/qt/qt5 using qt6 tag.
#    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/kde-qt6.sh ${EXTERNALS_DOWNLOAD_DIR} ${ENABLE_QTVERSION}
#    DEPENDERS download
#)
