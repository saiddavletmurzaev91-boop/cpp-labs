# Лабораторная работа № 3
## Реализация динамического массива (аналог vector). Итераторы

**Студент:** Давлетмурзаев Сайд-Эмин Тимурович  
**Группа:** ПИЖ-б-о-25-1  
**Преподаватель:** Белик Ангелина Игоревна  
**Уровень:** продвинутый  
**Вариант:** 8  
**Язык программирования:** C++17

---

## 1. Цель работы

Реализовать шаблонный класс `Vector<T>`, изучить управление динамической памятью, закрепить правило пяти, реализовать итераторы и дополнительную функциональность варианта 8 — сохранение вектора в файл и загрузку из файла.

## 2. Теоретическая часть

**Динамический массив** — структура данных, элементы которой размещаются в динамической памяти, а размер может изменяться во время выполнения программы.

**Размер (`size`)** — число элементов, фактически хранящихся в контейнере. **Ёмкость (`capacity`)** — количество элементов, для которых уже выделена память. Когда ёмкости недостаточно, массив получает больший блок памяти. В данной реализации ёмкость при расширении удваивается, поэтому добавление в конец выполняется за амортизированное `O(1)`.

**Правило пяти** включает деструктор, конструктор копирования, оператор присваивания копированием, конструктор перемещения и оператор присваивания перемещением. Эти методы нужны классу, который самостоятельно владеет динамической памятью.

**Итераторы** позволяют последовательно обращаться к элементам контейнера. В данной реализации используются указатели `T*` и `const T*`; методы `begin()` и `end()` обеспечивают перебор через цикл с итераторами и range-based `for`.

**Сериализация** — сохранение состояния объекта в файл с возможностью последующего восстановления. Реализованы методы `saveToFile()` и `loadFromFile()`. Для используемого формата тип `T` должен поддерживать потоковые операторы `<<` и `>>`.

## 3. Постановка задачи

Для продвинутого уровня, варианта 8, необходимо реализовать шаблонный динамический массив `Vector<T>` с поддержкой правила пяти и итераторов, а также добавить сериализацию: `saveToFile()` и `loadFromFile()`.

Программа должна демонстрировать добавление элементов, доступ по индексу, перебор итераторами, копирование и перемещение объекта, сохранение и загрузку данных, удаление последнего элемента, очистку контейнера и обработку выхода за границы.

## 4. Листинг программы

Ниже приведён полный исходный код программы из файла [`main.cpp`](main.cpp).

