#
# SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Network,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Xml,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>

                    ${CMAKE_SOURCE_DIR}/core/libs/dplugins/webservices/o2/src
)

if(ENABLE_QWEBENGINE)
    include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::WebEngineWidgets,INTERFACE_INCLUDE_DIRECTORIES>)
else()
    include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::WebKitWidgets,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

if(KF5KIO_FOUND)
    include_directories($<TARGET_PROPERTY:KF5::KIOCore,INTERFACE_INCLUDE_DIRECTORIES>
                        $<TARGET_PROPERTY:KF5::KIOFileWidgets,INTERFACE_INCLUDE_DIRECTORIES>
    )
endif()

set(libdpluginsinterface_SRCS

    ${CMAKE_CURRENT_SOURCE_DIR}/core/dplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/dplugingeneric.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/dplugineditor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/dpluginrawimport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/dplugindimg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/dpluginauthor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/dpluginaction.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/setup/dpluginloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/setup/dpluginloader_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/setup/dpluginconfview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/setup/dpluginconfviewgeneric.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/setup/dpluginconfvieweditor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/setup/dpluginconfviewdimg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/setup/dpluginsetup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/setup/dpluginaboutdlg.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/iface/dinfointerface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/iface/dmetainfoiface.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/dplugindialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/dwizardpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/dwizarddlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/dsavesettingswidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/dwizardpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/dpreviewmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/dpreviewimage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/ditemslist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/ditemslist_item.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/ditemslist_view.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/webservices/wscomboboxdelegate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/webservices/wscomboboxintermediate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/webservices/wstooldialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/webservices/wstoolutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/webservices/wssettingswidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/webservices/wsselectuserdlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/webservices/wslogindialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/webservices/wsnewalbumdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/webservices/wssettings.cpp
)

#if(ENABLE_QWEBENGINE)
#    set(libdpluginsinterface_SRCS
#        ${libdpluginsinterface_SRCS}
#        ${CMAKE_CURRENT_SOURCE_DIR}/webservices/webwidget_qwebengine.cpp
#       )
#else()
#    set(libdpluginsinterface_SRCS
#        ${libdpluginsinterface_SRCS}
#        ${CMAKE_CURRENT_SOURCE_DIR}/webservices/webwidget.cpp
#       )
#endif()

# Used by digikamcore
add_library(core_dpluginsinterface_obj OBJECT ${libdpluginsinterface_SRCS})

target_compile_definitions(core_dpluginsinterface_obj
                           PRIVATE
                           digikamcore_EXPORTS
)

# Parse O2 library code and rules at end, and compilation rules remove important Qt definitions.

if(WIN32)
    # NOTE: this O2 export symbols rule must be on top level.
    add_definitions(-DO2_DLL_EXPORT)
endif()

add_subdirectory(webservices)
