# Script to build QtWebkit for digiKam bundle.
#
# SPDX-FileCopyrightText: 2015-2022 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
#

set(EXTPREFIX_qtwebkit "${EXTPREFIX}")

set(QtWebKit_Conf  -DENABLE_GEOLOCATION=OFF
                   -DENABLE_DEVICE_ORIENTATION=OFF
                   -DENABLE_QT_WEBCHANNEL=OFF
                   -DENABLE_MEDIA_SOURCE=OFF
                   -DENABLE_VIDEO=OFF
                   -DENABLE_WEB_AUDIO=OFF
                   -DENABLE_PRINT_SUPPORT=OFF
                   -DENABLE_QT_GESTURE_EVENTS=OFF
                   -DENABLE_INSPECTOR_UI=ON
                   -DENABLE_API_TESTS=OFF
                   -DENABLE_TOOLS=OFF
                   -DENABLE_JIT=OFF
                   -DUSE_GSTREAMER=OFF
                   -DUSE_MEDIA_FOUNDATION=OFF
                   -DUSE_QT_MULTIMEDIA=OFF
                   -DUSE_LIBHYPHEN=OFF
                   -DUSE_SYSTEM_MALLOC=1
                   -DUSE_WOFF2=ON
                   -DPORT=Qt
)

JoinListAsString("${QtWebKit_Conf}" " " BASH_OPTIONS)

if(APPLE)

    ExternalProject_Add(ext_qtwebkit
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

        URL https://download.qt.io/snapshots/ci/qtwebkit/5.212/latest/src/submodules/qtwebkit-everywhere-src-5.13.1.tar.xz
        # NOTE: this is a CI tarball, it change each month. So no MD5 sum to use here.
        #URL_MD5 8070564af80809459bf6336431a9890b

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/qtwebkit-fixrpath-macports.patch

        CONFIGURE_COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/../../../../bootstrap.macports <SOURCE_DIR>/. &&
                          <SOURCE_DIR>/bootstrap.macports ${EXTPREFIX_qtwebkit} MinSizeRel x86_64 ${BASH_OPTIONS}

        BUILD_COMMAND cd <SOURCE_DIR>/build && $(MAKE)

        INSTALL_COMMAND cd <SOURCE_DIR>/build && make install/fast

        UPDATE_COMMAND ""
        BUILD_IN_SOURCE 1
        ALWAYS 0
    )

else() # Linux

    ExternalProject_Add(ext_qtwebkit
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

        URL https://download.qt.io/snapshots/ci/qtwebkit/5.212/latest/src/submodules/qtwebkit-everywhere-src-5.13.1.tar.xz
        # NOTE: this is a CI tarball, it change each month. So no MD5 sum to use here.
        #URL_MD5 8070564af80809459bf6336431a9890b

        INSTALL_DIR ${EXTPREFIX_qtwebkit}

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/qtwebkit-fixdeps-linux.patch

        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_qtwebkit}
                   -DCMAKE_BUILD_TYPE=MinSizeRel
                   ${GLOBAL_PROFILE}
                   -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                   ${QtWebKit_Conf}
                   -Wno-dev

        UPDATE_COMMAND ""
        BUILD_IN_SOURCE 1
        ALWAYS 0
    )

endif()
