# Script to build KF6 Frameworks for digiKam bundle.
#
# SPDX-FileCopyrightText: 2015-2022 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
#

set(EXTPREFIX_frameworks "${EXTPREFIX}")

function(InstallKF5Component
         name
         version
         argoptions
         patch)

    if(MINGW)

        # Extract MD5 sums of each KF5 components directly from server.
        execute_process(COMMAND
                            wget -q -O - -o /dev/null https://download.kde.org/stable/frameworks/${version}/${name}-${version}.0.tar.xz.md5
                        COMMAND
                            grep "tar.xz"
                        COMMAND
                            cut -d " " -f1
                        RESULT_VARIABLE
                            wgetresult
                        OUTPUT_VARIABLE
                            md5sm
                        OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        message(STATUS "--------------------------------")
        message(STATUS "KF6 component    : ${name}")
        message(STATUS "Component version: ${version}")
        message(STATUS "Component MD5    : ${md5sm}")
        message(STATUS "Configure options: ${argoptions}")
        message(STATUS "Patch to apply   : ${patch}")

        if(NOT "${patch}" STREQUAL "")
            set(patch "${PATCH_COMMAND};-p1;-i;${CMAKE_CURRENT_SOURCE_DIR}/${patch}")
        endif()

        ExternalProject_Add(ext_${name}
                            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
                            URL https://download.kde.org/stable/frameworks/${version}/${name}-${version}.0.tar.xz
                            URL_MD5 ${md5sm}

                            INSTALL_DIR ${EXTPREFIX_frameworks}

                            CMAKE_ARGS -DMXE_TOOLCHAIN=${MXE_TOOLCHAIN}
                                       -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                                       -DCMAKE_COLOR_MAKEFILE=ON
                                       -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
                                       -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON
                                       -DCMAKE_TOOLCHAIN_FILE=${MXE_TOOLCHAIN}
                                       -DCMAKE_FIND_PREFIX_PATH=${CMAKE_PREFIX_PATH}
                                       -DCMAKE_SYSTEM_INCLUDE_PATH=${CMAKE_PREFIX_PATH}/include
                                       -DCMAKE_INCLUDE_PATH=${CMAKE_PREFIX_PATH}/include
                                       -DCMAKE_LIBRARY_PATH=${CMAKE_PREFIX_PATH}/lib
                                       -DBUILD_TESTING=OFF
                                       -DZLIB_ROOT=${CMAKE_PREFIX_PATH}
                                       -DINSTALL_ROOT=${MXE_INSTALL_PREFIX}
                                       ${GLOBAL_PROFILE}
                                       "${argoptions}"
                                       -Wno-dev

                            PATCH_COMMAND ${patch}
                            UPDATE_COMMAND ""
                            ALWAYS 0
        )

    elseif(APPLE)

        # Extract MD5 sums of each KF5 components directly from server.
        execute_process(COMMAND
                            wget -q -O - -o /dev/null https://download.kde.org/stable/frameworks/${version}/${name}-${version}.0.tar.xz.md5
                        COMMAND
                            grep "tar.xz"
                        COMMAND
                            cut -d " " -f1
                        RESULT_VARIABLE
                            wgetresult
                        OUTPUT_VARIABLE
                            md5sm
                        OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        message(STATUS "--------------------------------")
        message(STATUS "KF6 component    : ${name}")
        message(STATUS "Component version: ${version}")
        message(STATUS "Component MD5    : ${md5sm}")
        message(STATUS "Configure options: ${argoptions}")
        message(STATUS "Patch to apply   : ${patch}")

        if(NOT "${patch}" STREQUAL "")
            set(patch "${PATCH_COMMAND};-p1;-i;${CMAKE_CURRENT_SOURCE_DIR}/${patch};&&")
        endif()

        ExternalProject_Add(ext_${name}
                            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
                            URL https://download.kde.org/stable/frameworks/${version}/${name}-${version}.0.tar.xz
                            URL_MD5 ${md5sm}

                            PATCH_COMMAND ${patch}
                                          cp ${CMAKE_CURRENT_SOURCE_DIR}/../../macports/fixbundledatapath.sh <SOURCE_DIR>/. &&
                                          <SOURCE_DIR>/fixbundledatapath.sh

                            CONFIGURE_COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/../../../../bootstrap.macports <SOURCE_DIR>/. &&
                                              sed -i '' -e "s/DBUILD_TESTING=ON/DBUILD_TESTING=OFF/g" <SOURCE_DIR>/bootstrap.macports &&
                                              <SOURCE_DIR>/bootstrap.macports ${EXTPREFIX_frameworks} debug x86_64 "${argoptions}"

                            BUILD_COMMAND cd <SOURCE_DIR>/build && $(MAKE)

                            INSTALL_COMMAND cd <SOURCE_DIR>/build && make install/fast

                            BUILD_IN_SOURCE 1

                            UPDATE_COMMAND ""
                            ALWAYS 0
        )

    else() # Linux

        # Extract MD5 sums of each KF5 components directly from server.
        execute_process(COMMAND
                            wget -q -O - -o /dev/null https://download.kde.org/stable/frameworks/${version}/${name}-${version}.0.tar.xz.md5
                        COMMAND
                            grep "tar.xz"
                        COMMAND
                            cut -d " " -f1
                        RESULT_VARIABLE
                            wgetresult
                        OUTPUT_VARIABLE
                            md5sm
                        OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        message(STATUS "--------------------------------")
        message(STATUS "KF6 component    : ${name}")
        message(STATUS "Component version: ${version}")
        message(STATUS "Component MD5    : ${md5sm}")
        message(STATUS "Configure options: ${argoptions}")
        message(STATUS "Patch to apply   : ${patch}")

        if(NOT "${patch}" STREQUAL "")
            set(patch "${PATCH_COMMAND};-p1;-i;${CMAKE_CURRENT_SOURCE_DIR}/${patch}")
        endif()

        ExternalProject_Add(ext_${name}
                            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
                            URL https://download.kde.org/stable/frameworks/${version}/${name}-${version}.0.tar.xz
                            URL_MD5 ${md5sm}

                            PATCH_COMMAND ${patch}

                            INSTALL_DIR ${EXTPREFIX_frameworks}
                            CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_frameworks}
                                       -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                                       ${GLOBAL_PROFILE}
                                       -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                                       -DBUILD_TESTING=OFF
                                       "${argoptions}"
                                       -Wno-dev
                            UPDATE_COMMAND ""
                            ALWAYS 0
        )

    endif()

endfunction()

if(APPLE)
    set(MacosThreadWeaverPatch threadweaver-macports.patch)
    set(MacosKauthBackend -DKAUTH_BACKEND_NAME=Apple)
else()
    set(KCalendarCorePatch kcalendarcore-libical.patch)
endif()

if(MINGW)
    set(BreezeIconsOptions -DBINARY_ICONS_RESOURCE=OFF)
else()
    set(BreezeIconsOptions -DBINARY_ICONS_RESOURCE=ON)
endif()

if (${ENABLE_QTVERSION} MATCHES "6.*")
    set(Qt6CommonOptions -DQT_MAJOR_VERSION=6 -DBUILD_WITH_QT6=ON -DEXCLUDE_DEPRECATED_BEFORE_AND_AT=5.94.0)
endif()

InstallKF5Component(extra-cmake-modules ${KF5_VERSION} "${Qt6CommonOptions}"                       "extracmakemodules-compiler-options.patch")
InstallKF5Component(kconfig             ${KF5_VERSION} "${Qt6CommonOptions}"                       "")
InstallKF5Component(breeze-icons        ${KF5_VERSION} "${Qt6CommonOptions};${BreezeIconsOptions}" "breeze-dropsvg-rccprefix.patch")
InstallKF5Component(solid               ${KF5_VERSION} "${Qt6CommonOptions}"                       "")
InstallKF5Component(kcoreaddons         ${KF5_VERSION} "${Qt6CommonOptions}"                       "kcoreaddons-mingw.patch")
InstallKF5Component(threadweaver        ${KF5_VERSION} "${Qt6CommonOptions}"                       "${MacosThreadWeaverPatch}")
InstallKF5Component(kwindowsystem       ${KF5_VERSION} "${Qt6CommonOptions}"                       "")
InstallKF5Component(karchive            ${KF5_VERSION} "${Qt6CommonOptions}"                       "")
InstallKF5Component(kdbusaddons         ${KF5_VERSION} "${Qt6CommonOptions}"                       "")
InstallKF5Component(ki18n               ${KF5_VERSION} "${Qt6CommonOptions}"                       "ki18n-mingw.patch")
InstallKF5Component(kcrash              ${KF5_VERSION} "${Qt6CommonOptions}"                       "")
InstallKF5Component(kcodecs             ${KF5_VERSION} "${Qt6CommonOptions}"                       "")
InstallKF5Component(kauth               ${KF5_VERSION} "${Qt6CommonOptions};${MacosKauthBackend}"  "")
InstallKF5Component(kguiaddons          ${KF5_VERSION} "${Qt6CommonOptions};-DWITH_WAYLAND=OFF"    "kguiaddons-mingw.patch")
InstallKF5Component(kwidgetsaddons      ${KF5_VERSION} "${Qt6CommonOptions}"                       "")
InstallKF5Component(kitemviews          ${KF5_VERSION} "${Qt6CommonOptions}"                       "")
InstallKF5Component(kcompletion         ${KF5_VERSION} "${Qt6CommonOptions}"                       "")
InstallKF5Component(kconfigwidgets      ${KF5_VERSION} "${Qt6CommonOptions}"                       "")
InstallKF5Component(kiconthemes         ${KF5_VERSION} "${Qt6CommonOptions}"                       "")
InstallKF5Component(kservice            ${KF5_VERSION} "${Qt6CommonOptions}"                       "")
InstallKF5Component(kglobalaccel        ${KF5_VERSION} "${Qt6CommonOptions}"                       "")
InstallKF5Component(kcalendarcore       ${KF5_VERSION} "${Qt6CommonOptions}"                       "${KCalendarCorePatch}")
InstallKF5Component(kxmlgui             ${KF5_VERSION} "${Qt6CommonOptions}"                       "")
InstallKF5Component(kbookmarks          ${KF5_VERSION} "${Qt6CommonOptions}"                       "")
InstallKF5Component(kimageformats       ${KF5_VERSION} "${Qt6CommonOptions}"                       "")
InstallKF5Component(knotifyconfig       ${KF5_VERSION} "${Qt6CommonOptions}"                       "knotifyconfig-drop-phonon.patch")
InstallKF5Component(knotifications      ${KF5_VERSION} "${Qt6CommonOptions}"                       "knotifications-drop-phonon.patch")
InstallKF5Component(kjobwidgets         ${KF5_VERSION} "${Qt6CommonOptions}"                       "")
InstallKF5Component(sonnet              ${KF5_VERSION} "${Qt6CommonOptions}"                       "")
InstallKF5Component(ktextwidgets        ${KF5_VERSION} "${Qt6CommonOptions}"                       "")
InstallKF5Component(kio                 ${KF5_VERSION} "${Qt6CommonOptions}"                       "kio-drop-ktextwidgets.patch")
InstallKF5Component(kinit               ${KF5_VERSION} "${Qt6CommonOptions}"                       "")
