#include <stdio.h> /* standard input/output library */
#include <stdlib.h> /* Standard C Library */
#include <string.h> /* String operations library */
#include <ctype.h> /* Library for useful character operations */
#include <limits.h> /* Library for definitions of common variable type characteristics*/

/*
	Name: Ignacio Gonzalez
	UTEID: ieg356
*/

FILE* infile = NULL;
FILE* outfile = NULL;

int toNum( char * pStr )
{
   char * t_ptr;
   char * orig_pStr;
   int t_length,k;
   int lNum, lNeg = 0;
   long int lNumLong;

   orig_pStr = pStr;
   if( *pStr == '#' )				/* decimal */
   {
     pStr++;
     if( *pStr == '-' )				/* dec is negative */
     {
       lNeg = 1;
       pStr++;
     }
     t_ptr = pStr;
     t_length = strlen(t_ptr);
     for(k=0;k < t_length;k++)
     {
       if (!isdigit(*t_ptr))
       {
	 printf("Error: invalid decimal operand, %s\n",orig_pStr);
	 exit(4);
       }
       t_ptr++;
     }
     lNum = atoi(pStr);
     if (lNeg)
       lNum = -lNum;

     return lNum;
   }
   else if( *pStr == 'x' || *pStr == 'X')	/* hex     */
   {
     pStr++;
     if( *pStr == '-' )				/* hex is negative */
     {
       lNeg = 1;
       pStr++;
     }
     t_ptr = pStr;
     t_length = strlen(t_ptr);
     for(k=0;k < t_length;k++)
     {
       if (!isxdigit(*t_ptr))
       {
	 printf("Error: invalid hex operand, %s\n",orig_pStr);
	 exit(4);
       }
       t_ptr++;
     }
     lNumLong = strtol(pStr, NULL, 16);    /* convert hex string into integer */
     lNum = (lNumLong > INT_MAX)? INT_MAX : lNumLong;
     if( lNeg )
       lNum = -lNum;
     return lNum;
   }
   else
   {
	printf( "Error: invalid operand, %s\n", orig_pStr);
	exit(4);  /* This has been changed from error code 3 to error code 4, see clarification 12 */
   }
}

int regToNum( char * pStr){
    char regString[4];
    strcpy(regString, pStr);
    *regString = '#';
    return toNum(regString);
}

#define MAX_LINE_LENGTH 255
enum
{
    DONE, OK, EMPTY_LINE
};

#define MAX_LABEL_LEN 20
#define MAX_SYMBOLS 255
typedef struct {
    int address;
    char label[MAX_LABEL_LEN + 1];	/* Question for the reader: Why do we need to add 1? */
} TableEntry;
TableEntry symbolTable[MAX_SYMBOLS];
int symbols = 0;

int origAddress;
int pc;

const char opcodes [28][6]= {
        "add", "and", "br", "brz", "brp", "brnp", "brn", "brnz", "brzp",
        "brnzp", "halt", "jmp", "jsr", "jsrr", "ldb", "ldw",
        "lea", "nop", "not", "ret", "lshf", "rshfl", "rshfa", "rti", "stb", "stw", "trap", "xor"
};

int isOpcode (char * opcode) {
    for (int i = 0; i < 28; i++)
        if (strcmp(opcodes[i], opcode) == 0)
            return i;
    //printf("%s is not an opcode\n", opcode);
    return -1;
}

int readAndParse( FILE * pInfile, char * pLine, char ** pLabel, char
** pOpcode, char ** pArg1, char ** pArg2, char ** pArg3, char ** pArg4
)
{
    char * lRet, * lPtr;
    int i;
    if( !fgets( pLine, MAX_LINE_LENGTH, pInfile ) )
        return( DONE );
    for( i = 0; i < strlen( pLine ); i++ )
        pLine[i] = tolower( pLine[i] );

    /* convert entire line to lowercase */
    *pLabel = *pOpcode = *pArg1 = *pArg2 = *pArg3 = *pArg4 = pLine + strlen(pLine);

    /* ignore the comments */
    lPtr = pLine;

    while( *lPtr != ';' && *lPtr != '\0' &&
           *lPtr != '\n' )
        lPtr++;

    *lPtr = '\0';
    if( !(lPtr = strtok( pLine, "\t\n ," ) ) )
        return( EMPTY_LINE );

    if( isOpcode( lPtr ) == -1 && lPtr[0] != '.' ) /* found a label */
    {
        *pLabel = lPtr;
        if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );
    }

    *pOpcode = lPtr;

    if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

    *pArg1 = lPtr;

    if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

    *pArg2 = lPtr;
    if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

    *pArg3 = lPtr;

    if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

    *pArg4 = lPtr;

    return( OK );
}
/* Note: MAX_LINE_LENGTH, OK, EMPTY_LINE, and DONE are defined values */

