#include <stdio.h>

/* ===========================================================
   Algoritma türleri
   =========================================================== */
#define ALGO_TAHOE    0
#define ALGO_RENO     1
#define ALGO_NEWRENO  2

/* ===========================================================
   Sender state — üç algoritma için ortak
   =========================================================== */
typedef struct {
    int cwnd;
    int ssthresh;
    int state;
    int algo;
    int recover_point;   /* NewReno için: fast recovery'ye girerken kaydedilen "en yüksek gönderilen paket" */
    int high_pkt;        /* Şu ana kadar gönderdiğimiz en yüksek paket numarası (basit sayaç) */
} SenderState;

#define SLOW_START      0
#define CONG_AVOIDANCE  1
#define FAST_RECOVERY   2

/* ===========================================================
   Olay türleri
   =========================================================== */
#define EVT_ACK         0   /* Normal RTT, tüm ACK'ler geldi */
#define EVT_TIMEOUT     1
#define EVT_3DUPACK     2
#define EVT_PARTIAL_ACK 3   /* Fast recovery sırasında kısmi ACK (bir kayıp daha var demek) */

/* ===========================================================
   Yardımcılar
   =========================================================== */
const char* state_name(int state) {
    if (state == SLOW_START)     return "SLOW_START";
    if (state == CONG_AVOIDANCE) return "CONG_AVOID";
    if (state == FAST_RECOVERY)  return "FAST_RECOV";
    return "?";
}

const char* event_name(int evt) {
    if (evt == EVT_ACK)         return "ACK received";
    if (evt == EVT_TIMEOUT)     return "TIMEOUT!";
    if (evt == EVT_3DUPACK)     return "3 DUP ACK!";
    if (evt == EVT_PARTIAL_ACK) return "Partial ACK";
    return "?";
}

void print_row(int rtt, SenderState *s, const char *event) {
    printf("%4d | %4d | %8d | %-10s | %s\n",
           rtt, s->cwnd, s->ssthresh, state_name(s->state), event);
}

/* ===========================================================
   Olay işleyicileri
   =========================================================== */

void handle_ack(SenderState *s) {
    if (s->state == SLOW_START) {
        s->cwnd = s->cwnd * 2;
        if (s->cwnd >= s->ssthresh) {
            s->cwnd = s->ssthresh;
            s->state = CONG_AVOIDANCE;
        }
        s->high_pkt += s->cwnd;
    } else if (s->state == CONG_AVOIDANCE) {
        s->cwnd = s->cwnd + 1;
        s->high_pkt += s->cwnd;
    } else if (s->state == FAST_RECOVERY) {
        /* Tam ACK geldi, fast recovery bitti */
        s->cwnd = s->ssthresh;
        s->state = CONG_AVOIDANCE;
        s->high_pkt += s->cwnd;
    }
}

void handle_timeout(SenderState *s) {
    /* Üç algoritma da timeout'ta aynı davranır */
    s->ssthresh = s->cwnd / 2;
    if (s->ssthresh < 2) s->ssthresh = 2;
    s->cwnd = 1;
    s->state = SLOW_START;
}

void handle_3dupack(SenderState *s) {
    if (s->algo == ALGO_TAHOE) {
        /* Tahoe panikler */
        s->ssthresh = s->cwnd / 2;
        if (s->ssthresh < 2) s->ssthresh = 2;
        s->cwnd = 1;
        s->state = SLOW_START;
    } else {
        /* Reno ve NewReno: yarıya in, fast recovery */
        s->ssthresh = s->cwnd / 2;
        if (s->ssthresh < 2) s->ssthresh = 2;
        s->cwnd = s->ssthresh;
        s->state = FAST_RECOVERY;
        s->recover_point = s->high_pkt;  /* NewReno için işaret koy */
    }
}

