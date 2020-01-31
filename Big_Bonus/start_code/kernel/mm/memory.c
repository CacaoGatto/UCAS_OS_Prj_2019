#include "mm.h"
#include "sched.h"
/*
pte_page* pte_reference[MAX_PAGE];
char physical_valid[P_ZONE];
uint16_t pte_count = 0;
uint8_t page_count = 0;
*/
uint8_t tlb_index = 0;

//TODO:Finish memory management functions here refer to mm.h and add any functions you need.

uint16_t alloc_page()
{
    uint16_t i, j = 0;
    uint8_t temp;
    for (i = 0; i < 0x200; i++) {
        temp = *((uint8_t*)(VM_BITMAPPING + i));
        if (temp != 0xff) break;
    }
    for (j = 0; j < 8; j++) {
        if ((temp & 0x01) == 0) break;
        else temp = temp >> 1;
    }
    temp = 0x01 << j;
    *((uint8_t*)(VM_BITMAPPING + i)) |= temp;
    return (i * 8 + j);
}

void free_page(uint32_t page)
{
    uint16_t i = page / 8;
    uint16_t j = page % 8;
    uint8_t temp = 0x01 << j;
    *((uint8_t*)(VM_BITMAPPING + i)) &= (~temp);
}

uint16_t alloc_top_pte(uint8_t page)
{
    uint16_t ref = alloc_page();
    *((uint16_t*)(PTE_BASE_ADDR + page * 2)) = ref;
    int i;
    for (i = 0; i < 0x2000; i++) {
        *((uint8_t*)(0xa0000000 + ref * 0x2000 + i)) = 0x00;
    }
    return ref;
}

/*
void init_page(uint8_t page_num) {
    uint32_t i;
    for (i = 0; i < ; i++) {
        *((uint8_t*)(VM_BASE_ADDR + i)) = 0x00000000;
    }
}

void init_pte(pte_t* item, uint32_t vpn, uint32_t pid) {
    uint16_t pfn = alloc_page();
    
    item->entryhi = (vpn<<13) | (pid & 0xff);
    item->entrylo = (pfn<<7) | 0x16;
};
*/
void do_TLB_Refill() {
    uint32_t cp_badvaddr = get_badvaddr();
    uint8_t page = cp_badvaddr >> 23;
    uint16_t pte = (cp_badvaddr - (page << 23)) >> 13;
    uint16_t ref = *((uint16_t*)(PTE_BASE_ADDR + page * 2));
    pid_t cur_pid = current_running->pid;
    
    if (ref == 0x0000) ref = alloc_top_pte(page);
    
    pte_t* item = (pte_t*)(0xa0000000 + ref * 0x2000 + pte * 8);
    if (item->entrylo == 0) {
        uint32_t pfn = (uint32_t)alloc_page();
        item->entryhi = (cp_badvaddr & 0xffffe000) | (cur_pid & 0xff);
        item->entrylo = (pfn<<7) | 0x16;
    }
    
    uint32_t ehi = item->entryhi;
    uint32_t elo = item->entrylo;
    pid_t old_pid = item->entryhi & 0xff;
    if (old_pid != cur_pid) {
        printk("memory conflict!!!");
        do_exit();
    }
    
    int i;
    for (i = 0; i < MAX_LOCKS; i++) {
        if (current_running->mems[i] == 0xffffffff) {
            current_running->mems[i] = cp_badvaddr >> 13;
            break;
        }
    }
    
    set_entryhi(ehi);
    set_entrylo(elo);
    uint32_t cp_index = get_index(ehi);
    if ((cp_index & 0x80000000) != 0) {
        set_index(tlb_index & 0x1f);
        tlb_index++;
    }
    tlb_fill();
}

/*
void do_TLB_Refill() {
    uint32_t cp_badvaddr = get_badvaddr();
    uint8_t page = cp_badvaddr >> 23;
    uint16_t pte = (cp_badvaddr - (page << 23)) >> 13;
    if (pte_reference[page] == (pte_page*)0xa0f00000) {
        init_page(page);
    }
    if (pte_reference[page]->pte_list[pte] == (pte_t*)0xa0f10000) {
        init_pte(page, pte, (uint32_t)current_running->pid);
    }
    
    uint32_t ehi = pte_reference[page]->pte_list[pte]->entryhi;
    uint32_t elo = pte_reference[page]->pte_list[pte]->entrylo;
    pid_t old_pid = ehi & 0xff;
    pid_t cur_pid = current_running->pid;
    if (old_pid != cur_pid) {
        if (old_pid) do_exit();
        ehi = (ehi & 0xffffff00) | (cur_pid & 0xff);
        pte_reference[page]->pte_list[pte]->entryhi = ehi;
    }
    
    int i;
    for (i = 0; i < MAX_LOCKS; i++) {
        if (current_running->mems[i].page < 0) {
            current_running->mems[i].page = page;
            current_running->mems[i].pte = pte;
        }
    }
    
    set_entryhi(ehi);
    set_entrylo(elo);
    uint32_t cp_index = get_index(ehi);
    if ((cp_index & 0x80000000) != 0) {
        set_index(tlb_index & 0x1f);
        tlb_index++;
    }
    tlb_fill();
}
*/
void mod_helper() {
    printk("!!!!!!!!!");/*
    uint32_t cp_badvaddr = get_badvaddr();
    uint8_t page = cp_badvaddr >> 23;
    uint16_t pte = (cp_badvaddr - (page << 23)) >> 13;
    uint32_t ehi = pte_reference[page]->pte_list[pte]->entryhi;
    uint32_t elo = pte_reference[page]->pte_list[pte]->entrylo;
    uint32_t pid = (uint32_t)current_running->pid;
    ehi = (ehi & 0xffffff00) | (pid & 0xff);
    set_entryhi(ehi);
    set_entrylo(elo);
    uint32_t cp_index = get_index();
    if ((cp_index & 0x80000000) != 0) {
        set_index(tlb_index & 0x1f);
        tlb_index++;
    }
    tlb_fill();*/
}
