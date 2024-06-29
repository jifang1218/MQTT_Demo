#cmake_minimum_required(VERSION 2.8.12)
cmake_minimum_required(VERSION 3.5)

set(PROJ_NAME MQTT_Demo)
project(${PROJ_NAME} VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
		MQTTLoginInfo.cpp
		MQTTLoginInfo.hpp
		client/MQTTClient.cpp
		client/MQTTClient.hpp
        Utils.cpp
        Utils.hpp
        client/ClientWindow.cpp
        client/ClientWindow.hpp
        server/ServerWindow.cpp
        server/ServerWindow.hpp
)

link_directories(/usr/local/lib)

# jsoncpp
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

# Qt config
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${PROJ_NAME}
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET ${PROJ_NAME} APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
		add_library(${PROJ_NAME} SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
		add_executable(${PROJ_NAME}
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# mqtt
list(APPEND CMAKE_MODULE_PATH "/usr/local/lib/cmake/eclipse-paho-mqtt-c")
include(eclipse-paho-mqtt-cConfig)

add_definitions(-DTEST=1)
target_include_directories(${PROJ_NAME} PRIVATE ${JSONCPP_INCLUDE_DIRS})
target_link_libraries(${PROJ_NAME}
	PRIVATE eclipse-paho-mqtt-c::paho-mqtt3a 
	${JSONCPP_LIBRARIES}
	Qt${QT_VERSION_MAJOR}::Core
	Qt${QT_VERSION_MAJOR}::Widgets
	Qt${QT_VERSION_MAJOR}::Gui
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
	set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.${PROJ_NAME})
endif()
set_target_properties(${PROJ_NAME} PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ${PROJ_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
	qt_finalize_executable(${PROJ_NAME})
endif()

