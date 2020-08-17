#include "test_runner.h"

#include <algorithm>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

using namespace std;

template <typename It>
class Range {
 public:
  Range(It begin, It end) : begin_(begin), end_(end) {}
  It begin() const { return begin_; }
  It end() const { return end_; }

 private:
  It begin_;
  It end_;
};

pair<string_view, optional<string_view>> SplitTwoStrict(
    string_view s,
    string_view delimiter = " ") {
  const size_t pos = s.find(delimiter);
  if (pos == s.npos) {
    return {s, nullopt};
  } else {
    return {s.substr(0, pos), s.substr(pos + delimiter.length())};
  }
}

vector<string_view> Split(string_view s, string_view delimiter = " ") {
  vector<string_view> parts;
  if (s.empty()) {
    return parts;
  }
  while (true) {
    const auto [lhs, rhs_opt] = SplitTwoStrict(s, delimiter);
    parts.push_back(lhs);
    if (!rhs_opt) {
      break;
    }
    s = *rhs_opt;
  }
  return parts;
}

class Domain {
 public:
  explicit Domain(string_view text) {
    vector<string_view> parts = Split(text, ".");
    parts_reversed_.assign(rbegin(parts), rend(parts));
  }

  size_t GetPartCount() const { return parts_reversed_.size(); }

  auto GetParts() const {
    return Range(rbegin(parts_reversed_), rend(parts_reversed_));
  }
  auto GetReversedParts() const {
    return Range(begin(parts_reversed_), end(parts_reversed_));
  }

  bool operator==(const Domain& other) const {
    return parts_reversed_ == other.parts_reversed_;
  }

 private:
  vector<string> parts_reversed_;
};

ostream& operator<<(ostream& stream, const Domain& domain) {
  bool first = true;
  for (const string_view part : domain.GetParts()) {
    if (!first) {
      stream << '.';
    } else {
      first = false;
    }
    stream << part;
  }
  return stream;
}

// domain is subdomain of itself
bool IsSubdomain(const Domain& subdomain, const Domain& domain) {
  const auto subdomain_reversed_parts = subdomain.GetReversedParts();
  const auto domain_reversed_parts = domain.GetReversedParts();
  return subdomain.GetPartCount() >= domain.GetPartCount() &&
         equal(begin(domain_reversed_parts), end(domain_reversed_parts),
               begin(subdomain_reversed_parts));
}

bool IsSubOrSuperDomain(const Domain& lhs, const Domain& rhs) {
  return lhs.GetPartCount() >= rhs.GetPartCount() ? IsSubdomain(lhs, rhs)
                                                  : IsSubdomain(rhs, lhs);
}

class DomainChecker {
 public:
  template <typename InputIt>
  DomainChecker(InputIt domains_begin, InputIt domains_end) {
    sorted_domains_.reserve(distance(domains_begin, domains_end));
    for (const Domain& domain : Range(domains_begin, domains_end)) {
      sorted_domains_.push_back(&domain);
    }
    sort(begin(sorted_domains_), end(sorted_domains_), IsDomainLess);
    sorted_domains_ = AbsorbSubdomains(move(sorted_domains_));
  }

  // Check if candidate is subdomain of some domain
  bool IsSubdomain(const Domain& candidate) const {
    const auto it = upper_bound(begin(sorted_domains_), end(sorted_domains_),
                                &candidate, IsDomainLess);
    if (it == begin(sorted_domains_)) {
      return false;
    }
    return ::IsSubdomain(candidate, **prev(it));
  }

 private:
  vector<const Domain*> sorted_domains_;

  static bool IsDomainLess(const Domain* lhs, const Domain* rhs) {
    const auto lhs_reversed_parts = lhs->GetReversedParts();
    const auto rhs_reversed_parts = rhs->GetReversedParts();
    return lexicographical_compare(
        begin(lhs_reversed_parts), end(lhs_reversed_parts),
        begin(rhs_reversed_parts), end(rhs_reversed_parts));
  }

  static vector<const Domain*> AbsorbSubdomains(vector<const Domain*> domains) {
    domains.erase(unique(begin(domains), end(domains),
                         [](const Domain* lhs, const Domain* rhs) {
                           return IsSubOrSuperDomain(*lhs, *rhs);
                         }),
                  end(domains));
    return domains;
  }
};

vector<Domain> ReadDomains(istream& in_stream = cin) {
  vector<Domain> domains;

  size_t count;
  in_stream >> count;
  domains.reserve(count);

  for (size_t i = 0; i < count; ++i) {
    string domain_text;
    in_stream >> domain_text;
    domains.emplace_back(domain_text);
  }
  return domains;
}

