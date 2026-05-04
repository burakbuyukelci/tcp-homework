#include <stdio.h>

/* ===========================================================
   Algoritma türleri
   =========================================================== */
#define ALGO_TAHOE  0
#define ALGO_RENO   1

/* ===========================================================
   TCP Sender state — hem Tahoe hem Reno için kullanılacak
   =========================================================== */
typedef struct {
    int cwnd;       /* Congestion window (MSS units) */
    int ssthresh;   /* Slow start threshold */
    int state;      /* SLOW_START / CONG_AVOIDANCE / FAST_RECOVERY */
    int algo;       /* ALGO_TAHOE veya ALGO_RENO */
} SenderState;

#define SLOW_START      0
#define CONG_AVOIDANCE  1
#define FAST_RECOVERY   2

/* ===========================================================
   Olay türleri — bir senaryoyu olaylar dizisi olarak tanımlayacağız
   =========================================================== */
#define EVT_ACK         0   /* Normal RTT, tüm ACK'ler geldi */
#define EVT_TIMEOUT     1   /* Timeout oldu */
#define EVT_3DUPACK     2   /* 3 duplicate ACK */

/* ===========================================================
   Yardımcı: state ismini stringe çevir
   =========================================================== */
const char* state_name(int state) {
    if (state == SLOW_START)     return "SLOW_START";
    if (state == CONG_AVOIDANCE) return "CONG_AVOID";
    if (state == FAST_RECOVERY)  return "FAST_RECOV";
    return "?";
}

const char* event_name(int evt) {
    if (evt == EVT_ACK)     return "ACK received";
    if (evt == EVT_TIMEOUT) return "TIMEOUT!";
    if (evt == EVT_3DUPACK) return "3 DUP ACK!";
    return "?";
}

/* ===========================================================
   Tablo satırı yazdır
   =========================================================== */
void print_row(int rtt, SenderState *s, const char *event) {
    printf("%4d | %4d | %8d | %-10s | %s\n",
           rtt, s->cwnd, s->ssthresh, state_name(s->state), event);
}

/* ===========================================================
   Olayları işle — algoritma türüne göre farklı davranır
   =========================================================== */

/* Normal RTT — tüm ACK'ler başarıyla geldi */
void handle_ack(SenderState *s) {
    if (s->state == SLOW_START) {
        /* Slow Start: cwnd ikiye katlanır */
        s->cwnd = s->cwnd * 2;
        if (s->cwnd >= s->ssthresh) {
            s->cwnd = s->ssthresh;
            s->state = CONG_AVOIDANCE;
        }
    } else if (s->state == CONG_AVOIDANCE) {
        /* Linear growth */
        s->cwnd = s->cwnd + 1;
    } else if (s->state == FAST_RECOVERY) {
        /* Reno: yeni ACK gelince fast recovery'den çık, CA'ya geç */
        s->cwnd = s->ssthresh;
        s->state = CONG_AVOIDANCE;
    }
}

/* TIMEOUT — Tahoe ve Reno burada AYNI davranır */
void handle_timeout(SenderState *s) {
    s->ssthresh = s->cwnd / 2;
    if (s->ssthresh < 2) s->ssthresh = 2;
    s->cwnd = 1;
    s->state = SLOW_START;
}

/* 3 DUPLICATE ACK — burada algoritma farkı var! */
void handle_3dupack(SenderState *s) {
    if (s->algo == ALGO_TAHOE) {
        /* Tahoe panikler: timeout gibi davranır */
        s->ssthresh = s->cwnd / 2;
        if (s->ssthresh < 2) s->ssthresh = 2;
        s->cwnd = 1;
        s->state = SLOW_START;
    } else {
        /* Reno: panik yok, fast recovery */
        s->ssthresh = s->cwnd / 2;
        if (s->ssthresh < 2) s->ssthresh = 2;
        s->cwnd = s->ssthresh;
        s->state = FAST_RECOVERY;
    }
}

/* Bir olayı state'e uygula */
void apply_event(SenderState *s, int evt) {
    if (evt == EVT_ACK)        handle_ack(s);
    else if (evt == EVT_TIMEOUT) handle_timeout(s);
    else if (evt == EVT_3DUPACK) handle_3dupack(s);
}

/* ===========================================================
   Senaryo çalıştır — bir olaylar dizisini sırayla uygula
   =========================================================== */
void run_simulation(const char *algo_name, int algo, int *events, int num_events) {
    SenderState s;
    s.cwnd = 1;
    s.ssthresh = 16;
    s.state = SLOW_START;
    s.algo = algo;

    printf("\n=== TCP %s Simulation ===\n", algo_name);
    printf("RTT  | cwnd | ssthresh | State      | Event\n");
    printf("-----+------+----------+------------+----------\n");
    print_row(0, &s, "INITIAL");

    for (int i = 0; i < num_events; i++) {
        apply_event(&s, events[i]);
        print_row(i + 1, &s, event_name(events[i]));
    }
}

/* ===========================================================
   MAIN
   =========================================================== */
int main(void) {
    /* Test senaryosu: 15 RTT
       - RTT 7'de timeout
       - RTT 12'de 3 duplicate ACK
       - Diğerlerinde normal ACK */
    int scenario[15] = {
        EVT_ACK, EVT_ACK, EVT_ACK, EVT_ACK, EVT_ACK, EVT_ACK,    /* 1-6 */
        EVT_TIMEOUT,                                              /* 7 */
        EVT_ACK, EVT_ACK, EVT_ACK, EVT_ACK,                       /* 8-11 */
        EVT_3DUPACK,                                              /* 12 */
        EVT_ACK, EVT_ACK, EVT_ACK                                 /* 13-15 */
    };

    /* Önce Tahoe'yu çalıştır */
    run_simulation("Tahoe", ALGO_TAHOE, scenario, 15);

    /* Sonra aynı senaryoyu Reno ile çalıştır */
    run_simulation("Reno", ALGO_RENO, scenario, 15);

    printf("\n=== Karsilastirma Notu ===\n");
    printf("RTT 7 (TIMEOUT): Her iki algoritma da cwnd'yi 1'e dusurdu.\n");
    printf("RTT 12 (3 DUP ACK): Tahoe cwnd=1'e dustu, Reno yariya dustu (Fast Recovery).\n");

    return 0;
}