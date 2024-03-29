/*
    Name 1: Ignacio Gonzalez
    UTEID 1: ieg356
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Instruction Level Simulator                         */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files: isaprogram   LC-3b machine language program file     */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void process_instruction();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
*/

#define WORDS_IN_MEM    0x08000 
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */


typedef struct System_Latches_Struct{

  int PC,		/* program counter */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P;		/* p condition bit */
  int REGS[LC_3b_REGS]; /* register file. */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int INSTRUCTION_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands                    */
/*                                                             */
/***************************************************************/
void help() {                                                    
  printf("----------------LC-3b ISIM Help-----------------------\n");
  printf("go               -  run program to completion         \n");
  printf("run n            -  execute program for n instructions\n");
  printf("mdump low high   -  dump memory from low to high      \n");
  printf("rdump            -  dump the register & bus values    \n");
  printf("?                -  display this help menu            \n");
  printf("quit             -  exit the program                  \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {                                                

  process_instruction();
  CURRENT_LATCHES = NEXT_LATCHES;
  INSTRUCTION_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {                                      
  int i;

  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating for %d cycles...\n\n", num_cycles);
  for (i = 0; i < num_cycles; i++) {
    if (CURRENT_LATCHES.PC == 0x0000) {
	    RUN_BIT = FALSE;
	    printf("Simulator halted\n\n");
	    break;
    }
    cycle();
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed                 */
/*                                                             */
/***************************************************************/
void go() {                                                     
  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating...\n\n");
  while (CURRENT_LATCHES.PC != 0x0000)
    cycle();
  RUN_BIT = FALSE;
  printf("Simulator halted\n\n");
}

/***************************************************************/ 
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {          
  int address; /* this is a byte address */

  printf("\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
  printf("-------------------------------------\n");
  for (address = (start >> 1); address <= (stop >> 1); address++)
    printf("  0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
  printf("\n");

  /* dump the memory contents into the dumpsim file */
  fprintf(dumpsim_file, "\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
  fprintf(dumpsim_file, "-------------------------------------\n");
  for (address = (start >> 1); address <= (stop >> 1); address++)
    fprintf(dumpsim_file, " 0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
  fprintf(dumpsim_file, "\n");
  fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */   
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {                               
  int k; 

  printf("\nCurrent register/bus values :\n");
  printf("-------------------------------------\n");
  printf("Instruction Count : %d\n", INSTRUCTION_COUNT);
  printf("PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
  printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  printf("Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    printf("%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
  printf("\n");

  /* dump the state information into the dumpsim file */
  fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
  fprintf(dumpsim_file, "-------------------------------------\n");
  fprintf(dumpsim_file, "Instruction Count : %d\n", INSTRUCTION_COUNT);
  fprintf(dumpsim_file, "PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
  fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  fprintf(dumpsim_file, "Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    fprintf(dumpsim_file, "%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
  fprintf(dumpsim_file, "\n");
  fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */  
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {                         
  char buffer[20];
  int start, stop, cycles;

  printf("LC-3b-SIM> ");

  scanf("%s", buffer);
  printf("\n");

  switch(buffer[0]) {
  case 'G':
  case 'g':
    go();
    break;

  case 'M':
  case 'm':
    scanf("%i %i", &start, &stop);
    mdump(dumpsim_file, start, stop);
    break;

  case '?':
    help();
    break;
  case 'Q':
  case 'q':
    printf("Bye.\n");
    exit(0);

  case 'R':
  case 'r':
    if (buffer[1] == 'd' || buffer[1] == 'D')
	    rdump(dumpsim_file);
    else {
	    scanf("%d", &cycles);
	    run(cycles);
    }
    break;

  default:
    printf("Invalid Command\n");
    break;
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {                                           
  int i;

  for (i=0; i < WORDS_IN_MEM; i++) {
    MEMORY[i][0] = 0;
    MEMORY[i][1] = 0;
  }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {                   
  FILE * prog;
  int ii, word, program_base;

  /* Open program file. */
  prog = fopen(program_filename, "r");
  if (prog == NULL) {
    printf("Error: Can't open program file %s\n", program_filename);
    exit(-1);
  }

  /* Read in the program. */
  if (fscanf(prog, "%x\n", &word) != EOF)
    program_base = word >> 1;
  else {
    printf("Error: Program file is empty\n");
    exit(-1);
  }

  ii = 0;
  while (fscanf(prog, "%x\n", &word) != EOF) {
    /* Make sure it fits. */
    if (program_base + ii >= WORDS_IN_MEM) {
	    printf("Error: Program file %s is too long to fit in memory. %x\n",
             program_filename, ii);
	    exit(-1);
    }

    /* Write the word to memory array. */
    MEMORY[program_base + ii][0] = word & 0x00FF;
    MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
    ii++;
  }

  if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = (program_base << 1);

  printf("Read %d words from program into memory.\n\n", ii);
}

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */ 
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename, int num_prog_files) { 
  int i;

  init_memory();
  for ( i = 0; i < num_prog_files; i++ ) {
    load_program(program_filename);
    while(*program_filename++ != '\0');
  }
  CURRENT_LATCHES.Z = 1;  
  NEXT_LATCHES = CURRENT_LATCHES;
    
  RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {                              
  FILE * dumpsim_file;

  /* Error Checking */
  if (argc < 2) {
    printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
           argv[0]);
    exit(1);
  }

  printf("LC-3b Simulator\n\n");

  initialize(argv[1], argc - 1);

  if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
    printf("Error: Can't open dumpsim file\n");
    exit(-1);
  }

  while (1)
    get_command(dumpsim_file);
    
}

/***************************************************************/
/* Do not modify the above code.
   You are allowed to use the following global variables in your
   code. These are defined above.

   MEMORY

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */

/***************************************************************/

int pows(int num, int a){
  //printf("Pows: %d, %d\n", num, a);
  int res = 1;
  for (int i = 0; i < a; i++){
    res *= num;
  }
  return res;
}

int getBit(int num, int i){
  //printf("Num: %08X, %04X\n", num, pows(2, i));
  return (num & pows(2, i)) >> i;
}

int sext(int num, int orig, int new){
  int sBit = getBit(num, orig-1);
  //printf("sBit: %d\n", sBit);
  if (sBit){ // true if negative num
    return (num | ((pows(2, new-orig) -1) << (orig))) & (pows(2, new) - 1);
  }
  else
    return num & (pows(2, new)-1); // Don't need to extend positive nums
}

int getReg(int num, int i){ // i is the lowest bit of the reg
  return ((num & pows(2, i+2)) + (num & pows(2, i+1)) + (num & pows(2, i))) >> i;
}

int add16(int num1, int num2){
  //printf("Add16: %08X %08X\n", sext(num1, 16, 32), sext(num2, 16, 32));
  return (sext(num1, 16, 32) + sext(num2, 16, 32)) & 0xFFFF;
}

void setCC(int reg){
  int val = sext(NEXT_LATCHES.REGS[reg], 16, 32);
  //printf("CC Check: %04X\n", val);
  NEXT_LATCHES.N = 0;
  NEXT_LATCHES.Z = 0;
  NEXT_LATCHES.P = 0;
  if (val == 0){
    NEXT_LATCHES.Z = 1;
  }
  else if (val < 0){
    NEXT_LATCHES.N = 1;
  }
  else {
    NEXT_LATCHES.P = 1;
  }
}

int getWord(int Address) { // Pass the address in lc3b
  // printf("Halves: %02X %02X\n", MEMORY[Address][1], MEMORY[Address][0]);
  return MEMORY[Address >> 1][0] + (MEMORY[Address >> 1][1] << 8);
  // MEMORY only holds bytes so we don't need to mask
}

int getByte(int Address) { // Pass the address in lc3b
  // printf("Byte: %02X\n", MEMORY[Address >> 1][Address % 1]);
  return MEMORY[Address >> 1][Address % 2];
  // MEMORY only holds bytes so we don't need to mask
}

void writeWord(int Address, int word){
  MEMORY[Address >> 1][0] = word & 0xFF;
  MEMORY[Address >> 1][1] = (word & 0xFF00) >> 8;
}

void writeByte(int Address, int word){
  MEMORY[Address >> 1][Address % 2] = word & 0xFF;
}

int rshfa(int word, int a){
  int sBit = getBit(word, 15); // get sign bit
  //printf("sBit: %d, a: %d\n", sBit, a);
  if (sBit == 0)
    return word >> a;  
  //printf("word >> a: %04X, or: %04X\n", word >> a, (pows(2, a) - 1) << (16 - a));
  return (word >> a) | ((pows(2, a) - 1) << (16 - a));
}

void process_instruction(){
  /*  function: process_instruction
    *  
    *    Process one instruction at a time  
    *       -Fetch one instruction
    *       -Decode 
    *       -Execute
    *       -Update NEXT_LATCHES
    */  
  
  int instruction = getWord(NEXT_LATCHES.PC);
  //printf("Instruction %X: 0x%04X\n", CURRENT_LATCHES.PC, instruction);
  NEXT_LATCHES.PC += 2;

  int opcode = (instruction & 0xF000) >> 12;
  //printf("Opcode: %X\n", opcode);

  switch (opcode){

    case 0:{ // BR *
      int n = getBit(instruction, 11);
      int z = getBit(instruction, 10);
      int p = getBit(instruction, 9);
      // printf("Curr PC: %04X\n", NEXT_LATCHES.PC + 0xFFFFFFFE);
      if ((n && NEXT_LATCHES.N) || (z && NEXT_LATCHES.Z) || (p && NEXT_LATCHES.P))
        NEXT_LATCHES.PC = add16(NEXT_LATCHES.PC, sext(instruction & 0x1FF, 9, 16) << 1); // PC is already incremented
      //printf("NZP: %X %X %X\n", n, z, p);
      //printf("Curr NZP: %X %X %X\n", NEXT_LATCHES.N, NEXT_LATCHES.Z, NEXT_LATCHES.P);
      //printf("Next PC: %04X, Offset: %04X\n", NEXT_LATCHES.PC, sext(instruction & 0x1FF, 9, 32) << 1);
      break;
    }
    case 1:{ // ADD *
      int DR = getReg(instruction, 9);
      //printf("DR: %d, Bit: %d\n", DR, getBit(instruction, 5));
      if(getBit(instruction, 5) == 0){
        //printf("SR: %d, SR2: %d\n", getReg(instruction, 6) + getReg(instruction, 0));
        NEXT_LATCHES.REGS[DR] = add16(NEXT_LATCHES.REGS[getReg(instruction, 6)], NEXT_LATCHES.REGS[getReg(instruction, 0)]);
      }
      else{
        NEXT_LATCHES.REGS[DR] = add16(NEXT_LATCHES.REGS[getReg(instruction, 6)], sext(instruction & 0x1F, 5, 32));
      }
      setCC(DR);
      break;
    }
    case 2:{ // LDB *
      int DR = getReg(instruction, 9);
      // printf("DR: %d\n", DR);
      // printf("TarA: %04X\n", add16(NEXT_LATCHES.REGS[getReg(instruction, 6)], sext(instruction & 0x3F, 6, 16)));
      int tar = getByte(add16(NEXT_LATCHES.REGS[getReg(instruction, 6)], sext(instruction & 0x3F, 6, 16)));
      // printf("Tar: %04X\n", tar);
      NEXT_LATCHES.REGS[DR] = sext(tar, 8, 16);
      setCC(DR);
      break;
    }
    case 3:{ // STB *
      int tarA = add16(NEXT_LATCHES.REGS[getReg(instruction, 6)], sext(instruction & 0x3F, 6, 16));
      //printf("tarA: %04X\n", tarA);
      writeByte(tarA, NEXT_LATCHES.REGS[getReg(instruction, 9)]);
      break;
    }
    case 4:{ // JSR *
      int sBit = getBit(instruction, 11);
      int temp = NEXT_LATCHES.PC;
      if (sBit == 0){
        NEXT_LATCHES.PC = NEXT_LATCHES.REGS[getReg(instruction, 6)];
      }
      else{
        NEXT_LATCHES.PC = add16(NEXT_LATCHES.PC, sext(instruction & 0x07FF, 11, 16) << 1);
      }
      NEXT_LATCHES.REGS[7] = temp;
      //printf("Next PC: %04X, Offset: %04X\n", NEXT_LATCHES.PC, sext(instruction & 0x1FF, 11, 32) << 1);
      break;
    }
    case 5:{ // AND *
      int sBit = getBit(instruction, 5);
      int DR = getReg(instruction, 9);
      if (sBit == 0){
        NEXT_LATCHES.REGS[DR] = NEXT_LATCHES.REGS[getReg(instruction, 6)] & NEXT_LATCHES.REGS[getReg(instruction, 0)];
      }
      else {
        NEXT_LATCHES.REGS[DR] = NEXT_LATCHES.REGS[getReg(instruction, 6)] & sext(instruction & 0x1F, 5, 16);
      }
      setCC(DR);
      break;
    }
    case 6:{ // LDW *
      int DR = getReg(instruction, 9);
      NEXT_LATCHES.REGS[DR] = getWord(add16(NEXT_LATCHES.REGS[getReg(instruction, 6)], sext(instruction & 0x3F, 6, 16) << 1));
      setCC(DR);
      break;
    }
    case 7:{ // STW *
      int tarA = add16(NEXT_LATCHES.REGS[getReg(instruction, 6)], sext(instruction & 0x3F, 6, 16) << 1);
      //printf("tarA: %04X\n", tarA);
      writeWord(tarA, NEXT_LATCHES.REGS[getReg(instruction, 9)]);
      break;
    }
    case 8:{ // RTI not implemented
      break;
    }
    case 9:{ // XOR *
      int DR = getReg(instruction, 9);
      int sBit = getBit(instruction, 5);
      if (sBit == 0){
        NEXT_LATCHES.REGS[DR] = NEXT_LATCHES.REGS[getReg(instruction, 6)] ^ NEXT_LATCHES.REGS[getReg(instruction, 0)];
      }
      else{
        NEXT_LATCHES.REGS[DR] = NEXT_LATCHES.REGS[getReg(instruction, 6)] ^ sext(instruction & 0x1F, 5, 16);
      }
      setCC(DR);
      break;
    }
    case 10:{ // Not used
      break;
    }
    case 11:{ // Not used
      break;
    }
    case 12:{ // JMP *
      int BR = getReg(instruction, 6);
      NEXT_LATCHES.PC = NEXT_LATCHES.REGS[BR];
      break;
    }
    case 13:{ // SHF *
      int DR = getReg(instruction, 9);
      int sBit5 = getBit(instruction, 5);
      int sBit4 = getBit(instruction, 4);
      if (sBit4 == 0){
        NEXT_LATCHES.REGS[DR] = (NEXT_LATCHES.REGS[getReg(instruction, 6)] << (instruction & 0xF)) & 0xFFFF;
      }
      else {
        if (sBit5 == 0){
          NEXT_LATCHES.REGS[DR] = (NEXT_LATCHES.REGS[getReg(instruction, 6)] >> (instruction & 0xF)) & 0xFFFF;
        }
        else{
          NEXT_LATCHES.REGS[DR] = rshfa(NEXT_LATCHES.REGS[getReg(instruction, 6)], instruction & 0xF) & 0xFFFF;
        }
      }
      setCC(DR);
      break;
    }
    case 14:{ // LEA *
      int DR = getReg(instruction, 9);
      NEXT_LATCHES.REGS[DR] = add16(NEXT_LATCHES.PC, sext(instruction & 0x01FF, 9, 16) << 1);
      break;
    }
    case 15:{ // TRAP *
      NEXT_LATCHES.REGS[7] = NEXT_LATCHES.PC;
      NEXT_LATCHES.PC = getWord((instruction & 0x00FF) << 1);
      //printf("TRAP Address: %04X\n", (instruction & 0x00FF) << 1);
      break;
    }
    default:
      break;
  }

}