int validLabel (char * label){
    if (strlen(label) > 20 || strlen(label) < 1)
        return -1;

    if (0 == isalpha((int) *label))
        return -1;

    return 0;
}

void firstPass()
{
    char lLine[MAX_LINE_LENGTH + 1], *lLabel, *lOpcode, *lArg1,
            *lArg2, *lArg3, *lArg4;

    int lRet;

    do
    {
        lRet = readAndParse( infile, lLine, &lLabel,
                             &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4 );
        if( lRet != DONE && lRet != EMPTY_LINE )
        {
            //printf("%s ! %s : %s %s %s %s\n", lLabel, lOpcode, lArg1, lArg2, lArg3, lArg4);
            if (validLabel(lLabel) == 0){
                symbolTable[symbols].address = pc;
                strcpy(symbolTable[symbols].label, lLabel);
                symbols++;
            }
            if (strcmp(lOpcode, ".orig") == 0){
                origAddress = toNum(lArg1);
                pc = origAddress;
            }
            else if (strcmp(lOpcode, ".end") == 0){
                lRet = DONE;
            }
            else
                pc += 2;
        }
    } while( lRet != DONE );

//    for (int i = 0; i < symbols; i++){
//        printf("%s : %d\n", symbolTable[i].label, symbolTable[i].address);
//    }
}

int offsetGet(char * arg){
    int offset;

    int address;
    if (isalpha(*arg)!=0)
        for (int i = 0; i < symbols; i++){
            if (strcmp(arg, symbolTable[i].label)==0)
                address=symbolTable[i].address;
        }
    else {
        address = pc + (toNum(arg) << 1);
    }
    offset = (address - (pc)) >> 1;

    //printf("%X %X %X\n", address, pc, offset);
    return offset + 1;
}

int createCode(char * label, char
* opcode, char * arg1, char * arg2, char * arg3, char * arg4){
    int machineCode = 0;

    if (strcmp(opcode, "add")==0){
        machineCode = 0b0001000000000000;

        int dr = regToNum(arg1)<<9;
        int sr1 = regToNum(arg2)<<6;
        int sr2;
        if(*arg3 == 'r')
            sr2 = regToNum(arg3);
        else {
            sr2 = (0b100000) + (toNum(arg3) & 0b11111); // Masking leading 1s from possible neg numbers
        }
        machineCode += dr + sr1 + sr2;
    }

    if (strcmp(opcode, "and")==0){
        machineCode = 0b0101000000000000;

        int dr = regToNum(arg1)<<9;
        int sr1 = regToNum(arg2)<<6;
        int sr2;
        if(*arg3 == 'r')
            sr2 = regToNum(arg3);
        else {
            sr2 = (0b100000) + (toNum(arg3) & 0b11111); // Masking leading 1s from possible neg numbers
        }
        machineCode += dr + sr1 + sr2;
    }

    if (*opcode == 'b' && *(opcode+1) == 'r'){ // Check if it's a number first
        int i = 2;
        int nzp;
        if (i == strlen(opcode))
            nzp = 7;
        else
            while(i<strlen(opcode)){
                if (*(opcode+i) == 'n')
                    nzp += 4;
                else if (*(opcode+i) == 'z')
                    nzp += 2;
                else if (*(opcode+i) == 'p')
                    nzp += 1;
                i++;
            }

        int offset = offsetGet(arg1);
        machineCode = (offset & 0b111111111) + (nzp << 9);
    }

    if (strcmp(opcode, "halt")==0 ){
        machineCode = 0xF025;
    }

    if (strcmp(opcode, "jmp")==0){
        machineCode = (0b1100 << 12) + (regToNum(arg1) << 6);
    }

    if (strcmp(opcode, "jsr")==0){
        machineCode = (0b01001 << 11);

        int offset = offsetGet(arg1);
        machineCode += (offset & 0b11111111111);
    }

    if (strcmp(opcode, "jsrr")==0){
        machineCode = (0b0100 << 12) + (regToNum(arg1) << 6);
    }

    if (strcmp(opcode, "ldb")==0){
        machineCode = (0b0010 << 12) + (regToNum(arg1) << 9) + (regToNum(arg2) << 6);
        machineCode += 0b111111 & toNum(arg3);
    }

    if (strcmp(opcode, "ldw")==0){
        machineCode = (0b0110 << 12) + (regToNum(arg1) << 9) + (regToNum(arg2) << 6);
        machineCode += 0b111111 & toNum(arg3);
    }

    if (strcmp(opcode, "lea")==0){
        machineCode = (0b1110 << 12) + (regToNum(arg1) << 9);

        int offset = offsetGet(arg2);
        machineCode += (offset & 0b111111111);
    }

    if (strcmp(opcode, "nop")==0){
        machineCode = 0x0000;
    }

    if (strcmp(opcode, "not")==0){
        machineCode = (0b1001 << 12) + (regToNum(arg1) << 9) + (regToNum(arg2) << 6) + 0b111111;
    }

    if (strcmp(opcode, "ret")==0){
        machineCode = 0b1100000111000000;
    }

    if (strcmp(opcode, "lshf")==0){
        machineCode = (0b1101 << 12) + (regToNum(arg1) << 9) + (regToNum(arg2) << 6) + (0b1111 & toNum(arg3));
    }

    if (strcmp(opcode, "rshfl")==0){
        machineCode = (0b1101 << 12) + (regToNum(arg1) << 9) + (regToNum(arg2) << 6) + (0b1111 & toNum(arg3)) + 0b010000;
    }

    if (strcmp(opcode, "rshfa")==0){
        machineCode = (0b1101 << 12) + (regToNum(arg1) << 9) + (regToNum(arg2) << 6) + (0b1111 & toNum(arg3)) + 0b110000;
    }

    if (strcmp(opcode, "rti")==0){
        machineCode = (0b1000 << 12);
    }

    if (strcmp(opcode, "stb")==0){
        machineCode = (0b0011 << 12) + (regToNum(arg1) << 9) + (regToNum(arg2) << 6) + (0b111111 & toNum(arg3));
    }

    if (strcmp(opcode, "stw")==0){
        machineCode = (0b0111 << 12) + (regToNum(arg1) << 9) + (regToNum(arg2) << 6) + (0b111111 & toNum(arg3));
    }

    if (strcmp(opcode, "trap")==0){
        machineCode = (0b11110000 << 8) + (0b11111111 & toNum(arg1));
    }

    if (strcmp(opcode, "xor")==0){
        machineCode = (0b1001 << 12);

        int dr = regToNum(arg1)<<9;
        int sr1 = regToNum(arg2)<<6;
        int sr2;
        if(*arg3 == 'r')
            sr2 = regToNum(arg3);
        else {
            sr2 = (0b100000) + (toNum(arg3) & 0b11111); // Masking leading 1s from possible neg numbers
        }
        machineCode += dr + sr1 + sr2;
    }

    return machineCode;
}

