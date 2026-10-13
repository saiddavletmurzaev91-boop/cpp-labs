#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using namespace std;
using Clock = chrono::steady_clock;

// Сортировка слиянием для строк в лексикографическом порядке.
void mergeStrings(vector<string>& values, size_t left, size_t mid, size_t right) {
    vector<string> merged;
    merged.reserve(right - left);

    size_t i = left;
    size_t j = mid;

    while (i < mid && j < right) {
        // При равенстве берём элемент из левой части: это сохраняет устойчивость.
        if (values[i] <= values[j]) {
            merged.push_back(values[i++]);
        } else {
            merged.push_back(values[j++]);
        }
    }

    while (i < mid) {
        merged.push_back(values[i++]);
    }
    while (j < right) {
        merged.push_back(values[j++]);
    }

    for (size_t k = 0; k < merged.size(); ++k) {
        values[left + k] = merged[k];
    }
}

void mergeSortStrings(vector<string>& values, size_t left, size_t right) {
    // Диапазон полуоткрытый: [left, right).
    if (right - left <= 1) {
        return;
    }

    const size_t mid = left + (right - left) / 2;
    mergeSortStrings(values, left, mid);
    mergeSortStrings(values, mid, right);
    mergeStrings(values, left, mid, right);
}

// Быстрая сортировка для целых чисел (для сравнительного анализа).
// Медиана из трёх уменьшает вероятность крайне неравномерного разбиения.
int partition(vector<int>& values, int low, int high) {
    const int middle = low + (high - low) / 2;
    if (values[low] > values[middle]) {
        swap(values[low], values[middle]);
    }
    if (values[low] > values[high]) {
        swap(values[low], values[high]);
    }
    if (values[middle] > values[high]) {
        swap(values[middle], values[high]);
    }
    swap(values[middle], values[high]);

    const int pivot = values[high];
    int i = low - 1;
    for (int j = low; j < high; ++j) {
        if (values[j] <= pivot) {
            ++i;
            swap(values[i], values[j]);
        }
    }

    swap(values[i + 1], values[high]);
    return i + 1;
}

// Рекурсивно обрабатываем меньшую часть, большую — циклом.
// Это ограничивает глубину стека O(log n), даже при неудачных разбиениях.
void quickSort(vector<int>& values, int low, int high) {
    while (low < high) {
        const int pivotIndex = partition(values, low, high);
        if (pivotIndex - low < high - pivotIndex) {
            quickSort(values, low, pivotIndex - 1);
            low = pivotIndex + 1;
        } else {
            quickSort(values, pivotIndex + 1, high);
            high = pivotIndex - 1;
        }
    }
}

// Восстановление свойства max-heap.
void heapify(vector<int>& values, int size, int root) {
    int largest = root;
    const int left = 2 * root + 1;
    const int right = 2 * root + 2;

    if (left < size && values[left] > values[largest]) {
        largest = left;
    }
    if (right < size && values[right] > values[largest]) {
        largest = right;
    }

    if (largest != root) {
        swap(values[root], values[largest]);
        heapify(values, size, largest);
    }
}

void heapSort(vector<int>& values) {
    const int size = static_cast<int>(values.size());

    for (int i = size / 2 - 1; i >= 0; --i) {
        heapify(values, size, i);
    }

    for (int i = size - 1; i > 0; --i) {
        swap(values[0], values[i]);
        heapify(values, i, 0);
    }
}

void printStrings(const vector<string>& values) {
    cout << '[';
    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) {
            cout << ", ";
        }
        cout << '"' << values[i] << '"';
    }
    cout << ']';
}

void printInts(const vector<int>& values) {
    cout << '[';
    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) {
            cout << ", ";
        }
        cout << values[i];
    }
    cout << ']';
}

template <typename Function>
long long measureMicroseconds(const vector<int>& source, Function sortFunction) {
    vector<int> values = source;
    const auto start = Clock::now();
    sortFunction(values);
    const auto finish = Clock::now();
    return chrono::duration_cast<chrono::microseconds>(finish - start).count();
}

