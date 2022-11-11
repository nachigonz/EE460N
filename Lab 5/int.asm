.ORIG x1200
ADD R6, R6, x-2
STW R0, R6, x0
ADD R6, R6, x-2
STW R1, R6, x0
ADD R6, R6, x-2
STW R2, R6, x0

LEA R1, pt
LDW R1, R1, x0
LEA R2, entries
LDW R2, R2, x0

loop LDW R0, R1, x0
AND R0, R0, x-2
STW R0, R1, x0

ADD R1, R1, x2
ADD R2, R2, x-1
BRp loop

LDW R2, R6, x0
ADD R6, R6, x2
LDW R1, R6, x0
ADD R6, R6, x2
LDW R0, R6, x0
ADD R6, R6, x2

RTI
pt .FILL x1000
entries .FILL x0080
.END