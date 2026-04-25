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

struct DijkstraStats {
    vector<long long> dist;
    int       pqMaxSize;
    long long distBytes;
    long long pqPeakBytes;
    long long dijkstraWorkingBytes;
};

DijkstraStats dijkstra(const vector<vector<pair<int,int>>>& adj, int src) {
    int V = adj.size();

    vector<long long> dist(V, LLONG_MAX);
    long long distBytes = (long long)V * sizeof(long long);

    typedef pair<long long, int> pli;
    priority_queue<pli, vector<pli>, greater<pli> > pq;

    dist[src] = 0;
    pq.emplace(0, src);

    int pqMaxSize = (int)pq.size(); // başlangıçta 1 eleman var

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();

        if (d > dist[u]) continue;

        for (auto [v, w] : adj[u]) {
            if (dist[u] != LLONG_MAX && dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                pq.emplace(dist[v], v);

                if ((int)pq.size() > pqMaxSize)
                    pqMaxSize = (int)pq.size();
            }
        }
    }

    long long pqPeakBytes          = (long long)pqMaxSize * sizeof(pli);
    long long dijkstraWorkingBytes  = distBytes + pqPeakBytes;

    return {dist, pqMaxSize, distBytes, pqPeakBytes, dijkstraWorkingBytes};
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

    DijkstraStats stats = dijkstra(adj, src);

    auto end = chrono::high_resolution_clock::now();
    long memAfter = getMemoryKB();

    double duration_s = chrono::duration_cast<chrono::microseconds>(
                            end - start).count() / 1000000.0;

    long long estimatedTotalBytes = adjTotalBytes + stats.dijkstraWorkingBytes;

    cout << "\n=== Runtime ===\n";
    cout << "Dijkstra runtime         : " << duration_s << " seconds\n";

    cout << "\n=== Graph Storage Memory (Approx.) ===\n";
    cout << "adj outer (vector heads) : " << adjOuterBytes / (1024.0*1024) << " MB"
         << "  (" << n << " x " << sizeof(vector<pair<int,int>>) << " byte)\n";
    cout << "adj inner (edges)        : " << adjInnerBytes / (1024.0*1024) << " MB"
         << "  (" << m << " x " << sizeof(pair<int,int>) << " byte)\n";
    cout << "adj TOPLAM               : " << adjTotalBytes / (1024.0*1024) << " MB\n";

    cout << "\n=== Dijkstra Working Memory (Approx.) ===\n";
    cout << "dist[] dizisi            : " << stats.distBytes    / (1024.0*1024) << " MB"
         << "  (" << n << " x " << sizeof(long long) << " byte)\n";
    cout << "Priority queue (peak)    : " << stats.pqPeakBytes  / (1024.0*1024) << " MB"
         << "  (peak " << stats.pqMaxSize << " eleman x "
         << sizeof(pair<long long,int>) << " byte)\n";
    cout << "Dijkstra working TOPLAM  : " << stats.dijkstraWorkingBytes / (1024.0*1024) << " MB\n";

    cout << "\n=== Tahmini Program Toplam Verisi (Approx.) ===\n";
    cout << "Graph + Dijkstra toplam  : " << estimatedTotalBytes / (1024.0*1024) << " MB\n";

    cout << "\n=== Process Peak Memory from OS (Reference) ===\n";
    cout << "OS peak memory           : " << memAfter  / 1024.0 << " MB\n";
    cout << "Delta (ref.)             : " << (memAfter - memBefore) / 1024.0 << " MB\n";

    cout << "Source node (1-based): " << src + 1 << "\n";
    cout << "Distance from source to itself: " << stats.dist[src] << "\n\n";

    return 0;
}