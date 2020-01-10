#pragma once

#include <cstdlib>
#include <algorithm>

using namespace std;


// Реализуйте шаблон SimpleVector
template <typename T>
class SimpleVector {
public:
    SimpleVector()
        : data_(nullptr),
          size_(0),
          capacity_(0) {}

    explicit SimpleVector(size_t size)
        : data_(new T[size]),
          size_(size),
          capacity_(size) {
        for(auto it = begin(); it != end(); ++it) {
            *it = T();
        }
    }

    ~SimpleVector() {delete[] data_;}

    T& operator[](size_t index) {return data_[index];}

    T* begin() {return data_;}

    T* end() {return data_ + size_;}

    const T* begin() const {return data_;}

    const T* end() const {return data_ + size_;}

    size_t Size() const {return size_;}

    size_t Capacity() const {return capacity_;}

    void PushBack(const T& value) {
        if(data_ == nullptr) {
            capacity_ = 1;
            data_ = new T[capacity_];
        }
        else if(size_ == capacity_) {
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
