#pragma once

#include <chrono>
#include <iostream>
#include <sstream>
#include <string>

#include "profile.h"

using namespace std;
using namespace chrono;

struct TotalDuration {
  string message_;
  steady_clock::duration value_;

  explicit TotalDuration(const string& msg = "")
      : message_(msg + ": "), value_(0) {}

  ~TotalDuration() {
    ostringstream os;
    os << message_ << duration_cast<milliseconds>(value_).count() << " ms"
       << endl;
    cerr << os.str();
  }
};

class AddDuration {
 public:
  explicit AddDuration(steady_clock::duration& dest)
      : add_to(dest), start(steady_clock::now()) {}

  explicit AddDuration(TotalDuration& dest) : AddDuration(dest.value_) {}

  ~AddDuration() { add_to += steady_clock::now() - start; }

 private:
  steady_clock::duration& add_to;
  steady_clock::time_point start;
};

#define ADD_DURATION(value) AddDuration UNIQ_ID(__LINE__){value};
