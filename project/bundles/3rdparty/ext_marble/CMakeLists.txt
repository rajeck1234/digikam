# Script to build Marble for digiKam bundle.
#
# SPDX-FileCopyrightText: 2015-2022 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
#

set(EXTPREFIX_marble "${EXTPREFIX}" )

GetKF5ApplicationMd5Sums(marble ${KA_VERSION})

if(MINGW)

    ExternalProject_Add(ext_marble
                        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
                        URL https://download.kde.org/stable/release-service/${KA_VERSION}/src/marble-${KA_VERSION}.tar.xz
                        URL_MD5 ${KF5_APPLICATION_MD5SUM}

                        INSTALL_DIR ${EXTPREFIX_marble}
                        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/marble-libs-install-mxe.patch &&
                                      cp ${CMAKE_CURRENT_SOURCE_DIR}/../../mxe/fixqtwebkitincludes.sh <SOURCE_DIR>/. &&
                                      <SOURCE_DIR>/fixqtwebkitincludes.sh

                        CMAKE_ARGS -DMXE_TOOLCHAIN=${MXE_TOOLCHAIN}
                                   -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                                   -DCMAKE_COLOR_MAKEFILE=ON
                                   -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_marble}
                                   -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON
                                   -DCMAKE_TOOLCHAIN_FILE=${MXE_TOOLCHAIN}
                                   -DCMAKE_FIND_PREFIX_PATH=${CMAKE_PREFIX_PATH}
                                   -DCMAKE_SYSTEM_INCLUDE_PATH=${CMAKE_PREFIX_PATH}/include
                                   -DCMAKE_INCLUDE_PATH=${CMAKE_PREFIX_PATH}/include
                                   -DCMAKE_LIBRARY_PATH=${CMAKE_PREFIX_PATH}/lib
                                   -DZLIB_ROOT=${CMAKE_PREFIX_PATH}
                                   ${GLOBAL_PROFILE}
                                   "${argoptions}"
                                   -DWITH_DESIGNER_PLUGIN=OFF
                                   -DBUILD_MARBLE_TESTS=OFF
                                   -DBUILD_MARBLE_TOOLS=OFF
                                   -DBUILD_MARBLE_EXAMPLES=OFF
                                   -DBUILD_MARBLE_APPS=OFF
                                   -DBUILD_WITH_DBUS=OFF
                                   -DBUILD_TESTING=OFF
                                   -Wno-dev

                        UPDATE_COMMAND ""
                        ALWAYS 0
    )

elseif(APPLE)

    ExternalProject_Add(ext_marble
                        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
                        URL https://download.kde.org/stable/release-service/${KA_VERSION}/src/marble-${KA_VERSION}.tar.xz
                        URL_MD5 ${KF5_APPLICATION_MD5SUM}

                        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/marble-libs-install-macports.patch &&
                                      cp ${CMAKE_CURRENT_SOURCE_DIR}/../../macports/fixbundledatapath.sh <SOURCE_DIR>/. &&
                                      <SOURCE_DIR>/fixbundledatapath.sh

                        CONFIGURE_COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/../../../../bootstrap.macports <SOURCE_DIR>/. &&
                                          <SOURCE_DIR>/bootstrap.macports ${EXTPREFIX_marble} debug x86_64 "-DWITH_DESIGNER_PLUGIN=OFF -DBUILD_MARBLE_TESTS=OFF -DBUILD_MARBLE_TOOLS=OFF -DBUILD_MARBLE_EXAMPLES=OFF -DBUILD_MARBLE_APPS=OFF -DBUILD_WITH_DBUS=OFF -DQTONLY=ON -DBUILD_TESTING=OFF -Wno-dev"

                        BUILD_COMMAND cd <SOURCE_DIR>/build && $(MAKE)

                        INSTALL_COMMAND cd <SOURCE_DIR>/build && make install/fast

                        BUILD_IN_SOURCE 1

                        UPDATE_COMMAND ""
                        ALWAYS 0
        )

else() # Linux

    ExternalProject_Add(ext_marble
                        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
                        URL https://download.kde.org/stable/release-service/${KA_VERSION}/src/marble-${KA_VERSION}.tar.xz
                        URL_MD5 ${KF5_APPLICATION_MD5SUM}

                        INSTALL_DIR ${EXTPREFIX_marble}
                        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/marble-libs-install-appimage.patch

                        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_marble}
                                   -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                                   ${GLOBAL_PROFILE}
                                   -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX_marble}
                                   -DWITH_DESIGNER_PLUGIN=OFF
                                   -DBUILD_MARBLE_TESTS=OFF
                                   -DBUILD_MARBLE_TOOLS=OFF
                                   -DBUILD_MARBLE_EXAMPLES=OFF
                                   -DBUILD_MARBLE_APPS=OFF
                                   -DBUILD_WITH_DBUS=OFF
                                   -DBUILD_TESTING=OFF
                                   -Wno-dev

                        UPDATE_COMMAND ""
                        ALWAYS 0
    )

endif()
