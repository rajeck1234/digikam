# Script to build AVIF library for digiKam bundle.
#
# SPDX-FileCopyrightText: 2015-2022 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
#

set(PREFIX_libavif "${EXTPREFIX}")

set(AVIF_Conf -DBUILD_SHARED_LIBS=ON
              -DAVIF_ENABLE_WERROR=OFF
              -DAVIF_CODEC_AOM=ON
              -DAVIF_CODEC_AOM_DECODE=ON
              -DAVIF_CODEC_AOM_ENCODE=ON
              -DAVIF_LOCAL_AOM=OFF
              -DAVIF_LOCAL_ZLIBPNG=OFF
              -DAVIF_LOCAL_JPEG=OFF
              -DAVIF_LOCAL_LIBYUV=OFF
              -DAVIF_LOCAL_ZLIBPNG=OFF
              -DAVIF_CODEC_DAV1D=OFF
              -DAVIF_LOCAL_DAV1D=OFF
              -DAVIF_CODEC_LIBGAV1=OFF
              -DAVIF_LOCAL_LIBGAV1=OFF
              -DAVIF_CODEC_RAV1E=OFF
              -DAVIF_LOCAL_RAV1E=OFF
              -DAVIF_CODEC_SVT=OFF
              -DAVIF_LOCAL_SVT=OFF
              -DAVIF_ENABLE_COVERAGE=OFF
              -DAVIF_BUILD_APPS=OFF
              -DAVIF_BUILD_EXAMPLES=OFF
              -DAVIF_BUILD_TESTS=OFF
)

JoinListAsString("${AVIF_Conf}" " " BASH_OPTIONS_AVIF)

if(MINGW)

    ExternalProject_Add(ext_libavif

        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

        GIT_REPOSITORY https://github.com/AOMediaCodec/libavif.git
        GIT_TAG v0.9.3

        CONFIGURE_COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/../../../../bootstrap.mxe <SOURCE_DIR>/. &&
                          <SOURCE_DIR>/bootstrap.mxe ${MXE_BUILDROOT} RelWithDebInfo ${BASH_OPTIONS_AVIF}

        BUILD_COMMAND cd <SOURCE_DIR>/build.mxe && $(MAKE) -j

        INSTALL_COMMAND cd <SOURCE_DIR>/build.mxe && $(MAKE) install/fast

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
        ALWAYS 0
    )

elseif(APPLE)

    ExternalProject_Add(ext_libavif

        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

        GIT_REPOSITORY https://github.com/AOMediaCodec/libavif.git
        GIT_TAG v0.9.3

        CONFIGURE_COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/../../../../bootstrap.macports <SOURCE_DIR>/. &&
                          <SOURCE_DIR>/bootstrap.macports ${PREFIX_libavif} RelWithDebInfo x86_64 ${BASH_OPTIONS_AVIF}

        BUILD_COMMAND cd <SOURCE_DIR>/build && $(MAKE)

        INSTALL_COMMAND cd <SOURCE_DIR>/build && $(MAKE) install/fast

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
        ALWAYS 0
    )

else() # Linux

    ExternalProject_Add(ext_libavif

        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

        GIT_REPOSITORY https://github.com/AOMediaCodec/libavif.git
        GIT_TAG v0.9.3

        CONFIGURE_COMMAND $(CMAKE_COMMAND) . -DCMAKE_INSTALL_PREFIX=${PREFIX_libavif}
                                             -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                                             ${AVIF_Conf}
                                             ${GLOBAL_PROFILE}

        BUILD_COMMAND cd <SOURCE_DIR> && $(MAKE)

        INSTALL_COMMAND cd <SOURCE_DIR> && $(MAKE) install/fast

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
        ALWAYS 0
    )

endif()
