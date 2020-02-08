#include <iostream>
#include <string>
#include <cstdint>
#include <map>
#include <queue>
#include <algorithm>

#include "test_runner.h"


using namespace std;


struct Reservation {
    int64_t time_;      //время
    string hotelName_;  //отель
    int clientID_;      //id клиента
    int roomCount_;     //количество забронированных номеров
};


class Booker {
public:
    explicit Booker() : reservations_(), reservedRooms_(), uniqueHotelClients_() {}

    void book(const Reservation& res) {
        reservations_.push(res);
        ++uniqueHotelClients_[res.hotelName_][res.clientID_];
        reservedRooms_[res.hotelName_] += res.roomCount_;

        auto timeDiff = abs(reservations_.back().time_ - reservations_.front().time_);
        if(timeDiff >= SECS_IN_DAY){
            while(!reservations_.empty() && timeDiff >= SECS_IN_DAY) {
                const auto& oldRes = reservations_.front();
                adjust(oldRes);
                reservations_.pop();
                timeDiff = abs(reservations_.back().time_ - reservations_.front().time_);
            }
        }
    }

    int roomsInHotel(const string& hotelName) const {
        if(!reservedRooms_.count(hotelName)) {
            return 0;
        }

        return reservedRooms_.at(hotelName);
    }

    int clientsInHotel(const string& hotelName) const {
        if(!uniqueHotelClients_.count(hotelName)) {
            return 0;
        }

        return uniqueHotelClients_.at(hotelName).size();
    }

private:
    static const int SECS_IN_DAY = 86400;

    queue<Reservation> reservations_;
    map<string, int> reservedRooms_;
    map<string, map<int, int>> uniqueHotelClients_;

    void adjust(const Reservation& oldRes) {
        reservedRooms_[oldRes.hotelName_] -= oldRes.roomCount_;

        auto& clientReservations = uniqueHotelClients_[oldRes.hotelName_][oldRes.clientID_];
        --clientReservations;
        if(clientReservations < 1) {
            uniqueHotelClients_[oldRes.hotelName_].erase(oldRes.clientID_);
        }
    }
};

void test() {
    Booker booker;

    auto clients = booker.clientsInHotel("Mariott");
    ASSERT_EQUAL(clients, 0);

    auto rooms = booker.roomsInHotel("Mariott");
    ASSERT_EQUAL(rooms, 0);

    booker.book({10, "FourSeasons", 1, 2});
    booker.book({10, "Mariott", 1, 1});
    booker.book({86409, "FourSeasons", 2, 1});

    clients = booker.clientsInHotel("FourSeasons");
    ASSERT_EQUAL(clients, 2);

    rooms = booker.roomsInHotel("FourSeasons");
    ASSERT_EQUAL(rooms, 3);

    clients = booker.clientsInHotel("Mariott");
    ASSERT_EQUAL(clients, 1);

    booker.book({86410, "Mariott", 2, 10});
    rooms = booker.roomsInHotel("FourSeasons");
    ASSERT_EQUAL(rooms, 1);

    rooms = booker.roomsInHotel("Mariott");
    ASSERT_EQUAL(rooms, 10);
}

int main() {
//    TestRunner tr;
//    RUN_TEST(tr, test);

    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Booker booker;

    int queryCount;
    cin >> queryCount;
    for(int i = 0; i < queryCount; ++i) {
        string queryType;
        cin >> queryType;
        if(queryType == "BOOK") {
            Reservation res;
            cin >> res.time_ >> res.hotelName_ >> res.clientID_ >> res.roomCount_;
            booker.book(res);
        }
        else if(queryType == "CLIENTS") {
            string hotelName;
            cin >> hotelName;
            cout << booker.clientsInHotel(hotelName) << "\n";
        }
        else if(queryType == "ROOMS") {
            string hotelName;
            cin >> hotelName;
            cout << booker.roomsInHotel(hotelName) << "\n";
        }
    }

    return 0;
}