```cpp
#include <cstddef>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#ifdef _WIN32
#include <windows.h>
#endif

template <typename T>
class Vector {
public:
    using value_type = T;
    using size_type = std::size_t;
    using iterator = T*;
    using const_iterator = const T*;

    Vector() noexcept = default;

    explicit Vector(size_type count) {
        if (count == 0) {
            return;
        }

        data_ = allocate(count);
        capacity_ = count;

        try {
            for (; size_ < count; ++size_) {
                std::allocator_traits<Allocator>::construct(
                    allocator_, data_ + size_);
            }
        } catch (...) {
            release();
            throw;
        }
    }

    Vector(size_type count, const T& value) {
        if (count == 0) {
            return;
        }

        data_ = allocate(count);
        capacity_ = count;

        try {
            for (; size_ < count; ++size_) {
                std::allocator_traits<Allocator>::construct(
                    allocator_, data_ + size_, value);
            }
        } catch (...) {
            release();
            throw;
        }
    }

    Vector(const Vector& other) {
        if (other.capacity_ == 0) {
            return;
        }

        data_ = allocate(other.capacity_);
        capacity_ = other.capacity_;

        try {
            for (; size_ < other.size_; ++size_) {
                std::allocator_traits<Allocator>::construct(
                    allocator_, data_ + size_, other.data_[size_]);
            }
        } catch (...) {
            release();
            throw;
        }
    }

    Vector(Vector&& other) noexcept
        : data_(std::exchange(other.data_, nullptr)),
          size_(std::exchange(other.size_, 0)),
          capacity_(std::exchange(other.capacity_, 0)) {}

    ~Vector() {
        release();
    }

    Vector& operator=(const Vector& other) {
        if (this != &other) {
            Vector copy(other);
            swap(copy);
        }
        return *this;
    }

    Vector& operator=(Vector&& other) noexcept {
        if (this != &other) {
            release();
            data_ = std::exchange(other.data_, nullptr);
            size_ = std::exchange(other.size_, 0);
            capacity_ = std::exchange(other.capacity_, 0);
        }
        return *this;
    }

    T& operator[](size_type index) {
        if (index >= size_) {
            throw std::out_of_range("Vector index out of range");
        }
        return data_[index];
    }

    const T& operator[](size_type index) const {
        if (index >= size_) {
            throw std::out_of_range("Vector index out of range");
        }
        return data_[index];
    }

    size_type size() const noexcept { return size_; }
    size_type capacity() const noexcept { return capacity_; }
    bool empty() const noexcept { return size_ == 0; }

    void reserve(size_type new_capacity) {
        if (new_capacity > capacity_) {
            reallocate(new_capacity);
        }
    }

    void push_back(const T& value) {
        if (size_ == capacity_) {
            T temporary(value);  // Supports push_back(v[0]) during reallocation.
            reserve(capacity_ == 0 ? 1 : capacity_ * 2);
            std::allocator_traits<Allocator>::construct(
                allocator_, data_ + size_, std::move_if_noexcept(temporary));
        } else {
            std::allocator_traits<Allocator>::construct(
                allocator_, data_ + size_, value);
        }
        ++size_;
    }

    void push_back(T&& value) {
        if (size_ == capacity_) {
            T temporary(std::move(value));
            reserve(capacity_ == 0 ? 1 : capacity_ * 2);
            std::allocator_traits<Allocator>::construct(
                allocator_, data_ + size_, std::move(temporary));
        } else {
            std::allocator_traits<Allocator>::construct(
                allocator_, data_ + size_, std::move(value));
        }
        ++size_;
    }

    void pop_back() {
        if (empty()) {
            throw std::out_of_range("Cannot pop_back from an empty Vector");
        }
        --size_;
        std::allocator_traits<Allocator>::destroy(allocator_, data_ + size_);
    }

    void clear() noexcept {
        destroy_elements();
        size_ = 0;
    }

    iterator begin() noexcept { return data_; }
    const_iterator begin() const noexcept { return data_; }
    const_iterator cbegin() const noexcept { return data_; }

    iterator end() noexcept { return data_ ? data_ + size_ : nullptr; }
    const_iterator end() const noexcept {
        return data_ ? data_ + size_ : nullptr;
    }
    const_iterator cend() const noexcept {
        return data_ ? data_ + size_ : nullptr;
    }

    void swap(Vector& other) noexcept {
        using std::swap;
        swap(data_, other.data_);
        swap(size_, other.size_);
        swap(capacity_, other.capacity_);
    }

    bool saveToFile(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file) {
            return false;
        }

        file << size_ << '\n';
        for (size_type i = 0; i < size_; ++i) {
            file << data_[i] << '\n';
            if (!file) {
                return false;
            }
        }
        return static_cast<bool>(file);
    }

    bool loadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file) {
            return false;
        }

        size_type count = 0;
        if (!(file >> count)) {
            return false;
        }

        Vector temporary;
        for (size_type i = 0; i < count; ++i) {
            T value{};
            if (!(file >> value)) {
                return false;
            }
            temporary.push_back(std::move(value));
        }

        swap(temporary);
        return true;
    }

private:
    using Allocator = std::allocator<T>;
    using AllocTraits = std::allocator_traits<Allocator>;

    Allocator allocator_;
    T* data_ = nullptr;
    size_type size_ = 0;
    size_type capacity_ = 0;

    T* allocate(size_type count) {
        return AllocTraits::allocate(allocator_, count);
    }

    void destroy_elements() noexcept {
        for (size_type i = 0; i < size_; ++i) {
            AllocTraits::destroy(allocator_, data_ + i);
        }
    }

    void release() noexcept {
        destroy_elements();
        if (data_) {
            AllocTraits::deallocate(allocator_, data_, capacity_);
        }
        data_ = nullptr;
        size_ = 0;
        capacity_ = 0;
    }

    void reallocate(size_type new_capacity) {
        if (new_capacity > max_size()) {
            throw std::length_error("Vector capacity is too large");
        }

        T* new_data = allocate(new_capacity);
        size_type constructed = 0;

        try {
            for (; constructed < size_; ++constructed) {
                AllocTraits::construct(
                    allocator_, new_data + constructed,
                    std::move_if_noexcept(data_[constructed]));
            }
        } catch (...) {
            for (size_type i = 0; i < constructed; ++i) {
                AllocTraits::destroy(allocator_, new_data + i);
            }
            AllocTraits::deallocate(allocator_, new_data, new_capacity);
            throw;
        }

        destroy_elements();
        if (data_) {
            AllocTraits::deallocate(allocator_, data_, capacity_);
        }

        data_ = new_data;
        capacity_ = new_capacity;
    }

    size_type max_size() const noexcept {
        return AllocTraits::max_size(allocator_);
    }
};

template <typename T>
void printVector(const Vector<T>& values) {
    for (const T& value : values) {
        std::cout << value << ' ';
    }
    std::cout << '\n';
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    std::cout << "=== Лабораторная работа № 3: Vector<T> ===\n\n";

    Vector<int> values;
    for (int value : {10, 20, 30, 40, 50}) {
        values.push_back(value);
    }

    std::cout << "Начальный вектор: ";
    printVector(values);
    std::cout << "Размер: " << values.size()
              << ", capacity: " << values.capacity() << '\n';
    std::cout << "values[2] = " << values[2] << "\n\n";

    std::cout << "Перебор итераторами: ";
    for (Vector<int>::iterator it = values.begin(); it != values.end(); ++it) {
        std::cout << *it << ' ';
    }
    std::cout << "\nПеребор range-based for: ";
    printVector(values);

    Vector<int> copied(values);
    copied.push_back(60);
    std::cout << "\nКопия после push_back(60): ";
    printVector(copied);

    Vector<int> moved(std::move(copied));
    std::cout << "Перемещённый вектор: ";
    printVector(moved);
    std::cout << "Размер moved-from вектора: " << copied.size() << '\n';

    const std::string filename = "vector_data.txt";
    if (values.saveToFile(filename)) {
        std::cout << "\nДанные сохранены в файл " << filename << '\n';
    } else {
        std::cout << "\nОшибка сохранения файла\n";
    }

    Vector<int> loaded;
    if (loaded.loadFromFile(filename)) {
        std::cout << "Загруженный вектор: ";
        printVector(loaded);
    } else {
        std::cout << "Ошибка загрузки файла\n";
    }

    loaded.pop_back();
    std::cout << "После pop_back(): ";
    printVector(loaded);

    loaded.clear();
    std::cout << "После clear(): size = " << loaded.size()
              << ", empty = " << (loaded.empty() ? "yes" : "no") << '\n';

    try {
        (void)values[100];
    } catch (const std::out_of_range& error) {
        std::cout << "Проверка исключения: " << error.what() << '\n';
    }

    Vector<std::string> words;
    words.push_back("memory");
    words.push_back("management");
    std::cout << "\nПроверка Vector<std::string>: ";
    printVector(words);

    return 0;
}
```

