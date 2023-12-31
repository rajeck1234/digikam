#
# SPDX-FileCopyrightText: 2010-2020, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include(MacroDPlugins)

if(HAVE_MEDIAPLAYER)

    set(QTMEDIA_BACKEND ${QTAV_LIBRARIES})

endif()


include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>

                    ${CMAKE_CURRENT_SOURCE_DIR}/wizard
)

set(videoslideshowplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/vidslidewizard.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/vidslideintropage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/vidslidealbumspage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/vidslideimagespage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/vidslidevideopage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/vidslideoutputpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/vidslidefinalpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/videoslideshowplugin.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN(NAME    VideoSlideShow
                           SOURCES ${videoslideshowplugin_SRCS}
                           DEPENDS ${QTMEDIA_BACKEND}
)
