#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <cmath>
#include <sys/resource.h>
using namespace std;

extern "C" {
    #include "dmmsy_lib/common.h"
    void ssp_duan_research(CSRGraph *g, node_t src, weight_t *d_out, node_t *pr_out);
}

long getMemoryKB() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_maxrss;
}

CSRGraph load_dimacs(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Dosya açılamadı: " << filename << "\n";
        exit(1);
    }

    string line;
    int n = 0, m = 0;

    while (getline(file, line)) {
        if (line.empty() || line[0] == 'c') continue;
        if (line[0] == 'p') {
            stringstream ss(line);
            string tmp1, tmp2;
            ss >> tmp1 >> tmp2 >> n >> m;
            break;
        }
    }

    vector<int> us, vs;
    vector<double> ws;
    double sum_w = 0.0;

    while (getline(file, line)) {
        if (line.empty() || line[0] != 'a') continue;
        char ch; int u, v, w;
        stringstream ss(line);
        ss >> ch >> u >> v >> w;
        u--; v--;
        us.push_back(u);
        vs.push_back(v);
        ws.push_back((double)w);
        sum_w += w;
    }
    file.close();

    CSRGraph g;
    g.n = (node_t)n;
    g.m = (node_t)m;
    g.offset = (node_t*)calloc(n + 1, sizeof(node_t));
    g.edges  = (Edge*)malloc(sizeof(Edge) * m);
    g.mean_weight = (m > 0) ? sum_w / m : 0.0;

    for (int i = 0; i < m; i++)
        g.offset[us[i] + 1]++;
    for (int i = 0; i < n; i++)
        g.offset[i + 1] += g.offset[i];

    vector<int> idx(n, 0);
    for (int i = 0; i < m; i++) {
        int u = us[i];
        int pos = g.offset[u] + idx[u]++;
        g.edges[pos].v = (node_t)vs[i];
        g.edges[pos].w = ws[i];
    }

    return g;
}

int main() {
    string filename = "USA-road-d.NE.gr";
    int src = 1372007;

    cout << "Reading graph from file: " << filename << "\n";
    CSRGraph g = load_dimacs(filename);
    cout << "Graph loaded: n=" << g.n << " m=" << g.m << "\n";

    if (src < 0 || src >= (int)g.n) {
        cout << "Geçersiz source node.\n";
        return 1;
    }

    long long csrOffsetBytes = (long long)(g.n + 1) * sizeof(node_t);
    long long csrEdgeBytes   = (long long)g.m * sizeof(Edge);
    long long csrTotalBytes  = csrOffsetBytes + csrEdgeBytes;

    weight_t *d  = (weight_t*)malloc(sizeof(weight_t) * g.n);
    node_t   *pr = (node_t*)  malloc(sizeof(node_t)   * g.n);

    long memBefore = getMemoryKB();
    auto start = chrono::high_resolution_clock::now();

    ssp_duan_research(&g, (node_t)src, d, pr);

    auto end = chrono::high_resolution_clock::now();
    long memAfter = getMemoryKB();

    double elapsed_s = chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000000.0;

    long long distBytes      = (long long)g.n * sizeof(weight_t);
    long long workspaceBytes =
        (long long)g.n * sizeof(weight_t)     +
        (long long)g.n * sizeof(node_t)       +
        (long long)(g.n+1) * sizeof(HeapNode) +
        (long long)g.n * sizeof(node_t)       +
        (long long)g.n * sizeof(node_t)       +
        (long long)g.n * sizeof(node_t)       +
        (long long)g.n * sizeof(node_t);
    long long totalWorkingBytes  = distBytes + workspaceBytes;
    long long estimatedTotalBytes = csrTotalBytes + totalWorkingBytes;

    cout << "\n=== Runtime ===\n";
    cout << "DMMSY runtime            : " << elapsed_s << " seconds\n";

    cout << "\n=== Graph Storage Memory (Approx.) ===\n";
    cout << "CSR offset array         : " << csrOffsetBytes / (1024.0*1024) << " MB"
         << "  (" << (g.n+1) << " x " << sizeof(node_t) << " byte)\n";
    cout << "CSR edges array          : " << csrEdgeBytes   / (1024.0*1024) << " MB"
         << "  (" << g.m << " x " << sizeof(Edge) << " byte)\n";
    cout << "CSR TOPLAM               : " << csrTotalBytes  / (1024.0*1024) << " MB\n";

    cout << "\n=== DMMSY Working Memory (Approx.) ===\n";
    cout << "dist[] dizisi            : " << distBytes          / (1024.0*1024) << " MB"
         << "  (" << g.n << " x " << sizeof(weight_t) << " byte)\n";
    cout << "Workspace (heap+buffers) : " << workspaceBytes     / (1024.0*1024) << " MB\n";
    cout << "DMMSY working TOPLAM     : " << totalWorkingBytes  / (1024.0*1024) << " MB\n";

    cout << "\n=== Tahmini Program Toplam Verisi (Approx.) ===\n";
    cout << "Graph + DMMSY toplam     : " << estimatedTotalBytes / (1024.0*1024) << " MB\n";

    cout << "\n=== Process Peak Memory from OS (Reference) ===\n";
    cout << "OS peak memory           : " << memAfter / 1024.0 << " MB\n";
    cout << "Delta (ref.)             : " << (memAfter - memBefore) / 1024.0 << " MB\n";

    cout << "Source node (1-based)    : " << src + 1 << "\n";
    cout << "Distance source to itself: " << d[src] << "\n";

    free(d);
    free(pr);
    free_graph(&g);
    return 0;
}