cmake_minimum_required (VERSION 3.20)
project ("FDR" VERSION 0.1 DESCRIPTION "Factorio-Discord-Relay in C++")

file(GLOB source_files CONFIGURE_DEPENDS "src/*.cpp" "include/*.h")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

add_executable(${PROJECT_NAME}
	${source_files}
)

find_package(DPP REQUIRED)

target_link_libraries(${PROJECT_NAME}
	${DPP_LIBRARIES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
	${DPP_INCLUDE_DIR}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
	CMAKE_STANDARD 17
	CMAKE_STANDARD_REQUIRED ON
)

