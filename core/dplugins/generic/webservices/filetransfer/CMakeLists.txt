#
# SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2018      by Maik Qualmann, <metzpinguin at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include(MacroDPlugins)

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::KIOCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::KIOFileWidgets,INTERFACE_INCLUDE_DIRECTORIES>
)

set(filetransferplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/ftexportwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ftexportwindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ftimportwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ftimportwindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ftplugin.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN(NAME    FileTransfer
                           SOURCES ${filetransferplugin_SRCS}
                           DEPENDS KF5::KIOCore KF5::KIOWidgets
)
