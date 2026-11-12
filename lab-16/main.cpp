#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
#include <iostream>
#include <vector>
#include <memory>
#include <fstream>
#include <string>
#include <iomanip>
#include <limits>

using namespace std;

// ===== БАЗОВЫЙ КЛАСС =====
class Person {
protected:
    int id;
    string name;
public:
    Person(int i, string n) : id(i), name(n) {}
    virtual ~Person() = default;
    int getId() const { return id; }
    virtual string type() const = 0;
    virtual void info() const = 0;
    virtual void save(ofstream& f) const = 0;
};

// ===== КЛИЕНТ =====
class Client : public Person {
    string plan;
    int months;
public:
    Client(int i, string n, string p, int m)
        : Person(i, n), plan(p), months(m) {}

    string type() const override { return "CLIENT"; }

    void info() const override {
        cout << "Клиент #" << id << ": " << name
             << ", абонемент: " << plan
             << ", месяцев: " << months << '\n';
    }

    void save(ofstream& f) const override {
        f << type() << ' ' << id << ' '
          << quoted(name) << ' ' << quoted(plan)
          << ' ' << months << '\n';
    }
};

// ===== ТРЕНЕР =====
class Trainer : public Person {
    string specialization;
    double salary;
public:
    Trainer(int i, string n, string s, double sal)
        : Person(i, n), specialization(s), salary(sal) {}

    string type() const override { return "TRAINER"; }

    void info() const override {
        cout << "Тренер #" << id << ": " << name
             << ", специализация: " << specialization
             << ", зарплата: " << salary << '\n';
    }

    void save(ofstream& f) const override {
        f << type() << ' ' << id << ' '
          << quoted(name) << ' '
          << quoted(specialization) << ' '
          << salary << '\n';
    }
};

// ===== ТРЕНИРОВКА =====
struct Workout {
    int clientId, trainerId;
    string date, title;

    void info() const {
        cout << title << " | дата: " << date
             << " | клиент #" << clientId
             << " | тренер #" << trainerId << '\n';
    }
};

// ===== ФАБРИКА ОБЪЕКТОВ =====
class PersonFactory {
public:
    static unique_ptr<Person> create(const string& t, int id,
                                     const string& n,
                                     const string& extra,
                                     double value) {
        if (t == "CLIENT")
            return make_unique<Client>(
                id, n, extra, static_cast<int>(value));
        if (t == "TRAINER")
            return make_unique<Trainer>(
                id, n, extra, value);
        return nullptr;
    }
};

// ===== СИСТЕМА СПОРТЗАЛА =====
class Gym {
    vector<unique_ptr<Person>> people;
    vector<Workout> workouts;
    int nextId = 1;

public:
    void addClient() {
        string n, p;
        int m;
        cout << "Имя клиента: ";
        getline(cin, n);
        cout << "Абонемент: ";
        getline(cin, p);
        cout << "Количество месяцев: ";
        cin >> m;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (m <= 0) {
            cout << "Некорректный срок.\n";
            return;
        }

        people.push_back(
            PersonFactory::create("CLIENT", nextId++, n, p, m));
        cout << "Клиент добавлен.\n";
    }

    void addTrainer() {
        string n, s;
        double salary;
        cout << "Имя тренера: ";
        getline(cin, n);
        cout << "Специализация: ";
        getline(cin, s);
        cout << "Зарплата: ";
        cin >> salary;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (salary < 0) {
            cout << "Некорректная зарплата.\n";
            return;
        }

        people.push_back(
            PersonFactory::create("TRAINER", nextId++, n, s, salary));
        cout << "Тренер добавлен.\n";
    }

    void addWorkout() {
        Workout w;
        cout << "ID клиента: ";
        cin >> w.clientId;
        cout << "ID тренера: ";
        cin >> w.trainerId;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (!findPerson(w.clientId, "CLIENT") ||
            !findPerson(w.trainerId, "TRAINER")) {
            cout << "Клиент или тренер не найден.\n";
            return;
        }

        cout << "Название тренировки: ";
        getline(cin, w.title);
        cout << "Дата: ";
        getline(cin, w.date);

        workouts.push_back(w);
        cout << "Тренировка добавлена.\n";
    }

