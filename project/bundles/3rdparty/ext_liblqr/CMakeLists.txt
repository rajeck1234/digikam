# Script to build liblqr for digiKam bundle.
#
# SPDX-FileCopyrightText: 2015-2022 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
#

SET(EXTPREFIX_liblqr "${EXTPREFIX}")

ExternalProject_Add(ext_liblqr
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

#    URL https://github.com/carlobaldassi/liblqr/archive/v0.4.2.tar.gz
#    URL_MD5 f7b4ecddd8a038f057e236347fd7767c

    GIT_REPOSITORY https://github.com/carlobaldassi/liblqr.git
#    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/liblqr-appimage.patch


    INSTALL_DIR ${EXTPREFIX_liblqr}
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix ${EXTPREFIX_liblqr}

    UPDATE_COMMAND ""
    BUILD_IN_SOURCE 1
    ALWAYS 0
)
