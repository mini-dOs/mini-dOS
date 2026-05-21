#include <stddef.h>
#include <stdlib.h>

int system(const char *command) {
    // mini-dOS에는 명령 처리기(셸 명령 실행기)가 없다.
    if (command == NULL)
        return 0;   // 명령 처리기 없음을 알림 (C 표준: 0 = 미가용)

    return -1;      // 명령 실행 미지원
}
