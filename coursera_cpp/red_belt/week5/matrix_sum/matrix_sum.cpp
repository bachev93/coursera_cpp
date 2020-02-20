#include <vector>
#include <future>
#include <algorithm>

#include "test_runner.h"
#include "paginator.h"

using namespace std;

template <typename ContainerOfVectors>
int64_t calculatePageSum(const ContainerOfVectors& matrix) {
    int64_t sum = 0;
    for(auto& row : matrix) {
        for(auto& elem : row) {
            sum += elem;
        }
    }

    return sum;
}

int64_t CalculateMatrixSum(const vector<vector<int>>& matrix) {
    auto matrixSize = matrix.size();
    auto pageSize = 1000;

    vector<future<int64_t>> futures;
    for(auto page : Paginate(matrix, pageSize)) {
        futures.push_back(async([page]{
            return calculatePageSum(page);
        }));
    }

    int64_t result = 0;
    for(auto& item : futures) {
        result += item.get();
    }

    return result;
}

void TestCalculateMatrixSum() {
    const vector<vector<int>> matrix = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12},
        {13, 14, 15, 16}
    };
    ASSERT_EQUAL(CalculateMatrixSum(matrix), 136);
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestCalculateMatrixSum);
}
