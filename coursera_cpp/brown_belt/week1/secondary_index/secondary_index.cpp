#include "test_runner.h"

#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <list>
#include <memory>

using namespace std;

struct Record {
    string id;
    string title;
    string user;
    int timestamp;
    int karma;
};


// Реализуйте этот класс
class Database {
public:
    bool Put(const Record& record) {
        if(indexID_.count(record.id)) {
            return false;
        }

        storage_.push_front(make_shared<Record>(record));
        auto storageIt = storage_.begin();
        auto indexUserIt = indexUser_.emplace(record.user, storageIt);
        auto indexTimeStampIt = indexTimeStamp_.emplace(record.timestamp, storageIt);
        auto indexKarmaIt = indexKarma_.emplace(record.karma, storageIt);
        indexID_.emplace(record.id, DataIterators{storageIt, indexUserIt, indexTimeStampIt, indexKarmaIt});
        return true;
    }

    const Record* GetById(const string& id) const {
        if(indexID_.count(id)) {
            auto indexIdElem = indexID_.at(id);
            return indexIdElem.storageIt_->get();
        }

        return nullptr;
    }

    bool Erase(const string& id) {
        if(indexID_.count(id)) {
            auto indexIdElem = indexID_.at(id);

            indexUser_.erase(indexIdElem.userIt_);
            indexTimeStamp_.erase(indexIdElem.timestampIt_);
            indexKarma_.erase(indexIdElem.karmaIt_);
            storage_.erase(indexIdElem.storageIt_);

            indexID_.erase(id);
            return true;
        }
        return false;
    }

    template <typename Callback>
    void RangeByTimestamp(int low, int high, Callback callback) const {
        auto beginIt = indexTimeStamp_.lower_bound(low);
        auto endIt = indexTimeStamp_.upper_bound(high);

        for(auto& it = beginIt; it != endIt; ++it) {
            if(!callback(*(*(it->second)))) {
                break;
            }
        }
    }

    template <typename Callback>
    void RangeByKarma(int low, int high, Callback callback) const {
        auto beginIt = indexKarma_.lower_bound(low);
        auto endIt = indexKarma_.upper_bound(high);

        for(auto& it = beginIt; it != endIt; ++it) {
            if(!callback(*(*(it->second)))) {
                break;
            }
        }
    }

    template <typename Callback>
    void AllByUser(const string& user, Callback callback) const {
        auto [beginIt, endIt] = indexUser_.equal_range(user);

        for(auto& it = beginIt; it != endIt; ++ it) {
            if(!callback(*(*(it->second)))) {
                break;
            }
        }
    }

private:
    list<shared_ptr<Record>> storage_;
    using StorageIt = list<shared_ptr<Record>>::iterator;

    template<typename Type>
    using IndexMultimap = multimap<Type, StorageIt>;

    struct DataIterators {
        StorageIt storageIt_;
        IndexMultimap<string>::iterator userIt_;
        IndexMultimap<int>::iterator timestampIt_;
        IndexMultimap<int>::iterator karmaIt_;
    };

    unordered_map<string, DataIterators> indexID_;
    IndexMultimap<string> indexUser_;
    IndexMultimap<int> indexTimeStamp_;
    IndexMultimap<int> indexKarma_;
};

void TestRangeBoundaries() {
    const int good_karma = 1000;
    const int bad_karma = -10;

    Database db;
    db.Put({"id1", "Hello there", "master", 1536107260, good_karma});
    db.Put({"id2", "O>>-<", "general2", 1536107260, bad_karma});

    int count = 0;
    db.RangeByKarma(bad_karma, good_karma, [&count](const Record&) {
        ++count;
        return true;
    });

    ASSERT_EQUAL(2, count);
}

void TestSameUser() {
    Database db;
    db.Put({"id1", "Don't sell", "master", 1536107260, 1000});
    db.Put({"id2", "Rethink life", "master", 1536107260, 2000});

    int count = 0;
    db.AllByUser("master", [&count](const Record&) {
        ++count;
        return true;
    });

    ASSERT_EQUAL(2, count);
}

void TestReplacement() {
    const string final_body = "Feeling sad";

    Database db;
    db.Put({"id", "Have a hand", "not-master", 1536107260, 10});
    db.Erase("id");
    db.Put({"id", final_body, "not-master", 1536107260, -10});

    auto record = db.GetById("id");
    ASSERT(record != nullptr);
    ASSERT_EQUAL(final_body, record->title);
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestRangeBoundaries);
    RUN_TEST(tr, TestSameUser);
    RUN_TEST(tr, TestReplacement);
    return 0;
}
