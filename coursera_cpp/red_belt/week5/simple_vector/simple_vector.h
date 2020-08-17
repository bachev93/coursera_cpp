#pragma once

#include <algorithm>

using namespace std;

template <typename T>
class SimpleVector {
 public:
  SimpleVector() : data_(nullptr), size_(0), capacity_(0) {}

  explicit SimpleVector(size_t size)
      : data_(new T[size]), size_(size), capacity_(size) {
    for (auto it = begin(); it != end(); ++it) {
      *it = T();
    }
  }

  SimpleVector(const SimpleVector& rhs)
      : data_(new T[rhs.capacity_]),
        size_(rhs.size_),
        capacity_(rhs.capacity_) {
    copy(rhs.begin(), rhs.end(), begin());
  }

  ~SimpleVector() { delete[] data_; }

  void operator=(const SimpleVector& rhs) {
    if (rhs.size_ <= capacity_) {
      // У нас достаточно памяти - просто копируем элементы
      copy(rhs.begin(), rhs.end(), begin());
      size_ = rhs.size_;
    } else {
      // Это так называемая идиома copy-and-swap.
      // Мы создаём временный вектор с помощью
      // конструктора копирования, а затем обмениваем его поля со своими.
      // Так мы достигаем двух целей:
      //  - избегаем дублирования кода в конструкторе копирования
      //    и операторе присваивания
      //  - обеспечиваем согласованное поведение конструктора копирования
      //    и оператора присваивания
      SimpleVector<T> tmp(rhs);
      swap(tmp.data_, data_);
      swap(tmp.size_, size_);
      swap(tmp.capacity_, capacity_);
    }
  }

  T& operator[](size_t index) { return data_[index]; }

  T* begin() { return data_; }

  T* end() { return data_ + size_; }

  const T* begin() const { return data_; }

  const T* end() const { return data_ + size_; }

  size_t Size() const { return size_; }

  size_t Capacity() const { return capacity_; }

  void PushBack(const T& value) {
    if (data_ == nullptr) {
      capacity_ = 1;
      data_ = new T[capacity_];
    } else if (size_ >= capacity_) {
      T* newData = new T[capacity_ * 2];
      capacity_ *= 2;

      copy(begin(), end(), newData);
      delete[] data_;
      data_ = newData;
    }

    data_[size_++] = value;
  }

 private:
  // Добавьте поля для хранения данных вектора
  T* data_;
  size_t size_;
  size_t capacity_;
};
