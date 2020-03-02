#include "test_runner.h"
#include <algorithm>
#include <memory>
#include <vector>

using namespace std;

template <typename RandomIt>
void MergeSort(RandomIt range_begin, RandomIt range_end) {
    // 1. Если диапазон содержит меньше 2 элементов, выходим из функции
    int rangeLength = range_end - range_begin;
    if(rangeLength < 2) {
        return;
    }

     // 2. Создаем вектор, содержащий все элементы текущего диапазона
    vector<typename RandomIt::value_type> res(make_move_iterator(range_begin),
                                              make_move_iterator(range_end));

    // 3. Разбиваем вектор на три равные части
    auto oneThird = res.begin() + rangeLength / 3;
    auto twoThird = res.begin() + rangeLength * 2 / 3;

    // 4. Вызываем функцию MergeSort от каждой трети вектора
    MergeSort(res.begin(), oneThird);
    MergeSort(oneThird, twoThird);
    MergeSort(twoThird, res.end());

     // 5. С помощью алгоритма merge cливаем первые две трети во временный вектор
    vector<typename RandomIt::value_type> tmp;
    merge(make_move_iterator(res.begin()), make_move_iterator(oneThird),
          make_move_iterator(oneThird), make_move_iterator(twoThird), back_inserter(tmp));

    // 6. С помощью алгоритма merge сливаем отсортированные части в исходный диапазон
    merge(make_move_iterator(tmp.begin()), make_move_iterator(tmp.end()),
          make_move_iterator(twoThird), make_move_iterator(res.end()), range_begin);
}

void TestIntVector() {
    vector<int> numbers = {6, 1, 3, 9, 1, 9, 8, 12, 1};
    MergeSort(begin(numbers), end(numbers));
    ASSERT(is_sorted(begin(numbers), end(numbers)));
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestIntVector);
    return 0;
}
