# Лабораторная работа № 7
## Алгоритмы сортировки. Вариант 8

**Студент:** Давлетмурзаев Сайд-Эмин Тимурович  
**Группа:** ПИЖ-б-о-25-1  
**Преподаватель:** Белик Ангелина Игоревна  
**Вариант:** 8  
**Язык программирования:** C++17

---

## 1. Цель работы

Изучить принципы работы алгоритмов сортировки, реализовать несколько алгоритмов на C++, сравнить их временную сложность и производительность, а также выполнить внешнюю сортировку слиянием для обработки больших файлов.

## 2. Теоретическая часть

### 2.1. Быстрая сортировка (Quick Sort)

Быстрая сортировка выбирает опорный элемент и переставляет элементы так, чтобы меньшие значения оказались с одной стороны, а большие — с другой. Затем обе части сортируются отдельно. В данной реализации опорный элемент выбирается методом медианы трёх: сравниваются первый, средний и последний элементы.

- Лучший случай: `O(n log n)`.
- Средний случай: `O(n log n)`.
- Худший случай: `O(n²)`.
- Дополнительная память: в среднем `O(log n)` для стека вызовов.

### 2.2. Сортировка слиянием (Merge Sort)

Сортировка слиянием делит массив на две части, рекурсивно сортирует их и объединяет в одну упорядоченную последовательность. Для объединения используется временный буфер.

- Лучший, средний и худший случаи: `O(n log n)`.
- Дополнительная память: `O(n)`.

### 2.3. Пирамидальная сортировка (Heap Sort)

Пирамидальная сортировка строит двоичную кучу, после чего последовательно переносит максимальный элемент в конец массива и восстанавливает свойство кучи.

- Лучший, средний и худший случаи: `O(n log n)`.
- Дополнительная память: `O(1)` для итеративной реализации.

### 2.4. Внешняя сортировка слиянием

Внешняя сортировка используется, когда весь набор данных не помещается в оперативную память. Файл разбивается на блоки ограниченного размера. Каждый блок сортируется отдельно и сохраняется на диск, после чего отсортированные блоки объединяются в выходной файл. При слиянии нескольких файлов можно использовать приоритетную очередь, позволяющую выбирать минимальный из текущих элементов.

Если размер блока равен `M`, а объём данных — `N`, количество начальных блоков примерно равно `ceil(N / M)`. Внешняя сортировка выполняет последовательные чтения и записи на диск, поэтому её скорость зависит не только от вычислений, но и от дисковой подсистемы.

## 3. Постановка задачи

Согласно варианту 8 требуется реализовать внешнюю сортировку слиянием для обработки больших файлов.

В рамках работы необходимо:
1. Реализовать быструю сортировку, сортировку слиянием и пирамидальную сортировку.
2. Реализовать внешнюю сортировку слиянием числового файла с ограниченным размером блока.
3. Предусмотреть генерацию тестового файла.
4. Измерить время выполнения алгоритмов на наборах разного размера.
5. Проверить корректность результата: упорядоченность элементов и сохранение их количества.
6. Сопоставить результаты с теоретической сложностью алгоритмов.

## 4. Листинг программы с комментариями

Полный исходный код `main.cpp` приведён ниже. Комментарии в коде обозначают реализованные алгоритмы и ключевые этапы обработки данных.

