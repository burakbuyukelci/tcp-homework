/*
 * CSE 320 - Computer Networks
 * Programming Assignment: TCP Congestion Control Algorithms
 *
 * Single-file simulator combining:
 *   - 6-node network topology with Dijkstra routing
 *   - Hop-by-hop message forwarding
 *   - TCP Tahoe / Reno / NewReno congestion control simulation
 *
 * Build:  gcc tcp_sim.c -o tcp_sim.exe
 * Run:    tcp_sim.exe
 */

#include <stdio.h>
#include <string.h>

/* ============================================================
   PART 1 - NETWORK TOPOLOGY
   ============================================================ */

#define N        6
#define INF      9999

const char* node_name[N] = { "A", "B", "C", "D", "E", "F" };
int         node_port[N] = { 5001, 5002, 5003, 5004, 5005, 5006 };

/* Adjacency matrix - link costs.
   INF means there is no direct link.
   Designed so that node A's routing table matches the assignment PDF. */
int graph[N][N] = {
    /*       A     B     C     D     E     F   */
    /* A */ {  0,    4,    7,   13,  INF,    5 },
    /* B */ {  4,    0,  INF,    8,    3,  INF },
    /* C */ {  7,  INF,    0,  INF,   12,  INF },
    /* D */ { 13,    8,  INF,    0,  INF,  INF },
    /* E */ {INF,    3,   12,  INF,    0,  INF },
    /* F */ {  5,  INF,  INF,  INF,  INF,    0 }
};

/* ============================================================
   PART 2 - DIJKSTRA SHORTEST-PATH ALGORITHM
   ============================================================ */

void dijkstra(int src, int dist[N], int prev[N]) {
    int visited[N];
    for (int i = 0; i < N; i++) {
        dist[i] = INF;
        prev[i] = -1;
        visited[i] = 0;
    }
    dist[src] = 0;

    for (int count = 0; count < N; count++) {
        int u = -1;
        int min_dist = INF;
        for (int i = 0; i < N; i++) {
            if (!visited[i] && dist[i] < min_dist) {
                min_dist = dist[i];
                u = i;
            }
        }
        if (u == -1) break;
        visited[u] = 1;

        for (int v = 0; v < N; v++) {
            if (!visited[v] && graph[u][v] != INF) {
                int new_dist = dist[u] + graph[u][v];
                if (new_dist < dist[v]) {
                    dist[v] = new_dist;
                    prev[v] = u;
                }
            }
        }
    }
}

void print_path(int src, int dst, int prev[N]) {
    if (dst == src) {
        printf("%s", node_name[src]);
        return;
    }
    if (prev[dst] == -1) {
        printf("(no path)");
        return;
    }
    print_path(src, prev[dst], prev);
    printf(" -> %s", node_name[dst]);
}

int find_next_hop(int src, int dst, int prev[N]) {
    if (src == dst) return -1;
    int curr = dst;
    while (prev[curr] != src && prev[curr] != -1) {
        curr = prev[curr];
    }
    if (prev[curr] == -1) return -1;
    return curr;
}

/* ============================================================
   PART 3 - ROUTING TABLE PRINTING
   ============================================================ */

void print_routing_table(int src) {
    int dist[N];
    int prev[N];
    dijkstra(src, dist, prev);

    printf("\nRouting table for node %s\n", node_name[src]);
    printf("Destination | Next Hop | Cost | Path\n");
    printf("------------+----------+------+--------------------\n");
    for (int dst = 0; dst < N; dst++) {
        if (dst == src) {
            printf("%-11s | %-8s | %-4d | %s\n",
                   node_name[dst], "-", 0, node_name[dst]);
        } else if (dist[dst] >= INF) {
            printf("%-11s | %-8s | %-4s | %s\n",
                   node_name[dst], "-", "INF", "(unreachable)");
        } else {
            int nh = find_next_hop(src, dst, prev);
            printf("%-11s | %-8s | %-4d | ",
                   node_name[dst],
                   (nh >= 0 ? node_name[nh] : "-"),
                   dist[dst]);
            print_path(src, dst, prev);
            printf("\n");
        }
    }
}

void print_topology(void) {
    printf("\n============================================\n");
    printf("  Network Topology (6 nodes)\n");
    printf("============================================\n");

    printf("\nLink costs (adjacency matrix):\n");
    printf("       ");
    for (int i = 0; i < N; i++) printf("%5s ", node_name[i]);
    printf("\n");
    for (int i = 0; i < N; i++) {
        printf("  %s  : ", node_name[i]);
        for (int j = 0; j < N; j++) {
            if (graph[i][j] == INF) printf("  INF ");
            else printf(" %4d ", graph[i][j]);
        }
        printf("\n");
    }

    printf("\nNode ports:\n");
    for (int i = 0; i < N; i++) {
        printf("  %s = %d\n", node_name[i], node_port[i]);
    }
}

/* ============================================================
   PART 4 - MESSAGE FORWARDING SIMULATION
   ============================================================ */

