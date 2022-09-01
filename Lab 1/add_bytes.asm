; Ignacio Gonzalez - ieg356

.ORIG x3000

LEA R0, BYTE1
LEA R1, BYTE2
LDW R0, R0, #0
LDW R1, R1, #0
LDB R0, R0, #0
LDB R1, R1, #0 ; R0 and R1 are loaded with byte1 and byte2

ADD R2, R0, R1 ; Sum of bytes
LEA R3, RES1
LDW R3, R3, #0
STB R2, R3, #0 ; Store lower byte of sum into res1

LEA R3, ONE 
LDW R3, R3, #0 ; Now R3 has -127
ADD R3, R3, R2 ; Sum - 127
BRp OVERFLOW
LEA R3, TWO
LDW R3, R3, #0 ; Now R3 has 128
ADD R3, R3, R2 ; Sum + 128
BRn OVERFLOW

AND R3, R3, #0 ; Put zero in R3
LEA R4, RES2
LDW R4, R4, #0 ; Put res2 in R4
STB R3, R3, #0 ; Store R3 to res2
HALT

OVERFLOW AND R3, R3, #1 ; Put a one in R3
LEA R4, RES2
LDW R4, R4, #0 ; Put res2 in R4
STB R3, R3, #0 ; Store R3 to res2
HALT

BYTE1 .FILL x3100
BYTE2 .FILL x3101
RES1 .FILL x3102
RES2 .FILL x3103
ONE .FILL #-127
TWO .FILL #128
.END