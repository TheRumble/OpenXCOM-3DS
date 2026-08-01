	.section .rodata
	.global common_zip
	.type common_zip, %object
	.align 8
common_zip:
	.incbin "common.zip"
common_zip_end:

	.global common_zip_size
	.type common_zip_size, %object
	.align 8
common_zip_size:
	.int common_zip_end - common_zip

	.global standard_zip
	.type standard_zip, %object
	.align 8
standard_zip:
	.incbin "standard.zip"
standard_zip_end:

	.global standard_zip_size
	.type standard_zip_size, %object
	.align 8
standard_zip_size:
	.int standard_zip_end - standard_zip
