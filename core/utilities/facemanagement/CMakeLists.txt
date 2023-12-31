#
# SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

set(libfacemanagement_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/database/faceutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/database/faceitemretriever.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/database/facescansettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/items/faceitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/items/facegroup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/items/facegroup_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/threads/facepreviewloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/threads/facepipeline.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/threads/facepipeline_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/threads/facepipelinepackage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/threads/parallelpipes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/threads/scanstatefilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/facescanwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/assignnamewidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/assignnamewidget_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/assignnamewidgetstates.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/bench/detectionbenchmarker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/bench/recognitionbenchmarker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/workers/detectionworker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/workers/recognitionworker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/workers/databasewriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/workers/trainerworker.cpp
)

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Sql,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
)

if(ENABLE_DBUS)
    include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::DBus,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

# Used by digikamgui
add_library(gui_facemanagement_obj OBJECT ${libfacemanagement_SRCS})

target_compile_definitions(gui_facemanagement_obj
                           PRIVATE
                           digikamgui_EXPORTS
)
