#include "test_runner.h"

#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace std;

enum class HttpCode {
  Ok = 200,
  NotFound = 404,
  Found = 302,
};

struct HttpHeader {
  string name, value;
};

struct ParsedResponse {
  int code;
  vector<HttpHeader> headers;
  string content;
};

class HttpResponse {
 public:
  explicit HttpResponse(HttpCode code);

  HttpResponse& AddHeader(string name, string value);
  HttpResponse& SetContent(string a_content);
  HttpResponse& SetCode(HttpCode a_code);

  friend ostream& operator<<(ostream& output, const HttpResponse& resp);

 private:
  ParsedResponse response_;
  const string ADDR_HEAD = "HTTP/1.1";
};

struct HttpRequest {
  string method, path, body;
  map<string, string> get_params;
};

pair<string, string> SplitBy(const string& what, const string& by) {
  size_t pos = what.find(by);
  if (by.size() < what.size() && pos < what.size() - by.size()) {
    return {what.substr(0, pos), what.substr(pos + by.size())};
  } else {
    return {what, {}};
  }
}

template <typename T>
T FromString(const string& s) {
  T x;
  istringstream is(s);
  is >> x;
  return x;
}

pair<size_t, string> ParseIdAndContent(const string& body) {
  auto [id_string, content] = SplitBy(body, " ");
  return {FromString<size_t>(id_string), content};
}

struct LastCommentInfo {
  size_t user_id, consecutive_count;
};

class CommentServer {
 public:
  void ServeRequest(const HttpRequest& req, ostream& os) {
    os << ServeRequest(req) << "\n";
  }

  HttpResponse ServeRequest(const HttpRequest& req) {
    HttpResponse response(HttpCode::NotFound);

    if (req.method == "POST") {
      if (req.path == "/add_user") {
        auto userId = users_.size();
        users_[userId];
        response.SetCode(HttpCode::Ok).SetContent(to_string(userId));
      } else if (req.path == "/add_comment") {
        auto [userId, comment] = ParseIdAndContent(req.body);
        if (users_.at(userId).isBanned_) {
          lastComment_ = {userId, 4};
          response.SetCode(HttpCode::Found).AddHeader("Location", "/captcha");
        } else if (lastComment_ && lastComment_->user_id == userId) {
          ++lastComment_->consecutive_count;
          if (lastComment_->consecutive_count > SPAM_LIMIT) {
            // user is spammer
            users_[userId].isBanned_ = true;
            response.SetCode(HttpCode::Found).AddHeader("Location", "/captcha");
          } else {
            users_[userId].comments_.push_back(move(comment));
            response.SetCode(HttpCode::Ok);
          }
        } else {
          lastComment_ = {userId, 1};
          users_[userId].comments_.push_back(move(comment));
          response.SetCode(HttpCode::Ok);
        }
      } else if (req.path == "/checkcaptcha") {
        auto [userId, answer] = ParseIdAndContent(req.body);
        if (answer == "42") {
          users_.at(userId).isBanned_ = false;

          if (lastComment_ && lastComment_->user_id == userId) {
            lastComment_->consecutive_count = 0;
          }

          response.SetCode(HttpCode::Ok);
        } else {
          response.SetCode(HttpCode::Found).AddHeader("Location", "/captcha");
        }
      }
    } else if (req.method == "GET") {
      if (req.path == "/user_comments") {
        auto userId = FromString<size_t>(req.get_params.at("user_id"));

        string content;
        for (const auto& comment : users_.at(userId).comments_) {
          content += comment + '\n';
        }

        response.SetCode(HttpCode::Ok).SetContent(content);
      } else if (req.path == "/captcha") {
        string question(
            "What's the answer for The Ultimate Question of"
            " Life, the Universe, and Everything?");
        response.SetCode(HttpCode::Ok).SetContent(question);
      }
    }

    return response;
  }

 private:
  const size_t SPAM_LIMIT = 3;

  struct UserInfo {
    bool isBanned_ = false;

    vector<string> comments_;
  };

  unordered_map<size_t, UserInfo> users_;
  optional<LastCommentInfo> lastComment_;
};

ostream& operator<<(ostream& output, const HttpHeader& h) {
  return output << h.name << ": " << h.value;
}

bool operator==(const HttpHeader& lhs, const HttpHeader& rhs) {
  return lhs.name == rhs.name && lhs.value == rhs.value;
}

