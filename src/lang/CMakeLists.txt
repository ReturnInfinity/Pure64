cmake_minimum_required(VERSION 2.6)

add_library("pure64-lang"
	"config.c"
	"token.c"
	"var.c")

add_executable("config-test" "config-test.c")
target_link_libraries("config-test" "pure64-lang")
add_test("config-test" "config-test")

add_executable("token-test" "token-test.c")
target_link_libraries("token-test" "pure64-lang")
add_test("token-test" "token-test")

enable_testing()
