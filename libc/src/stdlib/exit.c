#include <stdint.h>
#include <stdlib.h>
#include <setjmp.h>
#include <asm/cpu.h>
#include <drivers/serial.h>

// exit() 복귀 지점(선택). 설정돼 있으면 halt 대신 이 지점으로 longjmp 한다.
// DOOM 실행 중에는 glue가 run_iwad의 setjmp 버퍼로 무장(arm)시켜, DOOM이 exit()를
// 호출해도 멈추지 않고 셸로 돌아오게 한다. NULL이면 종래대로 정지.
static jmp_buf *s_exit_jmp = (void *)0;

void libc_set_exit_jmp(jmp_buf *env) {
    s_exit_jmp = env;
}

__attribute__((noreturn))
void exit(int status) {
    // 표준 C exit은 stdio 스트림 flush + atexit 핸들러 실행도 수행한다.
    // mini-dOS는 stdio 미구현이고 DOOM은 자체 exit_funcs를 돌리므로 현재는 생략.
    // stdio 구현 시 이 자리에 fflush 훅을 추가할 것.

    // 호스트(셸)가 보는 종료 코드 관례: 하위 8비트만 유효
    serial_write("[exit] status=");
    serial_write_dec((uint64_t)((unsigned int)status & 0xFFu));
    serial_write("\r\n");

    // 복귀 지점이 설정돼 있으면 그쪽으로 점프(셸 복귀). 그렇지 않으면 정지.
    if (s_exit_jmp)
        longjmp(*s_exit_jmp, 1);

    // 프로세스 모델이 없으므로 복귀할 곳이 없다 → 인터럽트 끄고 정지
    cli();
    for (;;)
        hlt();
}