istream& operator>>(istream& input, ParsedResponse& r) {
  string line;
  getline(input, line);

  {
    istringstream code_input(line);
    string dummy;
    code_input >> dummy >> r.code;
  }

  size_t content_length = 0;

  r.headers.clear();
  while (getline(input, line) && !line.empty()) {
    if (auto [name, value] = SplitBy(line, ": "); name == "Content-Length") {
      istringstream length_input(value);
      length_input >> content_length;
    } else {
      r.headers.push_back({std::move(name), std::move(value)});
    }
  }

  r.content.resize(content_length);
  input.read(r.content.data(), r.content.size());
  return input;
}

HttpResponse::HttpResponse(HttpCode code)
    : response_{static_cast<int>(code), {}, string()} {}

HttpResponse& HttpResponse::AddHeader(string name, string value) {
  response_.headers.push_back({move(name), move(value)});
  return *this;
}

HttpResponse& HttpResponse::SetContent(string a_content) {
  response_.content = move(a_content);
  return *this;
}

HttpResponse& HttpResponse::SetCode(HttpCode a_code) {
  response_.code = static_cast<int>(a_code);
  return *this;
}

ostream& operator<<(ostream& output, const HttpResponse& resp) {
  output << resp.ADDR_HEAD << " " << resp.response_.code << " ";

  switch (static_cast<HttpCode>(resp.response_.code)) {
    case HttpCode::Ok:
      output << "OK";
      break;
    case HttpCode::Found:
      output << "Found";
      break;
    case HttpCode::NotFound:
      output << "Not found";
      break;
    default:
      throw invalid_argument("Unknown http code");
      break;
  }
  output << "\n";

  for (const auto& header : resp.response_.headers) {
    output << header << "\n";
  }

  if (!resp.response_.content.empty()) {
    output << "Content-Length: " << resp.response_.content.size() << "\n\n";
    output << resp.response_.content;
  } else {
    output << "\n";
  }

  return output;
}

void Test(CommentServer& srv,
          const HttpRequest& request,
          const ParsedResponse& expected) {
  stringstream ss;
  srv.ServeRequest(request, ss);
  ParsedResponse resp;
  ss >> resp;
  ASSERT_EQUAL(resp.code, expected.code);
  ASSERT_EQUAL(resp.headers, expected.headers);
  ASSERT_EQUAL(resp.content, expected.content);
}

template <typename CommentServer>
void TestServer() {
  CommentServer cs;

  const ParsedResponse ok{200};
  const ParsedResponse redirect_to_captcha{302, {{"Location", "/captcha"}}, {}};
  const ParsedResponse not_found{404};

  Test(cs, {"POST", "/add_user"}, {200, {}, "0"});
  Test(cs, {"POST", "/add_user"}, {200, {}, "1"});
  Test(cs, {"POST", "/add_comment", "0 Hello"}, ok);
  Test(cs, {"POST", "/add_comment", "1 Hi"}, ok);
  Test(cs, {"POST", "/add_comment", "1 Buy my goods"}, ok);
  Test(cs, {"POST", "/add_comment", "1 Enlarge"}, ok);
  Test(cs, {"POST", "/add_comment", "1 Buy my goods"}, redirect_to_captcha);
  Test(cs, {"POST", "/add_comment", "0 What are you selling?"}, ok);
  Test(cs, {"POST", "/add_comment", "1 Buy my goods"}, redirect_to_captcha);

  Test(cs, {"GET", "/user_comments", "", {{"user_id", "0"}}},
       {200, {}, "Hello\nWhat are you selling?\n"});

  Test(cs, {"GET", "/user_comments", "", {{"user_id", "1"}}},
       {200, {}, "Hi\nBuy my goods\nEnlarge\n"});

  Test(cs, {"GET", "/captcha"},
       {200,
        {},
        {"What's the answer for The Ultimate Question of Life, the "
         "Universe, and Everything?"}});

  Test(cs, {"POST", "/checkcaptcha", "1 24"}, redirect_to_captcha);
  Test(cs, {"POST", "/checkcaptcha", "1 42"}, ok);
  Test(cs, {"POST", "/add_comment", "1 Sorry! No spam any more"}, ok);

  Test(cs, {"GET", "/user_comments", "", {{"user_id", "1"}}},
       {200, {}, "Hi\nBuy my goods\nEnlarge\nSorry! No spam any more\n"});

  Test(cs, {"GET", "/user_commntes"}, not_found);
  Test(cs, {"POST", "/add_uesr"}, not_found);
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestServer<CommentServer>);
}
