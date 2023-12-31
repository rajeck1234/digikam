# Script to build Exiv2 for digiKam bundle.
#
# SPDX-FileCopyrightText: 2015-2023 by Gilles Caulier  <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

set(PREFIX_ext_exiv2 "${EXTPREFIX}")

set(Exiv2_Conf -DBUILD_SHARED_LIBS=ON
               -DEXIV2_ENABLE_XMP=ON
               -DEXIV2_ENABLE_EXTERNAL_XMP=OFF
               -DEXIV2_ENABLE_PNG=ON
               -DEXIV2_ENABLE_NLS=ON
               -DEXIV2_ENABLE_BMFF=ON
               -DEXIV2_ENABLE_BROTLI=OFF
               -DEXIV2_ENABLE_PRINTUCS2=ON
               -DEXIV2_ENABLE_LENSDATA=ON
               -DEXIV2_ENABLE_VIDEO=OFF
               -DEXIV2_ENABLE_WEBREADY=OFF
               -DEXIV2_ENABLE_DYNAMIC_RUNTIME=OFF
               -DEXIV2_ENABLE_CURL=OFF
               -DEXIV2_ENABLE_SSH=OFF
               -DEXIV2_ENABLE_INIH=OFF
               -DEXIV2_BUILD_SAMPLES=OFF
               -DEXIV2_BUILD_PO=OFF
               -DEXIV2_BUILD_EXIV2_COMMAND=OFF
               -DEXIV2_BUILD_UNIT_TESTS=OFF
               -DEXIV2_BUILD_FUZZ_TESTS=OFF
               -DEXIV2_BUILD_DOC=OFF
               -DEXIV2_TEAM_EXTRA_WARNINGS=OFF
               -DEXIV2_TEAM_WARNINGS_AS_ERRORS=OFF
               -DEXIV2_TEAM_USE_SANITIZERS=OFF
               -D EXIV2_TEAM_OSS_FUZZ=OFF
               -DEXIV2_TEAM_PACKAGING=OFF
)

if(MINGW)

    set(Exiv2_Conf ${Exiv2_Conf}
               -DEXIV2_ENABLE_WIN_UNICODE=ON
    )

endif()

JoinListAsString("${Exiv2_Conf}" " " BASH_OPTIONS)

if(MINGW)

    ExternalProject_Add(ext_exiv2
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

        GIT_REPOSITORY https://github.com/Exiv2/exiv2.git

        # To use official release tag
        GIT_TAG v0.28.0
#       GIT_TAG 0.27-maintenance

        PATCH_COMMAND git reset --hard &&
                      ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/exiv2-mxe-mmap.patch

        CONFIGURE_COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/../../../scripts/create_manifest.sh <SOURCE_DIR>/. &&
                          cd <SOURCE_DIR> &&
                          <SOURCE_DIR>/create_manifest.sh ${EXTERNALS_DOWNLOAD_DIR} exiv2 &&
                          cp -f ${CMAKE_CURRENT_SOURCE_DIR}/../../../../bootstrap.mxe <SOURCE_DIR>/. &&
                          <SOURCE_DIR>/bootstrap.mxe ${MXE_BUILDROOT} RelWithDebInfo ${BASH_OPTIONS}

        BUILD_COMMAND cd <SOURCE_DIR>/build.mxe && $(MAKE) -j

        INSTALL_COMMAND cd <SOURCE_DIR>/build.mxe && $(MAKE) install/fast

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
        ALWAYS 0
    )


elseif(APPLE)

    ExternalProject_Add(ext_exiv2
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

        # To use official release tarball

        GIT_REPOSITORY https://github.com/Exiv2/exiv2.git

        # To use official release tag
        GIT_TAG v0.28.0
#       GIT_TAG 0.27-maintenance

        CONFIGURE_COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/../../../scripts/create_manifest.sh <SOURCE_DIR>/. &&
                          cd <SOURCE_DIR> &&
                          <SOURCE_DIR>/create_manifest.sh ${EXTERNALS_DOWNLOAD_DIR} exiv2 &&
                          cp -f ${CMAKE_CURRENT_SOURCE_DIR}/../../../../bootstrap.macports <SOURCE_DIR>/. &&
                          <SOURCE_DIR>/bootstrap.macports ${PREFIX_ext_exiv2} RelWithDebInfo x86_64 ${BASH_OPTIONS}

        BUILD_COMMAND cd <SOURCE_DIR>/build && $(MAKE)

        INSTALL_COMMAND cd <SOURCE_DIR>/build && $(MAKE) install/fast

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
        ALWAYS 0
    )

else() # Linux

    ExternalProject_Add(ext_exiv2
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

        GIT_REPOSITORY https://github.com/Exiv2/exiv2.git

        # To use official release tag
        GIT_TAG v0.28.0
#       GIT_TAG 0.27-maintenance

        CONFIGURE_COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/../../../scripts/create_manifest.sh <SOURCE_DIR>/. &&
                          cd <SOURCE_DIR> &&
                          <SOURCE_DIR>/create_manifest.sh ${EXTERNALS_DOWNLOAD_DIR} exiv2 &&
                          $(CMAKE_COMMAND) . -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_exiv2}
                                             -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                                             -DICONV_INCLUDE_DIR=${PREFIX_ext_exiv2}/include
                                             ${Exiv2_Conf}
                                             ${GLOBAL_PROFILE}

        BUILD_COMMAND cd <SOURCE_DIR> && $(MAKE)

        INSTALL_COMMAND cd <SOURCE_DIR> && $(MAKE) install/fast

        UPDATE_COMMAND ""
        ALWAYS 0
    )

endif()
