cmake_minimum_required(VERSION 2.6)

add_executable("pure64"
	"fstream.c"
	"memory.c"
	"pure64.c"
	"util.c")

target_link_libraries("pure64" "pure64-core" "pure64-lang")