void secondPass(){
    char lLine[MAX_LINE_LENGTH + 1], *lLabel, *lOpcode, *lArg1,
            *lArg2, *lArg3, *lArg4;

    int lRet;
    pc = origAddress;

    do
    {
        lRet = readAndParse( infile, lLine, &lLabel,
                             &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4 );
        if( lRet != DONE && lRet != EMPTY_LINE )
        {
            //printf("%s ! %s : %s %s %s %s\n", lLabel, lOpcode, lArg1, lArg2, lArg3, lArg4);
            //printf("%s\n", lOpcode);

            pc+=2;
            if (strcmp(lOpcode, ".end") == 0){
                lRet = DONE;
            }
            else if (strcmp(lOpcode, ".orig")==0)
                fprintf(outfile, "0x%04X\n", toNum(lArg1) & 0xFFFF);
            else if (strcmp(lOpcode, ".fill")==0)
                fprintf(outfile, "0x%04X\n", toNum(lArg1) & 0xFFFF);
            else {
                int machineCode = createCode(lLabel, lOpcode, lArg1, lArg2, lArg3, lArg4);
                fprintf(outfile, "0x%04X\n", machineCode & 0xFFFF);
            }
        }
    } while( lRet != DONE );
}

int main(int argc, char* argv[]) {
    char *prgName   = NULL;
    char *iFileName = NULL;
    char *oFileName = NULL;

    prgName   = argv[0];
    iFileName = argv[1];
    oFileName = argv[2];

    if (iFileName == NULL){
        printf("Please pass an input filename!\n");
        exit(0);
    }
    else if (oFileName == NULL){
        printf("Please pass an output filename!\n");
        exit(0);
    }

    printf("program name = '%s'\n", prgName);
    printf("input file name = '%s'\n", iFileName);
    printf("output file name = '%s'\n", oFileName);

    /* open the source file */
    infile = fopen(iFileName, "r");
    outfile = fopen(oFileName, "w");

    if (!infile) {
        printf("Error: Cannot open file '%s'\n", iFileName);
        exit(4);
    }
    if (!outfile) {
        printf("Error: Cannot open file '%s'\n", oFileName);
        exit(4);
    }

    /* Do stuff with files */

    firstPass();

    rewind(infile);
    secondPass();

    fclose(infile);
    fclose(outfile);

    return 0;
}
