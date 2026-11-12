
#include <algorithm>
#include <chrono>
#include <cctype>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;
using Clock = chrono::steady_clock;

// ======================== AVL TREE ========================

struct AVLNode {
    int key, height = 1;
    AVLNode *left = nullptr, *right = nullptr;
    explicit AVLNode(int v) : key(v) {}
};

class AVLTree {
    AVLNode* root = nullptr;

    int h(AVLNode* n) { return n ? n->height : 0; }
    int bf(AVLNode* n) { return h(n->left) - h(n->right); }

    void update(AVLNode* n) {
        n->height = 1 + max(h(n->left), h(n->right));
    }

    AVLNode* rightRotate(AVLNode* y) {
        AVLNode* x = y->left;
        y->left = x->right;
        x->right = y;
        update(y);
        update(x);
        return x;
    }

    AVLNode* leftRotate(AVLNode* x) {
        AVLNode* y = x->right;
        x->right = y->left;
        y->left = x;
        update(x);
        update(y);
        return y;
    }

    AVLNode* balance(AVLNode* n) {
        if (!n) return n;
        update(n);
        if (bf(n) > 1) {
            if (bf(n->left) < 0)
                n->left = leftRotate(n->left);
            return rightRotate(n);
        }
        if (bf(n) < -1) {
            if (bf(n->right) > 0)
                n->right = rightRotate(n->right);
            return leftRotate(n);
        }
        return n;
    }

    AVLNode* insert(AVLNode* n, int key) {
        if (!n) return new AVLNode(key);
        if (key < n->key) n->left = insert(n->left, key);
        else if (key > n->key) n->right = insert(n->right, key);
        else return n;
        return balance(n);
    }

    AVLNode* minNode(AVLNode* n) {
        while (n->left) n = n->left;
        return n;
    }

    AVLNode* erase(AVLNode* n, int key) {
        if (!n) return nullptr;
        if (key < n->key) n->left = erase(n->left, key);
        else if (key > n->key) n->right = erase(n->right, key);
        else {
            if (!n->left || !n->right) {
                AVLNode* child = n->left ? n->left : n->right;
                delete n;
                return child;
            }
            AVLNode* successor = minNode(n->right);
            n->key = successor->key;
            n->right = erase(n->right, successor->key);
        }
        return balance(n);
    }

    bool contains(AVLNode* n, int key) {
        if (!n) return false;
        if (n->key == key) return true;
        return contains(key < n->key ? n->left : n->right, key);
    }

    AVLNode* findLCA(AVLNode* n, int a, int b) {
        if (a < n->key && b < n->key)
            return findLCA(n->left, a, b);
        if (a > n->key && b > n->key)
            return findLCA(n->right, a, b);
        return n;
    }

    void print(AVLNode* n, int depth) {
        if (!n) return;
        print(n->right, depth + 4);
        cout << string(depth, ' ') << n->key
             << " (h=" << n->height << ")\n";
        print(n->left, depth + 4);
    }

    void clear(AVLNode* n) {
        if (!n) return;
        clear(n->left);
        clear(n->right);
        delete n;
    }

public:
    ~AVLTree() { clear(root); }
    void insert(int x) { root = insert(root, x); }
    void remove(int x) { root = erase(root, x); }
    bool search(int x) { return contains(root, x); }
    int height() { return h(root); }

    int lca(int a, int b) {
        if (!search(a) || !search(b)) return -1;
        return findLCA(root, a, b)->key;
    }

    void show() {
        if (!root) cout << "Дерево пустое\n";
        else print(root, 0);
    }
};

// ======================== ORDINARY BST ========================

struct BSTNode {
    int key;
    BSTNode *left = nullptr, *right = nullptr;
    explicit BSTNode(int v) : key(v) {}
};

class BST {
    BSTNode* root = nullptr;

public:
    ~BST() { clear(); }

    void insert(int x) {
        BSTNode** p = &root;
        while (*p) {
            if (x < (*p)->key) p = &(*p)->left;
            else if (x > (*p)->key) p = &(*p)->right;
            else return;
        }
        *p = new BSTNode(x);
    }

    bool search(int x) {
        BSTNode* p = root;
        while (p) {
            if (p->key == x) return true;
            p = x < p->key ? p->left : p->right;
        }
        return false;
    }

    void clear() {
        vector<BSTNode*> s;
        if (root) s.push_back(root);
        while (!s.empty()) {
            BSTNode* n = s.back();
            s.pop_back();
            if (n->left) s.push_back(n->left);
            if (n->right) s.push_back(n->right);
            delete n;
        }
        root = nullptr;
    }
};

// ======================== PREFIX TREE: TRIE ========================

struct TrieNode {
    unordered_map<char, TrieNode*> next;
    bool end = false;
};

class Trie {
    TrieNode* root = new TrieNode();

    string normalize(string s) {
        for (char& c : s)
            c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
        return s;
    }

