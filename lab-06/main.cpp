#include <algorithm>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <limits>
#include <string>
#include <vector>

using namespace std;

// Задача 1 варианта 8: рекурсивное вычисление факториала.
unsigned long long factorial(int n) {
    if (n < 0) {
        throw invalid_argument("Факториал отрицательного числа не определён");
    }
    if (n <= 1) {
        return 1;
    }
    return static_cast<unsigned long long>(n) * factorial(n - 1);
}

// Задача 2 варианта 8: вычисление числа сочетаний C(n, k).
unsigned long long combinations(int n, int k) {
    if (n < 0 || k < 0 || k > n) {
        throw invalid_argument("Требуется n >= 0 и 0 <= k <= n");
    }
    if (k == 0 || k == n) {
        return 1;
    }
    // Симметрия C(n, k) = C(n, n-k) уменьшает глубину рекурсии.
    if (k > n - k) {
        k = n - k;
    }
    return combinations(n - 1, k - 1) + combinations(n - 1, k);
}

// Задача 3 варианта 8: вывести все разложения n на положительные слагаемые.
// Слагаемые выводятся в невозрастающем порядке, чтобы не дублировать варианты.
void printPartitions(int remaining, int maxPart, vector<int>& current) {
    if (remaining == 0) {
        cout << "  ";
        for (size_t i = 0; i < current.size(); ++i) {
            if (i > 0) {
                cout << " + ";
            }
            cout << current[i];
        }
        cout << '\n';
        return;
    }

    for (int part = min(remaining, maxPart); part >= 1; --part) {
        current.push_back(part);
        printPartitions(remaining - part, part, current);
        current.pop_back();
    }
}

// Задача 4 варианта 8: рекурсивная генерация всех подмножеств.
void generateSubsets(
    const vector<int>& values,
    size_t index,
    vector<int>& current
) {
    if (index == values.size()) {
        cout << "  {";
        for (size_t i = 0; i < current.size(); ++i) {
            if (i > 0) {
                cout << ", ";
            }
            cout << current[i];
        }
        cout << "}\n";
        return;
    }

    // Ветка 1: не включать текущий элемент.
    generateSubsets(values, index + 1, current);

    // Ветка 2: включить текущий элемент.
    current.push_back(values[index]);
    generateSubsets(values, index + 1, current);
    current.pop_back();
}

// Пузырьковая сортировка с остановкой, если обменов больше нет.
void bubbleSort(vector<int>& values) {
    for (size_t i = 0; i + 1 < values.size(); ++i) {
        bool swapped = false;
        for (size_t j = 0; j + 1 < values.size() - i; ++j) {
            if (values[j] > values[j + 1]) {
                swap(values[j], values[j + 1]);
                swapped = true;
            }
        }
        if (!swapped) {
            break;
        }
    }
}

// Сортировка вставками. Сдвиги вместо обменов сохраняют устойчивость.
void insertionSort(vector<int>& values) {
    for (size_t i = 1; i < values.size(); ++i) {
        int key = values[i];
        size_t j = i;
        while (j > 0 && values[j - 1] > key) {
            values[j] = values[j - 1];
            --j;
        }
        values[j] = key;
    }
}

// Устойчивая сортировка выбором: минимум вставляется на место,
// а промежуточные элементы сдвигаются вправо, сохраняя порядок равных.
void stableSelectionSort(vector<int>& values) {
    for (size_t i = 0; i + 1 < values.size(); ++i) {
        size_t minIndex = i;
        for (size_t j = i + 1; j < values.size(); ++j) {
            if (values[j] < values[minIndex]) {
                minIndex = j;
            }
        }

        if (minIndex != i) {
            int minimum = values[minIndex];
            for (size_t j = minIndex; j > i; --j) {
                values[j] = values[j - 1];
            }
            values[i] = minimum;
        }
    }
}

void printVector(const vector<int>& values) {
    cout << '[';
    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) {
            cout << ", ";
        }
        cout << values[i];
    }
    cout << ']';
}

