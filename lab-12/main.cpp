
#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <limits>
#include <random>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <clocale>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

using namespace std;

const long long INF = numeric_limits<long long>::max() / 4;

using Matrix = vector<vector<long long>>;
using Next = vector<vector<int>>;
using EdgeList = vector<vector<pair<int, int>>>;

struct DijkstraResult {
    vector<long long> dist;
    vector<int> prev;
};

struct FloydResult {
    Matrix dist;
    Next next;
    bool negativeCycle = false;
};

struct Graph {
    int n;
    EdgeList adj;
    Matrix matrix;

    Graph(int size) : n(size), adj(size),
        matrix(size, vector<long long>(size, INF)) {
        for (int i = 0; i < n; i++) matrix[i][i] = 0;
    }

    void addEdge(int u, int v, int w) {
        matrix[u][v] = min(matrix[u][v], (long long)w);
        for (auto& e : adj[u]) {
            if (e.first == v) {
                e.second = min(e.second, w);
                return;
            }
        }
        adj[u].push_back({v, w});
    }

    // Дейкстра: O((V + E) log V), только для весов >= 0.
    DijkstraResult dijkstra(int start) const {
        DijkstraResult r;
        r.dist.assign(n, INF);
        r.prev.assign(n, -1);

        for (const auto& edges : adj)
            for (auto e : edges)
                if (e.second < 0) {
                    cout << "Ошибка: отрицательный вес для Дейкстры!\n";
                    return r;
                }

        priority_queue<pair<long long, int>,
            vector<pair<long long, int>>,
            greater<pair<long long, int>>> pq;

        r.dist[start] = 0;
        pq.push({0, start});

        while (!pq.empty()) {
            auto [d, u] = pq.top();
            pq.pop();

            if (d != r.dist[u]) continue;

            for (auto [v, w] : adj[u]) {
                if (d + w < r.dist[v]) {
                    r.dist[v] = d + w;
                    r.prev[v] = u;
                    pq.push({r.dist[v], v});
                }
            }
        }
        return r;
    }

    // Восстановление пути Дейкстры.
    vector<int> dijkstraPath(int s, int v,
                             const vector<int>& prev) const {
        vector<int> path;
        if (v != s && prev[v] == -1) return path;

        for (int x = v; x != -1; x = prev[x]) {
            path.push_back(x);
            if (x == s) break;
        }

        if (path.back() != s) return {};
        reverse(path.begin(), path.end());
        return path;
    }

    // Флойд — Уоршелл: O(V^3).
    FloydResult floyd() const {
        FloydResult r;
        r.dist = matrix;
        r.next.assign(n, vector<int>(n, -1));

        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                if (i != j && matrix[i][j] != INF)
                    r.next[i][j] = j;

        for (int k = 0; k < n; k++)
            for (int i = 0; i < n; i++) {
                if (r.dist[i][k] == INF) continue;
                for (int j = 0; j < n; j++) {
                    if (r.dist[k][j] == INF) continue;
                    long long d = r.dist[i][k] + r.dist[k][j];
                    if (d < r.dist[i][j]) {
                        r.dist[i][j] = d;
                        r.next[i][j] = r.next[i][k];
                    }
                }
            }

        for (int i = 0; i < n; i++)
            if (r.dist[i][i] < 0)
                r.negativeCycle = true;

        return r;
    }

    // Восстановление пути Флойда — Уоршелла.
    vector<int> floydPath(int s, int v, const Next& next) const {
        if (next[s][v] == -1) return {};
        vector<int> path = {s};

        while (s != v) {
            s = next[s][v];
            if (s == -1 || path.size() > (size_t)n) return {};
            path.push_back(s);
        }
        return path;
    }

    // Эксцентриситет: максимум кратчайших расстояний.
    vector<long long> eccentricity(const Matrix& d) const {
        vector<long long> e(n, 0);
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++) {
                if (d[i][j] == INF) {
                    e[i] = INF;
                    break;
                }
                e[i] = max(e[i], d[i][j]);
            }
        return e;
    }
};

void printPath(const vector<int>& path) {
    if (path.empty()) {
        cout << "нет пути";
        return;
    }
    for (size_t i = 0; i < path.size(); i++) {
        if (i) cout << " -> ";
        cout << path[i];
    }
}

