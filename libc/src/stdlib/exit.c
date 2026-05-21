#include <stdint.h>
#include <stdlib.h>
#include <asm/cpu.h>
#include <drivers/serial.h>

__attribute__((noreturn))
void exit(int status) {
    // 표준 C exit은 stdio 스트림 flush + atexit 핸들러 실행도 수행한다.
    // mini-dOS는 stdio 미구현이고 DOOM은 자체 exit_funcs를 돌리므로 현재는 생략.
    // stdio 구현 시 이 자리에 fflush 훅을 추가할 것.

    // 호스트(셸)가 보는 종료 코드 관례: 하위 8비트만 유효
    serial_write("[exit] status=");
    serial_write_dec((uint64_t)((unsigned int)status & 0xFFu));
    serial_write("\r\n");

    // 프로세스 모델이 없으므로 복귀할 곳이 없다 → 인터럽트 끄고 정지
    cli();
    for (;;)
        hlt();
}
