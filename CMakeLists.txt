cmake_minimum_required(VERSION 3.16)
project(threat_db LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql)
find_package(SQLite3 REQUIRED)

add_executable(threat_db
    src/main.cpp
    src/Database.cpp
    src/AuthDialog.cpp
    src/RegisterDialog.cpp
    src/MainWindow.cpp
    src/ThreatForm.cpp
)

target_link_libraries(threat_db
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
    SQLite::SQLite3
)

target_include_directories(threat_db PRIVATE src)