vector<bool> CheckDomains(const vector<Domain>& banned_domains,
                          const vector<Domain>& domains_to_check) {
  const DomainChecker checker(begin(banned_domains), end(banned_domains));

  vector<bool> check_results;
  check_results.reserve(domains_to_check.size());
  for (const Domain& domain_to_check : domains_to_check) {
    check_results.push_back(!checker.IsSubdomain(domain_to_check));
  }

  return check_results;
}

void PrintCheckResults(const vector<bool>& check_results,
                       ostream& out_stream = cout) {
  for (const bool check_result : check_results) {
    out_stream << (check_result ? "Good" : "Bad") << "\n";
  }
}

void testSplit() {
  auto res = Split("abra kadabra boo");
  vector<string_view> vec = {"abra", "kadabra", "boo"};
  ASSERT_EQUAL(res, vec);
}

void testReversedDomain() {
  Domain domain("msc.m.yandex.ru");
  auto res = domain.GetReversedParts();
  vector<string> resultVec(res.begin(), res.end());

  vector<string> vec = {"ru", "yandex", "m", "msc"};

  ASSERT_EQUAL(resultVec, vec);
}

void testSubDomain() {
  Domain domain("yandex.ru");
  auto subDomain = domain;
  ASSERT(IsSubdomain(subDomain, domain))
}

void testSubDomainWrongArgs() {
  Domain domain("ru");
  Domain subDomain("msc.m.yandex.ru");
  ASSERT(IsSubdomain(subDomain, domain));
}

void testAbsorbSubDomain() {
  vector<Domain> domains = {Domain("ya.ru"), Domain("m.ya.ru")};
  DomainChecker checker(domains.begin(), domains.end());
  ASSERT(checker.IsSubdomain(Domain("pets.ya.ru")));
}

void testReversedAnswers() {
  vector<Domain> bannedDomains = {Domain("ya.ru"), Domain("com")};
  vector<Domain> domainsToCheck = {Domain("maps.me"), Domain("google.com"),
                                   Domain("m.ya.ru")};

  ostringstream out;
  auto answer = CheckDomains(bannedDomains, domainsToCheck);
  PrintCheckResults(answer, out);

  string result(
      "Good\n"
      "Bad\n"
      "Bad\n");
  ASSERT_EQUAL(out.str(), result);
}

void testReadDomains() {
  vector<string> vec = {"maps.me", "yandex.ru", "google.com"};
  vector<Domain> domains;
  stringstream str_stream;
  str_stream << vec.size() << "\n";
  for (const auto& str : vec) {
    domains.emplace_back(str);
    str_stream << str << "\n";
  }

  auto res = ReadDomains(str_stream);

  ASSERT_EQUAL(res, domains);
}

void common_test() {
  vector<string> bannedVec = {"ya.ru", "maps.me", "m.ya.ru", "com"};
  stringstream str_stream;
  str_stream << bannedVec.size() << "\n";
  for (const auto& str : bannedVec) {
    str_stream << str << "\n";
  }
  auto bannedDomains = ReadDomains(str_stream);

  DomainChecker checker(bannedDomains.begin(), bannedDomains.end());
  ASSERT(checker.IsSubdomain(Domain("mayak.com")));

  vector<string> checkVec = {"ya.ru",          "ya.com",   "m.maps.me",
                             "moscow.m.ya.ru", "maps.com", "maps.ru",
                             "ya.ya",          "com.me",   "ya.com.me"};
  str_stream.str("");
  str_stream << checkVec.size() << "\n";
  for (const auto& str : checkVec) {
    str_stream << str << "\n";
  }
  auto domainsToCheck = ReadDomains(str_stream);

  auto answer = CheckDomains(bannedDomains, domainsToCheck);
  ostringstream out;
  PrintCheckResults(answer, out);

  string res(
      "Bad\n"
      "Bad\n"
      "Bad\n"
      "Bad\n"
      "Bad\n"
      "Good\n"
      "Good\n"
      "Good\n"
      "Good\n");
  ASSERT_EQUAL(out.str(), res);
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, testSplit);
  RUN_TEST(tr, testReversedDomain);
  RUN_TEST(tr, testSubDomain);
  RUN_TEST(tr, testSubDomainWrongArgs);
  RUN_TEST(tr, testAbsorbSubDomain);

  RUN_TEST(tr, testReversedAnswers);
  RUN_TEST(tr, testReadDomains);
  RUN_TEST(tr, common_test);

  const vector<Domain> banned_domains = ReadDomains();
  const vector<Domain> domains_to_check = ReadDomains();
  PrintCheckResults(CheckDomains(banned_domains, domains_to_check));
  return 0;
}
