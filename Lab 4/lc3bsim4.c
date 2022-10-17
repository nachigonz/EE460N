/***************************************************************/
/*                                                             */
/*   LC-3b Simulator                                           */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         isaprogram   LC-3b machine language program file    */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void eval_micro_sequencer();
void cycle_memory();
void eval_bus_drivers();
void drive_bus();
void latch_datapath_values();

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
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64
#define INITIAL_STATE_NUMBER 18

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
enum CS_BITS {                                                  
    IRD,
    MEMCHECK,
    COND2, COND1, COND0,
    J5, J4, J3, J2, J1, J0,
    LD_MAR,
    LD_MDR,
    LD_IR,
    LD_BEN,
    LD_REG,
    LD_CC,
    LD_PC,
    GATE_PC,
    GATE_MDR,
    GATE_ALU,
    GATE_MARMUX,
    GATE_SHF,
    PCMUX1, PCMUX0,
    DRMUX1, DRMUX0,
    SR1MUX1, SR1MUX0,
    ADDR1MUX,
    ADDR2MUX1, ADDR2MUX0,
    MARMUX,
    ALUK1, ALUK0,
    MIO_EN,
    R_W,
    DATA_SIZE,
    LSHF1,
    GATE_TEMP,
    LD_TEMP,
    LD_PRIV,
    LD_PRIO,
    PSRMUX,
    GATE_PSR,
    LD_SSP,
    LD_USP,
    SPMUX1, SPMUX0,
    GATE_SP,
    GATE_OLDPC,
    LD_VECT,
    GATE_VECT,
    VECTMUX,
    LD_EXCV,
    EXCVMUX1, EXCVMUX0,
/* MODIFY: you have to add all your new control signals */
    CONTROL_STORE_BITS
} CS_BITS;

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x)           { return(x[IRD]); }
int GetMEMCHECK(int *x)      { return(x[MEMCHECK]); }
int GetCOND(int *x)          { return((x[COND2] << 2) + (x[COND1] << 1) + x[COND0]); }
int GetJ(int *x)             { return((x[J5] << 5) + (x[J4] << 4) +
				      (x[J3] << 3) + (x[J2] << 2) +
				      (x[J1] << 1) + x[J0]); }
