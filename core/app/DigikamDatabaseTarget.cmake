#
# SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# SPDX-FileCopyrightText: 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
# SPDX-FileCopyrightText: 2015      by Mohamed_Anwer, <m_dot_anwer at gmx dot com>
# SPDX-FileCopyrightText: 2018      by Mario Frank,   <mario dot frank at uni minus potsdam dot de>
#
# SPDX-License-Identifier: BSD-3-Clause
#
# digiKam database shared library target
#

add_library(digikamdatabase SHARED $<TARGET_OBJECTS:digikamdatabase_obj>)

set_target_properties(digikamdatabase PROPERTIES
                      VERSION ${DIGIKAM_VERSION_SHORT}
                      SOVERSION ${DIGIKAM_VERSION_SHORT}
)

target_compile_definitions(digikamdatabase
                           PRIVATE
                           digikamdatabase_EXPORTS
)

target_include_directories(digikamdatabase
                           PRIVATE
                           ${DIGIKAM_TARGET_INCLUDES}
)

add_dependencies(digikamdatabase digikamcore)

# All codes from this target are exported with digikam_database_export.h header and DIGIKAM_DATABASE_EXPORT macro.
generate_export_header(digikamdatabase
                       BASE_NAME digikam_database
                       EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/utils/digikam_database_export.h"
)

# NOTE: all this target dependencies must be private and not exported to prevent inherited dependencies on external plugins.

target_link_libraries(digikamdatabase

                      PRIVATE

                      Qt${QT_VERSION_MAJOR}::Core
                      Qt${QT_VERSION_MAJOR}::Gui
                      Qt${QT_VERSION_MAJOR}::Sql

                      KF5::ConfigCore
                      KF5::Solid
                      KF5::I18n

                      digikamcore

                      opencv_core
                      opencv_objdetect
                      opencv_imgproc
                      opencv_imgcodecs
                      opencv_dnn
                      opencv_ml
                      opencv_flann
)

if(ENABLE_DBUS)

    target_link_libraries(digikamdatabase
                          PRIVATE
                          Qt${QT_VERSION_MAJOR}::DBus
    )

endif()

### Install Rules ###############################################################################################################

install(TARGETS digikamdatabase EXPORT DigikamDatabaseConfig ${INSTALL_TARGETS_DEFAULT_ARGS})
install(EXPORT DigikamDatabaseConfig DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/DigikamDatabase" NAMESPACE Digikam::)

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/DigikamDatabaseConfigVersion.cmake
                                 VERSION ${DIGIKAM_VERSION_SHORT}
                                 COMPATIBILITY SameMajorVersion)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/DigikamDatabaseConfigVersion.cmake
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/DigikamDatabase")

if(APPLE)
    install(FILES "$<TARGET_FILE:digikamdatabase>.dSYM" DESTINATION "${CMAKE_INSTALL_LIBDIR}" CONFIGURATIONS Debug RelWithDebInfo)
endif()
