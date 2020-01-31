#include "mm.h"
#include "sched.h"

pte_page* pte_reference[MAX_PAGE];
char physical_valid[P_ZONE];
uint16_t pte_count = 0;
uint8_t page_count = 0;

uint8_t tlb_index = 0;

//TODO:Finish memory management functions here refer to mm.h and add any functions you need.

void init_page(uint8_t page_num) {
    pte_reference[page_num] = (pte_page*)(PTE_SRORE_BASE+(page_count++)*0x2000);
    int i;
    for (i = 0; i < MAX_PTE; i++) {
        pte_reference[page_num]->pte_list[i] = (pte_t*)0xa0f10000;
    }
};

void init_pte(uint8_t page, uint16_t pte, uint32_t pid) {
    uint32_t vpn = page*MAX_PTE + pte;
    uint32_t pfn;
    for (pfn = 0; pfn < P_ZONE; pfn++) {
        if (physical_valid[pfn]) break;
    }
    
    physical_valid[pfn] = 0;
    pfn += P_MAPPING_BASE;
    
    pte_reference[page]->pte_list[pte] = (pte_t*)(0xa1000000-(++pte_count)*0x10);
    pte_reference[page]->pte_list[pte]->entryhi = (vpn<<13) | (pid & 0xff);
    pte_reference[page]->pte_list[pte]->entrylo = (pfn<<7) | 0x16;
};

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

void mod_helper() {
    printk("!!!!!!!!!");
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
    tlb_fill();
}