int GetLD_MAR(int *x)        { return(x[LD_MAR]); }
int GetLD_MDR(int *x)        { return(x[LD_MDR]); }
int GetLD_IR(int *x)         { return(x[LD_IR]); }
int GetLD_BEN(int *x)        { return(x[LD_BEN]); }
int GetLD_REG(int *x)        { return(x[LD_REG]); }
int GetLD_CC(int *x)         { return(x[LD_CC]); }
int GetLD_PC(int *x)         { return(x[LD_PC]); }
int GetGATE_PC(int *x)       { return(x[GATE_PC]); }
int GetGATE_MDR(int *x)      { return(x[GATE_MDR]); }
int GetGATE_ALU(int *x)      { return(x[GATE_ALU]); }
int GetGATE_MARMUX(int *x)   { return(x[GATE_MARMUX]); }
int GetGATE_SHF(int *x)      { return(x[GATE_SHF]); }
int GetPCMUX(int *x)         { return((x[PCMUX1] << 1) + x[PCMUX0]); }
int GetDRMUX(int *x)         { return((x[DRMUX1] << 1) + x[DRMUX0]); }
int GetSR1MUX(int *x)        { return((x[SR1MUX1] << 1) + x[SR1MUX0]); }
int GetADDR1MUX(int *x)      { return(x[ADDR1MUX]); }
int GetADDR2MUX(int *x)      { return((x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); }
int GetMARMUX(int *x)        { return(x[MARMUX]); }
int GetALUK(int *x)          { return((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x)        { return(x[MIO_EN]); }
int GetR_W(int *x)           { return(x[R_W]); }
int GetDATA_SIZE(int *x)     { return(x[DATA_SIZE]); } 
int GetLSHF1(int *x)         { return(x[LSHF1]); }
int GetGATE_TEMP(int *x)     { return(x[GATE_TEMP]); }
int GetLD_TEMP(int *x)       { return(x[LD_TEMP]); }
int GetLD_PRIV(int *x)       { return(x[LD_PRIV]); }
int GetLD_PRIO(int *x)       { return(x[LD_PRIO]); }
int GetPSRMUX(int *x)        { return(x[PSRMUX]); }
int GetGATE_PSR(int *x)      { return(x[GATE_PSR]); }
int GetLD_SSP(int *x)        { return(x[LD_SSP]); }
int GetLD_USP(int *x)        { return(x[LD_USP]); }
int GetSPMUX(int *x)         { return((x[SPMUX1] << 1) + x[SPMUX0]); }
int GetGATE_SP(int *x)       { return(x[GATE_SP]); }
int GetGATE_OLDPC(int *x)    { return(x[GATE_OLDPC]); }
int GetLD_VECT(int *x)       { return(x[LD_VECT]); }
int GetGATE_VECT(int *x)     { return(x[GATE_VECT]); }
int GetVECTMUX(int *x)       { return(x[VECTMUX]); }
int GetLD_EXCV(int *x)       { return(x[LD_EXCV]); }
int GetEXCVMUX(int *x)       { return((x[EXCVMUX1] << 1) + x[EXCVMUX0]); }
/* MODIFY: you can add more Get functions for your new control signals */

/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
   There are two write enable signals, one for each byte. WE0 is used for 
   the least significant byte of a word. WE1 is used for the most significant 
   byte of a word. */

#define WORDS_IN_MEM    0x08000 
#define MEM_CYCLES      5
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */
int BUS;	/* value of the bus */

typedef struct System_Latches_Struct{

int PC,		/* program counter */
    MDR,	/* memory data register */
    MAR,	/* memory address register */
    IR,		/* instruction register */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P,		/* p condition bit */
    BEN;        /* ben register */

int READY;	/* ready bit */
  /* The ready bit is also latched as you dont want the memory system to assert it 
     at a bad point in the cycle*/

int REGS[LC_3b_REGS]; /* register file. */

int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microintruction */

int STATE_NUMBER; /* Current State Number - Provided for debugging */ 

/* For lab 4 */
int INTV; /* Interrupt vector register */
int EXCV; /* Exception vector register */
int SSP; /* Initial value of system stack pointer */
/* MODIFY: You may add system latches that are required by your implementation */

int TEMP;
int INT, INT_PRIO;
int PRIV, PRIO;
int USP;
int VECT_REG;

} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int CYCLE_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands.                   */
/*                                                             */
/***************************************************************/
void help() {                                                    
    printf("----------------LC-3bSIM Help-------------------------\n");
    printf("go               -  run program to completion       \n");
    printf("run n            -  execute program for n cycles    \n");
    printf("mdump low high   -  dump memory from low to high    \n");
    printf("rdump            -  dump the register & bus values  \n");
    printf("?                -  display this help menu          \n");
    printf("quit             -  exit the program                \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {                                                

  eval_micro_sequencer();   
  cycle_memory();
  eval_bus_drivers();
  drive_bus();
  latch_datapath_values();

   /*
        SIMULATING AN INTERRUPT TIMER:
  */
  if (CYCLE_COUNT == 300){
    NEXT_LATCHES.INTV = 0x01;
    NEXT_LATCHES.INT_PRIO = 0x01;
    NEXT_LATCHES.INT = 1;
  }

  CURRENT_LATCHES = NEXT_LATCHES;

  CYCLE_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles.                 */
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
/* Purpose   : Simulate the LC-3b until HALTed.                 */
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

    printf("\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	printf("  0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    printf("\n");

    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	fprintf(dumpsim_file, " 0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
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
    printf("Cycle Count  : %d\n", CYCLE_COUNT);
    printf("PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    printf("IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    printf("STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    printf("BUS          : 0x%0.4x\n", BUS);
    printf("MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    printf("MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	printf("%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file, "PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    fprintf(dumpsim_file, "STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    fprintf(dumpsim_file, "BUS          : 0x%0.4x\n", BUS);
    fprintf(dumpsim_file, "MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    fprintf(dumpsim_file, "MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	fprintf(dumpsim_file, "%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
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
/* Procedure : init_control_store                              */
/*                                                             */
/* Purpose   : Load microprogram into control store ROM        */ 
/*                                                             */
/***************************************************************/
void init_control_store(char *ucode_filename) {                 
    FILE *ucode;
    int i, j, index;
    char line[200];

    printf("Loading Control Store from file: %s\n", ucode_filename);

    /* Open the micro-code file. */
    if ((ucode = fopen(ucode_filename, "r")) == NULL) {
	printf("Error: Can't open micro-code file %s\n", ucode_filename);
	exit(-1);
    }

    /* Read a line for each row in the control store. */
    for(i = 0; i < CONTROL_STORE_ROWS; i++) {
	if (fscanf(ucode, "%[^\n]\n", line) == EOF) {
	    printf("Error: Too few lines (%d) in micro-code file: %s\n",
		   i, ucode_filename);
	    exit(-1);
	}

	/* Put in bits one at a time. */
	index = 0;

	for (j = 0; j < CONTROL_STORE_BITS; j++) {
	    /* Needs to find enough bits in line. */
	    if (line[index] == '\0') {
		printf("Error: Too few control bits in micro-code file: %s\nLine: %d\n",
		       ucode_filename, i);
		exit(-1);
	    }
	    if (line[index] != '0' && line[index] != '1') {
		printf("Error: Unknown value in micro-code file: %s\nLine: %d, Bit: %d\n",
		       ucode_filename, i, j);
		exit(-1);
	    }

	    /* Set the bit in the Control Store. */
	    CONTROL_STORE[i][j] = (line[index] == '0') ? 0:1;
	    index++;
	}

	/* Warn about extra bits in line. */
	if (line[index] != '\0')
	    printf("Warning: Extra bit(s) in control store file %s. Line: %d\n",
		   ucode_filename, i);
    }
    printf("\n");
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

/***************************************************************/
/*                                                             */
/* Procedure : initialize                                      */
/*                                                             */
/* Purpose   : Load microprogram and machine language program  */ 
/*             and set up initial state of the machine.        */
/*                                                             */
/***************************************************************/
void initialize(char *argv[], int num_prog_files) { 
    int i;
    init_control_store(argv[1]);

    init_memory();
    for ( i = 0; i < num_prog_files; i++ ) {
	load_program(argv[i + 2]);
    }
    CURRENT_LATCHES.Z = 1;
    CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);
    CURRENT_LATCHES.SSP = 0x3000; /* Initial value of system stack pointer */

    CURRENT_LATCHES.PRIV = 1; // Initialize to user mode with priority 0
    CURRENT_LATCHES.PRIO = 0;

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
    if (argc < 3) {
	printf("Error: usage: %s <micro_code_file> <program_file_1> <program_file_2> ...\n",
	       argv[0]);
	exit(1);
    }

    printf("LC-3b Simulator\n\n");

    initialize(argv, argc - 2);

    if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
	printf("Error: Can't open dumpsim file\n");
	exit(-1);
    }

    while (1)
	get_command(dumpsim_file);

}

/***************************************************************/
/* Do not modify the above code, except for the places indicated 
   with a "MODIFY:" comment.

   Do not modify the rdump and mdump functions.

   You are allowed to use the following global variables in your
   code. These are defined above.

   CONTROL_STORE
   MEMORY
   BUS

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

void setCC(){
  int val = sext(BUS, 16, 32);
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

int getOpcode(int IR){
    return ((IR & ((pows(2, 4)-1) << 12)) >> 12);
}

int memoryCount = 0;

void eval_micro_sequencer() {

  /* 
   * Evaluate the address of the next state according to the 
   * micro sequencer logic. Latch the next microinstruction.
   */
  int* currU = CURRENT_LATCHES.MICROINSTRUCTION;
  int nextJ;

  if (GetIRD(currU)){
    nextJ = getOpcode(NEXT_LATCHES.IR);
    // printf("IR: 0x%04X\n", CURRENT_LATCHES.IR);
    memcpy(NEXT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[nextJ], sizeof(int)*CONTROL_STORE_BITS);
  }
  else{
    switch (GetCOND(currU)){
        case 0: // 00
            nextJ = GetJ(currU);
            break;
        case 1: // 01
            nextJ = GetJ(currU) | (NEXT_LATCHES.READY << 1);
            break;
        case 2: // 10
            nextJ = GetJ(currU) | (NEXT_LATCHES.BEN << 2);
            break;
        case 3: // 11
            nextJ = GetJ(currU) | (getBit(NEXT_LATCHES.IR, 11));
            break;
        case 4: // User Privilege Mode
            nextJ = GetJ(currU) | (NEXT_LATCHES.PRIV << 3);
            break;
        case 5: // Interrupt Present Check
            if(NEXT_LATCHES.INT_PRIO > NEXT_LATCHES.PRIO ){
                nextJ = GetJ(currU) | (NEXT_LATCHES.INT << 4);
                NEXT_LATCHES.INT = 0;
            }
            else {
                nextJ = GetJ(currU);
            }
            break;
        default:
            break;
    }

    memcpy(NEXT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[nextJ], sizeof(int)*CONTROL_STORE_BITS);
  }
  NEXT_LATCHES.STATE_NUMBER = nextJ;
//   printf("Next state: %02d, Curr Ready: %d\n", nextJ, NEXT_LATCHES.READY);
}


void cycle_memory() {
 
  /* 
   * This function emulates memory and the WE logic. 
   * Keep track of which cycle of MEMEN we are dealing with.  
   * If fourth, we need to latch Ready bit at the end of 
   * cycle to prepare microsequencer for the fifth cycle.  
   */

  if(GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION)){
    if (++memoryCount == 4){ // 4th cycle
        NEXT_LATCHES.READY = 1;
    }
    else if (memoryCount == 5){ // 5th cycle
        NEXT_LATCHES.READY = 0;
        memoryCount = 0;

        // Read/write stuff with memory:
        if (GetR_W(CURRENT_LATCHES.MICROINSTRUCTION)){ // Write
            if (GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION)){ // Word
                writeWord(NEXT_LATCHES.MAR, NEXT_LATCHES.MDR);
            }
            else { // Byte
                writeByte(NEXT_LATCHES.MAR, NEXT_LATCHES.MDR & 0x00FF);
            }
        }
        else { // Read
            NEXT_LATCHES.MDR = getWord(NEXT_LATCHES.MAR); // No need to 0 the [0] since I'm getting the whole word anyways
        }
    }
  }
}

int BusNext;

void eval_bus_drivers() {

  /* 
   * Datapath routine emulating operations before driving the bus.
   * Evaluate the input of tristate drivers 
   *             Gate_MARMUX,
   *		 Gate_PC,
   *		 Gate_ALU,
   *		 Gate_SHF,
   *		 Gate_MDR.
   */    
  int* curr = CURRENT_LATCHES.MICROINSTRUCTION;

  if (GetGATE_ALU(curr)){ // SR1Out, SR2Out, SR1Mux, Sr1Mux, ALUK
    int OP1;
    int SR1;
    int OP2;
    int SR2;
    if (GetSR1MUX(curr) == 2){
        SR1 = 6;
    }
    else if (GetSR1MUX(curr)){
        SR1 = getReg(NEXT_LATCHES.IR, 6);
    }
    else{
        SR1 = getReg(NEXT_LATCHES.IR, 9);
    }
    OP1 = NEXT_LATCHES.REGS[SR1];
    if (getBit(NEXT_LATCHES.IR, 5)){ // imm5
        OP2 = sext(NEXT_LATCHES.IR & 0x1F, 5, 16);
    }
    else{ // SR2
        SR2 = getReg(NEXT_LATCHES.IR, 0);
        OP2 = NEXT_LATCHES.REGS[SR2];
    }
    switch (GetALUK(curr)){
        case 0: // Add
            BusNext = add16(OP1, OP2);
            break;
        case 1: // AND
            BusNext = OP1 & OP2;
            break;
        case 2: // XOR
            BusNext = OP1 ^ OP2;
            break;
        case 3:
            BusNext = OP1;
            break;
    }
  }
  if (GetGATE_MARMUX(curr)){ // SR1Out, SR1Mux, ADDR1Mux, LSHF1, ADDR2Mux, MARMUX
    if(GetMARMUX(curr)){ //Address Adder
        int OP1;
        int OP2;
        switch (GetADDR2MUX(curr)){
            case 0: 
                OP1 = 0;
                break;
            case 1:
                OP1 = sext(NEXT_LATCHES.IR & 0x3F, 6, 16);
                break;
            case 2:
                OP1 = sext(NEXT_LATCHES.IR & 0x1FF, 9, 16);
                break;
            case 3:
                OP1 = sext(NEXT_LATCHES.IR & 0x7FF, 11, 16);
                break;
        }
        if (GetLSHF1(curr)) OP1 = OP1 << 1;
        if (GetADDR1MUX(curr)){ // Get SR1
            int SR;
            if (GetSR1MUX(curr)){
                SR = getReg(NEXT_LATCHES.IR, 6);
            }
            else{
                SR = getReg(NEXT_LATCHES.IR, 9);
            }
            OP2 = NEXT_LATCHES.REGS[SR];
        }
        else { // Get PC
            OP2 = NEXT_LATCHES.PC;
        }
        BusNext = add16(OP1, OP2);
    }
    else{ // 7-0 Vector
        int mar = NEXT_LATCHES.IR & 0x00FF;
        mar = mar << 1;
        BusNext = mar;
    }
  }
  if (GetGATE_MDR(curr)){
    if (GetDATA_SIZE(curr)){ // Word
        BusNext = NEXT_LATCHES.MDR;
    }
    else{ // Byte
        if (getBit(NEXT_LATCHES.MAR, 0)){ // Upper byte
            BusNext = (NEXT_LATCHES.MDR & 0xFF00) >> 8;
        }
        else { // Lower Byte
            BusNext = NEXT_LATCHES.MDR & 0x00FF;
        }
        BusNext = sext(BusNext, 8, 16);
    }
  }
  if (GetGATE_PC(curr)){
    BusNext = NEXT_LATCHES.PC;
  }
  if (GetGATE_SHF(curr)){ // SR1Out, SR1Mux, IR[5:0]
    int SR;
    int OP;
    if (GetSR1MUX(curr)){
        SR = getReg(NEXT_LATCHES.IR, 6);
    }
    else{
        SR = getReg(NEXT_LATCHES.IR, 9);
    }
    OP = NEXT_LATCHES.REGS[SR];

    int shiftBits = (NEXT_LATCHES.IR & 0x30) >> 4;
    int shift = NEXT_LATCHES.IR & 0x0F;

    switch (shiftBits){
        case 0: // LSHF
            BusNext = (OP << shift) & 0xFFFF;
            break;
        case 1: // RSHFL
            BusNext = (OP >> shift) & 0xFFFF;
            break;
        case 3: // RSHFA
            BusNext = rshfa(OP, shift);
            break;
        default:
            break;
    }
  }
  if (GetGATE_TEMP(curr)){
    BusNext = NEXT_LATCHES.TEMP;
  }
  if (GetGATE_PSR(curr)){
    int psr = NEXT_LATCHES.P;
    psr += NEXT_LATCHES.Z << 1;
    psr += NEXT_LATCHES.N << 2;
    psr += NEXT_LATCHES.PRIO << 8;
    psr += NEXT_LATCHES.PRIV << 15;
    BusNext = psr;
  }
  if (GetGATE_SP(curr)){
    switch (GetSPMUX(curr)){
        case 0:
            BusNext = (NEXT_LATCHES.REGS[6] + 2) & 0xFFFF;
            break;
        case 1:
            BusNext = (NEXT_LATCHES.REGS[6] - 2) & 0xFFFF;
            break;
        case 2:
            BusNext = NEXT_LATCHES.SSP;
            break;
        case 3:
            BusNext = NEXT_LATCHES.USP;
            break;
        default:
            break;
    }
  }
  if (GetGATE_OLDPC(curr)){
    BusNext = (NEXT_LATCHES.PC - 2) & 0xFFFF;
  }
  if (GetGATE_VECT(curr)){
    BusNext = NEXT_LATCHES.VECT_REG;
  }
}


void drive_bus() {

  /* 
   * Datapath routine for driving the bus from one of the 5 possible 
   * tristate drivers. 
   */       

  BUS = BusNext;

}


void latch_datapath_values() {

  /* 
   * Datapath routine for computing all functions that need to latch
   * values in the data path at the end of this cycle.  Some values
   * require sourcing the bus; therefore, this routine has to come 
   * after drive_bus.
   */       

    int* curr = CURRENT_LATCHES.MICROINSTRUCTION;

    if (GetLD_USP(curr)){
        NEXT_LATCHES.USP = NEXT_LATCHES.REGS[6];
    }
    if (GetLD_SSP(curr)){
        NEXT_LATCHES.SSP = NEXT_LATCHES.REGS[6];
    }
    if (GetLD_MAR(curr)){
        NEXT_LATCHES.MAR = BUS;
    }
    if (GetLD_MDR(curr)){
        if (!GetMIO_EN(curr)){
            if(GetDATA_SIZE(curr)){ // Word
                NEXT_LATCHES.MDR = BUS;
            }
            else { // Byte
                NEXT_LATCHES.MDR = BUS & 0x00FF;
            }
        }
    }
    if (GetLD_IR(curr)){
        NEXT_LATCHES.IR = BUS;
    }
    if (GetLD_BEN(curr)){
        NEXT_LATCHES.BEN = getBit(NEXT_LATCHES.IR, 11) & NEXT_LATCHES.N;
        NEXT_LATCHES.BEN |= getBit(NEXT_LATCHES.IR, 10) & NEXT_LATCHES.Z;
        NEXT_LATCHES.BEN |= getBit(NEXT_LATCHES.IR, 9) & NEXT_LATCHES.P;
    }
    if (GetLD_REG(curr)){
        if (GetDRMUX(curr) == 2){
            NEXT_LATCHES.REGS[6] = BUS;
        }
        else if(GetDRMUX(curr)){ // R7
            NEXT_LATCHES.REGS[7] = BUS;
        }
        else { // IR 11.9
            NEXT_LATCHES.REGS[getReg(NEXT_LATCHES.IR, 9)] = BUS;
        }
    }
    if(GetLD_CC(curr)){
        if (GetPSRMUX(curr)){
            NEXT_LATCHES.N = (BUS & 4) >> 2;
            NEXT_LATCHES.Z = (BUS & 2) >> 1;
            NEXT_LATCHES.P = (BUS & 1);
        }
        else {
            setCC();
        }
    }
    if(GetLD_PC(curr)){
        switch(GetPCMUX(curr)){
            case 0: // PC + 2
                NEXT_LATCHES.PC += 2;
                break;
            case 1: // Bus
                NEXT_LATCHES.PC = BUS;
                break;
            case 2: {// Adder
                int OP1;
                int OP2;
                switch (GetADDR2MUX(curr)){
                    case 0: 
                        OP1 = 0;
                        break;
                    case 1:
                        OP1 = sext(NEXT_LATCHES.IR & 0x3F, 6, 16);
                        break;
                    case 2:
                        OP1 = sext(NEXT_LATCHES.IR & 0x1FF, 9, 16);
                        break;
                    case 3:
                        OP1 = sext(NEXT_LATCHES.IR & 0x7FF, 11, 16);
                        break;
                }
                if (GetLSHF1(curr)) OP1 = OP1 << 1;
                if (GetADDR1MUX(curr)){ // Get SR1
                    int SR;
                    if (GetSR1MUX(curr)){
                        SR = getReg(NEXT_LATCHES.IR, 6);
                    }
                    else{
                        SR = getReg(NEXT_LATCHES.IR, 9);
                    }
                    OP2 = NEXT_LATCHES.REGS[SR];
                }
                else { // Get PC
                    OP2 = NEXT_LATCHES.PC;
                }
                NEXT_LATCHES.PC = add16(OP1, OP2);
                break;
            }
            case 3:
                break;
        }
    }
    if (GetLD_TEMP(curr)){
        NEXT_LATCHES.TEMP = BUS;
    }
    if (GetLD_PRIV(curr)){
        if (GetPSRMUX(curr)){
            NEXT_LATCHES.PRIV = 0;
        }
        else{
            NEXT_LATCHES.PRIV = getBit(BUS, 15);
        }
    }
    if (GetLD_PRIO(curr)){
        if (GetPSRMUX(curr)){
            NEXT_LATCHES.PRIO = 0;
        }
        else{
            NEXT_LATCHES.PRIO = getBit(BUS, 15);
        }
    }
    if (GetLD_VECT(curr)){
        if (GetVECTMUX(curr)){
            NEXT_LATCHES.VECT_REG = 0x0200 + (NEXT_LATCHES.EXCV << 1);
            NEXT_LATCHES.INT_PRIO = 0x07;
        }
        else{
            NEXT_LATCHES.VECT_REG = 0x0200 + (NEXT_LATCHES.INTV << 1);
        }
    }
    if (GetLD_EXCV(curr)){
        if (GetEXCVMUX(curr) == 2){
            NEXT_LATCHES.EXCV = 0x04;
        }
        else if (GetEXCVMUX(curr)){
            NEXT_LATCHES.EXCV = 0x03;
        }
        else {
            NEXT_LATCHES.EXCV = 0x02;
        }
    }

    if (GetMEMCHECK(curr)){ // Check for any memory access errors
        if ((NEXT_LATCHES.STATE_NUMBER != 28 && NEXT_LATCHES.MAR >= 0 && NEXT_LATCHES.MAR <= 0x02FFF && NEXT_LATCHES.PRIV) || // Access Privilege Violation
            (NEXT_LATCHES.STATE_NUMBER == 28 && NEXT_LATCHES.MAR > 0x01FF && NEXT_LATCHES.MAR <= 0x02FFF && NEXT_LATCHES.PRIV)){ // Checking for the TRAP memory access state
            memcpy(NEXT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[47], sizeof(int)*CONTROL_STORE_BITS);
            NEXT_LATCHES.STATE_NUMBER = 47;
        }
        else if (GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION) && getBit(NEXT_LATCHES.MAR, 0)){ // Unaligned Memory Access
            memcpy(NEXT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[48], sizeof(int)*CONTROL_STORE_BITS);
            NEXT_LATCHES.STATE_NUMBER = 48;
        }
    }
}