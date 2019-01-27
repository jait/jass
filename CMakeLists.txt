cmake_minimum_required (VERSION 2.6)
project(jass)

if(CMAKE_COMPILER_IS_GNUCC OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -flto -Wall")

endif()
add_subdirectory(src)

