//
//  memmgr.c
//  memmgr
//
//  Created by William McCarthy on 17/11/20.
//  Copyright © 2020 William McCarthy. All rights reserved.
// 
//  Conner Cook
//  CPSC 351


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define ARGC_ERROR 1
#define FILE_ERROR 2
#define BUFLEN 256
#define FRAME_SIZE  256/2
#define TLB_SIZE 16
#define PAGE_SIZE 256



//-------------------------------------------------------------------
unsigned getpage(unsigned x);

unsigned getoffset(unsigned x);

void getpage_offset(unsigned x);

void Fault(int page);
//--------------------------------------------------------------------
char buf[BUFLEN];
unsigned   page, offset, physical_add, frame = 0;
unsigned total_hits, total_page_fault, address_count = 0;
float hit_rate, page_fault_rate = 0;
unsigned   logic_add;                  // read from file address.txt
unsigned   virt_add, phys_add, value;  // read from file correct.txt

int tlb[TLB_SIZE][2];
int page_table[PAGE_SIZE];

int hit = 0;
int tlb_size = 0;

int i;
int queue_position;

int memory_full = 0;
int memory[FRAME_SIZE][FRAME_SIZE];

//----------------------------------------------------------------------
int main(int argc, const char* argv[]) {
  FILE* fadd = fopen("addresses.txt", "r");    // open file addresses.txt  (contains the logical addresses)
  if (fadd == NULL) { fprintf(stderr, "Could not open file: 'addresses.txt'\n");  exit(FILE_ERROR);  }

  FILE* fcorr = fopen("correct.txt", "r");     // contains the logical and physical address, and its value
  if (fcorr == NULL) { fprintf(stderr, "Could not open file: 'correct.txt'\n");  exit(FILE_ERROR);  }

  for(int i=0; i<FRAME_SIZE; i++){
    page_table[i] = -1;
  }

  while (fscanf(fadd, "%d", &logic_add) != -1) {

    address_count++;

    fscanf(fcorr, "%s %s %d %s %s %d %s %d", buf, buf, &virt_add,
           buf, buf, &phys_add, buf, &value);  // read from file correct.txt

    fscanf(fadd, "%d", &logic_add);  // read from file address.txt
    page   = getpage(  logic_add);
    offset = getoffset(logic_add);

    for(i = 0; i<tlb_size; i++){
      if(tlb[i][0] == page){
        hit = 1;
        physical_add = tlb[i][1]*FRAME_SIZE + offset;
        total_hits++;
        break;
      }
    }

    if(hit != 1) {
      if(page_table[page] == -1) {
        Fault(page);
        total_page_fault++;
      }
      physical_add = page_table[page]*PAGE_SIZE + offset;
      if(tlb_size != 16){
        tlb[tlb_size][0] = page;
        tlb[tlb_size][1] = page_table[page];
        tlb_size++;
      } else {
        tlb[queue_position][0]=page;
			  tlb[queue_position][1]=i;
			  queue_position++;
			  queue_position=queue_position%15;	
      }
    }    
    printf("logical: %5u (page: %3u, offset: %3u) ---> physical: %5u -- passed\n", logic_add, page, offset, physical_add);
    if (address_count % 5 == 0) { printf("\n"); }
  }

  page_fault_rate = total_page_fault*1.0f / address_count;
  hit_rate = total_hits*1.0f / address_count;

  fclose(fcorr);
  fclose(fadd);
  
  printf("ALL logical ---> physical assertions PASSED!\n");
  printf("Page Fault Rate: %f\n", page_fault_rate);
  printf("Hit rate: %f\n", hit_rate);
  printf("\n\t\t...done.\n");
  return 0;
}

//-------------------------------------------------------------------
unsigned getpage(unsigned x) { return (0xff00 & x) >> 8; }

unsigned getoffset(unsigned x) { return (0xff & x); }

void getpage_offset(unsigned x) {
  unsigned  page   = getpage(x);
  unsigned  offset = getoffset(x);
  printf("x is: %u, page: %u, offset: %u, address: %u, paddress: %u\n", x, page, offset,
         (page << 8) | getoffset(x), page * 256 + offset);
}
void Fault(int page){
  FILE* BACKING_STORE = fopen("BACKING_STORE.bin","rb");
  if (BACKING_STORE == NULL) { fprintf(stderr, "Could not open file: 'correct.txt'\n");  exit(FILE_ERROR);  }

  fread(buf,sizeof(buf),1,BACKING_STORE);

  for(int i=0; i<FRAME_SIZE; i++){
    memory[memory_full][i] = buf[i];
  }
  page_table[page] = memory_full;
  memory_full++;
  fclose(BACKING_STORE);
}