#include <algorithm>
#include <future>
#include <vector>

#include "test_runner.h"
//#include "paginator.h"

using namespace std;

template <typename Iterator>
class IteratorRange {
 public:
  IteratorRange(Iterator begin, Iterator end)
      : first(begin), last(end), size_(distance(first, last)) {}

  Iterator begin() const { return first; }

  Iterator end() const { return last; }

  size_t size() const { return size_; }

 private:
  Iterator first, last;
  size_t size_;
};

template <typename Iterator>
class Paginator {
 private:
  vector<IteratorRange<Iterator>> pages;

 public:
  Paginator(Iterator begin, Iterator end, size_t page_size) {
    for (size_t left = distance(begin, end); left > 0;) {
      size_t current_page_size = min(page_size, left);
      Iterator current_page_end = next(begin, current_page_size);
      pages.push_back({begin, current_page_end});

      left -= current_page_size;
      begin = current_page_end;
    }
  }

  auto begin() const { return pages.begin(); }

  auto end() const { return pages.end(); }

  size_t size() const { return pages.size(); }
};

template <typename C>
auto Paginate(C& c, size_t page_size) {
  return Paginator(begin(c), end(c), page_size);
}

template <typename ContainerOfVectors>
int64_t calculatePageSum(const ContainerOfVectors& matrix) {
  int64_t sum = 0;
  for (auto& row : matrix) {
    for (auto& elem : row) {
      sum += elem;
    }
  }

  return sum;
}

int64_t CalculateMatrixSum(const vector<vector<int>>& matrix) {
  auto pageSize = 1000;

  vector<future<int64_t>> futures;
  for (auto page : Paginate(matrix, pageSize)) {
    futures.push_back(async([page] { return calculatePageSum(page); }));
  }

  int64_t result = 0;
  for (auto& item : futures) {
    result += item.get();
  }

  return result;
}

void TestCalculateMatrixSum() {
  const vector<vector<int>> matrix = {
      {1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 16}};
  ASSERT_EQUAL(CalculateMatrixSum(matrix), 136);
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestCalculateMatrixSum);
}
