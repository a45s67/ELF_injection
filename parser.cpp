#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<elf.h>

//using namespace std;


typedef struct ELF_FORMAT{
    Elf64_Ehdr *ehdr;
    Elf64_Phdr *phdr;
    Elf64_Shdr *shdr;
    
}elf_format;

void parseelf(char* data_buf){
    
    elf_format hdrs;

    hdrs.ehdr = (Elf64_Ehdr*) data_buf;

    printf("e_shstrndx:%d\n",hdrs.ehdr->e_shstrndx);
    hdrs.phdr = (Elf64_Phdr*)(data_buf + hdrs.ehdr->e_phoff);
    hdrs.shdr = (Elf64_Shdr*)(data_buf + hdrs.ehdr->e_shoff);

    printf("ph nums:%d\n",hdrs.ehdr->e_phnum);
    printf("sh nums:%d\n",hdrs.ehdr->e_shnum);
    
    for(int ind = 0; ind<hdrs.ehdr->e_phnum;ind++){
       
        Elf64_Phdr* phdr = &hdrs.phdr[ind];
        int x = phdr->p_flags&PF_X, w = phdr->p_flags&PF_W, r = phdr->p_flags&PF_R;
        x? x='x':x='-';
        w? w='w':w='-';
        r? r='r':r='r';

        printf("PH%d:\n",ind);
        printf("ptype:%d,\n"
                "p_offset:0x%08x,\t p_vaddr:%#08x,\t p_paddr:%#08x\n"
                "p_filesz:%#08x,\t p_memsz:%#08x,\t p_flags:%c%c%c\n"
                "p_align:%d \n=======\n",
                phdr->p_type,phdr->p_offset,phdr->p_vaddr,phdr->p_paddr,
                phdr->p_filesz,phdr->p_memsz,x,w,r,
                phdr->p_align);
    }


    const char * const symtab = hdrs.shdr[hdrs.ehdr->e_shstrndx].sh_offset + data_buf;
    for( int ind=0; ind<hdrs.ehdr->e_shnum; ind++ ){
        Elf64_Shdr &shdr = hdrs.shdr[ind];
        if (shdr.sh_type==SHT_SYMTAB){
            //symtab = &shdr;
            printf("sec %d = symtb\n",ind);
            break;
        }
    }
    for( int ind=0; ind<hdrs.ehdr->e_shnum; ind++ ){
        Elf64_Shdr &shdr = hdrs.shdr[ind];
        printf("Section Header - %d \n",ind);
        printf("sh_name: %s\n"
                "sh_type: %d,\tsh_flags: %d\n"
                "addr:\t0x%08x,\n"

                "offset:\t%#010x,\n"

                "size:\t%#010x\n"
                "sh_link: %d,\tsh_info: %d,\tsh_addralign: %#010x\n"
                "sh_entsize: %#010x\n========\n",
                shdr.sh_name+symtab,
                shdr.sh_type,shdr.sh_flags,
                shdr.sh_addr,shdr.sh_offset,shdr.sh_size,
                shdr.sh_link,shdr.sh_info,shdr.sh_addralign,
                shdr.sh_entsize);



    }
}






int main(){


    FILE *file;

    file = fopen("a.out","rb");
    if(file==0){
        puts("open failed.");
        exit(1);
    }
    
    struct stat fileStat;

    if(fstat(fileno(file),&fileStat)){
        perror("fstat");
    }
    long filesz = fileStat.st_size;

    char *data_buf = (char*)malloc(sizeof(char)*filesz);

    fread(data_buf,1,filesz,file);

    parseelf(data_buf);



    return 0;
}
