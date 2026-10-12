#ifdef _WIN32
#include <windows.h>
#endif
#include <iostream>
#include <memory>
#include <string>
#include <vector>

class Booking {
private:
    std::string guestName_;
    int nights_;
    static int bookingCount_;

public:
    Booking(const std::string& guestName, int nights)
        : guestName_(guestName), nights_(nights) {
        ++bookingCount_;  // Увеличиваем число активных бронирований.
    }

    ~Booking() {
        --bookingCount_;  // Уменьшаем счётчик при уничтожении объекта.
    }

    Booking(const Booking&) = delete;
    Booking& operator=(const Booking&) = delete;

    const std::string& getGuestName() const {
        return guestName_;
    }

    int getNights() const {
        return nights_;
    }

    static int getBookingCount() {
        return bookingCount_;
    }

    void print() const {
        std::cout << "Гость: " << guestName_
                  << ", ночей: " << nights_ << '\n';
    }
};

int Booking::bookingCount_ = 0;

class Room {
private:
    int number_;
    int capacity_;
    std::vector<std::unique_ptr<Booking>> bookings_;

public:
    Room(int number, int capacity)
        : number_(number), capacity_(capacity) {}

    Room(const Room&) = delete;
    Room& operator=(const Room&) = delete;

    int getNumber() const {
        return number_;
    }

    void addBooking(const std::string& guestName, int nights) {
        if (guestName.empty()) {
            std::cout << "Ошибка: имя гостя не может быть пустым.\n";
            return;
        }
        if (nights <= 0) {
            std::cout << "Ошибка: количество ночей должно быть больше нуля.\n";
            return;
        }

        bookings_.push_back(
            std::make_unique<Booking>(guestName, nights)
        );
        std::cout << "Бронирование добавлено в номер "
                  << number_ << ".\n";
    }

    void print() const {
        std::cout << "Номер " << number_
                  << ", вместимость: " << capacity_ << '\n';

        if (bookings_.empty()) {
            std::cout << "  Бронирований нет.\n";
            return;
        }

        for (const auto& booking : bookings_) {
            std::cout << "  ";
            booking->print();
        }
    }
};

class Hotel {
private:
    std::string name_;
    std::vector<Room*> rooms_;

    explicit Hotel(const std::string& name) : name_(name) {}

    Hotel(const Hotel&) = delete;
    Hotel& operator=(const Hotel&) = delete;

public:
    static Hotel& getInstance() {
        static Hotel instance("Северная звезда");
        return instance;
    }

    void addRoom(Room* room) {
        if (room == nullptr) {
            std::cout << "Ошибка: нельзя добавить нулевой указатель.\n";
            return;
        }

        for (const Room* existing : rooms_) {
            if (existing == room) {
                std::cout << "Этот номер уже добавлен в гостиницу.\n";
                return;
            }
        }

        rooms_.push_back(room);
        std::cout << "Номер " << room->getNumber()
                  << " добавлен в гостиницу.\n";
    }

    void print() const {
        std::cout << "\nГостиница: " << name_ << '\n';

        if (rooms_.empty()) {
            std::cout << "Номера не добавлены.\n";
            return;
        }

        for (const Room* room : rooms_) {
            if (room != nullptr) {
                room->print();
            }
        }
    }
};

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    Hotel& hotel1 = Hotel::getInstance();
    Hotel& hotel2 = Hotel::getInstance();

    std::cout << "Singleton работает: "
              << (&hotel1 == &hotel2 ? "да" : "нет") << '\n';

    Room room101(101, 2);
    Room room202(202, 3);

    hotel1.addRoom(&room101);
    hotel1.addRoom(&room202);
    hotel1.addRoom(nullptr);  // Проверка нулевого указателя.

    room101.addBooking("Иван Петров", 3);
    room101.addBooking("Мария Иванова", 2);
    room202.addBooking("Алексей Смирнов", 1);
    room202.addBooking("", 2);  // Проверка пустого имени.
    room202.addBooking("Ольга Соколова", 0);  // Проверка числа ночей.

    hotel1.print();

    std::cout << "\nКоличество активных бронирований: "
              << Booking::getBookingCount() << '\n';

    {
        Room temporaryRoom(303, 1);
        temporaryRoom.addBooking("Временный гость", 1);
        std::cout << "Перед уничтожением временного номера: "
                  << Booking::getBookingCount() << '\n';
    }

    std::cout << "После уничтожения временного номера: "
              << Booking::getBookingCount() << '\n';

    std::cout << "\nHotel хранит невладеющие указатели на номера Room.\n";
    return 0;
}
