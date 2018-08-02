#include <sys/syscall.h>

    .text
    
    .global write
write:
    mov	    $SYS_write, %eax
    jmp	    csys
    
    .global open
open:
    mov	    $SYS_open, %eax
    jmp	    csys
    
    .global close
close:
    mov	    $SYS_close, %eax
    jmp	    csys
    
    .global exit
exit:
    mov	    $SYS_exit, %eax
    jmp	    csys

    .global __syscall
__syscall:
    mov     $0xc6, %eax
csys:
    int     $0x80
    jb      cerror
    ret

cerror:
    mov	    %eax, errno
    mov	    $-1, %eax
    ret

    .extern main
    .global _start
_start:
    mov	    $stack, %eax
    mov	    %eax, %esp
    xor	    %ebp, %ebp
#    movw    $0x1b, %ax
#    movw    %ax, %gs
    jmp	    main

    .data
errno:
    .long   0

    .space  0x300000
stack:
