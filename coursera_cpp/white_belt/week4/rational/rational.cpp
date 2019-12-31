#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <set>

using namespace std;


class Rational {
public:
    Rational() : numerator_(0), denominator_(1) {}

    Rational(int numerator, int denominator) :
        numerator_(numerator), denominator_(denominator) {
        if(denominator_ < 0) {
            numerator_ = -numerator_;
            denominator_ = -denominator_;
        }

        //наибольший общий делитель для сокращения дроби
        int a = abs(numerator_), b = abs(denominator_);
        while (a > 0 && b > 0) {
            if(a > b) {
                a %= b;
            }
            else {
                b %= a;
            }
        }
        int divisor = a +b;

        numerator_ /= divisor;
        denominator_ /= divisor;

        if (numerator_ == 0) {
            denominator_ = 1;
        }
    }

    int Numerator() const {return numerator_;}

    int Denominator() const {return denominator_;}

    void setNumerator(int numerator) {numerator_ = numerator;}

    void setDenominator(int denominator) {denominator_ = denominator;}

private:
    int numerator_;
    int denominator_;
};


bool operator ==(const Rational& lhs, const Rational& rhs) {
    return lhs.Numerator() == rhs.Numerator() &&
            lhs.Denominator() == rhs.Denominator();
}

Rational operator +(const Rational& lhs, const Rational& rhs) {
    int denominator = lhs.Denominator() * rhs.Denominator();
    int numerator = lhs.Numerator() * rhs.Denominator() +
            rhs.Numerator() * lhs.Denominator();

    return Rational(numerator, denominator);
}

Rational operator -(const Rational& lhs, const Rational& rhs) {
    int denominator = lhs.Denominator() * rhs.Denominator();
    int numerator = lhs.Numerator() * rhs.Denominator() -
            rhs.Numerator() * lhs.Denominator();

    return Rational(numerator, denominator);
}

Rational operator *(const Rational& lhs, const Rational& rhs) {
    return Rational(lhs.Numerator() * rhs.Numerator(),
                    lhs.Denominator() * rhs.Denominator());
}

Rational operator /(const Rational& lhs, const Rational& rhs) {
    return Rational(lhs.Numerator() * rhs.Denominator(),
                    lhs.Denominator() * rhs.Numerator());
}

ostream& operator <<(ostream& stream, const Rational& rat) {
    stream << rat.Numerator() << "/" << rat.Denominator();
    return stream;
}

bool operator <(const Rational& lhs, const Rational& rhs) {
    int lNum = lhs.Numerator() * rhs.Denominator();
    int rNum = rhs.Numerator() * lhs.Denominator();

    return lNum < rNum;
}

istream& operator >>(istream& stream, Rational& rat) {
    if(stream) {
        int numerator = rat.Numerator(), denominator = rat.Denominator();

        stream >> numerator;
        stream.ignore(1);
        stream >> denominator;

        Rational tmpRat(numerator, denominator);
        rat.setNumerator(tmpRat.Numerator());
        rat.setDenominator(tmpRat.Denominator());
    }

    return stream;
}


