cmake_minimum_required (VERSION 3.11)
project(serial_analyzer)

# custom CMake Modules are located in the cmake directory.
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(ExternalAnalyzerSDK)

set(SOURCES 
src/SerialAnalyzer.cpp
src/SerialAnalyzer.h
src/SerialAnalyzerResults.cpp
src/SerialAnalyzerResults.h
src/SerialAnalyzerSettings.cpp
src/SerialAnalyzerSettings.h
src/SerialSimulationDataGenerator.cpp
src/SerialSimulationDataGenerator.h
)

add_analyzer_plugin(serial_analyzer SOURCES ${SOURCES})
