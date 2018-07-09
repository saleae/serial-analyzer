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

add_library(serial_analyzer SHARED ${SOURCES})
target_link_libraries(serial_analyzer PRIVATE Saleae::AnalyzerSDK)

if(MSVC)
    install(TARGETS serial_analyzer RUNTIME DESTINATION ".")
else()
    install(TARGETS serial_analyzer LIBRARY DESTINATION ".")
endif()

set_post_build_destination(serial_analyzer)
