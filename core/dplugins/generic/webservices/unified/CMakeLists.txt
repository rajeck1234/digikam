#
# SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include(MacroDPlugins)

include_directories($<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Network,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Xml,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>

                    ${CMAKE_CURRENT_SOURCE_DIR}/manager
                    ${CMAKE_CURRENT_SOURCE_DIR}/wizard
)

set(unifiedplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/wsauthentication.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/wstalker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/wsalbumspage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/wsfinalpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/wsimagespage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/wsintropage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/wssettingspage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/wsauthenticationpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/wswizard.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/unifiedplugin.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN(NAME    Unified
                           SOURCES ${unifiedplugin_SRCS}
                           DEPENDS ${QWEB_BACKEND}
)
