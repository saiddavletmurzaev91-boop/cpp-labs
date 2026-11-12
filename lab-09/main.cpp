#include <climits>
#include <algorithm>
#include <iostream>
#include <limits>
#include <queue>
#include <random>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

struct Node {
    int data;
    Node* left = nullptr;
    Node* right = nullptr;

    explicit Node(int value) : data(value) {}
};

// Визуализация дерева.
void printTree(Node* node, int space = 0) {
    if (!node) return;
    printTree(node->right, space + 5);
    cout << string(space, ' ') << node->data << '\n';
    printTree(node->left, space + 5);
}

// Проверка корректности BST.
bool validateBST(Node* node, long long min = LLONG_MIN,
                 long long max = LLONG_MAX) {
    if (!node) return true;
    if (node->data <= min || node->data >= max) return false;
    return validateBST(node->left, min, node->data) &&
           validateBST(node->right, node->data, max);
}

// Освобождение памяти.
void clear(Node* node) {
    if (!node) return;
    clear(node->left);
    clear(node->right);
    delete node;
}

// ==================================================
// ОБЫЧНОЕ БИНАРНОЕ ДЕРЕВО
// ==================================================

class BinaryTree {
    Node* root = nullptr;

    void preOrder(Node* n) {
        if (!n) return;
        cout << n->data << ' ';
        preOrder(n->left);
        preOrder(n->right);
    }

    void inOrder(Node* n) {
        if (!n) return;
        inOrder(n->left);
        cout << n->data << ' ';
        inOrder(n->right);
    }

    void postOrder(Node* n) {
        if (!n) return;
        postOrder(n->left);
        postOrder(n->right);
        cout << n->data << ' ';
    }

    bool degenerate(Node* n) {
        if (!n) return true;
        if (n->left && n->right) return false;
        return degenerate(n->left) && degenerate(n->right);
    }

public:
    BinaryTree() {
        root = new Node(10);
        root->left = new Node(5);
        root->right = new Node(15);
        root->left->left = new Node(3);
        root->left->right = new Node(7);
        root->right->left = new Node(12);
        root->right->right = new Node(18);
    }

    ~BinaryTree() { clear(root); }

    void show() {
        cout << "\nСтруктура дерева:\n";
        printTree(root);

        cout << "\nPreOrder: ";
        preOrder(root);

        cout << "\nInOrder: ";
        inOrder(root);

        cout << "\nPostOrder: ";
        postOrder(root);

        cout << "\nLevelOrder: ";
        queue<Node*> q;
        q.push(root);

        while (!q.empty()) {
            Node* n = q.front();
            q.pop();
            cout << n->data << ' ';
            if (n->left) q.push(n->left);
            if (n->right) q.push(n->right);
        }

        cout << "\nВырожденное дерево: "
             << (degenerate(root) ? "Да" : "Нет");
        cout << "\nЯвляется BST: "
             << (validateBST(root) ? "Да" : "Нет") << '\n';
    }
};

// ==================================================
// БИНАРНОЕ ДЕРЕВО ПОИСКА
// ==================================================

class BST {
    Node* root = nullptr;

    Node* insert(Node* n, int value) {
        if (!n) return new Node(value);
        if (value < n->data) n->left = insert(n->left, value);
        else if (value > n->data) n->right = insert(n->right, value);
        return n;
    }

    bool search(Node* n, int value) {
        if (!n) return false;
        if (n->data == value) return true;
        return value < n->data
            ? search(n->left, value)
            : search(n->right, value);
    }

    Node* remove(Node* n, int value) {
        if (!n) return nullptr;

        if (value < n->data) n->left = remove(n->left, value);
        else if (value > n->data) n->right = remove(n->right, value);
        else {
            // Удаление листа.
            if (!n->left && !n->right) {
                delete n;
                return nullptr;
            }

            // Удаление узла с одним потомком.
            if (!n->left || !n->right) {
                Node* child = n->left ? n->left : n->right;
                delete n;
                return child;
            }

            // Удаление узла с двумя потомками.
            Node* successor = n->right;
            while (successor->left) successor = successor->left;
            n->data = successor->data;
            n->right = remove(n->right, successor->data);
        }
        return n;
    }

