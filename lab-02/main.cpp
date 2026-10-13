
#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <functional>
#include <stdexcept>
#include <utility>
#include <unordered_map>
#include <chrono>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#endif

template <typename K, typename V>
class HashMap {
private:
    std::vector<std::list<std::pair<K, V>>> buckets_;
    size_t size_;

    size_t getIndex(const K& key) const {
        return std::hash<K>{}(key) % buckets_.size();
    }

    void rehash() {
        size_t newCapacity = buckets_.size() * 2;
        std::vector<std::list<std::pair<K, V>>> newBuckets(
            newCapacity
        );

        for (const auto& bucket : buckets_) {
            for (const auto& item : bucket) {
                size_t index =
                    std::hash<K>{}(item.first) % newCapacity;
                newBuckets[index].push_back(item);
            }
        }

        buckets_ = std::move(newBuckets);
    }

public:
    HashMap() : buckets_(8), size_(0) {}

    void put(const K& key, const V& value) {
        size_t index = getIndex(key);

        for (auto& item : buckets_[index]) {
            if (item.first == key) {
                item.second = value;
                return;
            }
        }

        buckets_[index].push_back({key, value});
        ++size_;

        if (size_ > buckets_.size() * 0.75) {
            rehash();
        }
    }

    V get(const K& key) const {
        size_t index = getIndex(key);

        for (const auto& item : buckets_[index]) {
            if (item.first == key) {
                return item.second;
            }
        }

        throw std::out_of_range("Ключ не найден");
    }

    bool containsKey(const K& key) const {
        size_t index = getIndex(key);

        for (const auto& item : buckets_[index]) {
            if (item.first == key) {
                return true;
            }
        }

        return false;
    }

    bool remove(const K& key) {
        size_t index = getIndex(key);

        for (auto it = buckets_[index].begin();
             it != buckets_[index].end(); ++it) {
            if (it->first == key) {
                buckets_[index].erase(it);
                --size_;
                return true;
            }
        }

        return false;
    }

    size_t size() const {
        return size_;
    }
};

template <typename Function>
long long measureTime(Function function) {
    auto start = std::chrono::high_resolution_clock::now();
    function();
    auto finish = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<
        std::chrono::microseconds
    >(finish - start).count();
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    std::cout << "=== Лабораторная работа № 2 ===\n";
    std::cout << "Вариант 8. HashMap<K, V>\n\n";

    HashMap<int, std::string> map;

    std::cout << "Основные операции:\n";

    map.put(1, "Иван");
    map.put(2, "Мария");
    map.put(3, "Пётр");

    std::cout << "Размер: " << map.size() << '\n';
    std::cout << "Значение по ключу 2: "
              << map.get(2) << '\n';

    std::cout << "Ключ 3 существует: "
              << (map.containsKey(3) ? "да" : "нет")
              << '\n';

    map.put(2, "Анна");

    std::cout << "После обновления ключа 2: "
              << map.get(2) << '\n';

    map.remove(1);

    std::cout << "Размер после удаления ключа 1: "
              << map.size() << '\n';

    std::cout << "Удаление отсутствующего ключа: "
              << (map.remove(10) ? "успешно" : "ключ не найден")
              << "\n\n";

    std::cout << "Проверка строковых ключей:\n";

    HashMap<std::string, int> stringMap;
    stringMap.put("яблоко", 10);
    stringMap.put("апельсин", 20);

    std::cout << "яблоко = "
              << stringMap.get("яблоко") << '\n';

    std::cout << "Ключ «апельсин» существует: "
              << (stringMap.containsKey("апельсин")
                      ? "да" : "нет")
              << "\n\n";

    std::cout << "=== Сравнение производительности ===\n";

    const int N = 100000;

    HashMap<int, int> customMap;
    std::unordered_map<int, int> standardMap;

    std::cout << "Количество элементов: " << N << "\n\n";

    long long customInsert = measureTime([&]() {
        for (int i = 0; i < N; ++i) {
            customMap.put(i, i * 2);
        }
    });

    long long standardInsert = measureTime([&]() {
        for (int i = 0; i < N; ++i) {
            standardMap[i] = i * 2;
        }
    });

    volatile int customResult = 0;
    volatile int standardResult = 0;

    long long customGet = measureTime([&]() {
        for (int i = 0; i < N; ++i) {
            customResult = customMap.get(i);
        }
    });

    long long standardGet = measureTime([&]() {
        for (int i = 0; i < N; ++i) {
            standardResult = standardMap.at(i);
        }
    });

    volatile size_t customContainsCount = 0;
    volatile size_t standardContainsCount = 0;

    long long customContains = measureTime([&]() {
        for (int i = 0; i < N; ++i) {
            customContainsCount += customMap.containsKey(i);
        }
    });

    long long standardContains = measureTime([&]() {
        for (int i = 0; i < N; ++i) {
            standardContainsCount +=
                standardMap.find(i) != standardMap.end();
        }
    });

    HashMap<int, int> customRemoveMap;
    std::unordered_map<int, int> standardRemoveMap;

    for (int i = 0; i < N; ++i) {
        customRemoveMap.put(i, i);
        standardRemoveMap[i] = i;
    }

    long long customRemove = measureTime([&]() {
        for (int i = 0; i < N; ++i) {
            customRemoveMap.remove(i);
        }
    });

    long long standardRemove = measureTime([&]() {
        for (int i = 0; i < N; ++i) {
            standardRemoveMap.erase(i);
        }
    });

    std::cout << std::left
              << std::setw(24) << "Операция"
              << std::right
              << std::setw(18) << "Своя HashMap (мкс)"
              << std::setw(18) << "STL (мкс)"
              << '\n';

    std::cout << std::string(60, '-') << '\n';

    std::cout << std::left << std::setw(24) << "Вставка"
              << std::right << std::setw(18) << customInsert
              << std::setw(18) << standardInsert << '\n';

    std::cout << std::left << std::setw(24) << "Поиск"
              << std::right << std::setw(18) << customGet
              << std::setw(18) << standardGet << '\n';

    std::cout << std::left << std::setw(24) << "Проверка ключа"
              << std::right << std::setw(18) << customContains
              << std::setw(18) << standardContains << '\n';

    std::cout << std::left << std::setw(24) << "Удаление"
              << std::right << std::setw(18) << customRemove
              << std::setw(18) << standardRemove << "\n\n";

    std::cout << "Итоговые размеры:\n";
    std::cout << "Своя HashMap: " << customMap.size() << '\n';
    std::cout << "std::unordered_map: "
              << standardMap.size() << '\n';

    std::cout << "Своя HashMap после удаления: "
              << customRemoveMap.size() << '\n';

    std::cout << "STL после удаления: "
              << standardRemoveMap.size() << "\n\n";

    std::cout << "=== Сложность операций ===\n";
    std::cout << "put: O(1) в среднем, O(n) в худшем случае\n";
    std::cout << "get: O(1) в среднем, O(n) в худшем случае\n";
    std::cout << "containsKey: O(1) в среднем, "
                 "O(n) в худшем случае\n";
    std::cout << "remove: O(1) в среднем, O(n) в худшем случае\n";
    std::cout << "size: O(1)\n";
    std::cout << "rehash: O(n) за одно перераспределение\n";
    std::cout << "Пространственная сложность: O(n)\n";

    return 0;
}
