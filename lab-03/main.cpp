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
