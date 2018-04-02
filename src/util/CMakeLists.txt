cmake_minimum_required(VERSION 2.6)

add_executable("pure64"
	"config.c"
	"fstream.c"
	"memory.c"
	"pure64.c"
	"token.c"
	"util.c")

target_link_libraries("pure64" "pure64lib")

add_executable("config-test"
	"config-test.c"
	"config.c"
	"token.c")

add_test("config-test" "config-test")

add_executable("token-test"
	"token.c"
	"token-test.c")

add_test("token-test" "token-test")

enable_testing()
