cmake_minimum_required(VERSION 3.19)
project(GUI LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(GUI
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    extractor.h extractor.cpp
    traceswidget.h traceswidget.cpp
    tracesscrollarea.h tracesscrollarea.cpp


)

target_link_libraries(GUI
    PRIVATE
        Qt::Core
        Qt::Widgets
)
target_link_libraries(GUI PRIVATE Qt6::Widgets)
target_link_libraries(GUI PRIVATE Qt6::Widgets)

include(GNUInstallDirs)

install(TARGETS GUI
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET GUI
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
