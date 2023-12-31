#
# SPDX-FileCopyrightText: 2010-2023 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include_directories(
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Sql,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Concurrent,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::Solid,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
)

if(ENABLE_DBUS)
    include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::DBus,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

set(libitemproperties_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/itempropertiessidebar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/itempropertiestab.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/itempropertiestab_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/itempropertiestab_helpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/itempropertiestab_setters.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/itemselectionpropertiestab.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/itempropertiesmetadatatab.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/itempropertiescolorstab.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/itempropertiestxtlabel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/captions/captionedit.cpp
)

if(${Marble_FOUND})
    set(libitemproperties_SRCS
        ${libitemproperties_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/geolocation/itempropertiesgpstab.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/geolocation/itemgpsmodelhelper.cpp
   )
endif()

# Used by digikamcore
add_library(core_itemproperties_obj OBJECT ${libitemproperties_SRCS})

target_compile_definitions(core_itemproperties_obj
                           PRIVATE
                           digikamcore_EXPORTS
)

set(libitemversionssidebar_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/history/itempropertiesversionstab.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/history/versionstreeview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/history/versionsoverlays.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/history/versionsdelegate.cpp
)

set(libitempropertiesdigikam_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/itempropertiessidebardb.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/import/importitempropertiessidebar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/import/importitempropertiestab.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/captions/itemdescedittab.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/captions/itemdescedittab_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/captions/itemdescedittab_tagsview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/captions/itemdescedittab_descview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/captions/itemdescedittab_infoview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/captions/disjointmetadata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/captions/disjointmetadata_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/captions/tagsedit.cpp
)

# Used by digikamgui
add_library(gui_itempropertiesdigikam_obj OBJECT ${libitemversionssidebar_SRCS} ${libitempropertiesdigikam_SRCS})

target_compile_definitions(gui_itempropertiesdigikam_obj
                           PRIVATE
                           digikamgui_EXPORTS
)
