#ifndef _SETJMP_H
#define _SETJMP_H

// x86_64 System V 콜리-세이브 컨텍스트:
//   [0]=rbx [1]=rbp [2]=r12 [3]=r13 [4]=r14 [5]=r15 [6]=rsp [7]=rip
// 시그널 마스크는 없는 환경이라 _setjmp 의미로만 동작한다.
typedef unsigned long jmp_buf[8];

int setjmp(jmp_buf env);
__attribute__((noreturn)) void longjmp(jmp_buf env, int val);

#endif
