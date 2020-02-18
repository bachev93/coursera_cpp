#pragma once

#include <algorithm>
#include <cstdint>

using namespace std;


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

    void PushBack(T value) {
        ExpandIfNeeded();
        data_[size_++] = move(value);
    }

private:
    T* data_;
    size_t size_;
    size_t capacity_;

    void ExpandIfNeeded() {
        if(size_ >= capacity_) {
            auto newCap = capacity_ == 0 ? 1 : 2 * capacity_;
            auto newData = new T[newCap];
            //используем перемещение
            move(begin(), end(), newData);
            delete[] data_;
            data_ = newData;
            capacity_ = newCap;
        }
    }
};
