#
# SPDX-FileCopyrightText: 2015-2022 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include(MacroDPlugins)

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

set(rawimportnativeplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/rawimportnativeplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawimport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawpreview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawsettingsbox.cpp
)

DIGIKAM_ADD_RAWIMPORT_PLUGIN(NAME    Native
                             SOURCES ${rawimportnativeplugin_SRCS}
)
