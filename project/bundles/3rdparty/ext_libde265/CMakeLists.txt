# Script to build libde265 for digiKam bundle.
#
# SPDX-FileCopyrightText: 2015-2023 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
#

set(PREFIX_libde265 "${EXTPREFIX}")

if(MINGW)

    ExternalProject_Add(ext_libde265

        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        GIT_REPOSITORY https://github.com/strukturag/libde265.git
        GIT_TAG v1.0.11

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/libde265-lib-only.patch

        CONFIGURE_COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/../../../../bootstrap.mxe <SOURCE_DIR>/. &&
                          <SOURCE_DIR>/bootstrap.mxe ${MXE_BUILDROOT} RelWithDebInfo

        BUILD_COMMAND cd <SOURCE_DIR>/build.mxe && $(MAKE)

        BUILD_IN_SOURCE 1

        INSTALL_COMMAND cd <SOURCE_DIR>/build.mxe/libde265 && cp -fv de265-version.h ../../libde265/ &&
                        cd <SOURCE_DIR>/build.mxe && $(MAKE) install/fast

        UPDATE_COMMAND ""
        ALWAYS 0
    )

elseif(APPLE)

    ExternalProject_Add(ext_libde265

        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        GIT_REPOSITORY https://github.com/strukturag/libde265.git
        GIT_TAG v1.0.11

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/libde265-lib-only.patch

        CONFIGURE_COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/../../../../bootstrap.macports <SOURCE_DIR>/. &&
                          <SOURCE_DIR>/bootstrap.macports ${PREFIX_libde265} RelWithDebInfo x86_64

        BUILD_COMMAND cd <SOURCE_DIR>/build && $(MAKE)

        BUILD_IN_SOURCE 1

        INSTALL_COMMAND cd <SOURCE_DIR>/build/libde265 && cp -fv de265-version.h ../../libde265/ &&
                        cd <SOURCE_DIR>/build && $(MAKE) install/fast

        UPDATE_COMMAND ""
        ALWAYS 0
    )

else() # Linux

    ExternalProject_Add(ext_libde265

        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        GIT_REPOSITORY https://github.com/strukturag/libde265.git
        GIT_TAG v1.0.11

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/libde265-lib-only.patch

        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_libde265}
                   -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                   ${GLOBAL_PROFILE}

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
        ALWAYS 0
    )

endif()
