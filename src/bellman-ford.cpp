#include <iostream>
#include <vector>
#include <queue>
#include <fstream>
#include <sstream>
#include <climits>
#include <chrono>
#include <sys/resource.h>
using namespace std;

long getMemoryKB() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_maxrss;
}

struct BellmanFordStats {
    vector<long long> dist;
    int       iterations;
    long long distBytes;
    long long totalBytes;
    bool      hasNegativeCycle;
};

BellmanFordStats bellmanFord(const vector<vector<pair<int,int>>>& adj, int src) {
    int V = adj.size();

    vector<long long> dist(V, LLONG_MAX);
    long long distBytes = (long long)V * sizeof(long long);

    dist[src] = 0;
    int iterations = 0;

    for (int iter = 0; iter < V - 1; iter++) {
        bool updated = false;
        iterations++;

        for (int u = 0; u < V; u++) {
            if (dist[u] == LLONG_MAX) continue;
            for (auto [v, w] : adj[u]) {
                if (dist[u] + w < dist[v]) {
                    dist[v] = dist[u] + w;
                    updated = true;
                }
            }
        }

        if (!updated) break;
    }

    // Negative cycle detection --> veri setimizde negatif ağırlık yok, bellman ford'un negatif ağırlık döngüsü tespit etme özelliğini göstermek için ekledim
    bool hasNegativeCycle = false;
    for (int u = 0; u < V; u++) {
        if (dist[u] == LLONG_MAX) continue;
        for (auto [v, w] : adj[u]) {
            if (dist[u] + w < dist[v]) {
                hasNegativeCycle = true;
                break;
            }
        }
        if (hasNegativeCycle) break;
    }

    long long totalBytes = distBytes;

    return {dist, iterations, distBytes, totalBytes, hasNegativeCycle};
}

int main() {
    string filename = "USA-road-d.NE.gr";
    cout << "Reading graph from file: " << filename << "\n";
    ifstream file(filename);

    if (!file.is_open()) {
        cout << "Dosya acilamadi.\n";
        return 1;
    }

    string line;
    int n = 0, m = 0;

    while (getline(file, line)) {
        if (line.empty() || line[0] == 'c') continue;
        if (line[0] == 'p') {
            string tmp1, tmp2;
            stringstream ss(line);
            ss >> tmp1 >> tmp2 >> n >> m;
            break;
        }
    }

    vector<vector<pair<int,int>>> adj(n);

    while (getline(file, line)) {
        if (line.empty() || line[0] != 'a') continue;
        char ch; int u, v, w;
        stringstream ss(line);
        ss >> ch >> u >> v >> w;
        u--; v--;
        adj[u].push_back({v, w});
    }

    file.close();

    // adj belleği hesapla
    long long adjOuterBytes = (long long)n * sizeof(vector<pair<int,int>>);
    long long adjInnerBytes = (long long)m * sizeof(pair<int,int>);
    long long adjTotalBytes = adjOuterBytes + adjInnerBytes;

    int src = 1372007;
    if (src < 0 || src >= n) {
        cout << "Gecersiz source node.\n";
        return 1;
    }

    long memBefore = getMemoryKB();
    auto start = chrono::high_resolution_clock::now();

    BellmanFordStats stats = bellmanFord(adj, src);

    auto end = chrono::high_resolution_clock::now();
    long memAfter = getMemoryKB();

    double duration_s = chrono::duration_cast<chrono::microseconds>(
                            end - start).count() / 1000000.0;

    long long estimatedTotalBytes = adjTotalBytes + stats.totalBytes;

    cout << "\n=== Runtime ===\n";
    cout << "Bellman-Ford runtime     : " << duration_s << " seconds\n";
    cout << "Iterations               : " << stats.iterations << "\n";
    cout << "Negative cycle           : " << (stats.hasNegativeCycle ? "DETECTED" : "None") << "\n";

    cout << "\n=== Graph Storage Memory (Approx.) ===\n";
    cout << "adj outer (vector heads) : " << adjOuterBytes / (1024.0*1024) << " MB"
         << "  (" << n << " x " << sizeof(vector<pair<int,int>>) << " byte)\n";
    cout << "adj inner (edges)        : " << adjInnerBytes / (1024.0*1024) << " MB"
         << "  (" << m << " x " << sizeof(pair<int,int>) << " byte)\n";
    cout << "adj TOPLAM               : " << adjTotalBytes / (1024.0*1024) << " MB\n";

    cout << "\n=== Bellman-Ford Working Memory (Approx.) ===\n";
    cout << "dist[] dizisi            : " << stats.distBytes / (1024.0*1024) << " MB"
         << "  (" << n << " x " << sizeof(long long) << " byte)\n";
    cout << "Priority queue           : 0 MB  (Bellman-Ford pq kullanmaz)\n";
    cout << "Bellman-Ford working TOP : " << stats.totalBytes / (1024.0*1024) << " MB\n";

    cout << "\n=== Tahmini Program Toplam Verisi (Approx.) ===\n";
    cout << "Graph + Bellman-Ford top : " << estimatedTotalBytes / (1024.0*1024) << " MB\n";

    cout << "\n=== Process Peak Memory from OS (Reference) ===\n";
    cout << "OS peak memory           : " << memAfter  / 1024.0 << " MB\n";
    cout << "Delta (ref.)             : " << (memAfter - memBefore) / 1024.0 << " MB\n";

    cout << "Source node (1-based): " << src + 1 << "\n";
    cout << "Distance from source to itself: " << stats.dist[src] << "\n";

    return 0;
}