```cpp
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <queue>
#include <random>
#include <string>
#include <vector>

using namespace std;
using Clock = chrono::steady_clock;
namespace fs = std::filesystem;

// -------------------- Merge Sort --------------------
void mergeSort(vector<int>& values, size_t left, size_t right) {
    if (right - left <= 1) return;
    const size_t mid = left + (right - left) / 2;
    mergeSort(values, left, mid);
    mergeSort(values, mid, right);
    vector<int> temp;
    temp.reserve(right - left);
    size_t i = left, j = mid;
    while (i < mid && j < right) {
        if (values[i] <= values[j]) temp.push_back(values[i++]);
        else temp.push_back(values[j++]);
    }
    while (i < mid) temp.push_back(values[i++]);
    while (j < right) temp.push_back(values[j++]);
    for (size_t k = 0; k < temp.size(); ++k) values[left + k] = temp[k];
}

// -------------------- Quick Sort --------------------
int partition(vector<int>& values, int low, int high) {
    // Медиана из первого, среднего и последнего элементов — опорный элемент.
    const int mid = low + (high - low) / 2;
    if (values[low] > values[mid]) swap(values[low], values[mid]);
    if (values[low] > values[high]) swap(values[low], values[high]);
    if (values[mid] > values[high]) swap(values[mid], values[high]);
    swap(values[mid], values[high]);
    const int pivot = values[high];
    int i = low - 1;
    for (int j = low; j < high; ++j) {
        if (values[j] <= pivot) swap(values[++i], values[j]);
    }
    swap(values[i + 1], values[high]);
    return i + 1;
}

// Рекурсивно сортируем меньшую часть, большую обрабатываем циклом.
void quickSort(vector<int>& values, int low, int high) {
    while (low < high) {
        const int p = partition(values, low, high);
        if (p - low < high - p) {
            quickSort(values, low, p - 1);
            low = p + 1;
        } else {
            quickSort(values, p + 1, high);
            high = p - 1;
        }
    }
}

// -------------------- Heap Sort --------------------
void heapify(vector<int>& values, int n, int root) {
    while (true) {
        int largest = root;
        const int left = 2 * root + 1;
        const int right = 2 * root + 2;
        if (left < n && values[left] > values[largest]) largest = left;
        if (right < n && values[right] > values[largest]) largest = right;
        if (largest == root) return;
        swap(values[root], values[largest]);
        root = largest;
    }
}

void heapSort(vector<int>& values) {
    const int n = static_cast<int>(values.size());
    for (int i = n / 2 - 1; i >= 0; --i) heapify(values, n, i);
    for (int i = n - 1; i > 0; --i) {
        swap(values[0], values[i]);
        heapify(values, i, 0);
    }
}

// -------------------- Внешняя сортировка слиянием --------------------
// Сортирует файл целых чисел, не загружая его целиком в память.
// Слияние выполняется ограниченными группами, чтобы не открывать
// сотни или тысячи файлов одновременно (важно для Windows).
bool mergeGroup(const vector<fs::path>& inputs, const fs::path& outputPath) {
    struct Node {
        int value;
        size_t fileIndex;
    };
    auto greaterNode = [](const Node& a, const Node& b) {
        return a.value > b.value;
    };
    priority_queue<Node, vector<Node>, decltype(greaterNode)> queue(greaterNode);
    vector<ifstream> files(inputs.size());

    for (size_t i = 0; i < inputs.size(); ++i) {
        files[i].open(inputs[i]);
        if (!files[i]) {
            cerr << "Не удалось открыть временный файл: " << inputs[i].string() << '\n';
            return false;
        }
        int value;
        if (files[i] >> value) queue.push({value, i});
        else if (!files[i].eof()) {
            cerr << "Ошибка чтения временного файла: " << inputs[i].string() << '\n';
            return false;
        }
    }

    ofstream output(outputPath, ios::trunc);
    if (!output) {
        cerr << "Не удалось создать файл: " << outputPath.string() << '\n';
        return false;
    }

    while (!queue.empty()) {
        const Node current = queue.top();
        queue.pop();
        output << current.value << '\n';
        if (!output) {
            cerr << "Ошибка записи в файл: " << outputPath.string() << '\n';
            return false;
        }

        int nextValue;
        if (files[current.fileIndex] >> nextValue) {
            queue.push({nextValue, current.fileIndex});
        } else if (!files[current.fileIndex].eof()) {
            cerr << "Ошибка чтения временного файла: "
                 << inputs[current.fileIndex].string() << '\n';
            return false;
        }
    }
    output.close();
    if (!output) {
        cerr << "Ошибка завершения записи файла: " << outputPath.string() << '\n';
        return false;
    }
    return true;
}

bool externalMergeSort(const string& inputName, const string& outputName,
                       size_t chunkSize) {
    if (chunkSize == 0) {
        cerr << "Размер блока должен быть больше нуля.\n";
        return false;
    }
    if (fs::absolute(inputName).lexically_normal() ==
        fs::absolute(outputName).lexically_normal()) {
        cerr << "Входной и выходной файлы должны иметь разные имена.\n";
        return false;
    }

    ifstream input(inputName);
    if (!input) {
        cerr << "Не удалось открыть входной файл: " << inputName << '\n';
        return false;
    }

    const fs::path tempDir = "merge_temp";
    error_code ec;
    fs::remove_all(tempDir, ec);
    ec.clear();
    fs::create_directories(tempDir, ec);
    if (ec) {
        cerr << "Не удалось создать временную папку: " << ec.message() << '\n';
        return false;
    }

    vector<fs::path> runs;
    vector<int> block;
    block.reserve(chunkSize);
    int value;
    while (input >> value) {
        block.push_back(value);
        if (block.size() == chunkSize) {
            mergeSort(block, 0, block.size());
            const fs::path runPath = tempDir / ("run_" + to_string(runs.size()) + ".txt");
            ofstream runOut(runPath);
            if (!runOut) {
                cerr << "Не удалось создать временный файл: " << runPath.string() << '\n';
                return false;
            }
            for (int number : block) runOut << number << '\n';
            runOut.close();
            if (!runOut) {
                cerr << "Ошибка записи временного файла: " << runPath.string() << '\n';
                return false;
            }
            runs.push_back(runPath);
            block.clear();
        }
    }
    if (!input.eof()) {
        cerr << "Ошибка чтения: входной файл должен содержать целые числа.\n";
        return false;
    }
    if (!block.empty()) {
        mergeSort(block, 0, block.size());
        const fs::path runPath = tempDir / ("run_" + to_string(runs.size()) + ".txt");
        ofstream runOut(runPath);
        if (!runOut) {
            cerr << "Не удалось создать временный файл: " << runPath.string() << '\n';
            return false;
        }
        for (int number : block) runOut << number << '\n';
        runOut.close();
        if (!runOut) {
            cerr << "Ошибка записи временного файла: " << runPath.string() << '\n';
            return false;
        }
        runs.push_back(runPath);
    }
    input.close();

    // Не открываем больше 64 файлов за один проход.
    constexpr size_t MAX_FAN_IN = 64;
    size_t passNumber = 0;
    while (runs.size() > 1) {
        vector<fs::path> nextRuns;
        for (size_t begin = 0; begin < runs.size(); begin += MAX_FAN_IN) {
            const size_t end = min(begin + MAX_FAN_IN, runs.size());
            vector<fs::path> group(runs.begin() + begin, runs.begin() + end);
            const fs::path mergedPath = tempDir /
                ("pass_" + to_string(passNumber) + "_" + to_string(nextRuns.size()) + ".txt");
            if (!mergeGroup(group, mergedPath)) return false;
            nextRuns.push_back(mergedPath);
            for (const auto& oldPath : group) fs::remove(oldPath, ec);
        }
        runs = move(nextRuns);
        ++passNumber;
    }

    // Переносим итоговый run в выходной файл. Пустой вход даёт пустой выход.
    const fs::path outputPath = fs::absolute(outputName);
    if (runs.empty()) {
        ofstream emptyOutput(outputPath, ios::trunc);
        if (!emptyOutput) {
            cerr << "Не удалось создать выходной файл: " << outputName << '\n';
            return false;
        }
    } else {
        error_code copyError;
        fs::copy_file(runs.front(), outputPath, fs::copy_options::overwrite_existing, copyError);
        if (copyError) {
            cerr << "Не удалось сохранить результат: " << copyError.message() << '\n';
            return false;
        }
    }

    fs::remove_all(tempDir, ec);
    return true;
}

template <typename SortFunction>
long long measureUs(const vector<int>& source, SortFunction sortFunction) {
    vector<int> copy = source;
    const auto start = Clock::now();
    sortFunction(copy);
    const auto finish = Clock::now();
    return chrono::duration_cast<chrono::microseconds>(finish - start).count();
}

void createExampleInput(const string& filename, size_t count) {
    ofstream out(filename);
    mt19937 rng(42);
    uniform_int_distribution<int> dist(-100000, 100000);
    for (size_t i = 0; i < count; ++i) out << dist(rng) << '\n';
}

int main(int argc, char* argv[]) {
    cout << "=== Лабораторная работа № 7 ===\n";
    cout << "Quick Sort, Merge Sort, Heap Sort и внешняя сортировка слиянием\n\n";

    if (argc == 4 && string(argv[1]) == "--make-test") {
        try { createExampleInput(argv[2], stoull(argv[3])); }
        catch (...) { cerr << "Неверное количество чисел.\n"; return 1; }
        cout << "Создан файл " << argv[2] << " с " << argv[3] << " числами.\n";
        return 0;
    }

    // Режим внешней сортировки: ./lab7 input.txt output.txt [размер_блока]
    if (argc >= 3) {
        size_t chunkSize = 100000;
        if (argc >= 4) {
            try { chunkSize = stoull(argv[3]); }
            catch (...) { cerr << "Неверный размер блока.\n"; return 1; }
        }
        const auto start = Clock::now();
        const bool success = externalMergeSort(argv[1], argv[2], chunkSize);
        const auto finish = Clock::now();
        if (!success) return 1;
        cout << "Внешняя сортировка завершена. Результат: " << argv[2] << '\n';
        cout << "Размер блока: " << chunkSize << " чисел\n";
        cout << "Время: " << chrono::duration_cast<chrono::milliseconds>(finish - start).count()
             << " мс\n";
        return 0;
    }

    cout << "Режим демонстрации. Для внешней сортировки используйте:\n";
    cout << "  ./lab7 numbers.txt sorted.txt 1000\n";
    cout << "Если numbers.txt отсутствует, создайте тестовый файл командой:\n";
    cout << "  ./lab7 --make-test numbers.txt 1000000\n\n";

    vector<int> sample{64, 34, 25, 12, 22, 11, 90, 5};
    vector<int> quick = sample, merged = sample, heap = sample;
    quickSort(quick, 0, static_cast<int>(quick.size()) - 1);
    mergeSort(merged, 0, merged.size());
    heapSort(heap);
    auto print = [](const vector<int>& v) {
        cout << '[';
        for (size_t i = 0; i < v.size(); ++i) cout << (i ? ", " : "") << v[i];
        cout << ']';
    };
    cout << "Проверка трёх алгоритмов на одном массиве:\n  Исходный: "; print(sample);
    cout << "\n  Quick:    "; print(quick);
    cout << "\n  Merge:    "; print(merged);
    cout << "\n  Heap:     "; print(heap);
    cout << "\n\nСравнение времени на почти отсортированных данных (мкс):\n";
    cout << "N\tQuick Sort\tMerge Sort\tHeap Sort\n";
    mt19937 rng(42);
    for (int n : {100, 1000, 10000}) {
        vector<int> data(n);
        for (int i = 0; i < n; ++i) data[i] = i;
        for (int k = 0; k < max(1, n / 100); ++k) {
            uniform_int_distribution<int> dist(0, n - 2);
            int index = dist(rng);
            swap(data[index], data[index + 1]);
        }
        cout << n << '\t'
             << measureUs(data, [](vector<int>& v) { quickSort(v, 0, static_cast<int>(v.size()) - 1); }) << '\t'
             << measureUs(data, [](vector<int>& v) { mergeSort(v, 0, v.size()); }) << '\t'
             << measureUs(data, [](vector<int>& v) { heapSort(v); }) << '\n';
    }
    return 0;
}
```

