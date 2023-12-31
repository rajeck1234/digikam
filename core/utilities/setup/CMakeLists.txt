#
# SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Sql,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Network,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::WidgetsAddons,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
)

set(libsetupshowfoto_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/editor/setupeditoriface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/editor/setupiofiles.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/editor/setupraw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/setupicc.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/downloader/filesdownloader.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/misc/systemsettingswidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/misc/systemsettings.cpp
)

# Used by digikamcore
add_library(core_setupcommon_obj OBJECT  ${libsetupshowfoto_SRCS})

target_compile_definitions(core_setupcommon_obj
                           PRIVATE
                           digikamcore_EXPORTS
)

# ---------------------------------------------------------------------

set(libsetup_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/setup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/setuptooltip.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/setuptemplate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/setupplugins.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/setuplighttable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/setupdatabase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/setupimagequalitysorter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/setuputils.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/misc/setupmisc.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/editor/setupeditor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/editor/setupversioning.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/album/setupalbumview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/album/setupcategory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/album/setupmime.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/setupmetadata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/setupmetadata_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/setupmetadata_behavior.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/setupmetadata_sidecars.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/setupmetadata_display.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/setupmetadata_rotation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/setupmetadata_baloo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/advancedmetadatatab.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/namespacelistview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata/namespaceeditdlg.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/collections/setupcollections.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/collections/setupcollectionview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/collections/dwitemdelegate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/collections/dwitemdelegate_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/collections/dwitemdelegatepool.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/camera/importfilterdlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/camera/setupcamera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/camera/cameraselection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/camera/cameralist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/camera/cameratype.cpp
)

if(ENABLE_MEDIAPLAYER)

    set(libsetup_SRCS
        ${libsetup_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/setupvideo.cpp
    )

endif()

# Used by digikamgui
add_library(gui_setup_obj OBJECT ${libsetup_SRCS})

target_compile_definitions(gui_setup_obj
                           PRIVATE
                           digikamgui_EXPORTS
)
