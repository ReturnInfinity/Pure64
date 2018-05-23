cmake_minimum_required(VERSION 2.6)

add_library("pure64-lang"
	"config.c"
	"parser.c"
	"scanner.c"
	"token.c"
	"var.c")

add_executable("config-test" "config-test.c")
target_link_libraries("config-test" "pure64-lang" "pure64-core")
add_test(NAME "ConfigurationTest" COMMAND "config-test")

add_executable("scanner-test" "scanner-test.c")
target_link_libraries("scanner-test" "pure64-lang" "pure64-core")
add_test(NAME "ScannerTest" COMMAND "scanner-test")

add_executable("parser-test" "parser-test.c")
target_link_libraries("parser-test" "pure64-lang" "pure64-core")
add_test(NAME "ParserTest" COMMAND "parser-test")

enable_testing()
