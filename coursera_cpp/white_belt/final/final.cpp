#include <iostream>
#include <string>
#include <map>
#include <set>
#include <exception>
#include <sstream>
#include <iomanip>

using namespace std;


class Date {
public:
    Date(int year = 0, int month = 0, int day = 0) :
        year_(year), month_(month), day_(day) {}

    int day() const {return day_;}
    int month() const {return month_;}
    int year() const {return year_;}

    void setDay(int newDay) {
        day_ = newDay;

        if(day_ < 1 || day_ > 31) {
            throw runtime_error("Day value is invalid: " + to_string(day_));
        }
    }

    void setMonth(int newMonth) {
        month_ = newMonth;

        if(month_ < 1 || month_ > 12) {
            throw runtime_error("Month value is invalid: " + to_string(month_));
        }
    }

    void setYear(int newYear) {year_ = newYear;}


private:
    int year_;
    int month_;
    int day_;
};

bool operator <(const Date& lhs, const Date& rhs) {
    if(lhs.year() == rhs.year()) {
        if(lhs.month() == rhs.month()) {
            return lhs.day() < rhs.day();
        }
        else {
            return lhs.month() < rhs.month();
        }
    }
    else {
        return lhs.year() < rhs.year();
    }
}

ostream& operator <<(ostream& out, const Date& date) {
    out << setfill('0') << setw(4) << date.year() << "-" << setw(2) << date.month() << "-" <<
           setw(2) << date.day();

    return out;
}

istream& operator >>(istream& in, Date& date) {
    string tmp;
    in >> tmp;

    stringstream stream(tmp);
    int year = 0, month = 0, day = 0;
    if(!(stream >> year)) {
        throw runtime_error("Wrong date format: " + tmp);
    }

    if(stream.peek() != '-'){
        throw runtime_error("Wrong date format: " + tmp);
    }
    stream.ignore(1);

    if(!(stream >> month)) {
        throw runtime_error("Wrong date format: " + tmp);
    }

    if(stream.peek() != '-'){
        throw runtime_error("Wrong date format: " + tmp);
    }
    stream.ignore(1);

    if(!(stream >> day)) {
        throw runtime_error("Wrong date format: " + tmp);
    }

    if(stream.peek() != EOF) {
        throw runtime_error("Wrong date format: " + tmp);
    }

    date.setYear(year);
    date.setMonth(month);
    date.setDay(day);

    return in;
}

class DataBase {
public:
    void addEvent(const Date& date, const string& event) {
        events_[date].insert(event);
    }

    void deleteEvent(const Date& date, const string& event) {
        if(events_.count(date) && events_[date].count(event)) {
            events_[date].erase(event);
            cout << "Deleted successfully" << endl;
        }
        else {
            cout << "Event not found" << endl;
        }
    }

    void deleteDate(const Date& date) {
        if(events_.count(date)) {
            int n = events_[date].size();
            events_.erase(date);
            cout << "Deleted " << n << " events" << endl;
        }
        else {
            cout << "Deleted " << 0 << " events" << endl;
        }
    }

    void printEvents(const Date& date) const {
        if(events_.count(date)) {
            for(const auto& event : events_.at(date)) {
                cout << event << endl;
            }
        }
        //TODO: а если нету ничего?
    }

    void printAll() const {
        for(const auto& item : events_) {
            for(const auto& event : item.second) {
                cout << item.first << " " << event << endl;
            }
        }
    }

private:
    map<const Date, set<string>> events_;
};


int main() {
    DataBase db;
    string line;
    try{
        while(getline(cin, line)) {
            stringstream stream(line);
            string command;
            stream >> command;

            if(command.empty()) {
                continue;
            }
            else if(command == "Add") {
                Date date;
                string event;
                stream >> date >> event;
                db.addEvent(date, event);
            }
            else if(command == "Del") {
                Date date;
                stream >> date;

                if(stream.peek() != EOF) {
                    string event;
                    stream >> event;
                    db.deleteEvent(date, event);
                }
                else {
                    db.deleteDate(date);
                }
            }
            else if(command == "Find") {
                Date date;
                stream >> date;
                db.printEvents(date);
            }
            else if(command == "Print") {
                db.printAll();
            }
            else {
                cout << "Unknown command: " << command << endl;
            }
        }
    }
    catch (exception& ex) {
        cout << ex.what() << endl;
    }

    return 0;
}