void handle_partial_ack(SenderState *s) {
    if (s->algo == ALGO_NEWRENO) {
        /* NewReno: partial ACK = "bir kayıp daha var, ama fast recovery'de kal".
           cwnd'yi azaltma, ssthresh'i değiştirme. Sadece kayıp paketi tekrar yolla. */
        /* Simülasyonda: cwnd ve state aynı kalıyor */
        /* (gerçekte cwnd biraz inflate olur ama detayını atlıyoruz) */
    } else if (s->algo == ALGO_RENO) {
        /* Reno: partial ACK gibi bir konsept yok. 
           Bu olay Reno için "yeni bir 3-dup-ACK" gibi davranır. */
        s->ssthresh = s->cwnd / 2;
        if (s->ssthresh < 2) s->ssthresh = 2;
        s->cwnd = s->ssthresh;
        s->state = FAST_RECOVERY;
    } else {
        /* Tahoe: partial ACK = yine panik */
        s->ssthresh = s->cwnd / 2;
        if (s->ssthresh < 2) s->ssthresh = 2;
        s->cwnd = 1;
        s->state = SLOW_START;
    }
}

void apply_event(SenderState *s, int evt) {
    if (evt == EVT_ACK)         handle_ack(s);
    else if (evt == EVT_TIMEOUT)     handle_timeout(s);
    else if (evt == EVT_3DUPACK)     handle_3dupack(s);
    else if (evt == EVT_PARTIAL_ACK) handle_partial_ack(s);
}

/* ===========================================================
   Bir senaryoyu çalıştır
   =========================================================== */
void run_simulation(const char *algo_name, int algo, int *events, int num_events) {
    SenderState s;
    s.cwnd = 1;
    s.ssthresh = 16;
    s.state = SLOW_START;
    s.algo = algo;
    s.recover_point = 0;
    s.high_pkt = 0;

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
    /* Senaryo 1: Tek timeout + tek 3-dup-ACK
       (Tahoe ile Reno farkını gösterir, NewReno burada Reno'yla aynı) */
    int scenario_basic[15] = {
        EVT_ACK, EVT_ACK, EVT_ACK, EVT_ACK, EVT_ACK, EVT_ACK,
        EVT_TIMEOUT,
        EVT_ACK, EVT_ACK, EVT_ACK, EVT_ACK,
        EVT_3DUPACK,
        EVT_ACK, EVT_ACK, EVT_ACK
    };

    /* Senaryo 2: Aynı pencerede iki kayıp 
       (3-dup-ACK + hemen ardından partial ACK)
       Burada NewReno ile Reno arasında fark çıkar! */
    int scenario_multiloss[10] = {
        EVT_ACK, EVT_ACK, EVT_ACK, EVT_ACK, EVT_ACK,    /* 5 RTT normal */
        EVT_3DUPACK,                                     /* 1. kayıp */
        EVT_PARTIAL_ACK,                                 /* 2. kayıp (aynı pencerede) */
        EVT_ACK, EVT_ACK, EVT_ACK
    };

    printf("################################################\n");
    printf("# SENARYO 1: Tek timeout + Tek 3-dup-ACK       #\n");
    printf("# (Tahoe vs Reno farkini gosterir)             #\n");
    printf("################################################\n");

    run_simulation("Tahoe",   ALGO_TAHOE,   scenario_basic, 15);
    run_simulation("Reno",    ALGO_RENO,    scenario_basic, 15);
    run_simulation("NewReno", ALGO_NEWRENO, scenario_basic, 15);

    printf("\n");
    printf("################################################\n");
    printf("# SENARYO 2: Ayni pencerede iki kayip          #\n");
    printf("# (Reno vs NewReno farkini gosterir)           #\n");
    printf("################################################\n");

    run_simulation("Tahoe",   ALGO_TAHOE,   scenario_multiloss, 10);
    run_simulation("Reno",    ALGO_RENO,    scenario_multiloss, 10);
    run_simulation("NewReno", ALGO_NEWRENO, scenario_multiloss, 10);

    printf("\n=== Karsilastirma Notlari ===\n");
    printf("Senaryo 1, RTT 12: Tahoe cwnd=1, Reno=4, NewReno=4\n");
    printf("  -> Tek kayipta Reno ve NewReno ayni davranir.\n\n");
    printf("Senaryo 2, RTT 6 ve 7: Reno iki kez yariya iner, NewReno bir kez.\n");
    printf("  -> Birden fazla kayipta NewReno daha verimli.\n");

    return 0;
}