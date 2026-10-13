#include <iostream>
#include <fstream>
#include <stdexcept>
#include <utility>
#include <string>
#include <clocale>

#ifdef _WIN32
#include <windows.h>
#endif

// Шаблонный динамический массив Vector<T>.
// Реализованы правило пяти, итераторы и сериализация.
template <typename T>
class Vector {
private:
    T* data_;
    std::size_t size_;
    std::size_t capacity_;

    void reallocate(std::size_t new_capacity) {
        if (new_capacity < size_) {
            throw std::length_error("New capacity is smaller than size");
        }

        T* new_data = new_capacity == 0 ? nullptr : new T[new_capacity];

        try {
            for (std::size_t i = 0; i < size_; ++i) {
                new_data[i] = data_[i];
            }
        } catch (...) {
            delete[] new_data;
            throw;
        }

        delete[] data_;
        data_ = new_data;
        capacity_ = new_capacity;
    }

public:
    using iterator = T*;
    using const_iterator = const T*;

    // Конструктор по умолчанию.
    Vector() noexcept : data_(nullptr), size_(0), capacity_(0) {}

    // Конструктор с заданным количеством элементов.
    explicit Vector(std::size_t count)
        : data_(count == 0 ? nullptr : new T[count]),
          size_(count),
          capacity_(count) {}

    // Конструктор с заполнением заданным значением.
    Vector(std::size_t count, const T& value) : Vector(count) {
        for (std::size_t i = 0; i < size_; ++i) {
            data_[i] = value;
        }
    }

    // Конструктор копирования: создаёт независимую копию.
    Vector(const Vector& other) : Vector(other.size_) {
        for (std::size_t i = 0; i < size_; ++i) {
            data_[i] = other.data_[i];
        }
    }

    // Конструктор перемещения: передаёт владение памятью.
    Vector(Vector&& other) noexcept
        : data_(other.data_),
          size_(other.size_),
          capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    // Деструктор освобождает выделенную память.
    ~Vector() {
        delete[] data_;
    }

    // Присваивание копированием с защитой от самоприсваивания.
    Vector& operator=(const Vector& other) {
        if (this != &other) {
            Vector temp(other);
            swap(temp);
        }
        return *this;
    }

    // Присваивание перемещением.
    Vector& operator=(Vector&& other) noexcept {
        if (this != &other) {
            delete[] data_;

            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;

            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }

    // Доступ к элементу с проверкой границ.
    T& operator[](std::size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Индекс Vector находится вне границ");
        }
        return data_[index];
    }

    const T& operator[](std::size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Индекс Vector находится вне границ");
        }
        return data_[index];
    }

    std::size_t size() const noexcept {
        return size_;
    }

    std::size_t capacity() const noexcept {
        return capacity_;
    }

    bool empty() const noexcept {
        return size_ == 0;
    }

    // Резервирование памяти.
    void reserve(std::size_t new_capacity) {
        if (new_capacity > capacity_) {
            reallocate(new_capacity);
        }
    }

    // Добавление элемента в конец с удвоением ёмкости.
    void push_back(const T& value) {
        if (size_ == capacity_) {
            const std::size_t new_capacity =
                capacity_ == 0 ? 1 : capacity_ * 2;
            reserve(new_capacity);
        }

        data_[size_] = value;
        ++size_;
    }

    // Удаление последнего элемента.
    void pop_back() {
        if (empty()) {
            throw std::underflow_error(
                "Нельзя удалить элемент из пустого Vector"
            );
        }
        --size_;
    }

    // Очистка элементов без освобождения зарезервированной памяти.
    void clear() noexcept {
        size_ = 0;
    }

    // Обмен содержимым векторов.
    void swap(Vector& other) noexcept {
        using std::swap;
        swap(data_, other.data_);
        swap(size_, other.size_);
        swap(capacity_, other.capacity_);
    }

    // Итераторы для range-based for и STL-алгоритмов.
    iterator begin() noexcept {
        return data_;
    }

    iterator end() noexcept {
        return data_ == nullptr ? nullptr : data_ + size_;
    }

    const_iterator begin() const noexcept {
        return data_;
    }

    const_iterator end() const noexcept {
        return data_ == nullptr ? nullptr : data_ + size_;
    }

    const_iterator cbegin() const noexcept {
        return begin();
    }

    const_iterator cend() const noexcept {
        return end();
    }

    // Сохранение вектора в текстовый файл.
    // Тип T должен поддерживать оператор вывода <<.
    bool saveToFile(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }

        file << size_ << '\n';
        for (std::size_t i = 0; i < size_; ++i) {
            file << data_[i] << '\n';
            if (!file) {
                return false;
            }
        }

        return static_cast<bool>(file);
    }

    // Загрузка из файла. При ошибке исходный вектор не изменяется.
    // Тип T должен поддерживать оператор ввода >>.
    bool loadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return false;
        }

        std::size_t count = 0;
        if (!(file >> count)) {
            return false;
        }

        Vector temp;
        temp.reserve(count);

        for (std::size_t i = 0; i < count; ++i) {
            T value{};
            if (!(file >> value)) {
                return false;
            }
            temp.push_back(value);
        }

        swap(temp);
        return true;
    }
};

// Вывод элементов вектора на экран.
template <typename T>
void printVector(const Vector<T>& values, const std::string& label) {
    std::cout << label;
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
    std::setlocale(LC_ALL, ".UTF-8");

    try {
        std::cout << "=== Лабораторная работа № 3: Vector<T> ===\n\n";

        Vector<int> values;
        for (int i = 1; i <= 5; ++i) {
            values.push_back(i * 10);
        }

        printVector(values, "Исходный вектор: ");
        std::cout << "Размер: " << values.size()
                  << ", ёмкость: " << values.capacity() << '\n';
        std::cout << "Доступ по индексу values[2]: "
                  << values[2] << "\n\n";

        std::cout << "Перебор через итераторы: ";
        for (auto it = values.begin(); it != values.end(); ++it) {
            std::cout << *it << ' ';
        }
        std::cout << '\n';

        std::cout << "Перебор через range-based for: ";
        for (int value : values) {
            std::cout << value << ' ';
        }
        std::cout << "\n\n";

        Vector<int> copied(values);
        copied.push_back(60);
        printVector(copied, "Копия после push_back(60): ");

        Vector<int> moved(std::move(copied));
        printVector(moved, "Вектор после перемещения: ");
        std::cout << "Размер источника после перемещения: "
                  << copied.size() << "\n\n";

        const std::string filename = "vector_data.txt";
        if (values.saveToFile(filename)) {
            std::cout << "Вектор сохранён в файл: "
                      << filename << '\n';
        } else {
            std::cerr << "Ошибка сохранения файла.\n";
            return 1;
        }

        Vector<int> loaded;
        if (loaded.loadFromFile(filename)) {
            printVector(loaded, "Вектор после загрузки из файла: ");
        } else {
            std::cerr << "Ошибка загрузки файла.\n";
            return 1;
        }

        loaded.pop_back();
        printVector(loaded, "После pop_back: ");

        loaded.clear();
        std::cout << "После clear: размер=" << loaded.size()
                  << ", пустой=" << (loaded.empty() ? "да" : "нет")
                  << '\n';

        try {
            std::cout << values[100] << '\n';
        } catch (const std::out_of_range& error) {
            std::cout << "Проверка выхода за границы: "
                      << error.what() << '\n';
        }
    } catch (const std::exception& error) {
        std::cerr << "Ошибка: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
