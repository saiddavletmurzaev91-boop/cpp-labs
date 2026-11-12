#include <clocale>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include <string>
#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <algorithm>
#include <limits>
#include <functional>

using namespace std;

const int INF = 1000000000;

class Graph {
private:
    int V;
    vector<vector<int>> adj;

    void dfsUtil(int v, vector<bool>& visited,
                 vector<int>& result) {
        visited[v] = true;
        result.push_back(v);

        for (int u = 0; u < V; u++)
            if (adj[v][u] != INF && !visited[u])
                dfsUtil(u, visited, result);
    }

public:
    Graph(int n) : V(n), adj(n, vector<int>(n, INF)) {}

    void addEdge(int u, int v, int w) {
        if (u >= 0 && u < V && v >= 0 && v < V)
            adj[u][v] = w;
    }

    void removeEdge(int u, int v) {
        if (u >= 0 && u < V && v >= 0 && v < V)
            adj[u][v] = INF;
    }

    bool hasEdge(int u, int v) {
        return u >= 0 && u < V && v >= 0 &&
               v < V && adj[u][v] != INF;
    }

    void printMatrix() {
        cout << "\nМатрица смежности:\n   ";
        for (int i = 0; i < V; i++) cout << i << " ";
        cout << endl;

        for (int i = 0; i < V; i++) {
            cout << i << ": ";
            for (int j = 0; j < V; j++) {
                if (adj[i][j] == INF) cout << "- ";
                else cout << adj[i][j] << " ";
            }
            cout << endl;
        }
    }

    vector<int> DFS(int start) {
        vector<bool> visited(V, false);
        vector<int> result;
        if (start >= 0 && start < V)
            dfsUtil(start, visited, result);
        return result;
    }

    vector<int> DFSIterative(int start) {
        vector<bool> visited(V, false);
        vector<int> result;
        if (start < 0 || start >= V) return result;

        stack<int> st;
        st.push(start);

        while (!st.empty()) {
            int v = st.top();
            st.pop();

            if (visited[v]) continue;
            visited[v] = true;
            result.push_back(v);

            for (int u = V - 1; u >= 0; u--)
                if (adj[v][u] != INF && !visited[u])
                    st.push(u);
        }
        return result;
    }

    vector<int> BFS(int start) {
        vector<bool> visited(V, false);
        vector<int> result;
        if (start < 0 || start >= V) return result;

        queue<int> q;
        q.push(start);
        visited[start] = true;

        while (!q.empty()) {
            int v = q.front();
            q.pop();
            result.push_back(v);

            for (int u = 0; u < V; u++) {
                if (adj[v][u] != INF && !visited[u]) {
                    visited[u] = true;
                    q.push(u);
                }
            }
        }
        return result;
    }

    vector<int> shortestBFS(int start, int end) {
        vector<int> dist(V, -1), prev(V, -1);
        if (start < 0 || start >= V ||
            end < 0 || end >= V) return {};

        queue<int> q;
        q.push(start);
        dist[start] = 0;

        while (!q.empty()) {
            int v = q.front();
            q.pop();

            for (int u = 0; u < V; u++) {
                if (adj[v][u] != INF && dist[u] == -1) {
                    dist[u] = dist[v] + 1;
                    prev[u] = v;
                    q.push(u);
                }
            }
        }

        if (dist[end] == -1) return {};

        vector<int> path;
        for (int v = end; v != -1; v = prev[v])
            path.push_back(v);

        reverse(path.begin(), path.end());
        return path;
    }

    int distanceBFS(int start, int end) {
        vector<int> path = shortestBFS(start, end);
        return path.empty() ? -1 : static_cast<int>(path.size()) - 1;
    }

