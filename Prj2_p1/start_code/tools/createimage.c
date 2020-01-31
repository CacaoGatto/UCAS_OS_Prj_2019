#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define os_size 384

void write_bootblock(FILE *image, FILE *bbfile, Elf32_Phdr *Phdr);
Elf32_Phdr *read_exec_file(FILE *opfile);
uint8_t count_kernel_sectors(Elf32_Phdr *Phdr);
void extent_opt(Elf32_Phdr *Phdr_bb, Elf32_Phdr *Phdr_k, int kernelsz);

Elf32_Phdr *read_exec_file(FILE *opfile)
{
    Elf32_Ehdr *Ehdr = (Elf32_Ehdr*)malloc(sizeof(Elf32_Ehdr));
    fread(Ehdr, sizeof(Elf32_Ehdr),1,opfile);
    fseek(opfile,Ehdr->e_phoff * sizeof(char),SEEK_SET);
    Elf32_Phdr *Phdr = (Elf32_Phdr*)malloc(sizeof(Elf32_Phdr));
    fread(Phdr, sizeof(char), Ehdr->e_phentsize,opfile);
    free(Ehdr);
    return Phdr;
}

uint8_t count_kernel_sectors(Elf32_Phdr *Phdr)
{
    uint8_t temp = (uint8_t) (Phdr->p_memsz / 512);
    temp += (Phdr->p_memsz % 512 != 0);
    return temp;
}

void write_bootblock(FILE *image, FILE *file, Elf32_Phdr *phdr)
{
    char buff[512];
    memset(buff,0,sizeof(buff));
	fseek(file,phdr->p_offset * sizeof(char),SEEK_SET);
    fread(buff,sizeof(char),phdr->p_filesz,file);
    fwrite(buff,sizeof(char),512,image);
}

void write_kernel(FILE *image, FILE *knfile, Elf32_Phdr *Phdr, int kernelsz)
{
    char buff[512*kernelsz];
    memset(buff,0,sizeof(buff));
	fseek(knfile,Phdr->p_offset * sizeof(char),SEEK_SET);
    fread(buff,sizeof(char),Phdr->p_filesz,knfile);
    fwrite(buff,sizeof(char),512*kernelsz,image);
}

void record_kernel_sectors(FILE *image, uint8_t kernelsz)
{
    fseek(image,os_size * sizeof(char),SEEK_SET);
    fputc(kernelsz,image);
    fseek(image,510 * sizeof(char),SEEK_SET);
    fputc(0x55,image);
    fputc(0xaa,image);
}

void extent_opt(Elf32_Phdr *Phdr_bb, Elf32_Phdr *Phdr_k, int kernelsz)
{
    printf("kernelsz:      0x%xB\n",Phdr_k->p_memsz);
	printf("filesz   :     0x%xB\n",Phdr_k->p_filesz);
    printf("offset  :      0x%x\n",Phdr_k->p_offset);
    printf("sector  :      0x%x\n",1+kernelsz);
}

int main()
{
    FILE* bbfile = fopen("bootblock","r+");
    Elf32_Phdr* bbphdr = read_exec_file(bbfile);
    FILE* ifile = fopen("image","w+");
    write_bootblock(ifile,bbfile,bbphdr);
    fclose(bbfile);
    free(bbphdr);
    FILE* knfile = fopen("main", "r+");
    Elf32_Phdr* knphdr = read_exec_file(knfile);
    uint8_t kernelsz = count_kernel_sectors(knphdr);
    write_kernel(ifile,knfile,knphdr,kernelsz);
    fclose(ifile);
    fclose(knfile);
    ifile = fopen("image","r+");
    record_kernel_sectors(ifile,kernelsz);
    fclose(ifile);
    extent_opt(bbphdr,knphdr,kernelsz);
    return 0;
}