    Person* findPerson(int id, const string& t) const {
        for (const auto& p : people)
            if (p->getId() == id && p->type() == t)
                return p.get();
        return nullptr;
    }

    void showPeople() const {
        cout << "\n=== КЛИЕНТЫ И ТРЕНЕРЫ ===\n";
        for (const auto& p : people)
            p->info();
        if (people.empty()) cout << "Записей нет.\n";
    }

    void showWorkouts() const {
        cout << "\n=== ТРЕНИРОВКИ ===\n";
        for (const auto& w : workouts)
            w.info();
        if (workouts.empty()) cout << "Тренировок нет.\n";
    }

    void save() const {
        ofstream f("gym_data.txt");
        if (!f) {
            cout << "Ошибка сохранения.\n";
            return;
        }

        f << nextId << '\n' << people.size() << '\n';
        for (const auto& p : people)
            p->save(f);

        f << workouts.size() << '\n';
        for (const auto& w : workouts)
            f << w.clientId << ' ' << w.trainerId << ' '
              << quoted(w.date) << ' ' << quoted(w.title) << '\n';

        cout << "Данные сохранены в gym_data.txt\n";
    }

    void load() {
        ifstream f("gym_data.txt");
        if (!f) {
            cout << "Файл сохранения не найден.\n";
            return;
        }

        size_t count;
        f >> nextId >> count;
        if (!f || count > 100000) {
            cout << "Ошибка формата файла.\n";
            return;
        }

        vector<unique_ptr<Person>> loaded;
        vector<Workout> loadedWorkouts;

        for (size_t i = 0; i < count; ++i) {
            string t, n, extra;
            int id;
            double value;
            f >> t >> id >> quoted(n) >> quoted(extra) >> value;

            if (!f) {
                cout << "Ошибка чтения данных.\n";
                return;
            }

            auto p = PersonFactory::create(t, id, n, extra, value);
            if (!p) {
                cout << "Неизвестный тип объекта.\n";
                return;
            }
            loaded.push_back(move(p));
        }

        size_t wc;
        f >> wc;
        if (!f || wc > 100000) {
            cout << "Ошибка чтения тренировок.\n";
            return;
        }

        for (size_t i = 0; i < wc; ++i) {
            Workout w;
            f >> w.clientId >> w.trainerId
              >> quoted(w.date) >> quoted(w.title);
            if (!f) {
                cout << "Ошибка чтения тренировки.\n";
                return;
            }
            loadedWorkouts.push_back(w);
        }

        people = move(loaded);
        workouts = move(loadedWorkouts);
        cout << "Данные загружены.\n";
    }

    void menu() {
        int c;
        do {
            cout << "\n===== СИСТЕМА УПРАВЛЕНИЯ СПОРТЗАЛОМ =====\n"
                 << "1. Добавить клиента\n"
                 << "2. Добавить тренера\n"
                 << "3. Добавить тренировку\n"
                 << "4. Показать клиентов и тренеров\n"
                 << "5. Показать тренировки\n"
                 << "6. Сохранить данные\n"
                 << "7. Загрузить данные\n"
                 << "0. Выход\n"
                 << "Выбор: ";

            if (!(cin >> c)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Введите номер пункта.\n";
                continue;
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            switch (c) {
                case 1: addClient(); break;
                case 2: addTrainer(); break;
                case 3: addWorkout(); break;
                case 4: showPeople(); break;
                case 5: showWorkouts(); break;
                case 6: save(); break;
                case 7: load(); break;
                case 0: cout << "Завершение программы.\n"; break;
                default: cout << "Неверный пункт меню.\n";
            }
        } while (c != 0);
    }
};

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    cout << "ЛАБОРАТОРНАЯ РАБОТА №16\n"
         << "Вариант 8. Продвинутый уровень\n";

    Gym gym;
    gym.menu();
    return 0;
}
