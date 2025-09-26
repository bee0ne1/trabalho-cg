cmake_minimum_required(VERSION 4.0)
project(T1CG)

# Habilita ferramentas automáticas do Qt (MOC, UIC, RCC)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(Qt6 COMPONENTS Widgets REQUIRED)

add_executable(T1CG main.cpp
        drawingcanvas.cpp
        drawingcanvas.h
        drawingcanvas.ui
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

# Vincula o nosso executável com as bibliotecas do Qt que o find_package encontrou.
target_link_libraries(T1CG PRIVATE Qt6::Widgets)

