#
# SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

file(GLOB libadvancedrename_SRCS
          ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/common/*.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/parser/*.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/parser/modifiers/*.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/parser/options/*.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/parser/options/database/*.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/parser/options/database/keys/*.cpp
)

file(GLOB libadvancedrename_UI
          ${CMAKE_CURRENT_SOURCE_DIR}/parser/modifiers/*.ui
          ${CMAKE_CURRENT_SOURCE_DIR}/parser/options/*.ui
)

ki18n_wrap_ui(libadvancedrename_SRCS ${libadvancedrename_UI})

# TODO for internal use, prefer complete include paths instead of this
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/common
                    ${CMAKE_CURRENT_SOURCE_DIR}/parser/options
                    ${CMAKE_CURRENT_SOURCE_DIR}/parser/options/database
                    ${CMAKE_CURRENT_SOURCE_DIR}/parser/options/database/keys
                    ${CMAKE_CURRENT_SOURCE_DIR}/parser/modifiers
)

include_directories(
    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Sql,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
)

# Used by digikamgui
add_library(gui_advancedrename_obj OBJECT ${libadvancedrename_SRCS})

target_compile_definitions(gui_advancedrename_obj
                           PRIVATE
                           digikamgui_EXPORTS
)
