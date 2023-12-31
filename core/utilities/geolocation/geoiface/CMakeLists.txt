#
# SPDX-FileCopyrightText: 2010-2020, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Xml,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Network,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Concurrent,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>

                    ${MARBLE_INCLUDE_DIR}
)

if(ENABLE_QWEBENGINE)
    include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::WebEngineWidgets,INTERFACE_INCLUDE_DIRECTORIES>)
else()
    include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::WebKitWidgets,INTERFACE_INCLUDE_DIRECTORIES>)
endif()


set(libgeoiface_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/backends/backend-rg.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/backends/backend-geonames-rg.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/backends/backend-geonamesUS-rg.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/backends/backend-osm-rg.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/backends/mapbackend.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/backends/backendmarble.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/backends/backendmarblelayer.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/backends/backendgooglemaps.cpp

                     ${CMAKE_CURRENT_SOURCE_DIR}/correlator/track_correlator.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/correlator/track_correlator_thread.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/correlator/track_listmodel.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/correlator/gpscorrelatorwidget.cpp

                     ${CMAKE_CURRENT_SOURCE_DIR}/reversegeocoding/rginfo.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/reversegeocoding/rgtagmodel.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/reversegeocoding/rgwidget.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/reversegeocoding/simpletreemodel.cpp

                     ${CMAKE_CURRENT_SOURCE_DIR}/tracks/trackreader.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/tracks/trackmanager.cpp

                     ${CMAKE_CURRENT_SOURCE_DIR}/lookup/lookupaltitude.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/lookup/lookupaltitudegeonames.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/lookup/lookupfactory.cpp

                     ${CMAKE_CURRENT_SOURCE_DIR}/dragdrop/mapdragdrophandler.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/dragdrop/gpsitemlistdragdrophandler.cpp

                     ${CMAKE_CURRENT_SOURCE_DIR}/items/gpsitemcontainer.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/items/gpsitemdelegate.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/items/gpsitemmodel.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/items/gpsitemsortproxymodel.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/items/gpsitemlist.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/items/gpsitemlistcontextmenu.cpp

                     ${CMAKE_CURRENT_SOURCE_DIR}/core/geodragdrophandler.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/core/geocoordinates.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/core/geoifacecommon.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/core/geomodelhelper.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/core/groupstatecomputer.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/core/gpscommon.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/core/gpsundocommand.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/core/gpsgeoifacemodelhelper.cpp

                     ${CMAKE_CURRENT_SOURCE_DIR}/bookmark/gpsbookmarkowner.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/bookmark/gpsbookmarkmodelhelper.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/bookmark/bookmarknode.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/bookmark/bookmarksmenu.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/bookmark/bookmarksmngr.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/bookmark/bookmarksdlg.cpp

                     ${CMAKE_CURRENT_SOURCE_DIR}/tiles/abstractmarkertiler.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/tiles/itemmarkertiler.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/tiles/tilegrouper.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/tiles/tileindex.cpp

                     ${CMAKE_CURRENT_SOURCE_DIR}/widgets/mapwidget.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/widgets/placeholderwidget.cpp
)

if(ENABLE_QWEBENGINE)
    set(libgeoiface_SRCS
        ${libgeoiface_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/htmlwidget_qwebengine.cpp
       )
else()
    set(libgeoiface_SRCS
        ${libgeoiface_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/htmlwidget_qwebkit.cpp
       )
endif()

# Marble translations need explicit loading
ecm_create_qm_loader(libgeoiface_SRCS marble_qt)

# Used by digikamcore
add_library(core_geoiface_obj OBJECT ${libgeoiface_SRCS})

target_compile_definitions(core_geoiface_obj
                           PRIVATE
                           digikamcore_EXPORTS
)
