#include "profile.h"
#include "test_runner.h"
//#include "paginator.h"

#include <functional>
#include <future>
#include <map>
#include <string>
#include <vector>

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

struct Stats {
  map<string, int> word_frequences;

  void operator+=(const Stats& other) {
    for (const auto& [word, frequency] : other.word_frequences) {
      word_frequences[word] += frequency;
    }
  }
};

Stats ExploreLine(const set<string>& key_words, const string& line) {
  Stats result;

  vector<string> strs;

  size_t pos = 0;
  const size_t posEnd = line.npos;

  while (pos != posEnd) {
    size_t space = line.find(' ', pos);
    strs.push_back(line.substr(pos, space - pos));

    if (space == posEnd) {
      pos = posEnd;
    } else {
      pos = line.find_first_not_of(' ', space);
    }
  }

  for (const auto& elem : strs) {
    if (key_words.count(elem)) {
      result.word_frequences[elem] += 1;
    }
  }

  return result;
}

Stats ExploreKeyWordsSingleThread(const set<string>& key_words,
                                  istream& input) {
  Stats result;
  for (string line; getline(input, line);) {
    result += ExploreLine(key_words, line);
  }
  return result;
}

template <typename vectorOfStrs>
Stats calculatePageStats(const set<string>& key_words,
                         const vectorOfStrs& vec) {
  Stats result;
  for (const auto& line : vec) {
    result += ExploreLine(key_words, line);
  }

  return result;
}

Stats ExploreKeyWords(const set<string>& key_words, istream& input) {
  // Реализуйте эту функцию
  vector<string> lines;
  for (string line; getline(input, line);) {
    lines.push_back(move(line));
  }

  auto pageSize = 1000;
  vector<future<Stats>> futures;

  for (auto page : Paginate(lines, pageSize)) {
    futures.push_back(async(
        [page, &key_words] { return calculatePageStats(key_words, page); }));
  }

  Stats result;

  for (auto& item : futures) {
    result += item.get();
  }

  return result;
}

void TestBasic() {
  const set<string> key_words = {"yangle", "rocks", "sucks", "all"};

  stringstream ss;
  ss << "this new yangle service really rocks\n";
  ss << "It sucks when yangle isn't available\n";
  ss << "10 reasons why yangle is the best IT company\n";
  ss << "yangle rocks others suck\n";
  ss << "Goondex really sucks, but yangle rocks. Use yangle\n";

  const auto stats = ExploreKeyWords(key_words, ss);
  const map<string, int> expected = {{"yangle", 6}, {"rocks", 2}, {"sucks", 1}};
  ASSERT_EQUAL(stats.word_frequences, expected);
}

void testSingleThread() {
  const set<string> key_words = {"yangle", "rocks", "sucks", "all"};

  stringstream ss;
  ss << "this new yangle service really rocks\n";
  ss << "It sucks when yangle isn't available\n";
  ss << "10 reasons why yangle is the best IT company\n";
  ss << "yangle rocks others suck\n";
  ss << "Goondex really sucks, but yangle rocks. Use yangle\n";

  const auto stats = ExploreKeyWordsSingleThread(key_words, ss);
  const map<string, int> expected = {{"yangle", 6}, {"rocks", 2}, {"sucks", 1}};
  ASSERT_EQUAL(stats.word_frequences, expected);
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestBasic);
  RUN_TEST(tr, testSingleThread);
}
