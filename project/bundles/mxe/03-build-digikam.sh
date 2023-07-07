#! /bin/bash

# Script to build digiKam using MXE
# This script must be run as sudo
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
exec > >(tee ./logs/build-digikam.full.log) 2>&1

#################################################################################################

echo "03-build-digikam.sh : build digiKam using MEX."
echo "---------------------------------------------------"

#################################################################################################
# Pre-processing checks

. ./config.sh
. ./common.sh
StartScript
ChecksCPUCores
RegisterRemoteServers

#################################################################################################
# Check if IcoTool CLI program is installed

if ! which icotool ; then
    echo "IcoTool is not installed"
    echo "See https://www.nongnu.org/icoutils/ for details."
    exit 1
else
    echo "Check IcoTool CLI passed..."
fi

#################################################################################################

# Paths rules
ORIG_PATH="$PATH"
ORIG_WD="`pwd`"

export PATH=$MXE_BUILDROOT/usr/bin:$MXE_INSTALL_PREFIX/qt5/bin:$PATH
cd $MXE_BUILDROOT

#################################################################################################
# Install out-dated dependencies

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

${MXE_BUILD_TARGETS}-cmake --build . --config RelWithDebInfo --target ext_heif       -- -j$CPU_CORES
cp $DOWNLOAD_DIR/heif_manifest.txt $ORIG_WD/data/
${MXE_BUILD_TARGETS}-cmake --build . --config RelWithDebInfo --target ext_exiv2      -- -j$CPU_CORES
cp $DOWNLOAD_DIR/exiv2_manifest.txt $ORIG_WD/data/
${MXE_BUILD_TARGETS}-cmake --build . --config RelWithDebInfo --target ext_lensfun    -- -j$CPU_CORES
cp $DOWNLOAD_DIR/lensfun_manifest.txt $ORIG_WD/data/

#################################################################################################
# Build digiKam in temporary directory and installation

# Clean up previous install (see bug #459276)

FILES=$(find "$MXE_BUILDROOT" -name \* | grep -E '(digikam|showfoto)') || true

for FILE in $FILES ; do
    if [[ -f $FILE || -d $FILE ]] ; then
        echo -e "   ==> $MXE_BUILDROOT/$FILE will be removed from previous install"
        rm -fr $MXE_BUILDROOT/$FILE
    fi
done

cd $BUILDING_DIR

if [ -d "$DK_BUILDTEMP/digikam-$DK_VERSION" ] ; then

    echo "---------- Updating existing $DK_BUILDTEMP"

    cd "$DK_BUILDTEMP"
    cd digikam-$DK_VERSION

    git reset --hard
    git pull

    rm -fr build.mxe
    mkdir -p build.mxe

else

    echo "---------- Creating $DK_BUILDTEMP"
    mkdir -p "$DK_BUILDTEMP"

    if [ $? -ne 0 ] ; then
        echo "---------- Cannot create $DK_BUILDTEMP directory."
        echo "---------- Aborting..."
        exit;
    fi

    cd "$DK_BUILDTEMP"
    echo -e "\n\n"
    echo "---------- Downloading digiKam $DK_VERSION"

    git clone --progress --verbose --branch $DK_VERSION --single-branch $DK_GITURL digikam-$DK_VERSION
    cd digikam-$DK_VERSION

    if [ $? -ne 0 ] ; then
        echo "---------- Cannot clone repositories."
        echo "---------- Aborting..."
        exit;
    fi

    mkdir build.mxe

fi

echo -e "\n\n"
echo "---------- Configure digiKam $DK_VERSION"

sed -e "s/DIGIKAMSC_COMPILE_PO=OFF/DIGIKAMSC_COMPILE_PO=ON/g"   ./bootstrap.mxe > ./tmp.mxe ; mv -f ./tmp.mxe ./bootstrap.mxe
sed -e "s/DBUILD_TESTING=ON/DBUILD_TESTING=OFF/g"               ./bootstrap.mxe > ./tmp.mxe ; mv -f ./tmp.mxe ./bootstrap.mxe
sed -e "s/DENABLE_DBUS=ON/DENABLE_DBUS=OFF/g"                   ./bootstrap.mxe > ./tmp.mxe ; mv -f ./tmp.mxe ./bootstrap.mxe
sed -e "s/DENABLE_DRMINGW=OFF/DENABLE_DRMINGW=ON/g"             ./bootstrap.mxe > ./tmp.mxe ; mv -f ./tmp.mxe ./bootstrap.mxe

chmod +x ./bootstrap.mxe

./bootstrap.mxe $MXE_BUILDROOT RelWithDebInfo

if [ $? -ne 0 ]; then
    echo "---------- Cannot configure digiKam $DK_VERSION."
    echo "---------- Aborting..."
    exit;
fi

echo -e "\n\n"
echo "---------- Building digiKam $DK_VERSION"

cd build.mxe
make -j$CPU_CORES

if [ $? -ne 0 ]; then
    echo "---------- Cannot compile digiKam $DK_VERSION."
    echo "---------- Aborting..."
    exit;
fi

cat $DK_BUILDTEMP/digikam-$DK_VERSION/build.mxe/core/app/utils/digikam_version.h   | grep "digikam_version\[\]" | awk '{print $6}' | tr -d '";'  > $ORIG_WD/data/RELEASEID.txt
cat $DK_BUILDTEMP/digikam-$DK_VERSION/build.mxe/core/app/utils/digikam_builddate.h | grep "define BUILD_DATE"   | awk '{print $3}' | tr -d '"\n' > $ORIG_WD/data/BUILDDATE.txt

# Copy manifests for rolling release codes included in digiKam core.
cp  $DK_BUILDTEMP/digikam-$DK_VERSION/core/libs/rawengine/libraw/libraw_manifest.txt $ORIG_WD/data
cp  $DK_BUILDTEMP/digikam-$DK_VERSION/core/libs/dplugins/webservices/o2/o2_manifest.txt $ORIG_WD/data

echo -e "\n\n"
echo "---------- Installing digiKam $DK_VERSION"
echo -e "\n\n"

make install/fast && cd "$ORIG_WD"

if [ $? -ne 0 ]; then
    echo "---------- Cannot install digiKam $DK_VERSION."
    echo "---------- Aborting..."
    exit;
fi

#################################################################################################
# Install Extra Plugins

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

${MXE_BUILD_TARGETS}-cmake --build . --config RelWithDebInfo --target ext_mosaicwall -- -j$CPU_CORES
${MXE_BUILD_TARGETS}-cmake --build . --config RelWithDebInfo --target ext_flowview   -- -j$CPU_CORES
${MXE_BUILD_TARGETS}-cmake --build . --config RelWithDebInfo --target ext_gmic_qt    -- -j$CPU_CORES

#################################################################################################

export PATH=$ORIG_PATH

TerminateScript