## 5. Результаты работы

### 5.1. Время выполнения алгоритмов

В таблице приведены результаты измерения времени на почти отсортированных данных, полученные при одном запуске программы.

| Количество элементов | Quick Sort, мкс | Merge Sort, мкс | Heap Sort, мкс |
|---:|---:|---:|---:|
| 100 | 1 | 6 | 3 |
| 1 000 | 10 | 63 | 33 |
| 10 000 | 74 | 605 | 438 |

Время зависит от процессора, параметров компиляции, состояния системы и характера входных данных. Поэтому значения являются результатами конкретного тестирования, а не универсальными константами.

### 5.2. Внешняя сортировка

Для проверки внешней сортировки использовался файл из 1 000 000 целых чисел с размером блока 1 000 элементов.

| Показатель | Результат |
|---|---:|
| Количество чисел во входном файле | 1 000 000 |
| Размер блока | 1 000 элементов |
| Количество чисел в выходном файле | 1 000 000 |
| Время выполнения | 852 мс |
| Проверка порядка возрастания | Пройдена |

После сортировки число элементов во входном и выходном файлах совпало. Проверка последовательности подтвердила порядок возрастания.

### 5.3. Скриншот

![Результаты запуска и проверки сортировки](07-sorting-results.png)


## 6. Анализ сложности каждого алгоритма

