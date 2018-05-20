SECTION .data
buf	db "????????"
eol	db 0xa
SECTION .text
global _start
_start:
	xor rax, rax
	mov ebp, 0x400
