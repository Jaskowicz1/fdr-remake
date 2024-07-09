cmake_minimum_required (VERSION 3.20)
project ("FDR" VERSION 1.0 DESCRIPTION "Factorio-Discord-Relay in C++")

file(GLOB source_files CONFIGURE_DEPENDS "src/*.cpp" "include/*.h")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

add_executable(${PROJECT_NAME}
	${source_files}
)

if(WIN32)
	find_package(dpp CONFIG REQUIRED)
	target_link_libraries(${PROJECT_NAME} PRIVATE 
		dpp::dpp
		wsock32
		ws2_32
	)
else()
	set_target_properties(${PROJECT_NAME} PROPERTIES LINK_SEARCH_START_STATIC 1)
	set_target_properties(${PROJECT_NAME} PROPERTIES LINK_SEARCH_END_STATIC 1)

	set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
	target_link_libraries(${PROJECT_NAME} libdpp.a)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
)

target_include_directories(${PROJECT_NAME} PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/include
)
