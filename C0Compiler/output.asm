.486
.model flat,stdcall
option casemap: none

include \masm32\include\masm32.inc
include \masm32\include\kernel32.inc
include \masm32\macros\macros.asm
include \masm32\include\msvcrt.inc

includelib \masm32\lib\msvcrt.lib
includelib \masm32\lib\masm32.lib
includelib \masm32\lib\kernel32.lib

printf proto C:dword,:vararg
scanf proto C:dword,:vararg

.data
_intin db "%d",0
_charin db "%c",0
_int db "%d",13,10,0
_char db "%c",13,10,0
_str db "%s",13,10,0
__base dword 0
__STR0 db "true ",0
__STR1 db "false ",0
__STR2 db "true ",0
__STR3 db "false ",0
__STR4 db "function1: ",0
__STR5 db "function2: ",0
.code
start:
main proc
PUSH EBP
MOV EBP, ESP
CALL @__main
invoke ExitProcess,0
@__function1:
PUSH EBP
MOV EBP, ESP
SUB ESP, 4
SUB ESP, 24
MOV DWORD PTR [EBP+-4], 5
MOV DWORD PTR [EBP+-8], 5
MOV DWORD PTR [EBP+-24], 1
MOV EAX, [EBP+16]
CMP EAX, [EBP+-24]
JNE __lbl2
SUB ESP, 4
MOV DWORD PTR [EBP+-28], 1
MOV EAX, [EBP+12]
CMP EAX, [EBP+-28]
JNE __lbl1
SUB ESP, 4
MOV DWORD PTR [EBP+-32], 97
MOV EAX, [EBP+8]
CMP EAX, [EBP+-32]
JNE __lbl0
INVOKE printf, offset _str, offset __STR0
SUB ESP, 4
MOV DWORD PTR [EBP+-36], 1
MOV EAX, [EBP+-36]
MOV ESP, EBP
POP EBP
RET
__lbl0:
__lbl1:
__lbl2:
INVOKE printf, offset _str, offset __STR1
SUB ESP, 4
MOV DWORD PTR [EBP+-40], 0
MOV EAX, [EBP+-40]
MOV ESP, EBP
POP EBP
RET
MOV ESP, EBP
POP EBP
RET
@__function2:
PUSH EBP
MOV EBP, ESP
SUB ESP, 4
SUB ESP, 4
MOV DWORD PTR [EBP+-4], 1
MOV EAX, [EBP+16]
CMP EAX, [EBP+-4]
JNE __lbl5
SUB ESP, 4
MOV DWORD PTR [EBP+-8], 1
MOV EAX, [EBP+12]
CMP EAX, [EBP+-8]
JNE __lbl4
SUB ESP, 4
MOV DWORD PTR [EBP+-12], 97
MOV EAX, [EBP+8]
CMP EAX, [EBP+-12]
JNE __lbl3
INVOKE printf, offset _str, offset __STR2
SUB ESP, 4
MOV DWORD PTR [EBP+-16], 1
MOV EAX, [EBP+-16]
MOV ESP, EBP
POP EBP
RET
__lbl3:
__lbl4:
__lbl5:
INVOKE printf, offset _str, offset __STR3
SUB ESP, 4
MOV DWORD PTR [EBP+-20], 0
MOV EAX, [EBP+-20]
MOV ESP, EBP
POP EBP
RET
MOV ESP, EBP
POP EBP
RET
@__main:
PUSH EBP
MOV EBP, ESP
SUB ESP, 4
INVOKE printf, offset _str, offset __STR4
SUB ESP, 16
MOV DWORD PTR [EBP+-4], 1
MOV DWORD PTR [EBP+-8], 1
MOV DWORD PTR [EBP+-12], 97
PUSH [EBP+-4]
PUSH [EBP+-8]
PUSH [EBP+-12]
CALL @__function1
MOV [EBP+-16], EAX
INVOKE printf, offset _int, DWORD PTR [EBP+-16]
INVOKE printf, offset _str, offset __STR5
SUB ESP, 16
MOV DWORD PTR [EBP+-20], 1
MOV DWORD PTR [EBP+-24], 2
MOV DWORD PTR [EBP+-28], 98
PUSH [EBP+-20]
PUSH [EBP+-24]
PUSH [EBP+-28]
CALL @__function2
MOV [EBP+-32], EAX
INVOKE printf, offset _int, DWORD PTR [EBP+-32]
MOV ESP, EBP
POP EBP
RET
main endp
end start