    vector<vector<int>> components() {
        vector<bool> visited(V, false);
        vector<vector<int>> result;

        for (int i = 0; i < V; i++) {
            if (visited[i]) continue;

            vector<int> component;
            queue<int> q;
            q.push(i);
            visited[i] = true;

            while (!q.empty()) {
                int v = q.front();
                q.pop();
                component.push_back(v);

                // Компоненты слабой связности:
                // направление рёбер не учитывается.
                for (int u = 0; u < V; u++) {
                    if ((adj[v][u] != INF || adj[u][v] != INF)
                        && !visited[u]) {
                        visited[u] = true;
                        q.push(u);
                    }
                }
            }
            result.push_back(component);
        }
        return result;
    }

    vector<int> dijkstra(int start, int end, int& total) {
        total = INF;
        if (start < 0 || start >= V ||
            end < 0 || end >= V) return {};

        for (int i = 0; i < V; i++)
            for (int j = 0; j < V; j++)
                if (adj[i][j] != INF && adj[i][j] < 0)
                    return {};

        vector<int> dist(V, INF), prev(V, -1);
        priority_queue<pair<int, int>,
            vector<pair<int, int>>,
            greater<pair<int, int>>> pq;

        dist[start] = 0;
        pq.push({0, start});

        while (!pq.empty()) {
            auto [d, v] = pq.top();
            pq.pop();

            if (d != dist[v]) continue;

            for (int u = 0; u < V; u++) {
                if (adj[v][u] == INF) continue;

                if (d + adj[v][u] < dist[u]) {
                    dist[u] = d + adj[v][u];
                    prev[u] = v;
                    pq.push({dist[u], u});
                }
            }
        }

        if (dist[end] == INF) return {};
        total = dist[end];

        vector<int> path;
        for (int v = end; v != -1; v = prev[v])
            path.push_back(v);

        reverse(path.begin(), path.end());
        return path;
    }
};

void printVector(const string& name, const vector<int>& v) {
    cout << name;
    if (v.empty()) cout << "нет данных";
    for (int x : v) cout << x << " ";
    cout << endl;
}

int main() {
    setlocale(LC_ALL, ".UTF-8");

    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
    #endif

    cout << "ЛАБОРАТОРНАЯ РАБОТА №11\n";
    cout << "Вариант 8. Продвинутый уровень\n";

    Graph g(7);

    g.addEdge(0, 1, 4);
    g.addEdge(0, 2, 2);
    g.addEdge(1, 3, 5);
    g.addEdge(2, 3, 1);
    g.addEdge(3, 4, 3);
    g.addEdge(4, 1, 2);
    g.addEdge(5, 6, 1);
    g.addEdge(6, 6, 0);

    cout << "\n=== ЧАСТЬ 1. ГРАФ ===\n";
    g.printMatrix();

    cout << "\nРебро 0 -> 1: "
         << (g.hasEdge(0, 1) ? "есть" : "нет") << endl;

    g.removeEdge(0, 1);
    cout << "После удаления 0 -> 1: "
         << (g.hasEdge(0, 1) ? "есть" : "нет") << endl;
    g.addEdge(0, 1, 4);

    cout << "\n=== ЧАСТЬ 2. DFS И BFS ===\n";
    printVector("Рекурсивный DFS: ", g.DFS(0));
    printVector("Итеративный DFS: ", g.DFSIterative(0));
    printVector("BFS: ", g.BFS(0));

    cout << "Расстояние от 0 до 4: "
         << g.distanceBFS(0, 4) << endl;
    printVector("Путь BFS: ", g.shortestBFS(0, 4));

    cout << "\n=== ЧАСТЬ 3. ПРИМЕНЕНИЕ ===\n";

    auto comps = g.components();
    cout << "Количество компонент слабой связности: "
         << comps.size() << endl;

    for (size_t i = 0; i < comps.size(); i++)
        printVector("Компонента " + to_string(i + 1) + ": ",
                    comps[i]);

    int weight;
    auto path = g.dijkstra(0, 4, weight);
    printVector("Путь Дейкстры: ", path);

    if (!path.empty())
        cout << "Суммарный вес: " << weight << endl;
    else
        cout << "Взвешенный путь не найден.\n";

    return 0;
}
