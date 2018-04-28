cmake_minimum_required(VERSION 2.6)

project("Pure64" C)

include("version.cmake")

option (PURE64_WITH_UTIL "Build the Pure64 utility program." ON)
option (PURE64_WITH_BOOTLOADER "Build the Pure64 boot loader." OFF)

include_directories("include")

add_subdirectory("src/lib")
add_subdirectory("src/lib/lang")

if (PURE64_WITH_BOOTLOADER)
	message(WARNING "CMake cannot currently build the bootloader.")
	add_subdirectory("src/arch/${PURE64_ARCH}")
	add_subdirectory("src/stage-three")
elseif (PURE64_WITH_UTIL)
	add_subdirectory("src/util")
endif (PURE64_WITH_BOOTLOADER)

enable_testing()

set (CPACK_PACKAGE_NAME "Pure64")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "A bootloader for 64-bit systems.")
set (CPACK_PACKAGE_VENDOR "ReturnInfinity")
set (CPACK_PACKAGE_VERSION_MAJOR ${PURE64_VERSION_MAJOR})
set (CPACK_PACKAGE_VERSION_MINOR ${PURE64_VERSION_MINOR})
set (CPACK_PACKAGE_VERSION_PATCH ${PURE64_VERSION_PATCH})
set (CPACK_PACKAGE_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
set (CPACK_PACKAGE_CONTACT "taylorcholberton@gmail.com")
include (CPack)
