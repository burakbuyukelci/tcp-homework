#include <stdio.h>

/* TCP Tahoe sender state */
typedef struct {
    int cwnd;       /* Congestion window size (in MSS units) */
    int ssthresh;   /* Slow start threshold */
    int state;      /* 0 = SLOW_START, 1 = CONG_AVOIDANCE */
} TahoeState;

#define SLOW_START      0
#define CONG_AVOIDANCE  1

/* Print one row of the simulation table */
void print_row(int rtt, TahoeState *s, const char *event) {
    const char *state_name = (s->state == SLOW_START) ? "SLOW_START" : "CONG_AVOID";
    printf("%4d | %4d | %8d | %-12s | %s\n",
           rtt, s->cwnd, s->ssthresh, state_name, event);
}

/* Handle a normal RTT where all ACKs arrive successfully */
void on_rtt_success(TahoeState *s) {
    if (s->state == SLOW_START) {
        /* Each ACK adds 1 to cwnd. cwnd ACKs arrive in 1 RTT.
           So cwnd doubles. */
        s->cwnd = s->cwnd * 2;

        /* Did we reach the threshold? Switch to Congestion Avoidance. */
        if (s->cwnd >= s->ssthresh) {
            s->cwnd = s->ssthresh;
            s->state = CONG_AVOIDANCE;
        }
    } else {
        /* Congestion Avoidance: cwnd grows by 1 per RTT (linear). */
        s->cwnd = s->cwnd + 1;
    }
}

/* Tahoe's reaction to a TIMEOUT */
void on_timeout(TahoeState *s) {
    s->ssthresh = s->cwnd / 2;
    if (s->ssthresh < 2) s->ssthresh = 2;
    s->cwnd = 1;
    s->state = SLOW_START;
}

/* Tahoe's reaction to 3 DUPLICATE ACKs.
   In Tahoe, this is treated the same as timeout: panic, reset to 1. */
void on_triple_dup_ack(TahoeState *s) {
    s->ssthresh = s->cwnd / 2;
    if (s->ssthresh < 2) s->ssthresh = 2;
    s->cwnd = 1;
    s->state = SLOW_START;
}

int main(void) {
    TahoeState s;
    s.cwnd = 1;
    s.ssthresh = 16;
    s.state = SLOW_START;

    printf("=== TCP Tahoe Simulation (Adim 2) ===\n");
    printf("RTT  | cwnd | ssthresh | State        | Event\n");
    printf("-----+------+----------+--------------+----------\n");

    print_row(0, &s, "INITIAL");

    /* Run 15 RTTs with two failure events injected */
    for (int rtt = 1; rtt <= 15; rtt++) {
        if (rtt == 7) {
            /* At RTT 7, simulate a timeout */
            on_timeout(&s);
            print_row(rtt, &s, "TIMEOUT!");
        } else if (rtt == 12) {
            /* At RTT 12, simulate 3 duplicate ACKs */
            on_triple_dup_ack(&s);
            print_row(rtt, &s, "3 DUP ACK!");
        } else {
            /* Normal RTT — all ACKs received */
            on_rtt_success(&s);
            print_row(rtt, &s, "ACK received");
        }
    }

    return 0;
}