| Алгоритм | Лучший случай | Средний случай | Худший случай | Дополнительная память |
|---|---|---|---|---|
| Quick Sort | `O(n log n)` | `O(n log n)` | `O(n²)` | В среднем `O(log n)` |
| Merge Sort | `O(n log n)` | `O(n log n)` | `O(n log n)` | `O(n)` |
| Heap Sort | `O(n log n)` | `O(n log n)` | `O(n log n)` | `O(1)` |
| Внешняя сортировка слиянием | Зависит от размера данных и числа проходов | Зависит от размера данных и числа проходов | Зависит от размера данных и числа проходов | Буфер блока и буферы слияния |

У быстрой сортировки хорошая средняя производительность, но при неудачном разбиении возможно квадратичное время. Сортировка слиянием гарантирует `O(n log n)`, но требует дополнительную память. Пирамидальная сортировка также гарантирует `O(n log n)` и использует постоянную дополнительную память.

Внешняя сортировка слиянием предназначена для файлов, не помещающихся в оперативную память. Её затраты определяются числом проходов по данным и объёмом дискового ввода-вывода. Точное число проходов зависит от размера блока, объёма файла и ограничения на количество одновременно объединяемых серий.

## 7. Сравнительная таблица производительности

| Критерий | Quick Sort | Merge Sort | Heap Sort | Внешняя сортировка слиянием |
|---|---|---|---|---|
| Средняя временная сложность | `O(n log n)` | `O(n log n)` | `O(n log n)` | Зависит от числа дисковых проходов |
| Худший случай | `O(n²)` | `O(n log n)` | `O(n log n)` | Зависит от объёма данных и проходов |
| Дополнительная память | Стек рекурсии | `O(n)` | `O(1)` | Ограниченные блоки и буферы |
| Работа с данными больше RAM | Нет, массив должен быть в памяти | Обычно нет в данной реализации | Нет, массив должен быть в памяти | Да |
| Основное преимущество | Хорошая средняя скорость | Предсказуемая сложность | Низкая дополнительная память | Обработка больших файлов |

