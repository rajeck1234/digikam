#
# SPDX-FileCopyrightText: 2010-2023, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

set(CMAKE_MIN_VERSION   "3.16")
set(ECM_MIN_VERSION     "5.55.0")

cmake_minimum_required(VERSION ${CMAKE_MIN_VERSION})

execute_process(COMMAND ${CMAKE_COMMAND} --version OUTPUT_VARIABLE VERSION_CMAKE_INFO)
message(STATUS "CMake version: ${VERSION_CMAKE_INFO}")

# ==============================================================================
# Information to update before to release this package.

# digiKam version
set(DIGIKAM_MAJOR_VERSION "8")
set(DIGIKAM_MINOR_VERSION "1")
set(DIGIKAM_PATCH_VERSION "0")

# Suffix to add at end of version string. Usual values are:
# "-git"   : alpha code unstable from git. Do not use in production
# "-beta1" : beta1 release.
# "-beta2" : beta2 release.
# "-beta3" : beta3 release.
# "-rc"    : release candidate.
# ""       : final release. Can be used in production.
set(DIGIKAM_SUFFIX_VERSION "")

# ==============================================================================
# Set env. variables accordingly.

# NOTE: This string is used to set libdigikamcore and libdigikamdatabase SO version ID
set(DIGIKAM_VERSION_SHORT
    "${DIGIKAM_MAJOR_VERSION}.${DIGIKAM_MINOR_VERSION}.${DIGIKAM_PATCH_VERSION}"
)

set(DIGIKAM_VERSION_STRING
    "${DIGIKAM_VERSION_SHORT}${DIGIKAM_SUFFIX_VERSION}"
)

# Core Database XML version
# We must set this variable here at top level because it is used in both
# libs/database/core and data/database
# Version history:
# 1 : Original database XML file, published in production.
# 2 : 08-08-2014 : Fix Images.names field size (see bug #327646).
# 3 : 05/11/2015 : Add Face DB schema.
set(DBCORECONFIG_XML_VERSION "3")

# ==============================================================================

project(digikam
        VERSION ${DIGIKAM_VERSION_SHORT}
        LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# See bug #454941 and the OpenCV detection in RelWithDebInfo target

if(MSVC)

    message(STATUS "Target Build Type: ${CMAKE_BUILD_TYPE}")

    if("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")

        message(STATUS "Switch Target Build Type to Release to prevent linking failure with OpenCV...")

        set(CMAKE_BUILD_TYPE "Release"
            CACHE STRING "Build type: Debug, Release, RelWithDebInfo or MinSizeRel" FORCE)

    endif()

endif()

message(STATUS "----------------------------------------------------------------------------------")
message(STATUS "Starting CMake configuration for ${PROJECT_NAME}")
message(STATUS "")

### ECM setup #########################################################################################################

find_package(ECM ${ECM_MIN_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/core/cmake/modules
                      ${CMAKE_SOURCE_DIR}/core/cmake/macros
                      ${CMAKE_SOURCE_DIR}/core/cmake/rules
                      ${ECM_MODULE_PATH}
)

# KF5 Framework macros
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

# ECM macros
include(ECMSetupVersion)
include(ECMOptionalAddSubdirectory)
include(ECMMarkNonGuiExecutable)
include(ECMGenerateHeaders)
include(ECMGeneratePriFile)
include(ECMInstallIcons)
include(ECMAddAppIcon)
include(ECMPoQmTools)
include(ECMAddTests)
include(ECMMarkAsTest)

# Cmake macros
include(CMakePackageConfigHelpers)
include(GenerateExportHeader)
include(CheckFunctionExists)
include(FeatureSummary)

# Local macros
include(MacroUtils)
include(MacroCompiler)
include(MacroOpenCV)
include(MacroJPEG)
include(MacroBoolTo01)

#######################################################################################################################

APPLY_COMMON_POLICIES()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTOUIC ON)
#set(CMAKE_AUTORCC ON)

if(MSVC)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

    # These settings comes with ecm 5.88 , so when we bump KF/ecm to 5.88+ we can remove these
    add_compile_options(/utf-8)
    if (CMAKE_CXX_STANDARD GREATER_EQUAL 11)
        # Ensure __cplusplus is correct, otherwise it defaults to 199711L which isn't true
        # https://docs.microsoft.com/en-us/cpp/build/reference/zc-cplusplus?view=msvc-160
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus")
    endif()
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY AUTOGEN_TARGETS_FOLDER "Generated")

### COMPILATION OPTIONS MANAGEMENT ####################################################################################

option(DIGIKAMSC_COMPILE_PO              "Build translations files (default=ON)"                                                  ON)
option(DIGIKAMSC_COMPILE_DIGIKAM         "Build digiKam core (default=ON)"                                                        ON)

#######################################################################################################################

if(DIGIKAMSC_COMPILE_PO)
    set_property(GLOBAL PROPERTY ALLOW_DUPLICATE_CUSTOM_TARGETS 1)
endif()

if(BUILD_TESTING)
    include(CTest)
endif()

######################################################################################################################

message(STATUS "----------------------------------------------------------------------------------")
message(STATUS "Top level compilation configuration for ${PROJECT_NAME}")
message(STATUS "")

if(DIGIKAMSC_COMPILE_DIGIKAM)
    if(NOT EXISTS ${CMAKE_SOURCE_DIR}/core)
        message(STATUS "${PROJECT_NAME} will be compiled....................... NO (source code not available)")
        set (DIGIKAMSC_COMPILE_DIGIKAM OFF)
    else()
        message(STATUS "${PROJECT_NAME} will be compiled....................... YES")
    endif()
else()
    message(STATUS     "${PROJECT_NAME} will be compiled....................... NO")
endif()

if(DIGIKAMSC_COMPILE_PO)
    message(STATUS "Translations will be compiled.................. YES")
else()
    message(STATUS "Translations will be compiled.................. NO")
endif()

if(BUILD_TESTING)
    message(STATUS "Tests code will be compiled.................... YES")
else()
    message(STATUS "Tests code will be compiled.................... NO")
endif()

######################################################################################################################

if(DIGIKAMSC_COMPILE_DIGIKAM)
    add_subdirectory(core)
endif()

if(DIGIKAMSC_COMPILE_PO)
    find_package(Gettext REQUIRED)
    find_package(KF5I18n NO_MODULE)
    ki18n_install(po)
endif()

if(BUILD_TESTING)

    # Some unit tests require data from the external "Digikam Test Data" git repository.
    # https://invent.kde.org/graphics/digikam-test-data
    #
    # Developers may choose to obtain the data using the "download-repos" script prior to running cmake.
    # For CI runners to run tests, the following custom target serves to perform the download automatically.
    # If the directory "test-data" has already been created, the target becomes a "no-op".
    #
    add_custom_command(OUTPUT ${CMAKE_SOURCE_DIR}/test-data 
                       COMMENT "Checkout unit-test data repository. Please wait..."
                       COMMAND git
                       ARGS clone https://invent.kde.org/graphics/digikam-test-data.git ${CMAKE_SOURCE_DIR}/test-data)
    add_custom_target(test-data ALL DEPENDS ${CMAKE_SOURCE_DIR}/test-data)

endif()
