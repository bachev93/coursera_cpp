#pragma once

#include "http_request.h"

#include <map>
#include <string>
#include <string_view>
#include <vector>

using namespace std;

class Stats {
 public:
  Stats();
  void AddMethod(string_view method);
  void AddUri(string_view uri);
  const map<string_view, int>& GetMethodStats() const;
  const map<string_view, int>& GetUriStats() const;

 private:
  // Ключевое слово inline позволяет определить статические члены
  // methods_, uris_ здесь, в .h-файле. Без
  // него нам бы пришлось объявить их здесь, а определеление вынести
  // в stats.cpp
  inline static const vector<string> methods_{"GET", "POST", "PUT", "DELETE",
                                              "UNKNOWN"};
  inline static const vector<string> uris_ = {"/",       "/order", "/product",
                                              "/basket", "/help",  "unknown"};

  map<string_view, int> methodStats_, uriStats_;
};

HttpRequest ParseRequest(string_view line);
