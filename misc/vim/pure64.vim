if exists("b:current_syntax")
	finish
endif

syn keyword Pure64Key arch bootsector disk_size kernel_path partition_scheme stage_three
hi def link Pure64Key Keyword

syn keyword Pure64Value mbr multiboot multiboot2 pxe x86_64 loader kernel
hi def link Pure64Value Constant

syn region Pure64String start='"' end='"'
hi def link Pure64String String

syn match Pure64Number '\d\+'
hi def link Pure64Number Number

syn match Pure64Comment "#.*$"
hi def link Pure64Comment Comment

