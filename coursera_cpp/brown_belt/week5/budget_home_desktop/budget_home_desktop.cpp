#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

#include "test_runner.h"

using namespace std;

namespace {
static const int SECONDS_IN_DAY = 60 * 60 * 24;
}

class Date {
 public:
  Date() : year_(0), month_(0), day_(0) {}

  friend istream& operator>>(istream& in, Date& date);
  friend ostream& operator<<(ostream& out, const Date& date);
  Date& operator++() { return *this; }

  time_t asTimeStamp() const {
    tm t;
    t.tm_sec = 0;
    t.tm_min = 0;
    t.tm_hour = 0;
    t.tm_mday = day_;
    t.tm_mon = month_ - 1;
    t.tm_year = year_ - 1900;
    t.tm_isdst = 0;
    return mktime(&t);
  }

  int year() const { return year_; }

  int month() const { return month_; }

  int day() const { return day_; }

 private:
  int year_;
  int month_;
  int day_;
};

istream& operator>>(istream& in, Date& date) {
  string str;
  in >> str;

  istringstream str_stream(move(str));
  str_stream >> date.year_;
  str_stream.ignore(1);
  str_stream >> date.month_;
  str_stream.ignore(1);
  str_stream >> date.day_;
  return in;
}

ostream& operator<<(ostream& out, const Date& date) {
  out << setfill('0') << setw(4) << date.year() << "-" << setw(2)
      << date.month() << "-" << setw(2) << date.day();

  return out;
}

int computeDaysDiff(const Date& from, const Date& to) {
  const time_t timeStampTo = to.asTimeStamp();
  const time_t timeStampFrom = from.asTimeStamp();
  return (timeStampTo - timeStampFrom) / SECONDS_IN_DAY;
}

class BudgetManager {
 public:
  double computeIncome(const Date& from, const Date& to) {
    double commonIncome = 0.;
    auto daysCnt = computeDaysDiff(from, to) + 1;
    auto fromAsStamp = from.asTimeStamp();
    for (int i = 0; i < daysCnt; ++i) {
      commonIncome += valByDate_[fromAsStamp + i * SECONDS_IN_DAY];
    }
    return commonIncome;
  }

  void earn(const Date& from, const Date& to, int value) {
    auto daysCnt = computeDaysDiff(from, to) + 1;
    double incomeByDay = static_cast<double>(value) / daysCnt;
    auto fromAsStamp = from.asTimeStamp();
    for (int i = 0; i < daysCnt; ++i) {
      valByDate_[fromAsStamp + i * SECONDS_IN_DAY] += incomeByDay;
    }
  }

  void payTax(const Date& from, const Date& to) {
    auto daysCnt = computeDaysDiff(from, to) + 1;
    auto fromAsStamp = from.asTimeStamp();
    for (int i = 0; i < daysCnt; ++i) {
      valByDate_[fromAsStamp + i * SECONDS_IN_DAY] *= 0.87;
    }
  }

 private:
  unordered_map<time_t, double> valByDate_;
};

void testReadDate() {
  string str("2020-07-22");
  istringstream input(str);
  Date date;
  input >> date;

  ostringstream out;
  out << date;
  ASSERT_EQUAL(out.str(), str);
}

void testIncome() {
  string str(
      "8\n"
      "Earn 2000-01-02 2000-01-06 20\n"
      "ComputeIncome 2000-01-01 2001-01-01\n"
      "PayTax 2000-01-02 2000-01-03\n"
      "ComputeIncome 2000-01-01 2001-01-01\n"
      "Earn 2000-01-03 2000-01-03 10\n"
      "ComputeIncome 2000-01-01 2001-01-01\n"
      "PayTax 2000-01-03 2000-01-03\n"
      "ComputeIncome 2000-01-01 2001-01-01\n");
  istringstream in_stream(str);
  ostringstream out_stream;
  BudgetManager budget;
  auto queryCnt = 0;
  in_stream >> queryCnt;
  for (int i = 0; i < queryCnt; ++i) {
    string queryType;
    in_stream >> queryType;

    Date from, to;
    in_stream >> from >> to;
    if (queryType == "ComputeIncome") {
      double value = budget.computeIncome(from, to);
      out_stream << setprecision(25) << value << '\n';
    } else if (queryType == "Earn") {
      int value;
      in_stream >> value;
      budget.earn(from, to, value);
    } else if (queryType == "PayTax") {
      budget.payTax(from, to);
    }
  }

  string result(
      "20\n"
      "18.96\n"
      "28.96\n"
      "27.2076\n");
  ASSERT_EQUAL(out_stream.str(), result);
}

void testIncome2() {
  string str(
      "4\n"
      "Earn 2000-01-02 2000-01-06 20\n"
      "ComputeIncome 2000-01-02 2000-01-06\n"
      "PayTax 2000-01-02 2000-01-03\n"
      "ComputeIncome 2000-01-02 2000-01-06\n");
  istringstream in(str);
  in.precision(25);
  ostringstream out;
  BudgetManager budget;

  auto queryCnt = 0;
  in >> queryCnt;
  for (int i = 0; i < queryCnt; ++i) {
    string queryType;
    in >> queryType;

    Date from, to;
    in >> from >> to;
    if (queryType == "ComputeIncome") {
      double value = budget.computeIncome(from, to);
      cout << value << '\n';
    } else if (queryType == "Earn") {
      int value;
      in >> value;
      budget.earn(from, to, value);
    } else if (queryType == "PayTax") {
      budget.payTax(from, to);
    }
  }
}

int main() {
  cout.precision(25);

  //    TestRunner tr;
  //    RUN_TEST(tr, testReadDate);
  //    RUN_TEST(tr, testIncome);
  //    RUN_TEST(tr, testIncome2);

  BudgetManager budget;
  int queryCnt = 0;
  cin >> queryCnt;
  for (int i = 0; i < queryCnt; ++i) {
    string queryType;
    cin >> queryType;

    Date from, to;
    cin >> from >> to;
    if (queryType == "ComputeIncome") {
      double value = budget.computeIncome(from, to);
      cout << setprecision(25) << value << '\n';
    } else if (queryType == "Earn") {
      int value;
      cin >> value;
      budget.earn(from, to, value);
    } else if (queryType == "PayTax") {
      budget.payTax(from, to);
    }
  }

  return 0;
}
