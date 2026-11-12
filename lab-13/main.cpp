
#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <numeric>
#include <random>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <cmath>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
using namespace std;
using Clock = chrono::high_resolution_clock;

struct Edge {
    int u, v;
    double w;
};

struct Graph {
    int n;
    vector<Edge> e;
    vector<vector<pair<int, double>>> adj;

    Graph(int n) : n(n), adj(n) {}

    void add(int u, int v, double w) {
        if (u == v || u < 0 || v < 0 || u >= n || v >= n) return;
        e.push_back({u, v, w});
        adj[u].push_back({v, w});
        adj[v].push_back({u, w});
    }
};

struct DSU {
    vector<int> p, r;

    DSU(int n) : p(n), r(n, 0) {
        iota(p.begin(), p.end(), 0);
    }

    int find(int x) {
        return p[x] == x ? x : p[x] = find(p[x]);
    }

    bool unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return false;
        if (r[a] < r[b]) swap(a, b);
        p[b] = a;
        if (r[a] == r[b]) ++r[a];
        return true;
    }
};

struct Result {
    vector<Edge> edges;
    double cost = 0;
    bool connected = false;
};

// Алгоритм Прима с priority_queue.
Result prim(const Graph& g, int start) {
    Result res;
    if (start < 0 || start >= g.n) return res;

    vector<bool> used(g.n, false);
    using Item = tuple<double, int, int>;
    priority_queue<Item, vector<Item>, greater<Item>> pq;
    pq.push({0, start, -1});

    while (!pq.empty()) {
        auto [w, v, from] = pq.top();
        pq.pop();
        if (used[v]) continue;

        used[v] = true;
        if (from != -1) {
            res.edges.push_back({from, v, w});
            res.cost += w;
        }

        for (auto [to, weight] : g.adj[v])
            if (!used[to]) pq.push({weight, to, v});
    }

    res.connected = res.edges.size() == static_cast<size_t>(g.n - 1);
    return res;
}

// Алгоритм Краскала с DSU.
Result kruskal(const Graph& g) {
    Result res;
    vector<Edge> edges = g.e;

    sort(edges.begin(), edges.end(),
         [](const Edge& a, const Edge& b) { return a.w < b.w; });

    DSU dsu(g.n);
    for (auto edge : edges) {
        if (dsu.unite(edge.u, edge.v)) {
            res.edges.push_back(edge);
            res.cost += edge.w;
        }
    }

    res.connected = res.edges.size() == static_cast<size_t>(g.n - 1);
    return res;
}

// Компоненты связности.
void components(const Graph& g) {
    DSU dsu(g.n);
    for (auto e : g.e) dsu.unite(e.u, e.v);

    vector<vector<int>> groups(g.n);
    for (int i = 0; i < g.n; ++i)
        groups[dsu.find(i)].push_back(i);

    cout << "\nКомпоненты связности:\n";
    int k = 0;
    for (auto& group : groups) {
        if (group.empty()) continue;
        cout << ++k << ": ";
        for (int v : group) cout << v << " ";
        cout << "\n";
    }
}

void print(const Result& r, const string& name) {
    cout << "\n" << name << ":\n";
    for (auto e : r.edges)
        cout << e.u << " - " << e.v << " (" << e.w << ")\n";

    cout << "Общий вес: " << r.cost << "\n";
    cout << (r.connected ? "MST построено\n" :
                              "Граф несвязный, построено лесное MST\n");
}

// Практическая задача: минимальная сеть дорог.
void cityTask() {
    vector<string> city = {"A", "B", "C", "D", "E", "F"};
    Graph g(6);

    g.add(0, 1, 4); g.add(0, 2, 3);
    g.add(1, 2, 2); g.add(1, 3, 2);
    g.add(2, 4, 3); g.add(3, 4, 5);
    g.add(4, 5, 5);

    Result r = kruskal(g);
    cout << "\n=== СТРОИТЕЛЬСТВО ДОРОГ ===\n";
    for (auto e : r.edges)
        cout << city[e.u] << " - " << city[e.v]
             << ": " << e.w << "\n";
    cout << "Минимальная стоимость: " << r.cost << "\n";
}

// Генерация связного графа заданной плотности.
Graph generate(int n, int density, mt19937& rng) {
    Graph g(n);
    uniform_real_distribution<double> weight(1, 100);
    vector<vector<bool>> used(n, vector<bool>(n, false));

    for (int i = 1; i < n; ++i) {
        g.add(i - 1, i, weight(rng));
        used[i - 1][i] = used[i][i - 1] = true;
    }

    int target = max(n - 1, n * (n - 1) / 2 * density / 100);
    uniform_int_distribution<int> vertex(0, n - 1);

    while (static_cast<int>(g.e.size()) < target) {
        int u = vertex(rng), v = vertex(rng);
        if (u != v && !used[u][v]) {
            g.add(u, v, weight(rng));
            used[u][v] = used[v][u] = true;
        }
    }
    return g;
}

// Сравнение времени работы алгоритмов.
void benchmark() {
    ofstream file("benchmark.csv");
    file << "Vertices,Density,Edges,Prim_us,Kruskal_us,Equal\n";

    mt19937 rng(42);
    cout << "\n=== СРАВНЕНИЕ ПРОИЗВОДИТЕЛЬНОСТИ ===\n";

    for (int n : {50, 100, 250, 500}) {
        for (int d : {20, 60, 90}) {
            Graph g = generate(n, d, rng);

            auto t1 = Clock::now();
            Result a = prim(g, 0);
            auto t2 = Clock::now();
            Result b = kruskal(g);
            auto t3 = Clock::now();

            double pt = chrono::duration<double, micro>(t2 - t1).count();
            double kt = chrono::duration<double, micro>(t3 - t2).count();
            bool equal = abs(a.cost - b.cost) < 1e-8;

            cout << n << " вершин, " << d << "%: Прим "
                 << pt << " мкс, Краскал " << kt
                 << " мкс, совпадение: " << (equal ? "да" : "нет") << "\n";

            file << n << "," << d << "," << g.e.size()
                 << "," << pt << "," << kt
                 << "," << (equal ? "yes" : "no") << "\n";
        }
    }
    cout << "Данные сохранены в benchmark.csv\n";
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    cout << fixed << setprecision(2);
    cout << "ЛАБОРАТОРНАЯ РАБОТА №13\nВариант 8. Продвинутый уровень\n";

    Graph g(6);
    g.add(0, 1, 4.5); g.add(0, 2, 3.2);
    g.add(1, 2, 2.7); g.add(1, 3, 2.1);
    g.add(2, 4, 3.8); g.add(3, 4, 5.4);
    g.add(3, 5, 6.2); g.add(4, 5, 2.9);

    Result p = prim(g, 0);
    Result k = kruskal(g);

    print(p, "АЛГОРИТМ ПРИМА");
    print(k, "АЛГОРИТМ КРАСКАЛА");

    cout << "\nПроверка веса: "
         << (abs(p.cost - k.cost) < 1e-8 ? "совпадает" : "не совпадает")
         << "\n";

    cout << "\n=== НЕСВЯЗНЫЙ ГРАФ ===\n";
    Graph disconnected(6);
    disconnected.add(0, 1, 2.5);
    disconnected.add(1, 2, 1.5);
    disconnected.add(3, 4, 3.5);
    print(kruskal(disconnected), "КРАСКАЛ");
    components(disconnected);

    cityTask();
    benchmark();
    return 0;
}