template <typename SortFunction>
long long measureMicroseconds(
    const vector<int>& source,
    SortFunction sortFunction
) {
    vector<int> copy = source;
    const auto start = chrono::steady_clock::now();
    sortFunction(copy);
    const auto finish = chrono::steady_clock::now();
    return chrono::duration_cast<chrono::microseconds>(
        finish - start
    ).count();
}

int main() {
    cout << "=== Лабораторная работа № 6 ===\n";
    cout << "Рекурсивные алгоритмы. Простые сортировки\n\n";

    cout << "1. Факториал:\n";
    for (int n : {0, 1, 5, 10}) {
        cout << "  " << n << "! = " << factorial(n) << '\n';
    }

    cout << "\n2. Число сочетаний:\n";
    cout << "  C(5, 2) = " << combinations(5, 2) << '\n';
    cout << "  C(8, 3) = " << combinations(8, 3) << '\n';

    cout << "\n3. Разложения числа 5 на положительные слагаемые:\n";
    vector<int> partition;
    printPartitions(5, 5, partition);

    cout << "\n4. Все подмножества множества {1, 2, 3}:\n";
    const vector<int> setValues{1, 2, 3};
    vector<int> subset;
    generateSubsets(setValues, 0, subset);

    cout << "\n5. Сравнение простых сортировок:\n";
    const vector<int> sample{7, 3, 9, 1, 5, 3, 8, 2};
    vector<int> bubble = sample;
    vector<int> insertion = sample;
    vector<int> selection = sample;

    bubbleSort(bubble);
    insertionSort(insertion);
    stableSelectionSort(selection);

    cout << "  Исходный массив: ";
    printVector(sample);
    cout << "\n  Bubble Sort:     ";
    printVector(bubble);
    cout << "\n  Insertion Sort:  ";
    printVector(insertion);
    cout << "\n  Stable Selection:";
    cout << ' ';
    printVector(selection);
    cout << "\n";

    cout << "\n6. Проверка устойчивой сортировки выбором:\n";
    // Проверка порядка равных ключей на парах (ключ, исходный индекс).
    // Основной алгоритм сортирует int; здесь демонстрируется принцип
    // устойчивости на ключах, имеющих одинаковые значения.
    vector<pair<int, char>> tagged{{2, 'A'}, {1, 'B'}, {2, 'C'}, {1, 'D'}};
    for (size_t i = 0; i + 1 < tagged.size(); ++i) {
        size_t minIndex = i;
        for (size_t j = i + 1; j < tagged.size(); ++j) {
            if (tagged[j].first < tagged[minIndex].first) {
                minIndex = j;
            }
        }
        if (minIndex != i) {
            auto minimum = tagged[minIndex];
            for (size_t j = minIndex; j > i; --j) {
                tagged[j] = tagged[j - 1];
            }
            tagged[i] = minimum;
        }
    }
    cout << "  Результат (ключ, метка): ";
    for (const auto& item : tagged) {
        cout << '(' << item.first << ',' << item.second << ") ";
    }
    cout << "\n  Равные ключи сохраняют исходный порядок: (1,B) перед (1,D), ";
    cout << "(2,A) перед (2,C).\n";

    cout << "\n7. Замер времени на массиве из 5000 элементов:\n";
    vector<int> data(5000);
    unsigned int seed = 42;
    for (int& value : data) {
        seed = seed * 1664525U + 1013904223U;
        value = static_cast<int>((seed >> 8) % 10000U);
    }

    cout << "  Bubble Sort: "
         << measureMicroseconds(data, bubbleSort) << " мкс\n";
    cout << "  Insertion Sort: "
         << measureMicroseconds(data, insertionSort) << " мкс\n";
    cout << "  Stable Selection Sort: "
         << measureMicroseconds(data, stableSelectionSort) << " мкс\n";

    cout << "\nПрограмма завершена.\n";
    return 0;
}
