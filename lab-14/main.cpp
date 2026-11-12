#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include <iostream>
#include <vector>
#include <functional>
#include <algorithm>
#include <iomanip>
#include <limits>

using namespace std;

int main() {
    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
    #endif
    cout << "ЛАБОРАТОРНАЯ РАБОТА №14\n";
    cout << "Вариант 8. Продвинутый уровень\n";

    // ===== ЗАДАЧА О РЮКЗАКЕ =====
    int n, W;

    cout << "\n=== ЗАДАЧА О РЮКЗАКЕ ===\n";
    cout << "Количество предметов: ";
    cin >> n;
    cout << "Вместимость рюкзака: ";
    cin >> W;

    if (!cin || n <= 0 || W < 0) {
        cout << "Некорректные входные данные.\n";
        return 1;
    }

    vector<int> weight(n), value(n);

    for (int i = 0; i < n; ++i) {
        cout << "Предмет " << i + 1 << " (вес, стоимость): ";
        cin >> weight[i] >> value[i];

        if (!cin || weight[i] <= 0 || value[i] < 0) {
            cout << "Некорректные данные предмета.\n";
            return 1;
        }
    }

    // Top-down: рекурсия с мемоизацией.
    vector<vector<long long>> dp(n + 1,
        vector<long long>(W + 1, -1));

    function<long long(int, int)> solve = [&](int i, int cap) {
        if (i == 0) return 0LL;

        long long& ans = dp[i][cap];
        if (ans != -1) return ans;

        ans = solve(i - 1, cap);

        if (weight[i - 1] <= cap) {
            ans = max(ans, value[i - 1] +
                solve(i - 1, cap - weight[i - 1]));
        }

        return ans;
    };

    for (int cap = 0; cap <= W; ++cap)
        solve(n, cap);

    cout << "\nТаблица рюкзака (мемоизация):\n";
    cout << "Предметы/Вес";

    for (int j = 0; j <= W; ++j)
        cout << setw(5) << j;
    cout << "\n";

    for (int i = 0; i <= n; ++i) {
        cout << setw(12) << i;
        for (int j = 0; j <= W; ++j)
            cout << setw(5) << dp[i][j];
        cout << "\n";
    }

    long long best = solve(n, W);
    int totalWeight = 0;
    vector<int> selected;

    // Восстановление выбранных предметов.
    int cap = W;

    for (int i = n; i > 0; --i) {
        if (solve(i, cap) != solve(i - 1, cap)) {
            selected.push_back(i);
            totalWeight += weight[i - 1];
            cap -= weight[i - 1];
        }
    }

    reverse(selected.begin(), selected.end());

    cout << "\nМаксимальная стоимость: " << best << "\n";
    cout << "Общий вес: " << totalWeight << "\n";
    cout << "Выбранные предметы: ";

    if (selected.empty()) cout << "нет";
    for (int id : selected) cout << id << " ";
    cout << "\n";

    // ===== НАИБОЛЬШАЯ ОБЩАЯ ПОДПОСЛЕДОВАТЕЛЬНОСТЬ =====
    cout << "\n=== НАИБОЛЬШАЯ ОБЩАЯ ПОДПОСЛЕДОВАТЕЛЬНОСТЬ ===\n";

    int m, k;
    cout << "Длина первой последовательности: ";
    cin >> m;
    cout << "Длина второй последовательности: ";
    cin >> k;

    if (!cin || m < 0 || k < 0) {
        cout << "Некорректные размеры последовательностей.\n";
        return 1;
    }

    vector<int> a(m), b(k);

    cout << "Первая последовательность: ";
    for (int& x : a) cin >> x;

    cout << "Вторая последовательность: ";
    for (int& x : b) cin >> x;

    if (!cin) {
        cout << "Ошибка ввода последовательностей.\n";
        return 1;
    }

    // Bottom-up: табличное динамическое программирование.
    vector<vector<int>> lcs(m + 1, vector<int>(k + 1, 0));

    for (int i = 1; i <= m; ++i) {
        for (int j = 1; j <= k; ++j) {
            if (a[i - 1] == b[j - 1])
                lcs[i][j] = lcs[i - 1][j - 1] + 1;
            else
                lcs[i][j] = max(lcs[i - 1][j], lcs[i][j - 1]);
        }
    }

    cout << "\nТаблица LCS:\n";
    cout << setw(5) << " ";

    for (int x : b) cout << setw(5) << x;
    cout << "\n";

    for (int i = 0; i <= m; ++i) {
        if (i == 0) cout << setw(5) << "-";
        else cout << setw(5) << a[i - 1];

        for (int j = 0; j <= k; ++j)
            cout << setw(5) << lcs[i][j];
        cout << "\n";
    }

    // Восстановление LCS.
    vector<int> result;
    int i = m, j = k;

    while (i > 0 && j > 0) {
        if (a[i - 1] == b[j - 1]) {
            result.push_back(a[i - 1]);
            --i;
            --j;
        } else if (lcs[i - 1][j] >= lcs[i][j - 1]) {
            --i;
        } else {
            --j;
        }
    }

    reverse(result.begin(), result.end());

    cout << "\nДлина LCS: " << lcs[m][k] << "\n";
    cout << "Наибольшая общая подпоследовательность: ";

    for (int x : result) cout << x << " ";
    if (result.empty()) cout << "нет";
    cout << "\n";

    return 0;
}
