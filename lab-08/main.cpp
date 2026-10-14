#include <algorithm>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <limits>
#include <string>
#include <utility>
#include <vector>

using namespace std;
using Clock = chrono::steady_clock;

struct Person {
    int age;
    string name;
};

// Двоичный поиск человека по возрасту.
// Предусловие: people отсортирован по age по возрастанию.
// Если возраст встречается несколько раз, возвращается одно из совпадений.
int binarySearchByAge(const vector<Person>& people, int targetAge) {
    int left = 0;
    int right = static_cast<int>(people.size()) - 1;

    while (left <= right) {
        const int mid = left + (right - left) / 2;

        if (people[mid].age == targetAge) {
            return mid;
        }
        if (people[mid].age < targetAge) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    return -1;
}

template <typename K, typename V>
class LinearProbingHashTable {
private:
    enum class State { Empty, Occupied, Deleted };

    struct Entry {
        K key{};
        V value{};
        State state = State::Empty;
    };

    vector<Entry> table;
    size_t itemCount = 0;
    size_t deletedCount = 0;

    size_t hashKey(const K& key) const {
        // Нормализация остатка нужна, в том числе, для отрицательных int.
        long long value = static_cast<long long>(key);
        long long capacity = static_cast<long long>(table.size());
        long long result = value % capacity;
        if (result < 0) result += capacity;
        return static_cast<size_t>(result);
    }

    void rehash(size_t newCapacity) {
        vector<Entry> oldTable = std::move(table);
        table.assign(newCapacity, Entry{});
        itemCount = 0;
        deletedCount = 0;

        for (const Entry& entry : oldTable) {
            if (entry.state == State::Occupied) {
                insert(entry.key, entry.value);
            }
        }
    }

public:
    explicit LinearProbingHashTable(size_t capacity = 16)
        : table(max<size_t>(capacity, 8)) {}

    void insert(const K& key, const V& value) {
        // Расширяем таблицу при коэффициенте загрузки выше 0.7.
        if ((itemCount + deletedCount + 1) * 10 >= table.size() * 7) {
            rehash(table.size() * 2);
        }

        size_t index = hashKey(key);
        size_t firstDeleted = table.size();

        for (size_t checked = 0; checked < table.size(); ++checked) {
            Entry& entry = table[index];

            if (entry.state == State::Occupied && entry.key == key) {
                entry.value = value;
                return;
            }
            if (entry.state == State::Deleted && firstDeleted == table.size()) {
                firstDeleted = index;
            }
            if (entry.state == State::Empty) {
                const size_t destination =
                    firstDeleted == table.size() ? index : firstDeleted;
                Entry& target = table[destination];
                target.key = key;
                target.value = value;
                target.state = State::Occupied;
                ++itemCount;
                if (firstDeleted != table.size()) --deletedCount;
                return;
            }
            index = (index + 1) % table.size();
        }

        // Защитная ветка: при заполнении таблицы расширяем и повторяем вставку.
        rehash(table.size() * 2);
        insert(key, value);
    }

    const V* find(const K& key) const {
        size_t index = hashKey(key);

        for (size_t checked = 0; checked < table.size(); ++checked) {
            const Entry& entry = table[index];

            if (entry.state == State::Empty) return nullptr;
            if (entry.state == State::Occupied && entry.key == key) {
                return &entry.value;
            }
            index = (index + 1) % table.size();
        }
        return nullptr;
    }

    bool remove(const K& key) {
        size_t index = hashKey(key);

        for (size_t checked = 0; checked < table.size(); ++checked) {
            Entry& entry = table[index];

            if (entry.state == State::Empty) return false;
            if (entry.state == State::Occupied && entry.key == key) {
                entry.state = State::Deleted;
                --itemCount;
                ++deletedCount;
                return true;
            }
            index = (index + 1) % table.size();
        }
        return false;
    }

    size_t size() const { return itemCount; }
    size_t capacity() const { return table.size(); }
};

template <typename Function>
long long measureMicroseconds(Function function) {
    const auto start = Clock::now();
    function();
    const auto finish = Clock::now();
    return chrono::duration_cast<chrono::microseconds>(finish - start).count();
}

void demonstrateVariantFeatures() {
    vector<Person> people = {
        {19, "Анна"}, {21, "Борис"}, {23, "Вера"},
        {25, "Глеб"}, {27, "Дарья"}
    };
    sort(people.begin(), people.end(),
         [](const Person& a, const Person& b) { return a.age < b.age; });

    const int found = binarySearchByAge(people, 23);
    cout << "Поиск Person по возрасту 23: ";
    if (found >= 0) {
        cout << people[found].name << " (индекс " << found << ")\n";
    } else {
        cout << "не найден\n";
    }
    cout << "Поиск возраста 100: "
         << (binarySearchByAge(people, 100) == -1 ? "не найден" : "найден")
         << "\n";

    LinearProbingHashTable<int, string> table(8);
    table.insert(10, "десять");
    table.insert(18, "восемнадцать"); // коллизия с ключом 10 при начальной ёмкости 8
    table.insert(26, "двадцать шесть");
    table.insert(-2, "минус два");
    if (const string* value = table.find(18)) {
        cout << "Хеш-таблица: ключ 18 -> " << *value << '\n';
    }
    table.insert(18, "обновлённое значение");
    if (const string* value = table.find(18)) {
        cout << "После обновления ключа 18 -> " << *value << '\n';
    }
    cout << "Удаление ключа 10: " << (table.remove(10) ? "успешно" : "не найден")
         << '\n';
    cout << "Поиск удалённого ключа 10: "
         << (table.find(10) == nullptr ? "не найден" : "найден") << "\n\n";
}

void runPerformanceComparison() {
    constexpr int QUERY_COUNT = 20000;
    cout << "Сравнение поиска (" << QUERY_COUNT << " запросов на каждый N)\n";
    cout << "N\tBinary Search, мкс\tHash Table, мкс\tПроверка\n";

    for (int n : {1000, 10000, 100000}) {
        vector<Person> people;
        people.reserve(static_cast<size_t>(n));
        LinearProbingHashTable<int, string> table(static_cast<size_t>(n) * 2);

        for (int i = 0; i < n; ++i) {
            // Уникальные возраста-ключи для однозначного сравнения поиска.
            people.push_back({i, "Person_" + to_string(i)});
            table.insert(i, "Person_" + to_string(i));
        }

        // Чередующиеся запросы по существующим и отсутствующим ключам.
        vector<int> queries;
        queries.reserve(QUERY_COUNT);
        for (int i = 0; i < QUERY_COUNT; ++i) {
            queries.push_back((i * 37LL) % (n + n / 10));
        }

        volatile long long binaryChecksum = 0;
        volatile long long hashChecksum = 0;

        const long long binaryTime = measureMicroseconds([&]() {
            for (int key : queries) {
                const int index = binarySearchByAge(people, key);
                if (index >= 0) binaryChecksum += people[index].age;
            }
        });

        const long long hashTime = measureMicroseconds([&]() {
            for (int key : queries) {
                const string* value = table.find(key);
                if (value != nullptr) hashChecksum += key;
            }
        });

        bool correct = true;
        for (int key : {0, n / 2, n - 1, n + 1, -1}) {
            const int index = binarySearchByAge(people, key);
            const string* value = table.find(key);
            const bool binaryFound = index >= 0;
            const bool hashFound = value != nullptr;
            if (binaryFound != hashFound) correct = false;
        }

        cout << n << '\t' << binaryTime << '\t' << hashTime << '\t'
             << (correct ? "OK" : "ERROR") << '\n';
        // Чексуммы используются, чтобы компилятор не удалил измеряемые циклы.
        if (binaryChecksum == numeric_limits<long long>::min() ||
            hashChecksum == numeric_limits<long long>::min()) {
            cerr << "Непредвиденное значение контрольной суммы.\n";
        }
    }
}

int main() {
    cout << "=== Лабораторная работа № 8 ===\n";
    cout << "Двоичный поиск и хеш-таблица с линейным пробированием\n\n";

    demonstrateVariantFeatures();
    runPerformanceComparison();
    return 0;
}
