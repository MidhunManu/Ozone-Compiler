    default rel
    section .data
    print_buf: times 32 db 0
    newline: db 10

    section .text
    print_u64:
        lea rsi, [print_buf + 31]
        xor rcx, rcx

        mov rax, rdi
        cmp rax, 0
        jne .loop

        mov byte [rsi], '0'
        mov rcx, 1
        jmp .write

    .loop:
        xor rdx, rdx
        mov rbx, 10
        div rbx
        add dl, '0'
        mov [rsi], dl
        dec rsi
        inc rcx
        test rax, rax
        jne .loop

        inc rsi

    .write:
        mov rax, 1
        mov rdi, 1
        mov rdx, rcx
        syscall
        ret
global _start
_start:
    mov rax, 2
    push rax
    mov rax, 3
    push rax
    pop rax
    pop rbx
    mul rbx
    push rax
    mov rax, 5
    push rax
    pop rax
    pop rbx
    sub rax, rbx
    push rax
    push QWORD [rsp + 0]

    pop rdi
    call print_u64
    mov rax, 1
    mov rdi, 1
    lea rsi, [newline]
    mov rdx, 1
    syscall
    mov rax, 60
    mov rdi, 0
    syscall