Graph generateGraph(int n, mt19937& gen) {
    Graph g(n);
    uniform_int_distribution<int> weight(1, 20);
    bernoulli_distribution edge(0.8);

    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if (i != j && edge(gen))
                g.addEdge(i, j, weight(gen));
    return g;
}

void benchmark() {
    cout << "\n=== СРАВНЕНИЕ ПРОИЗВОДИТЕЛЬНОСТИ ===\n";
    ofstream file("benchmark.csv");
    file << "Vertices,Dijkstra_us,Floyd_us\n";

    mt19937 gen(42);
    for (int n : {10, 20, 50, 100}) {
        Graph g = generateGraph(n, gen);
        int start = 0;

        auto t1 = chrono::steady_clock::now();
        DijkstraResult d = g.dijkstra(start);
        auto t2 = chrono::steady_clock::now();
        FloydResult f = g.floyd();
        auto t3 = chrono::steady_clock::now();

        bool correct = d.dist == f.dist[start];

        double td = chrono::duration<double, micro>(t2 - t1).count();
        double tf = chrono::duration<double, micro>(t3 - t2).count();

        cout << n << " вершин | Дейкстра: " << td
             << " мкс | Флойд: " << tf
             << " мкс | Проверка: "
             << (correct ? "совпадает" : "ошибка") << '\n';

        file << n << ',' << td << ',' << tf << '\n';
    }
    file.close();
    cout << "Результаты сохранены в benchmark.csv\n";
}

int main() {
    setlocale(LC_ALL, ".UTF-8");
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    cout << "ЛАБОРАТОРНАЯ РАБОТА №12\n";
    cout << "Вариант 8. Продвинутый уровень\n";

    // Демонстрационный ориентированный граф.
    Graph g(5);
    g.addEdge(0, 1, 2);
    g.addEdge(1, 2, 1);
    g.addEdge(2, 3, 2);
    g.addEdge(3, 4, 1);
    g.addEdge(2, 4, 3);
    g.addEdge(0, 4, 10);

    cout << "\n=== АЛГОРИТМ ДЕЙКСТРЫ ===\n";
    DijkstraResult d = g.dijkstra(0);

    for (int i = 0; i < g.n; i++) {
        cout << "До вершины " << i << ": ";
        if (d.dist[i] == INF) cout << "недостижима\n";
        else {
            cout << d.dist[i] << ", путь: ";
            printPath(g.dijkstraPath(0, i, d.prev));
            cout << '\n';
        }
    }

    cout << "\n=== АЛГОРИТМ ФЛОЙДА — УОРШЕЛЛА ===\n";
    FloydResult f = g.floyd();

    cout << "Отрицательный цикл: "
         << (f.negativeCycle ? "обнаружен" : "не обнаружен") << '\n';

    for (int i = 0; i < g.n; i++)
        for (int j = 0; j < g.n; j++) {
            cout << i << " -> " << j << ": ";
            if (f.dist[i][j] == INF) cout << "нет пути\n";
            else {
                cout << f.dist[i][j] << ", путь: ";
                printPath(g.floydPath(i, j, f.next));
                cout << '\n';
            }
        }

    cout << "\n=== ЭКСЦЕНТРИСИТЕТЫ ===\n";
    auto e = g.eccentricity(f.dist);
    for (int i = 0; i < g.n; i++) {
        cout << i << ": ";
        if (e[i] == INF) cout << "бесконечность\n";
        else cout << e[i] << '\n';
    }

    // Проверка отрицательного ребра и отрицательного цикла.
    cout << "\n=== ПРОВЕРКА ОТРИЦАТЕЛЬНЫХ ВЕСОВ ===\n";
    Graph negative(3);
    negative.addEdge(0, 1, 4);
    negative.addEdge(1, 2, -6);
    negative.addEdge(0, 2, 5);
    cout << "Граф с отрицательным ребром: ";
    cout << (negative.floyd().negativeCycle ?
        "есть отрицательный цикл\n" : "отрицательного цикла нет\n");

    negative.addEdge(2, 0, 1);
    cout << "После добавления ребра 2 -> 0: ";
    cout << (negative.floyd().negativeCycle ?
        "обнаружен отрицательный цикл\n" : "цикла нет\n");

    cout << "\n=== СРАВНЕНИЕ ПРОИЗВОДИТЕЛЬНОСТИ ===\n";
    benchmark();

    return 0;
}
