# Script to build Qt for digiKam bundle.
#
# Copyright (c) 2015-2022 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

SET(EXTPREFIX_qt "${EXTPREFIX}")

IF(NOT ENABLE_QTWEBENGINE)
    SET(DROP_QTWEBENGINE_DEPS
        -skip qtwebengine                 # No need Chromium browser support (QtWebkit instead)
        -skip qtwebchannel                # QtWebChannel support ==> QWebEngine dependency
        -skip qtquickcontrols             # QtQuick support      ==> QWebEngine dependency
        )
ENDIF()

# NOTE: even if OpenSSL libs paths are passed as arguments, an hardcoded patch is necessary else linking fail.

ExternalProject_Add(ext_qt
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

    URL https://download.qt.io/official_releases/qt/5.15/5.15.7/single/qt-everywhere-opensource-src-5.15.7.tar.xz
    URL_MD5 b30147f4fd91c8dc1b7e61cbc7e6b8a6

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/qt-appimage-support.patch
               && ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/qt-appimage-openssl-linked.patch
               && ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/qt-icu-hack.patch

    CMAKE_ARGS -DOPENSSL_LIBS='-l${EXTPREFIX}/local/lib/libssl.a -l${EXTPREFIX}/local/lib/libcrypto.a -lpthread -ldl'
               -DOPENSSL_INCDIR='${EXTPREFIX}/local/include/openssl/'
               -DOPENSSL_LIBDIR='${EXTPREFIX}/local/lib'

    CONFIGURE_COMMAND <SOURCE_DIR>/configure
                  -prefix ${EXTPREFIX_qt}
                  -verbose
                  -release
                  -opensource
                  -confirm-license
                  -sql-sqlite                       # Compile Sqlite SQL plugin
                  -sql-mysql                        # Compile Mysql SQL plugin
#                  -iconv                            # International string conversion
                  -fontconfig
                  -system-freetype                  # Use system font rendering lib https://doc.qt.io/qt-5/qtgui-attribution-freetype.html
                  -openssl-linked                   # hard link to static version of ssl libraries
                  -nomake tests                     # Do not build test codes
                  -nomake examples                  # Do not build basis example codes
                  -no-compile-examples              # Do not build extra example codes
                  -no-qml-debug
                  -no-mtdev
                  -no-journald
                  -no-syslog
                  -no-tslib
                  -no-directfb
                  -no-linuxfb
                  -no-libproxy
                  -no-pch
                  -qt-zlib
                  -qt-pcre
                  -qt-harfbuzz
                  -xcb
                  -skip qt3d                        # 3D core
                  -skip qtactiveqt                  # No need ActiveX support
                  -skip qtandroidextras             # For embedded devices only
                  -skip qtcharts                    # No need data models charts support
                  -skip qtconnectivity              # For embedded devices only
                  -skip qtdatavis3d                 # no need 3D data visualizations support
                  -skip qtdoc                       # No need documentation
                  -skip qtgamepad                   # No need gamepad hardware support.
                  -skip qtgraphicaleffects          # No need Advanced graphical effects in GUI
                  -skip qtlocation                  # No need geolocation
                  -skip qtlottie                    # No need Adobe QtQuick After Effect animations integration
                  -skip qtmacextras                 # For MacOS devices only
                  -skip qtmultimedia                # No need multimedia support (replaced by QtAV+ffmpeg)
                  -skip qtpurchasing                # No need in-app purchase of products support
                  -skip qtquickcontrols2            # QtQuick support for QML
                  -skip qtremoteobjects             # No need sharing QObject properties between processes support
                  -skip qtscript                    # No need scripting (deprecated)
                  -skip qtscxml                     # No need SCXML state machines support
                  -skip qtsensors                   # For embedded devices only
                  -skip qtserialbus                 # No need serial bus support
                  -skip qtserialport                # No need serial port support
                  -skip qtspeech                    # No need speech synthesis support
                  -skip qtvirtualkeyboard           # No need virtual keyboard support
                  -skip qtwebglplugin               # No need browser OpenGL extension support
                  -skip qtwebsockets                # No need websocket support
                  -skip qtwebview                   # QML extension for QWebEngine
                  -skip qtwinextras                 # For Windows devices only
                  ${DROP_QTWEBENGINE_DEPS}

    UPDATE_COMMAND ""
    BUILD_IN_SOURCE 1
    ALWAYS 0
)
