#include <stdio.h>

/* ===========================================================
   Topoloji sabitleri
   =========================================================== */
#define N        6
#define INF      9999

const char* node_name[N] = { "A", "B", "C", "D", "E", "F" };
int         node_port[N] = { 5001, 5002, 5003, 5004, 5005, 5006 };

int graph[N][N] = {
    /*       A     B     C     D     E     F   */
    /* A */ {  0,    4,    7,   13,  INF,    5 },
    /* B */ {  4,    0,  INF,    8,    3,  INF },
    /* C */ {  7,  INF,    0,  INF,   12,  INF },
    /* D */ { 13,    8,  INF,    0,  INF,  INF },
    /* E */ {INF,    3,   12,  INF,    0,  INF },
    /* F */ {  5,  INF,  INF,  INF,  INF,    0 }
};

/* ===========================================================
   Dijkstra
   =========================================================== */
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

/* ===========================================================
   Path yazdir, next hop bul
   =========================================================== */
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

/* ===========================================================
   YENI: Mesaj gonderme simulasyonu
   src dugumunden dst dugumune mesaj gonder.
   Her dugum kendi routing tablosuna gore next hop'u bulup
   ekrana log atar.
   =========================================================== */
void send_message(int src, int dst, const char *message) {
    printf("\n>> Sending message from %s to %s: \"%s\"\n",
           node_name[src], node_name[dst], message);

    if (src == dst) {
        printf("[%s] Source and destination are the same.\n", node_name[src]);
        return;
    }

    /* Kaynak dugumun tablosundan next hop'u bul */
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

    /* Hop hop ilerle */
    int current = next_hop;
    while (current != dst) {
        /* Bu dugumun tablosundan next hop'u bul */
        int d2[N], p2[N];
        dijkstra(current, d2, p2);
        int nh = find_next_hop(current, dst, p2);

        printf("[%s] Forwarding message from %s to %s, next hop %s\n",
               node_name[current], node_name[src], node_name[dst],
               node_name[nh]);

        current = nh;

        /* Sonsuz dongu koruyucu */
        if (nh == -1) {
            printf("[%s] Routing failed.\n", node_name[current]);
            return;
        }
    }

    /* Hedefe vardik */
    printf("[%s] Received message from %s: %s\n",
           node_name[dst], node_name[src], message);
}

/* ===========================================================
   MAIN — PDF'deki iki ornegi test ediyoruz
   =========================================================== */
int main(void) {
    printf("============================================\n");
    printf("  Routing Tables\n");
    printf("============================================\n");

    /* A'nin routing tablosu — PDF'deki ile karsilastir */
    print_routing_table(0);  /* A */
    print_routing_table(5);  /* F */

    printf("\n");
    printf("============================================\n");
    printf("  Message Forwarding Examples\n");
    printf("============================================\n");

    /* Ornek 1: A'dan D'ye */
    send_message(0, 3, "hello_from_A_to_D");

    /* Ornek 2: F'den E'ye */
    send_message(5, 4, "hello_from_F_to_E");

    /* Ek ornek: C'den D'ye (ilginc bir yol) */
    send_message(2, 3, "hello_from_C_to_D");

    return 0;
}