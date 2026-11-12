#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
#include <limits>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <unordered_set>
#include <algorithm>
#include <cctype>

using namespace std;

// Приведение строки к нижнему регистру.
string lower(string s) {
    for (char& c : s)
        c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    return s;
}

// Префикс-функция КМП.
vector<int> prefixFunction(const string& s) {
    vector<int> pi(s.size());

    for (int i = 1; i < static_cast<int>(s.size()); ++i) {
        int j = pi[i - 1];

        while (j > 0 && s[i] != s[j])
            j = pi[j - 1];

        if (s[i] == s[j]) ++j;
        pi[i] = j;
    }

    return pi;
}

// Поиск всех вхождений образца.
vector<int> KMP(const string& text, const string& pattern) {
    vector<int> result;

    if (pattern.empty()) return result;

    string t = lower(text);
    string p = lower(pattern);
    vector<int> pi = prefixFunction(p);

    int j = 0;

    for (int i = 0; i < static_cast<int>(t.size()); ++i) {
        while (j > 0 && t[i] != p[j])
            j = pi[j - 1];

        if (t[i] == p[j]) ++j;

        if (j == static_cast<int>(p.size())) {
            result.push_back(i - j + 1);
            j = pi[j - 1];
        }
    }

    return result;
}

int main() {
#ifdef _WIN32
    #define NOMINMAX
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif

    cout << "ЛАБОРАТОРНАЯ РАБОТА №15\n";
    cout << "Вариант 8. Продвинутый уровень\n";

    // ===== АЛГОРИТМ КМП =====
    cout << "\n=== ПОИСК КМП ===\n";
    cout << "1 - Ввод с клавиатуры\n";
    cout << "2 - Чтение из файла\n";
    cout << "Выбор: ";

    int choice;
    cin >> choice;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    string text;

    if (choice == 2) {
        string filename;
        cout << "Имя файла: ";
        getline(cin, filename);

        ifstream file(filename);

        if (!file) {
            cout << "Ошибка открытия файла.\n";
            return 1;
        }

        string line;
        while (getline(file, line)) {
            text += line + "\n";
        }
    } else if (choice == 1) {
        cout << "Введите текст: ";
        getline(cin, text);
    } else {
        cout << "Неверный выбор.\n";
        return 1;
    }

    string pattern;
    cout << "Введите образец для поиска: ";
    getline(cin, pattern);

    if (pattern.empty()) {
        cout << "Образец не должен быть пустым.\n";
        return 1;
    }

    vector<int> pi = prefixFunction(lower(pattern));
    vector<int> positions = KMP(text, pattern);

    cout << "\nПрефикс-функция:\n";
    for (int x : pi) cout << x << " ";
    cout << "\n";

    cout << "\nНайденные позиции (нумерация с 0):\n";

    if (positions.empty()) {
        cout << "Совпадений нет.\n";
    } else {
        for (int pos : positions)
            cout << pos << " ";
        cout << "\n";
    }

    cout << "Количество вхождений: " << positions.size() << "\n";

    // ===== UNORDERED_SET =====
    cout << "\n=== УНИКАЛЬНЫЕ ЭЛЕМЕНТЫ STL ===\n";

    int n;
    cout << "Количество элементов: ";
    cin >> n;

    if (!cin || n < 0) {
        cout << "Некорректное количество.\n";
        return 1;
    }

    unordered_set<string> unique;
    cout << "Введите элементы:\n";

    for (int i = 0; i < n; ++i) {
        string x;
        cin >> x;
        unique.insert(x);
    }

    vector<string> sorted(unique.begin(), unique.end());
    sort(sorted.begin(), sorted.end());

    cout << "\nУникальные элементы:\n";
    for (const string& x : sorted)
        cout << x << " ";

    cout << "\nКоличество уникальных элементов: "
         << unique.size() << "\n";

    return 0;
}
