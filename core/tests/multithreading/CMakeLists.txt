#
# SPDX-FileCopyrightText: 2010-2021, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

include_directories(
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
)

set(multicorerawtopng_gui_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processordlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawtopngconverterthread.cpp
)

add_executable(multicorerawtopng_gui ${multicorerawtopng_gui_SRCS})

target_link_libraries(multicorerawtopng_gui

                      digikamcore

                      ${COMMON_TEST_LINK}
)