На приведённом тесте Quick Sort показал наименьшее время. Это не означает, что он всегда быстрее: результат зависит от размера и распределения входных данных. Для файлов, превышающих объём оперативной памяти, предназначена внешняя сортировка.

## 8. Ответы на контрольные вопросы

### 1. Что такое сортировка?
Сортировка — упорядочивание элементов по заданному признаку, например по возрастанию числового значения.

### 2. Как работает быстрая сортировка?
Выбирается опорный элемент, массив разделяется на части относительно него, после чего части сортируются отдельно.

### 3. Какова средняя сложность Quick Sort?
`O(n log n)`. В худшем случае сложность может достигать `O(n²)`.

### 4. Что даёт выбор медианы трёх?
Выбор медианы первого, среднего и последнего элементов помогает уменьшить вероятность неудачного выбора опорного элемента на некоторых типах входных данных, но не исключает худший случай `O(n²)`.

### 5. Как работает Merge Sort?
Массив делится на части, части сортируются рекурсивно, затем объединяются в отсортированную последовательность.

### 6. Какова сложность Merge Sort?
`O(n log n)` по времени и `O(n)` дополнительной памяти для обычной реализации с буфером.

### 7. Что такое двоичная куча?
Это структура данных в виде почти полного бинарного дерева, удовлетворяющая свойству кучи. В max-heap значение родителя не меньше значений его детей.

### 8. Как работает Heap Sort?
Строится max-heap, максимальный элемент меняется местами с последним элементом неотсортированной части, после чего куча восстанавливается.

### 9. Какова сложность Heap Sort?
`O(n log n)` по времени и `O(1)` дополнительной памяти для итеративной реализации.

### 10. Когда применяется внешняя сортировка?
Когда данные слишком велики, чтобы целиком разместить их в оперативной памяти.

### 11. Что такое серия при внешней сортировке?
Серия — отдельный блок данных, который был отсортирован в памяти и записан во временный файл.

### 12. Для чего используется слияние серий?
Оно объединяет несколько отсортированных файлов в более крупный отсортированный файл, постепенно формируя итоговый результат.

### 13. Зачем нужна приоритетная очередь?
Она позволяет выбирать минимальный из текущих элементов объединяемых серий, не просматривая все элементы каждый раз.

### 14. Почему ограничивается число одновременно открытых файлов?
Операционная система ограничивает число файловых дескрипторов. Ограничение числа серий также помогает контролировать использование системных ресурсов.

### 15. Как проверить правильность результата?
Нужно убедиться, что каждый следующий элемент не меньше предыдущего, а количество элементов до и после сортировки одинаково.

## 9. Выводы

В лабораторной работе реализованы быстрая сортировка, сортировка слиянием и пирамидальная сортировка, а также внешняя сортировка слиянием для числовых файлов. Проведены измерения времени на наборах разного размера и проверена корректность результата для файла из 1 000 000 чисел.

Сравнение показало, что на использованных почти отсортированных данных Quick Sort выполнил сортировку быстрее остальных рассмотренных алгоритмов. Теоретический анализ позволил сопоставить результаты с асимптотической сложностью. Внешняя сортировка демонстрирует подход к обработке больших наборов данных с использованием временных файлов и ограниченного объёма оперативной памяти.

## 10. Ссылка на Git-репозиторий

[https://github.com/saiddavletmurzaev91-boop/cpp-labs](https://github.com/saiddavletmurzaev91-boop/cpp-labs)
