#! /bin/bash

# Script to build extra libraries using MEX.
#
# SPDX-FileCopyrightText: 2015-2022 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Halt and catch errors
set -eE
trap 'PREVIOUS_COMMAND=$THIS_COMMAND; THIS_COMMAND=$BASH_COMMAND' DEBUG
trap 'echo "FAILED COMMAND: $PREVIOUS_COMMAND"' ERR

#################################################################################################
# Manage script traces to log file

mkdir -p ./logs
exec > >(tee ./logs/build-extralibs.full.log) 2>&1

#################################################################################################

echo "02-build-extralibs.sh : build extra libraries using MEX."
echo "--------------------------------------------------------"

#################################################################################################
# Pre-processing checks

. ./config.sh
. ./common.sh
StartScript
ChecksCPUCores
RegisterRemoteServers

#################################################################################################

# Paths rules
ORIG_PATH="$PATH"
ORIG_WD="`pwd`"

export PATH=$MXE_BUILDROOT/usr/bin:$MXE_INSTALL_PREFIX/qt5/bin:$PATH

#################################################################################################

# Create the build dir for the 3rdparty deps
if [ ! -d $BUILDING_DIR ] ; then
    mkdir $BUILDING_DIR
fi

if [ ! -d $DOWNLOAD_DIR ] ; then
    mkdir $DOWNLOAD_DIR
fi

cd $BUILDING_DIR
rm -rf $BUILDING_DIR/* || true

${MXE_BUILD_TARGETS}-cmake $ORIG_WD/../3rdparty \
                           -DMXE_TOOLCHAIN=${MXE_TOOLCHAIN} \
                           -DMXE_BUILDROOT=${MXE_BUILDROOT} \
                           -DMXE_BUILD_TARGETS=${MXE_BUILD_TARGETS} \
                           -DMXE_ARCHBITS=${MXE_ARCHBITS} \
                           -DMXE_INSTALL_PREFIX=${MXE_INSTALL_PREFIX} \
                           -DCMAKE_BUILD_TYPE=RelWithDebInfo \
                           -DCMAKE_COLOR_MAKEFILE=ON \
                           -DCMAKE_INSTALL_PREFIX=${MXE_INSTALL_PREFIX} \
                           -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON \
                           -DCMAKE_TOOLCHAIN_FILE=${MXE_TOOLCHAIN} \
                           -DCMAKE_FIND_PREFIX_PATH=${CMAKE_PREFIX_PATH} \
                           -DCMAKE_SYSTEM_INCLUDE_PATH=${CMAKE_PREFIX_PATH}/include \
                           -DCMAKE_INCLUDE_PATH=${CMAKE_PREFIX_PATH}/include \
                           -DCMAKE_LIBRARY_PATH=${CMAKE_PREFIX_PATH}/lib \
                           -DZLIB_ROOT=${CMAKE_PREFIX_PATH} \
                           -DINSTALL_ROOT=${MXE_INSTALL_PREFIX} \
                           -DEXTERNALS_DOWNLOAD_DIR=$DOWNLOAD_DIR \
                           -DKA_VERSION=$DK_KA_VERSION \
                           -DKF5_VERSION=$DK_KF5_VERSION \
                           -DENABLE_QTVERSION=$DK_QTVERSION \
                           -DENABLE_QTWEBENGINE=$DK_QTWEBENGINE

# NOTE: The order to compile each component here is very important.

# core KF5 frameworks dependencies
${MXE_BUILD_TARGETS}-cmake --build . --config RelWithDebInfo --target ext_extra-cmake-modules -- -j$CPU_CORES
${MXE_BUILD_TARGETS}-cmake --build . --config RelWithDebInfo --target ext_kconfig             -- -j$CPU_CORES
${MXE_BUILD_TARGETS}-cmake --build . --config RelWithDebInfo --target ext_breeze-icons        -- -j$CPU_CORES
${MXE_BUILD_TARGETS}-cmake --build . --config RelWithDebInfo --target ext_kcoreaddons         -- -j$CPU_CORES
${MXE_BUILD_TARGETS}-cmake --build . --config RelWithDebInfo --target ext_kwindowsystem       -- -j$CPU_CORES
${MXE_BUILD_TARGETS}-cmake --build . --config RelWithDebInfo --target ext_solid               -- -j$CPU_CORES
${MXE_BUILD_TARGETS}-cmake --build . --config RelWithDebInfo --target ext_sonnet              -- -j$CPU_CORES
${MXE_BUILD_TARGETS}-cmake --build . --config RelWithDebInfo --target ext_threadweaver        -- -j$CPU_CORES
${MXE_BUILD_TARGETS}-cmake --build . --config RelWithDebInfo --target ext_karchive            -- -j$CPU_CORES
${MXE_BUILD_TARGETS}-cmake --build . --config RelWithDebInfo --target ext_kdbusaddons         -- -j$CPU_CORES
${MXE_BUILD_TARGETS}-cmake --build . --config RelWithDebInfo --target ext_ki18n               -- -j$CPU_CORES
${MXE_BUILD_TARGETS}-cmake --build . --config RelWithDebInfo --target ext_kcrash              -- -j$CPU_CORES
${MXE_BUILD_TARGETS}-cmake --build . --config RelWithDebInfo --target ext_kcodecs             -- -j$CPU_CORES
${MXE_BUILD_TARGETS}-cmake --build . --config RelWithDebInfo --target ext_kguiaddons          -- -j$CPU_CORES
${MXE_BUILD_TARGETS}-cmake --build . --config RelWithDebInfo --target ext_kwidgetsaddons      -- -j$CPU_CORES
${MXE_BUILD_TARGETS}-cmake --build . --config RelWithDebInfo --target ext_kitemviews          -- -j$CPU_CORES
${MXE_BUILD_TARGETS}-cmake --build . --config RelWithDebInfo --target ext_kcompletion         -- -j$CPU_CORES
${MXE_BUILD_TARGETS}-cmake --build . --config RelWithDebInfo --target ext_kconfigwidgets      -- -j$CPU_CORES
${MXE_BUILD_TARGETS}-cmake --build . --config RelWithDebInfo --target ext_kiconthemes         -- -j$CPU_CORES
${MXE_BUILD_TARGETS}-cmake --build . --config RelWithDebInfo --target ext_kservice            -- -j$CPU_CORES
${MXE_BUILD_TARGETS}-cmake --build . --config RelWithDebInfo --target ext_kxmlgui             -- -j$CPU_CORES
${MXE_BUILD_TARGETS}-cmake --build . --config RelWithDebInfo --target ext_kbookmarks          -- -j$CPU_CORES
${MXE_BUILD_TARGETS}-cmake --build . --config RelWithDebInfo --target ext_kimageformats       -- -j$CPU_CORES

# Extra support for digiKam

# Geolocation support
${MXE_BUILD_TARGETS}-cmake --build . --config RelWithDebInfo --target ext_marble              -- -j$CPU_CORES

# Calendar support
#${MXE_BUILD_TARGETS}-cmake --build . --config RelWithDebInfo --target ext_kcalendarcore       -- -j$CPU_CORES

# Marble install shared lib at wrong place.
mv $MXE_INSTALL_PREFIX/libmarble* $MXE_INSTALL_PREFIX/bin || true
mv $MXE_INSTALL_PREFIX/libastro*  $MXE_INSTALL_PREFIX/bin || true

#################################################################################################

cd "$ORIG_WD"

export PATH=$ORIG_PATH

TerminateScript