void send_message(int src, int dst, const char *message) {
    printf("\n>> Sending message from %s to %s: \"%s\"\n",
           node_name[src], node_name[dst], message);

    if (src == dst) {
        printf("[%s] Source and destination are the same.\n", node_name[src]);
        return;
    }

    int dist[N], prev[N];
    dijkstra(src, dist, prev);

    if (dist[dst] >= INF) {
        printf("[%s] Destination %s unreachable.\n",
               node_name[src], node_name[dst]);
        return;
    }

    int next_hop = find_next_hop(src, dst, prev);
    printf("[%s] Destination %s, next hop %s\n",
           node_name[src], node_name[dst], node_name[next_hop]);

    int current = next_hop;
    while (current != dst) {
        int d2[N], p2[N];
        dijkstra(current, d2, p2);
        int nh = find_next_hop(current, dst, p2);
        if (nh == -1) {
            printf("[%s] Routing failed.\n", node_name[current]);
            return;
        }
        printf("[%s] Forwarding message from %s to %s, next hop %s\n",
               node_name[current], node_name[src], node_name[dst],
               node_name[nh]);
        current = nh;
    }

    printf("[%s] Received message from %s: %s\n",
           node_name[dst], node_name[src], message);
}

/* ============================================================
   PART 5 - TCP CONGESTION CONTROL
   ============================================================ */

#define ALGO_TAHOE    0
#define ALGO_RENO     1
#define ALGO_NEWRENO  2

#define SLOW_START      0
#define CONG_AVOIDANCE  1
#define FAST_RECOVERY   2

#define EVT_ACK         0
#define EVT_TIMEOUT     1
#define EVT_3DUPACK     2
#define EVT_PARTIAL_ACK 3

typedef struct {
    int cwnd;
    int ssthresh;
    int state;
    int algo;
    int recover_point;
    int high_pkt;
} SenderState;

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
        s->cwnd = s->ssthresh;
        s->state = CONG_AVOIDANCE;
        s->high_pkt += s->cwnd;
    }
}

void handle_timeout(SenderState *s) {
    s->ssthresh = s->cwnd / 2;
    if (s->ssthresh < 2) s->ssthresh = 2;
    s->cwnd = 1;
    s->state = SLOW_START;
}

void handle_3dupack(SenderState *s) {
    if (s->algo == ALGO_TAHOE) {
        s->ssthresh = s->cwnd / 2;
        if (s->ssthresh < 2) s->ssthresh = 2;
        s->cwnd = 1;
        s->state = SLOW_START;
    } else {
        /* Reno and NewReno */
        s->ssthresh = s->cwnd / 2;
        if (s->ssthresh < 2) s->ssthresh = 2;
        s->cwnd = s->ssthresh;
        s->state = FAST_RECOVERY;
        s->recover_point = s->high_pkt;
    }
}

void handle_partial_ack(SenderState *s) {
    if (s->algo == ALGO_NEWRENO) {
        /* NewReno: stay in fast recovery, do not shrink cwnd */
    } else if (s->algo == ALGO_RENO) {
        /* Reno: treats as another 3-dup-ACK -> halve again */
        s->ssthresh = s->cwnd / 2;
        if (s->ssthresh < 2) s->ssthresh = 2;
        s->cwnd = s->ssthresh;
        s->state = FAST_RECOVERY;
    } else {
        /* Tahoe: panic */
        s->ssthresh = s->cwnd / 2;
        if (s->ssthresh < 2) s->ssthresh = 2;
        s->cwnd = 1;
        s->state = SLOW_START;
    }
}

void apply_event(SenderState *s, int evt) {
    if (evt == EVT_ACK)              handle_ack(s);
    else if (evt == EVT_TIMEOUT)     handle_timeout(s);
    else if (evt == EVT_3DUPACK)     handle_3dupack(s);
    else if (evt == EVT_PARTIAL_ACK) handle_partial_ack(s);
}

void run_simulation(const char *algo_name, int algo,
                    int *events, int num_events) {
    SenderState s;
    s.cwnd = 1;
    s.ssthresh = 16;
    s.state = SLOW_START;
    s.algo = algo;
    s.recover_point = 0;
    s.high_pkt = 0;

    printf("\n--- TCP %s Simulation ---\n", algo_name);
    printf("RTT  | cwnd | ssthresh | State      | Event\n");
    printf("-----+------+----------+------------+----------\n");
    print_row(0, &s, "INITIAL");

    for (int i = 0; i < num_events; i++) {
        apply_event(&s, events[i]);
        print_row(i + 1, &s, event_name(events[i]));
    }
}

/* ============================================================
   PART 6 - PRE-DEFINED SCENARIOS
   ============================================================ */

/* Scenario 1: 1 timeout + 1 triple-dup-ACK
   Demonstrates Tahoe-vs-Reno difference. */
void scenario_basic(int algo) {
    int events[15] = {
        EVT_ACK, EVT_ACK, EVT_ACK, EVT_ACK, EVT_ACK, EVT_ACK,
        EVT_TIMEOUT,
        EVT_ACK, EVT_ACK, EVT_ACK, EVT_ACK,
        EVT_3DUPACK,
        EVT_ACK, EVT_ACK, EVT_ACK
    };
    const char *name = (algo == ALGO_TAHOE) ? "Tahoe" :
                       (algo == ALGO_RENO)  ? "Reno"  : "NewReno";
    run_simulation(name, algo, events, 15);
}

