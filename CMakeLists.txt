cmake_minimum_required(VERSION 2.6)

project("Pure64" C)

include("version.cmake")

option (PURE64_WITH_UTIL "Build the Pure64 utility program." ON)

include_directories("include")

add_subdirectory("src/lib")

if (PURE64_WITH_UTIL)
	add_subdirectory("src/util")
endif (PURE64_WITH_UTIL)

set (CPACK_PACKAGE_NAME "Pure64")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "A bootloader for 64-bit systems.")
set (CPACK_PACKAGE_VENDOR "ReturnInfinity")
set (CPACK_PACKAGE_VERSION_MAJOR ${PURE64_VERSION_MAJOR})
set (CPACK_PACKAGE_VERSION_MINOR ${PURE64_VERSION_MINOR})
set (CPACK_PACKAGE_VERSION_PATCH ${PURE64_VERSION_PATCH})
set (CPACK_PACKAGE_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
set (CPACK_PACKAGE_CONTACT "taylorcholberton@gmail.com")
include (CPack)
