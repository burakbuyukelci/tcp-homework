#include <stdio.h>

/* ===========================================================
   Topoloji sabitleri
   =========================================================== */
#define N        6        /* Dugum sayisi */
#define INF      9999     /* Sonsuzluk yerine */

/* Dugum isimleri (indeks ile esleme: 0=A, 1=B, ..., 5=F) */
const char* node_name[N] = { "A", "B", "C", "D", "E", "F" };
int         node_port[N] = { 5001, 5002, 5003, 5004, 5005, 5006 };

/* Komsuluk matrisi — PDF'deki haritaya gore
   PDF'deki A icin beklenen routing tablosuna uyacak sekilde */
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
   Routing table girisi
   =========================================================== */
typedef struct {
    int cost;          /* Toplam maliyet */
    int next_hop;      /* Hedefe giderken ilk gidilen dugum */
    int prev[N];       /* Path geri-takibi icin: her dugumun "ondeki" dugumu */
} RouteEntry;

/* ===========================================================
   Dijkstra algoritmasi
   src: kaynak dugum indeksi
   dist[]: cikis - her hedefe minimum maliyet
   prev[]: cikis - her hedef icin "bir oncekinin" indeksi (path icin)
   =========================================================== */
void dijkstra(int src, int dist[N], int prev[N]) {
    int visited[N];

    /* Baslangic */
    for (int i = 0; i < N; i++) {
        dist[i] = INF;
        prev[i] = -1;
        visited[i] = 0;
    }
    dist[src] = 0;

    /* N kere: en kucuk maliyetli ziyaret edilmemis dugumu bul, isle */
    for (int count = 0; count < N; count++) {
        /* En kucuk dist'e sahip ziyaret edilmemisi bul */
        int u = -1;
        int min_dist = INF;
        for (int i = 0; i < N; i++) {
            if (!visited[i] && dist[i] < min_dist) {
                min_dist = dist[i];
                u = i;
            }
        }

        if (u == -1) break;  /* erisilemeyen kalan dugum yok */
        visited[u] = 1;

        /* u'nun komsularini guncelle */
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
   Path'i ekrana yazdir (recursive)
   src'den dst'ye giderken aradaki tum dugumler
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

/* ===========================================================
   src'den dst'ye next hop bul
   prev[] geri-takip dizisinden next hop'u cikar
   =========================================================== */
int find_next_hop(int src, int dst, int prev[N]) {
    if (src == dst) return -1;  /* kendisine */

    /* dst'den geriye giderek src'nin hemen sonrasini bul */
    int curr = dst;
    while (prev[curr] != src && prev[curr] != -1) {
        curr = prev[curr];
    }
    if (prev[curr] == -1) return -1;  /* erisilemez */
    return curr;
}

/* ===========================================================
   Bir dugum icin routing tablosu yazdir
   =========================================================== */
void print_routing_table(int src) {
    int dist[N];
    int prev[N];
    dijkstra(src, dist, prev);

    printf("\nRouting table for node %s\n", node_name[src]);
    printf("--------------------------------------------------\n");
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
   MAIN
   =========================================================== */
int main(void) {
    printf("============================================\n");
    printf("  Network Topology (6 nodes: A,B,C,D,E,F)\n");
    printf("============================================\n");

    /* Topolojiyi yazdir */
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

    /* Tum dugumler icin routing tablosu */
    for (int i = 0; i < N; i++) {
        print_routing_table(i);
    }

    return 0;
}