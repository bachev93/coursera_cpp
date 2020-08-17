#include <list>
#include <string>

#include "test_runner.h"

using namespace std;

class Editor {
 public:
  // Реализуйте конструктор по умолчанию и объявленные методы
  Editor() : data_(), exchangeBuff_(), currentPos_(data_.begin()) {}

  void Left() {
    if (data_.empty()) {
      return;
    }

    if (currentPos_ != data_.begin()) {
      --currentPos_;
    } else {
      currentPos_ = data_.begin();
    }
  }

  void Right() {
    if (data_.empty()) {
      return;
    }

    if (currentPos_ != data_.end()) {
      ++currentPos_;
    }
  }

  void Insert(char token) {
    currentPos_ = data_.insert(currentPos_, token);
    ++currentPos_;
  }

  void Cut(size_t tokens = 1) {
    exchangeBuff_.clear();

    size_t distanceToEnd = distance(currentPos_, data_.end());

    if (distanceToEnd < tokens) {
      copy(currentPos_, data_.end(), back_inserter(exchangeBuff_));
      currentPos_ = data_.erase(currentPos_, data_.end());
    } else {
      copy(currentPos_, next(currentPos_, tokens),
           back_inserter(exchangeBuff_));
      currentPos_ = data_.erase(currentPos_, next(currentPos_, tokens));
    }
  }

  void Copy(size_t tokens = 1) {
    exchangeBuff_.clear();

    size_t distanceToEnd = distance(currentPos_, data_.end());

    if (distanceToEnd < tokens) {
      copy(currentPos_, data_.end(), back_inserter(exchangeBuff_));
    } else {
      copy(currentPos_, next(currentPos_, tokens),
           back_inserter(exchangeBuff_));
    }
  }

  void Paste() {
    auto buffSize = exchangeBuff_.size();
    auto it =
        data_.insert(currentPos_, exchangeBuff_.begin(), exchangeBuff_.end());
    currentPos_ = next(it, buffSize);
  }

  string GetText() const {
    if (data_.empty()) {
      return "";
    }

    return string(data_.begin(), data_.end());
  }

 private:
  list<char> data_;
  list<char> exchangeBuff_;
  using Iterator = list<char>::iterator;
  Iterator currentPos_;
};

void TypeText(Editor& editor, const string& text) {
  for (char c : text) {
    editor.Insert(c);
  }
}

void TestEditing() {
  {
    Editor editor;

    const size_t text_len = 12;
    const size_t first_part_len = 7;
    TypeText(editor, "hello, world");
    for (size_t i = 0; i < text_len; ++i) {
      editor.Left();
    }

    editor.Cut(first_part_len);

    for (size_t i = 0; i < text_len - first_part_len; ++i) {
      editor.Right();
    }

    TypeText(editor, ", ");
    editor.Paste();
    editor.Left();
    editor.Left();
    editor.Cut(3);

    ASSERT_EQUAL(editor.GetText(), "world, hello");
  }
  {
    Editor editor;

    TypeText(editor, "misprnit");
    editor.Left();
    editor.Left();
    editor.Left();
    editor.Cut(1);
    editor.Right();
    editor.Paste();

    ASSERT_EQUAL(editor.GetText(), "misprint");
  }
}

void TestReverse() {
  Editor editor;

  const string text = "esreveR";
  for (char c : text) {
    editor.Insert(c);
    editor.Left();
  }

  ASSERT_EQUAL(editor.GetText(), "Reverse");
}

void TestNoText() {
  Editor editor;
  ASSERT_EQUAL(editor.GetText(), "");

  editor.Left();
  editor.Left();
  editor.Right();
  editor.Right();
  editor.Copy(0);
  editor.Cut(0);
  editor.Paste();

  ASSERT_EQUAL(editor.GetText(), "");
}

void TestEmptyBuffer() {
  Editor editor;

  editor.Paste();
  TypeText(editor, "example");
  editor.Left();
  editor.Left();
  editor.Paste();
  editor.Right();
  editor.Paste();
  editor.Copy(0);
  editor.Paste();
  editor.Left();
  editor.Cut(0);
  editor.Paste();

  ASSERT_EQUAL(editor.GetText(), "example");
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestEditing);
  RUN_TEST(tr, TestReverse);
  RUN_TEST(tr, TestNoText);
  RUN_TEST(tr, TestEmptyBuffer);
  return 0;
}
