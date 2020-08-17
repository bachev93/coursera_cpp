#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "test_runner.h"

using namespace std;

// рекурсивное дерево доменов
// флаг isEnd_ указывает на то, что забаненный домен завершается на
// данном уровне
struct Domains {
  Domains() : isEnd_(false), sub_domains_() {}

  bool isEnd_;
  map<string, Domains> sub_domains_;
};

vector<string> split_by(string_view s, char sep = '.') {
  vector<string> res;
  while (!s.empty()) {
    size_t pos = s.find(sep);
    res.emplace_back(s.substr(0, pos));
    s.remove_prefix(pos != s.npos ? pos + 1 : s.size());
  }
  return res;
}

class DomainManager {
 public:
  DomainManager() = default;

  void read_banned_domains(istream& input = cin) {
    size_t count;
    input >> count;

    for (size_t i = 0; i < count; ++i) {
      string domain;
      input >> domain;
      auto splitted_dom = split_by(domain);

      Domains* current_lvl = &banned_domains_;
      for (auto it = splitted_dom.rbegin(), itEnd = splitted_dom.rend();
           it != itEnd; ++it) {
        current_lvl = &current_lvl->sub_domains_[*it];
      }
      current_lvl->isEnd_ = true;
    }
  }

  bool is_banned(string_view domain) {
    auto splitted_dom = split_by(domain);
    Domains* current_lvl = &banned_domains_;
    for (auto it = splitted_dom.rbegin(), itEnd = splitted_dom.rend();
         it != itEnd; ++it) {
      if (current_lvl->sub_domains_.count(*it)) {
        if (current_lvl->sub_domains_[*it].isEnd_ == true) {
          return true;
        }
        current_lvl = &current_lvl->sub_domains_[*it];
      } else {
        break;
      }
    }
    return false;
  }

 private:
  Domains banned_domains_;
};

void test_split_by() {
  string str("msc.m.ya.ru");

  auto res = split_by(str);

  for (const auto& elem : res) {
    cout << elem << " ";
  }
  cout << endl;
}

void test_read_domains() {
  stringstream str_stream;
  vector<string> banned_domains = {"ya.ru", "maps.me", "m.ya.ru", "com"};
  str_stream << banned_domains.size() << "\n";
  for (auto& domain : banned_domains) {
    str_stream << move(domain) << "\n";
  }

  DomainManager manager;
  manager.read_banned_domains(str_stream);

  ostringstream out;
  vector<string> domains = {"com"};
  for (const auto& domain : domains) {
    if (manager.is_banned(domain)) {
      out << "Bad\n";
    } else {
      out << "Good\n";
    }
  }

  string res = {"Bad\n"};
  ASSERT_EQUAL(out.str(), res);
}

void common_test() {
  stringstream str_stream;
  vector<string> banned_domains = {"ya.ru", "maps.me", "m.ya.ru", "com"};
  str_stream << banned_domains.size() << "\n";
  for (auto& domain : banned_domains) {
    str_stream << move(domain) << "\n";
  }

  DomainManager manager;
  manager.read_banned_domains(str_stream);

  ostringstream out;
  vector<string> domains = {"ya.ru",          "ya.com",   "m.maps.me",
                            "moscow.m.ya.ru", "maps.com", "maps.ru",
                            "ya.ya"};
  for (const auto& domain : domains) {
    if (manager.is_banned(domain)) {
      out << "Bad\n";
    } else {
      out << "Good\n";
    }
  }

  string res(
      "Bad\n"
      "Bad\n"
      "Bad\n"
      "Bad\n"
      "Bad\n"
      "Good\n"
      "Good\n");
  ASSERT_EQUAL(out.str(), res);
}

int main() {
  // Для ускорения чтения данных отключается синхронизация
  // cin и cout с stdio,
  // а также выполняется отвязка cin от cout
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  //    TestRunner tr;
  //    RUN_TEST(tr, test_split_by);
  //    RUN_TEST(tr, test_read_domains);
  //    RUN_TEST(tr, common_test);

  DomainManager manager;
  manager.read_banned_domains();

  size_t domains_cnt;
  cin >> domains_cnt;
  for (size_t i = 0; i < domains_cnt; ++i) {
    string domain;
    cin >> domain;

    if (manager.is_banned(domain)) {
      cout << "Bad\n";
    } else {
      cout << "Good\n";
    }
  }

  return 0;
}