## 5. Результаты работы

Программа скомпилирована с ключами `-std=c++17 -Wall -Wextra -pedantic` и выполнена в MSYS2 UCRT64.

Проверены:
- добавление элементов и изменение ёмкости;
- получение элемента по индексу;
- перебор через итераторы и range-based `for`;
- копирование и перемещение вектора;
- сохранение в `vector_data.txt` и последующая загрузка;
- удаление последнего элемента через `pop_back()`;
- очистка через `clear()`;
- обработка обращения к индексу за пределами вектора.

### 5.1. Скриншот результата

![Результат выполнения программы](03-vector-results.png)

## 6. Анализ сложности операций

Пусть `n` — количество элементов вектора.

| Операция | Средняя / амортизированная сложность | Худший случай |
|---|---:|---:|
| `size()` | `O(1)` | `O(1)` |
| `capacity()` | `O(1)` | `O(1)` |
| `empty()` | `O(1)` | `O(1)` |
| `operator[]` | `O(1)` | `O(1)` |
| `begin()`, `end()` | `O(1)` | `O(1)` |
| `push_back()` | `O(1)` амортизированно | `O(n)` при расширении |
| `pop_back()` | `O(1)` | `O(1)` |
| `clear()` | `O(1)` | `O(1)` |
| `reserve()` | `O(1)`, если расширение не нужно | `O(n)` при перераспределении |
| Конструктор копирования | `O(n)` | `O(n)` |
| Присваивание копированием | `O(n)` | `O(n)` |
| Конструктор перемещения | `O(1)` | `O(1)` |
| Присваивание перемещением | `O(1)` | `O(1)` |
| `saveToFile()` | `O(n)` | `O(n)` |
| `loadFromFile()` | `O(n)` | `O(n)` |

