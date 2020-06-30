#pragma once

#include <memory>

namespace RAII {
template <typename Provider>
class Booking {
    using BookingId = typename Provider::BookingId;

public:
    Booking(Booking&& other)
        : providerPtr_(other.providerPtr_),
          id_(other.id_) {
        other.providerPtr_ = nullptr;
    }

    Booking(Provider* provider, const BookingId& id)
        : providerPtr_(provider),
          id_(id) {}

    Booking& operator =(Booking&& other) {
        std::swap(providerPtr_, other.providerPtr_);
        std::swap(id_, other.id_);
        return *this;
    }

    ~Booking() {
        if(providerPtr_) {
            providerPtr_->CancelOrComplete(*this);
        }
    }
private:
    Provider* providerPtr_;
    BookingId id_;

    Booking() = delete;
    Booking(const Booking& other) = delete;
    Booking& operator =(const Booking& other) = delete;
};
}
