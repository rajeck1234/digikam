#
# SPDX-FileCopyrightText: 2010-2020, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include(MacroDPlugins)

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Concurrent,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::ConfigWidgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::Service,INTERFACE_INCLUDE_DIRECTORIES>

                    ${LIBXSLT_INCLUDE_DIR}
                    ${LIBXML2_INCLUDE_DIR}

                    ${CMAKE_CURRENT_SOURCE_DIR}/generator
                    ${CMAKE_CURRENT_SOURCE_DIR}/parameters
                    ${CMAKE_CURRENT_SOURCE_DIR}/wizard
)

add_definitions(${LIBXSLT_DEFINITIONS}
                ${LIBXML2_DEFINITIONS}
)

set(htmlgalleryplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/parameters/abstractthemeparameter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parameters/intthemeparameter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parameters/listthemeparameter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parameters/captionthemeparameter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parameters/stringthemeparameter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parameters/colorthemeparameter.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/invisiblebuttongroup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/htmlintropage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/htmlselectionpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/htmlthemepage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/htmloutputpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/htmlfinalpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/htmlimagesettingspage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/htmlparameterspage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/htmlwizard.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/generator/galleryxmlutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generator/gallerynamehelper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generator/galleryelementfunctor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generator/galleryconfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generator/galleryelement.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generator/gallerytheme.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generator/galleryinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generator/gallerygenerator.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/htmlgalleryplugin.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN(NAME    HtmlGallery
                           SOURCES ${htmlgalleryplugin_SRCS}
                           DEPENDS ${LIBXSLT_EXSLT_LIBRARIES}
                                   ${LIBXSLT_LIBRARIES}
                                   ${LIBXML2_LIBRARIES}
                                   Qt${QT_VERSION_MAJOR}::Concurrent
)

install(DIRECTORY themes DESTINATION ${KDE_INSTALL_FULL_DATADIR}/digikam/)