int main() {
    //5th case:
    {
        const set<Rational> rs = {{1, 2}, {1, 25}, {3, 4}, {3, 4}, {1, 2}};
        if (rs.size() != 3) {
            cout << "Wrong amount of items in the set" << endl;
            return 1;
        }

        vector<Rational> v;
        for (auto x : rs) {
            v.push_back(x);
        }
        if (v != vector<Rational>{{1, 25}, {1, 2}, {3, 4}}) {
            cout << "Rationals comparison works incorrectly" << endl;
            return 2;
        }
    }

    {
        map<Rational, int> count;
        ++count[{1, 2}];
        ++count[{1, 2}];

        ++count[{2, 3}];

        if (count.size() != 2) {
            cout << "Wrong amount of items in the map" << endl;
            return 3;
        }
    }

    //4th case:
    //    {
    //        ostringstream output;
    //        output << Rational(-6, 8);
    //        if (output.str() != "-3/4") {
    //            cout << "Rational(-6, 8) should be written as \"-3/4\"" << endl;
    //            return 1;
    //        }
    //    }

    //    {
    //        istringstream input("5/7");
    //        Rational r;
    //        input >> r;
    //        bool equal = r == Rational(5, 7);
    //        if (!equal) {
    //            cout << "5/7 is incorrectly read as " << r << endl;
    //            return 2;
    //        }
    //    }

    //    {
    //        istringstream input("5/7 10/8");
    //        Rational r1, r2;
    //        input >> r1 >> r2;
    //        bool correct = r1 == Rational(5, 7) && r2 == Rational(5, 4);
    //        if (!correct) {
    //            cout << "Multiple values are read incorrectly: " << r1 << " " << r2 << endl;
    //            return 3;
    //        }

    //        input >> r1;
    //        input >> r2;
    //        correct = r1 == Rational(5, 7) && r2 == Rational(5, 4);
    //        if (!correct) {
    //            cout << "Read from empty stream shouldn't change arguments: " << r1 << " " << r2 << endl;
    //            return 4;
    //        }
    //    }


    //3rd case:
    //    {
    //        Rational a(2, 3);
    //        Rational b(4, 3);
    //        Rational c = a * b;
    //        bool equal = c == Rational(8, 9);
    //        if (!equal) {
    //            cout << "2/3 * 4/3 != 8/9" << endl;
    //            return 1;
    //        }
    //    }

    //    {
    //        Rational a(5, 4);
    //        Rational b(15, 8);
    //        Rational c = a / b;
    //        bool equal = c == Rational(2, 3);
    //        if (!equal) {
    //            cout << "5/4 / 15/8 != 2/3" << endl;
    //            return 2;
    //        }
    //    }


    //2nd case:
    //    {
    //        Rational r1(4, 6);
    //        Rational r2(2, 3);
    //        bool equal = r1 == r2;
    //        if (!equal) {
    //            cout << "4/6 != 2/3" << endl;
    //            return 1;
    //        }
    //    }

    //    {
    //        Rational a(2, 3);
    //        Rational b(4, 3);
    //        Rational c = a + b;
    //        bool equal = c == Rational(2, 1);
    //        if (!equal) {
    //            cout << "2/3 + 4/3 != 2" << endl;
    //            return 2;
    //        }
    //    }

    //    {
    //        Rational a(5, 7);
    //        Rational b(2, 9);
    //        Rational c = a - b;
    //        bool equal = c == Rational(31, 63);
    //        if (!equal) {
    //            cout << "5/7 - 2/9 != 31/63" << endl;
    //            return 3;
    //        }
    //    }


    //1st case:
    //    {
    //        const Rational r(3, 10);
    //        if (r.Numerator() != 3 || r.Denominator() != 10) {
    //            cout << "Rational(3, 10) != 3/10" << endl;
    //            return 1;
    //        }
    //    }

    //    {
    //        const Rational r(8, 12);
    //        if (r.Numerator() != 2 || r.Denominator() != 3) {
    //            cout << "Rational(8, 12) != 2/3" << endl;
    //            return 2;
    //        }
    //    }

    //    {
    //        const Rational r(-4, 6);
    //        if (r.Numerator() != -2 || r.Denominator() != 3) {
    //            cout << "Rational(-4, 6) != -2/3" << endl;
    //            return 3;
    //        }
    //    }

    //    {
    //        const Rational r(4, -6);
    //        if (r.Numerator() != -2 || r.Denominator() != 3) {
    //            cout << "Rational(4, -6) != -2/3" << endl;
    //            return 3;
    //        }
    //    }

    //    {
    //        const Rational r(0, 15);
    //        if (r.Numerator() != 0 || r.Denominator() != 1) {
    //            cout << "Rational(0, 15) != 0/1" << endl;
    //            return 4;
    //        }
    //    }

    //    {
    //        const Rational defaultConstructed;
    //        if (defaultConstructed.Numerator() != 0 || defaultConstructed.Denominator() != 1) {
    //            cout << "Rational() != 0/1" << endl;
    //            return 5;
    //        }
    //    }

    cout << "OK" << endl;
    return 0;
}
