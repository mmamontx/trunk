	mov rcx, 16
M1:	
	push rcx
	push rax
	
	and rax, 0xf0000000
	shr rax, 60
	
	cmp rax, 9h
	jle M2
	add rax, 27h
M2:
	add rax, 30h
	mov qword [buf], rax
	
	mov rax, 4;'write' system call.
	mov rbx, 1
	mov rcx, buf
	mov rdx, 8
	int 0x80
	
	pop rax
	shl rax, 4
	pop rcx
	
	loop M1
EXIT:
	mov rax, 4;'write' system call.
	mov rbx, 1
	mov rcx, eol
	mov rdx, 1
	int 0x80
	
	mov rax, 1;'exit' system call.
	mov rbx, 0
	int 0x80