    int height(Node* n) {
        if (!n) return 0;
        return 1 + max(height(n->left), height(n->right));
    }

    void inOrder(Node* n) {
        if (!n) return;
        inOrder(n->left);
        cout << n->data << ' ';
        inOrder(n->right);
    }

public:
    ~BST() { clear(root); }

    void insert(int value) { root = insert(root, value); }
    bool search(int value) { return search(root, value); }
    void remove(int value) { root = remove(root, value); }
    int height() { return height(root); }

    bool valid() { return validateBST(root); }

    void show() {
        if (!root) {
            cout << "Дерево пустое.\n";
            return;
        }
        printTree(root);
    }

    void sorted() {
        if (!root) {
            cout << "Дерево пустое.\n";
            return;
        }
        inOrder(root);
        cout << '\n';
    }

    bool minMax(int& minimum, int& maximum) {
        if (!root) return false;

        Node* minNode = root;
        Node* maxNode = root;

        while (minNode->left) minNode = minNode->left;
        while (maxNode->right) maxNode = maxNode->right;

        minimum = minNode->data;
        maximum = maxNode->data;
        return true;
    }
};

// ==================================================
// ГЛАВНАЯ ПРОГРАММА
// ==================================================

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif

    cout << "ЛАБОРАТОРНАЯ РАБОТА №9\n";
    cout << "Бинарное дерево. Обходы. BST\n";
    cout << "Вариант 8. Продвинутый уровень\n";

    // Часть 1. Обычное бинарное дерево.
    cout << "\n========== ЧАСТЬ 1 ==========\n";
    BinaryTree binaryTree;
    binaryTree.show();

    // Часть 2. Операции с BST.
    cout << "\n========== ЧАСТЬ 2 ==========\n";
    BST example;
    for (int x : {50, 30, 70, 20, 40, 60, 80})
        example.insert(x);

    cout << "Исходное дерево:\n";
    example.show();

    cout << "InOrder: ";
    example.sorted();

    cout << "Поиск числа 40: "
         << (example.search(40) ? "Найдено" : "Не найдено") << '\n';

    cout << "Проверка BST: "
         << (example.valid() ? "Корректное" : "Некорректное") << '\n';

    cout << "Удаление числа 30:\n";
    example.remove(30);
    example.show();

    // Часть 3. Дерево из 20 случайных чисел.
    cout << "\n========== ЧАСТЬ 3 ==========\n";
    vector<int> numbers(100);
    for (int i = 0; i < 100; ++i) numbers[i] = i + 1;

    random_device rd;
    mt19937 gen(rd());
    shuffle(numbers.begin(), numbers.end(), gen);

    BST tree;
    cout << "Случайные числа:\n";

    for (int i = 0; i < 20; ++i) {
        cout << numbers[i] << ' ';
        tree.insert(numbers[i]);
    }

    cout << "\n\nПостроенное дерево:\n";
    tree.show();

    cout << "\nОтсортированная последовательность:\n";
    tree.sorted();

    int minimum, maximum;
    if (tree.minMax(minimum, maximum)) {
        cout << "Минимум: " << minimum << '\n';
        cout << "Максимум: " << maximum << '\n';
    }

    cout << "Высота дерева: " << tree.height() << '\n';
    cout << "Проверка BST: "
         << (tree.valid() ? "Да" : "Нет") << '\n';

    int value;
    cout << "\nВведите число для удаления: ";

    if (!(cin >> value)) {
        cout << "Ошибка ввода.\n";
        return 1;
    }

    if (tree.search(value)) {
        tree.remove(value);
        cout << "Элемент удалён.\n";
    } else {
        cout << "Элемент не найден.\n";
    }

    cout << "\nДерево после удаления:\n";
    tree.show();

    cout << "InOrder после удаления: ";
    tree.sorted();

    cout << "Новая высота: " << tree.height() << '\n';
    cout << "Проверка BST после удаления: "
         << (tree.valid() ? "Да" : "Нет") << '\n';

    cout << "\nПрограмма завершена.\n";
    return 0;
}