Дополнительная память контейнера составляет `O(capacity)`. При перераспределении памяти создаётся новый массив, элементы копируются, а старый массив освобождается.

## 7. Ответы на контрольные вопросы

**1. Почему динамический массив называется динамическим?**  
Потому что его размер может изменяться во время выполнения программы. При необходимости контейнер выделяет дополнительную память.

**2. Что такое ёмкость (`capacity`) и чем она отличается от размера (`size`)?**  
`size` показывает количество хранящихся элементов. `capacity` показывает, сколько элементов помещается в уже выделенную память без её расширения.

**3. Почему при расширении массива ёмкость удваивается, а не увеличивается на фиксированное число?**  
Удвоение уменьшает частоту перераспределений памяти. Благодаря этому `push_back()` имеет амортизированную сложность `O(1)`.

**4. Какова амортизированная сложность `push_back()`? Почему?**  
`O(1)` амортизированно. Большинство вставок выполняется за постоянное время, а редкие расширения требуют копирования элементов и занимают `O(n)`.

**5. Что такое правило трёх? Когда оно применяется?**  
Правило трёх говорит, что классу, которому необходим пользовательский деструктор, конструктор копирования или оператор присваивания копированием, обычно нужны все три. Оно особенно важно для классов, самостоятельно управляющих ресурсами.

**6. Что такое правило пяти? Чем оно отличается от правила трёх?**  
Правило пяти включает методы правила трёх и дополнительно конструктор перемещения и оператор присваивания перемещением. Перемещение позволяет передавать владение ресурсом без копирования всех элементов.

**7. В чём разница между конструктором копирования и оператором присваивания?**  
Конструктор копирования создаёт новый объект на основе существующего. Оператор присваивания копированием заменяет содержимое уже существующего объекта.

**8. Зачем нужна проверка `if (this != &other)` в операторе присваивания?**  
Она защищает от самоприсваивания, когда объект присваивается самому себе. Без проверки можно случайно удалить ресурс до того, как его содержимое будет скопировано.

**9. Что такое итератор? Как он связан с указателем?**  
Итератор — средство доступа к элементам контейнера. В данной реализации обычные указатели `T*` и `const T*` используются как итераторы.

**10. Зачем нужны методы `begin()` и `end()`?**  
`begin()` возвращает итератор на первый элемент, а `end()` — итератор за последним элементом. Они позволяют обходить контейнер стандартным способом.

**11. Как реализовать range-based `for` для своего контейнера?**  
Контейнер должен предоставлять методы `begin()` и `end()`, возвращающие совместимые итераторы. В этой реализации они возвращают указатели на элементы.

**12. Что такое глубокое копирование? Чем оно отличается от поверхностного?**  
Глубокое копирование создаёт собственную копию данных. Поверхностное копирование указателя лишь копирует адрес памяти и может привести к совместному владению одним ресурсом и двойному освобождению.

**13. Что такое семантика перемещения и зачем она нужна?**  
Семантика перемещения позволяет передать ресурсы от временного или другого объекта вместо копирования всех данных. Это уменьшает затраты на операции с динамической памятью.

**14. Почему конструктор перемещения помечается `noexcept`?**  
`noexcept` сообщает, что перемещение не выбрасывает исключений. Это помогает стандартным контейнерам выбирать безопасные и эффективные операции перемещения. В данной реализации конструктор перемещения только передаёт указатель и размеры.

**15. Как правильно освобождать память в деструкторе?**  
Память, выделенную через `new[]`, освобождают оператором `delete[]`. Деструктор `Vector` вызывает `delete[] data_`.

## 8. Вывод

В ходе лабораторной работы реализован шаблонный динамический массив `Vector<T>`. Были закреплены навыки управления динамической памятью, реализации правила пяти и работы с итераторами. Дополнительно реализованы сохранение элементов в файл и загрузка из файла, а также проверка выхода за границы контейнера. Программа скомпилирована и выполнена; основные операции продемонстрированы.

## 9. Репозиторий

Ссылка на GitHub: https://github.com/saiddavletmurzaev91-boop/cpp-labs
