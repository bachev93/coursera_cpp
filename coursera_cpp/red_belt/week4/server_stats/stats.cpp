#include <iostream>
#include <vector>

#include "stats.h"

Stats::Stats() : methodStats_(), uriStats_() {
  for (const auto& key : methods_) {
    methodStats_[key] = 0;
  }

  for (const auto& key : uris_) {
    uriStats_[key] = 0;
  }
}

void Stats::AddMethod(string_view method) {
  if (!methodStats_.count(method)) {
    ++methodStats_[methods_.back()];
    return;
  }
  ++methodStats_[method];
}

void Stats::AddUri(string_view uri) {
  if (!uriStats_.count(uri)) {
    ++uriStats_[uris_.back()];
    return;
  }
  ++uriStats_[uri];
}

const map<string_view, int>& Stats::GetMethodStats() const {
  return methodStats_;
}

const map<string_view, int>& Stats::GetUriStats() const {
  return uriStats_;
}

HttpRequest ParseRequest(string_view line) {
  vector<string_view> vec;
  vec.reserve(3);

  size_t pos = line.find_first_not_of(' ');
  auto endPos = string_view::npos;
  while (pos != endPos) {
    auto space = line.find(' ', pos);

    vec.push_back(line.substr(pos, space - pos));

    if (space == endPos) {
      pos = endPos;
    } else {
      pos = space + 1;
    }
  }

  return {vec[0], vec[1], vec[2]};
}
