#pragma once

#include <array>
#include <stdexcept>

using namespace std;

template <typename T, size_t N>
class StackVector {
 public:
  explicit StackVector(size_t a_size = 0);

  T& operator[](size_t index);
  const T& operator[](size_t index) const;

  auto begin();
  auto end();
  auto begin() const;
  auto end() const;

  size_t Size() const;
  size_t Capacity() const;

  void PushBack(const T& value);
  T PopBack();

 private:
  size_t capacity_;
  size_t size_;
  array<T, N> data_;
};

template <typename T, size_t N>
StackVector<T, N>::StackVector(size_t a_size)
    : capacity_(N), size_(a_size), data_() {
  if (a_size > capacity_) {
    throw invalid_argument(
        "error in construtor: size can't"
        " be more than capacity");
  }
}

template <typename T, size_t N>
T& StackVector<T, N>::operator[](size_t index) {
  return data_[index];
}

template <typename T, size_t N>
const T& StackVector<T, N>::operator[](size_t index) const {
  return data_[index];
}

template <typename T, size_t N>
auto StackVector<T, N>::begin() {
  return data_.begin();
}

template <typename T, size_t N>
auto StackVector<T, N>::end() {
  return data_.begin() + size_;
}

template <typename T, size_t N>
auto StackVector<T, N>::begin() const {
  return data_.begin();
}

template <typename T, size_t N>
auto StackVector<T, N>::end() const {
  return data_.begin() + size_;
}

template <typename T, size_t N>
size_t StackVector<T, N>::Size() const {
  return size_;
}

template <typename T, size_t N>
size_t StackVector<T, N>::Capacity() const {
  return capacity_;
}

template <typename T, size_t N>
void StackVector<T, N>::PushBack(const T& value) {
  if (size_ >= capacity_) {
    throw overflow_error("index out of range");
  } else {
    data_[size_++] = value;
  }
}

template <typename T, size_t N>
T StackVector<T, N>::PopBack() {
  if (size_ == 0) {
    throw underflow_error("can't PopBack, StackVector is empty");
  } else {
    auto val = data_[--size_];
    return val;
  }
}
