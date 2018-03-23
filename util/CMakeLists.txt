cmake_minimum_required(VERSION 2.6)

project("Pure64" C)

include_directories("../include")

add_library("pure64lib" SHARED
	"../src/lib/dap.c"
	"../src/lib/dir.c"
	"../src/lib/error.c"
	"../src/lib/file.c"
	"../src/lib/fs.c"
	"../src/lib/gpt.c"
	"../src/lib/mbr.c"
	"../src/lib/misc.c"
	"../src/lib/partition.c"
	"../src/lib/path.c"
	"../src/lib/stream.c"
	"../src/lib/string.c"
	"../src/lib/uuid.c")

set_target_properties("pure64lib" PROPERTIES OUTPUT_NAME "pure64")

add_executable("pure64"
	"../src/util/fstream.c"
	"../src/util/util.c"
	"../src/util/pure64.c")

target_link_libraries("pure64" "pure64lib")
