cmake_minimum_required(VERSION 3.19)
project(GUI LANGUAGES CXX)

if (MSVC)
    add_compile_options(/D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)


find_package(Qt6 REQUIRED COMPONENTS Core Widgets)


add_executable(GUI
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    extractor.h
    extractor.cpp
    traceswidget.h
    traceswidget.cpp
    tracesscrollarea.h
    tracesscrollarea.cpp
)


target_link_libraries(GUI
    PRIVATE
        Qt6::Core
        Qt6::Widgets
)


qt6_wrap_ui(GUI_UI_HEADERS
    mainwindow.ui
)
target_sources(GUI PRIVATE ${GUI_UI_HEADERS})


include(GNUInstallDirs)

install(TARGETS GUI
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)