/* Scenario 2: 2 losses in same window (3-dup-ACK + partial-ACK)
   Demonstrates Reno-vs-NewReno difference. */
void scenario_multiloss(int algo) {
    int events[10] = {
        EVT_ACK, EVT_ACK, EVT_ACK, EVT_ACK, EVT_ACK,
        EVT_3DUPACK,
        EVT_PARTIAL_ACK,
        EVT_ACK, EVT_ACK, EVT_ACK
    };
    const char *name = (algo == ALGO_TAHOE) ? "Tahoe" :
                       (algo == ALGO_RENO)  ? "Reno"  : "NewReno";
    run_simulation(name, algo, events, 10);
}

void compare_all_basic(void) {
    printf("\n############################################\n");
    printf("# Basic Scenario: timeout + triple-dup-ACK #\n");
    printf("############################################\n");
    scenario_basic(ALGO_TAHOE);
    scenario_basic(ALGO_RENO);
    scenario_basic(ALGO_NEWRENO);
    printf("\nObservation:\n");
    printf("- TIMEOUT (RTT 7): all three reduce cwnd to 1.\n");
    printf("- 3 DUP ACK (RTT 12): Tahoe to 1, Reno and NewReno halve.\n");
}

void compare_all_multiloss(void) {
    printf("\n############################################\n");
    printf("# Multi-loss Scenario: 2 losses in window  #\n");
    printf("############################################\n");
    scenario_multiloss(ALGO_TAHOE);
    scenario_multiloss(ALGO_RENO);
    scenario_multiloss(ALGO_NEWRENO);
    printf("\nObservation:\n");
    printf("- Reno halves twice (8 -> 4) after partial ACK.\n");
    printf("- NewReno halves once and stays in fast recovery.\n");
    printf("- NewReno recovers fastest.\n");
}

/* ============================================================
   PART 7 - INTERACTIVE MENU
   ============================================================ */

int read_node_index(const char *prompt) {
    char buf[16];
    printf("%s", prompt);
    if (scanf("%15s", buf) != 1) return -1;
    if (buf[0] >= 'a' && buf[0] <= 'z') buf[0] -= 32;  /* uppercase */
    if (buf[0] < 'A' || buf[0] > 'F') return -1;
    return buf[0] - 'A';
}

void menu_routing_table(void) {
    int idx = read_node_index("Enter node (A-F): ");
    if (idx < 0) {
        printf("Invalid node.\n");
        return;
    }
    print_routing_table(idx);
}

void menu_all_routing_tables(void) {
    for (int i = 0; i < N; i++) print_routing_table(i);
}

void menu_send_message(void) {
    int src = read_node_index("Source node (A-F): ");
    if (src < 0) { printf("Invalid.\n"); return; }
    int dst = read_node_index("Destination node (A-F): ");
    if (dst < 0) { printf("Invalid.\n"); return; }

    char msg[128];
    printf("Message text (no spaces): ");
    if (scanf("%127s", msg) != 1) { printf("Invalid.\n"); return; }

    send_message(src, dst, msg);
}

int main(void) {
    int choice;

    printf("============================================\n");
    printf("  TCP Congestion Control + Network Sim    \n");
    printf("  CSE 320 - Programming Assignment        \n");
    printf("============================================\n");

    while (1) {
        printf("\n--- MENU ---\n");
        printf(" 1. Show network topology\n");
        printf(" 2. Show routing table for one node\n");
        printf(" 3. Show routing tables for ALL nodes\n");
        printf(" 4. Send a message between two nodes\n");
        printf(" 5. Run TCP Tahoe (basic scenario)\n");
        printf(" 6. Run TCP Reno (basic scenario)\n");
        printf(" 7. Run TCP NewReno (basic scenario)\n");
        printf(" 8. Compare all three (basic scenario)\n");
        printf(" 9. Compare all three (multi-loss scenario)\n");
        printf("10. Demo: PDF examples (A->D and F->E)\n");
        printf(" 0. Exit\n");
        printf("Choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input.\n");
            return 1;
        }

        switch (choice) {
            case 1:  print_topology(); break;
            case 2:  menu_routing_table(); break;
            case 3:  menu_all_routing_tables(); break;
            case 4:  menu_send_message(); break;
            case 5:  scenario_basic(ALGO_TAHOE); break;
            case 6:  scenario_basic(ALGO_RENO); break;
            case 7:  scenario_basic(ALGO_NEWRENO); break;
            case 8:  compare_all_basic(); break;
            case 9:  compare_all_multiloss(); break;
            case 10:
                send_message(0, 3, "hello_from_A_to_D");
                send_message(5, 4, "hello_from_F_to_E");
                break;
            case 0:
                printf("Goodbye!\n");
                return 0;
            default:
                printf("Unknown choice.\n");
        }
    }
    return 0;
}