int main() {
    cout << "=== Лабораторная работа № 7 ===\n";
    cout << "Быстрая сортировка, сортировка слиянием, пирамидальная сортировка\n\n";

    cout << "1. Вариант 8: сортировка строк слиянием\n";
    vector<string> words{
        "pear", "apple", "orange", "banana", "apple", "grape", "kiwi"
    };
    cout << "  До сортировки:    ";
    printStrings(words);
    mergeSortStrings(words, 0, words.size());
    cout << "\n  После сортировки: ";
    printStrings(words);
    cout << "\n";

    cout << "\n2. Проверка граничных случаев\n";
    vector<string> empty;
    mergeSortStrings(empty, 0, empty.size());
    vector<string> one{"only"};
    mergeSortStrings(one, 0, one.size());
    vector<string> duplicates{"pear", "pear", "apple", "apple"};
    mergeSortStrings(duplicates, 0, duplicates.size());
    cout << "  Пустой массив: ";
    printStrings(empty);
    cout << "\n  Один элемент:   ";
    printStrings(one);
    cout << "\n  Повторы:        ";
    printStrings(duplicates);
    cout << "\n";

    cout << "\n3. Сравнение трёх алгоритмов на одном массиве\n";
    vector<int> sample{64, 34, 25, 12, 22, 11, 90, 5};
    vector<int> quick = sample;
    vector<int> merge = sample;
    vector<int> heap = sample;

    quickSort(quick, 0, static_cast<int>(quick.size()) - 1);
    // Для сравнения здесь используется та же реализация слияния для int.
    // Локальная функция merge sort для int.
    const auto mergeSortInts = [](vector<int>& values, auto&& self,
                                  size_t left, size_t right) -> void {
        if (right - left <= 1) {
            return;
        }
        const size_t mid = left + (right - left) / 2;
        self(values, self, left, mid);
        self(values, self, mid, right);
        vector<int> temp;
        temp.reserve(right - left);
        size_t i = left;
        size_t j = mid;
        while (i < mid && j < right) {
            if (values[i] <= values[j]) {
                temp.push_back(values[i++]);
            } else {
                temp.push_back(values[j++]);
            }
        }
        while (i < mid) temp.push_back(values[i++]);
        while (j < right) temp.push_back(values[j++]);
        for (size_t k = 0; k < temp.size(); ++k) {
            values[left + k] = temp[k];
        }
    };
    mergeSortInts(merge, mergeSortInts, 0, merge.size());
    heapSort(heap);

    cout << "  Исходный массив: ";
    printInts(sample);
    cout << "\n  Quick Sort:      ";
    printInts(quick);
    cout << "\n  Merge Sort:      ";
    printInts(merge);
    cout << "\n  Heap Sort:       ";
    printInts(heap);
    cout << "\n";

    cout << "\n4. Замер времени на почти отсортированных массивах\n";
    const vector<int> sizes{100, 1000, 10000};
    mt19937 generator(42);

    cout << "  N | Quick Sort (мкс) | Merge Sort (мкс) | Heap Sort (мкс)\n";
    cout << "  ---------------------------------------------------------\n";
    for (int n : sizes) {
        vector<int> data(n);
        for (int i = 0; i < n; ++i) {
            data[i] = i;
        }

        // Небольшое число соседних перестановок создаёт почти отсортированный массив.
        for (int i = 0; i < max(1, n / 100); ++i) {
            uniform_int_distribution<int> distribution(0, n - 2);
            const int index = distribution(generator);
            swap(data[index], data[index + 1]);
        }

        const auto quickTime = measureMicroseconds(data, [](vector<int>& values) {
            quickSort(values, 0, static_cast<int>(values.size()) - 1);
        });
        const auto mergeTime = measureMicroseconds(data, [](vector<int>& values) {
            const auto sortInts = [](vector<int>& items, auto&& self,
                                     size_t left, size_t right) -> void {
                if (right - left <= 1) return;
                const size_t mid = left + (right - left) / 2;
                self(items, self, left, mid);
                self(items, self, mid, right);
                vector<int> temp;
                temp.reserve(right - left);
                size_t i = left, j = mid;
                while (i < mid && j < right) {
                    if (items[i] <= items[j]) temp.push_back(items[i++]);
                    else temp.push_back(items[j++]);
                }
                while (i < mid) temp.push_back(items[i++]);
                while (j < right) temp.push_back(items[j++]);
                for (size_t k = 0; k < temp.size(); ++k) items[left + k] = temp[k];
            };
            sortInts(values, sortInts, 0, values.size());
        });
        const auto heapTime = measureMicroseconds(data, heapSort);
        cout << "  " << n << " | " << quickTime << " | "
             << mergeTime << " | " << heapTime << '\n';
    }

    cout << "\nПрограмма завершена.\n";
    return 0;
}