    void clear(TrieNode* n) {
        for (auto& p : n->next) clear(p.second);
        delete n;
    }

public:
    ~Trie() { clear(root); }

    void insert(string word) {
        TrieNode* p = root;
        for (char c : normalize(word)) {
            if (!p->next.count(c)) p->next[c] = new TrieNode();
            p = p->next[c];
        }
        p->end = true;
    }

    bool search(string word) {
        TrieNode* p = root;
        for (char c : normalize(word)) {
            if (!p->next.count(c)) return false;
            p = p->next[c];
        }
        return p->end;
    }

    bool remove(string word) {
        word = normalize(word);
        TrieNode* p = root;
        vector<TrieNode*> path{root};

        for (char c : word) {
            if (!p->next.count(c)) return false;
            p = p->next[c];
            path.push_back(p);
        }

        if (!p->end) return false;
        p->end = false;

        for (int i = static_cast<int>(word.size()); i > 0; --i) {
            TrieNode* n = path[i];
            if (n->end || !n->next.empty()) break;
            path[i - 1]->next.erase(word[i - 1]);
            delete n;
        }
        return true;
    }

    int importFile(const string& filename) {
        ifstream file(filename);
        if (!file) return -1;
        string word;
        int count = 0;
        while (file >> word) {
            insert(word);
            ++count;
        }
        return count;
    }
};

// ======================== BENCHMARKS ========================

template<class Tree>
double insertTime(const vector<int>& data) {
    Tree tree;
    auto start = Clock::now();
    for (int x : data) tree.insert(x);
    return chrono::duration<double, milli>(Clock::now() - start).count();
}

template<class Tree>
double searchTime(const vector<int>& data,
                  const vector<int>& queries) {
    Tree tree;
    for (int x : data) tree.insert(x);

    volatile int found = 0;
    auto start = Clock::now();
    for (int x : queries) found += tree.search(x);

    return chrono::duration<double, milli>(Clock::now() - start).count();
}

void benchmarks() {
    mt19937 gen(random_device{}());
    cout << "\nТип данных | N | AVL вставка | BST вставка"
         << " | AVL поиск | BST поиск (мс)\n";

    for (int n : {1000, 5000, 10000}) {
        vector<int> randomData(n), sortedData(n);
        for (int i = 0; i < n; ++i)
            randomData[i] = sortedData[i] = i + 1;

        shuffle(randomData.begin(), randomData.end(), gen);

        vector<int> queries;
        for (int i = 1; i <= min(n, 1000); ++i)
            queries.push_back(i);

        for (int type = 0; type < 2; ++type) {
            const auto& data = type == 0 ? randomData : sortedData;
            cout << (type == 0 ? "Случайные" : "Отсортированные")
                 << " | " << n
                 << " | " << insertTime<AVLTree>(data)
                 << " | " << insertTime<BST>(data)
                 << " | " << searchTime<AVLTree>(data, queries)
                 << " | " << searchTime<BST>(data, queries) << '\n';
        }
    }
}

// ======================== MAIN ========================

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif

    cout << "ЛАБОРАТОРНАЯ РАБОТА №10\n"
         << "AVL-дерево и префиксное дерево Trie\n"
         << "Вариант 8. Продвинутый уровень\n";

    cout << "\n========== ЧАСТЬ 1. AVL ==========\n";
    AVLTree avl;
    for (int x : {30, 20, 40, 10, 25, 35, 50, 5, 15})
        avl.insert(x);

    cout << "Структура AVL:\n";
    avl.show();
    cout << "Высота: " << avl.height() << '\n';
    cout << "Поиск 25: " << (avl.search(25) ? "найден" : "нет") << '\n';
    cout << "LCA для 5 и 15: " << avl.lca(5, 15) << '\n';

    avl.remove(20);
    cout << "После удаления 20:\n";
    avl.show();

    cout << "\n========== ЧАСТЬ 2. TRIE ==========\n";
    Trie trie;
    for (string s : {"tree", "trie", "algorithm", "data", "structure"})
        trie.insert(s);

    cout << "Поиск tree: " << (trie.search("tree") ? "найдено" : "нет") << '\n';
    cout << "Поиск trie: " << (trie.search("trie") ? "найдено" : "нет") << '\n';
    cout << "Удаление tree: " << (trie.remove("tree") ? "успешно" : "нет") << '\n';
    cout << "Поиск tree после удаления: "
         << (trie.search("tree") ? "найдено" : "нет") << '\n';

    int count = trie.importFile("words.txt");
    if (count >= 0)
        cout << "Импортировано слов: " << count << '\n';
    else
        cout << "words.txt не найден, используются встроенные слова.\n";

    cout << "\n========== ЧАСТЬ 3. СРАВНЕНИЕ ==========\n";
    benchmarks();

    cout << "\nПрограмма завершена.\n";
    return 0;
}
