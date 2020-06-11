#include "test_runner.h"
#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;


struct Email {
//    Email(const string& from_, const string& to_, const string& body_)
//        : from(from_), to(to_), body(body_) {}
//    Email() = default;

    string from;
    string to;
    string body;

    friend istream& operator>>(istream& in, Email& email) {
        string from, to, body;

        getline(in, from);
        getline(in, to);
        getline(in, body);

        email = Email{move(from), move(to), move(body)};

        return in;
    }

    friend ostream& operator<<(ostream& out, const Email& email) {
        out << email.from << '\n' <<
               email.to << '\n' <<
               email.body;
        return out;
    }
};


class Worker {
public:
    virtual ~Worker() = default;
    virtual void Process(unique_ptr<Email> email) = 0;
    virtual void Run() {
        // только первому worker-у в пайплайне нужно это имплементировать
        throw logic_error("Unimplemented");
    }

protected:
    // реализации должны вызывать PassOn, чтобы передать объект дальше
    // по цепочке обработчиков
    void PassOn(unique_ptr<Email> email) const {
        if(next_) {
            next_->Process(move(email));
        }
    }

public:
    void SetNext(unique_ptr<Worker> next) {
        next_ = move(next);
    }

private:
    unique_ptr<Worker> next_;
};


class Reader : public Worker {
public:
    explicit Reader(istream& in)
        : input_(in) {}

    void Process(unique_ptr<Email>) override {
        // не делаем ничего
    }

    void Run() override {
        Email email = {};
        while(input_ >> email) {
            PassOn(make_unique<Email>(move(email)));
        }
    }

private:
    istream& input_;
};


class Filter : public Worker {
public:
    using Function = function<bool(const Email&)>;

    explicit Filter(Function predicate)
        : predicate_(move(predicate)) {}

    void Process(unique_ptr<Email> email) override {
        if(predicate_(*email)) {
            PassOn(move(email));
        }
    }


private:
    // реализуйте класс
    Function predicate_;
};


class Copier : public Worker {
public:
    // реализуйте класс
    Copier(string addr)
        : address_(move(addr)) {}

    void Process(unique_ptr<Email> email) override {
        if(address_ != email->to) {
            auto copy = make_unique<Email>(*email);
            copy->to = address_;
            PassOn(move(email));
            PassOn(move(copy));
        } else {
            PassOn(move(email));
        }
    }

private:
    string address_;
};


class Sender : public Worker {
public:
    // реализуйте класс
    Sender(ostream& os)
        : out_(os) {}

    void Process(unique_ptr<Email> email) override {
        out_ << *email << '\n';
        PassOn(move(email));
    }

private:
    ostream& out_;
};


// реализуйте класс
class PipelineBuilder {
public:
    // добавляет в качестве первого обработчика Reader
    explicit PipelineBuilder(istream& in)
        : workers_() {
        workers_.push_back(make_unique<Reader>(in));
    }

    // добавляет новый обработчик Filter
    PipelineBuilder& FilterBy(Filter::Function filter) {
        workers_.push_back(make_unique<Filter>(move(filter)));
        return *this;
    }

    // добавляет новый обработчик Copier
    PipelineBuilder& CopyTo(string recipient) {
        workers_.push_back(make_unique<Copier>(recipient));
        return *this;
    }

    // добавляет новый обработчик Sender
    PipelineBuilder& Send(ostream& out) {
        workers_.push_back(make_unique<Sender>(out));
        return *this;
    }

    // возвращает готовую цепочку обработчиков
    unique_ptr<Worker> Build(){
        for(size_t i = workers_.size() - 1; i > 0; --i) {
            workers_[i -1]->SetNext(move(workers_[i]));
        }

        return move(workers_[0]);
    }

private:
    vector<unique_ptr<Worker>> workers_;
};


void TestSanity() {
    string input = (
                "erich@example.com\n"
                "richard@example.com\n"
                "Hello there\n"

                "erich@example.com\n"
                "ralph@example.com\n"
                "Are you sure you pressed the right button?\n"

                "ralph@example.com\n"
                "erich@example.com\n"
                "I do not make mistakes of that kind\n"
                );
    istringstream inStream(input);
    ostringstream outStream;

    PipelineBuilder builder(inStream);
    builder.FilterBy([](const Email& email) {
        return email.from == "erich@example.com";
    });
    builder.CopyTo("richard@example.com");
    builder.Send(outStream);
    auto pipeline = builder.Build();

    pipeline->Run();

    string expectedOutput = (
                "erich@example.com\n"
                "richard@example.com\n"
                "Hello there\n"

                "erich@example.com\n"
                "ralph@example.com\n"
                "Are you sure you pressed the right button?\n"

                "erich@example.com\n"
                "richard@example.com\n"
                "Are you sure you pressed the right button?\n"
                );

    ASSERT_EQUAL(expectedOutput, outStream.str());
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestSanity);
    return 0;
}
