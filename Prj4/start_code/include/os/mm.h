#ifndef INCLUDE_MM_H_
#define INCLUDE_MM_H_

#include "type.h"

#define TLB_ENTRY_NUMBER 64
#define MAX_PAGE 256
#define MAX_PTE  1024
#define P_ZONE   2048
#define PTE_SRORE_BASE 0xa0e00000
#define P_MAPPING_BASE 0x00000800

typedef struct pte{
    uint32_t entryhi;
    uint32_t entrylo;
} pte_t;

typedef struct ref{
    pte_t* pte_list[MAX_PTE];
//    int valid;
} pte_page;

//pte_t pte_list[MAX_PTE];
extern pte_page* pte_reference[MAX_PAGE];
extern char physical_valid[P_ZONE];

void do_TLB_Refill();
void do_page_fault();

void mod_helper();

void init_page(uint8_t);
void init_pte(uint8_t, uint16_t, uint32_t);

extern uint16_t pte_count;
extern uint8_t page_count;

#endif
