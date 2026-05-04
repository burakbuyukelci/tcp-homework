#include <stdio.h>

int main(void) {
    int cwnd = 1;
    int ssthresh = 16;
    int step = 0;

    printf("=== TCP Tahoe Simulation ===\n");
    printf("Step | cwnd | ssthresh | State        | Event\n");
    printf("-----+------+----------+--------------+----------\n");

    for (step = 1; step <= 20; step++) {
        if (step == 10) {
            ssthresh = cwnd / 2;
            if (ssthresh < 2) ssthresh = 2;
            cwnd = 1;
            printf("%4d | %4d | %8d | RESET        | TIMEOUT!\n",
                   step, cwnd, ssthresh);
            continue;
        }

        if (cwnd < ssthresh) {
            cwnd = cwnd + 1;
            printf("%4d | %4d | %8d | SLOW_START   | ACK received\n",
                   step, cwnd, ssthresh);
        } else {
            cwnd = cwnd + 1;
            printf("%4d | %4d | %8d | CONG_AVOID   | ACK received\n",
                   step, cwnd, ssthresh);
        }
    }

    return 0;
}