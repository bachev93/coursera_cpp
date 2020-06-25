#include <unordered_map>
#include <list>
#include <mutex>
#include <iostream>

#include "Common.h"

using namespace std;

class LruCache : public ICache {
public:
    LruCache(
            shared_ptr<IBooksUnpacker> books_unpacker,
            const Settings& settings
            ) : booksUnpacker_(move(books_unpacker)), settings_(settings),
                currentCommonSize_(), mut_(), rangedBooks_(), bookByName_() {
        // реализуйте метод
    }

    BookPtr GetBook(const string& book_name) override {
        lock_guard guard(mut_);
        // реализуйте метод
        if(bookByName_.count(book_name)) {
//            lock_guard guard(mut_);

            BookPtr bookPtr = move(*bookByName_[book_name]);
            rangedBooks_.erase(bookByName_[book_name]);
            rangedBooks_.push_back(move(bookPtr));
            bookByName_[book_name] = --rangedBooks_.end();
        } else {
//            lock_guard guard(mut_);
            auto book = booksUnpacker_->UnpackBook(book_name);

            if(book->GetContent().size() > settings_.max_memory) {
                return move(book);
            }

            rangedBooks_.push_back(move(book));
            bookByName_[book_name] = --rangedBooks_.end();
            currentCommonSize_ += rangedBooks_.back()->GetContent().size();

            while(currentCommonSize_ > settings_.max_memory) {
                BookPtr bookPtr = move(*rangedBooks_.begin());
                bookByName_.erase(bookPtr->GetName());
                currentCommonSize_ -= bookPtr->GetContent().size();
                rangedBooks_.erase(rangedBooks_.begin());
            }
        }

        return *bookByName_[book_name];
    }
private:
    shared_ptr<IBooksUnpacker> booksUnpacker_;
    const Settings settings_;
    size_t currentCommonSize_;
    mutex mut_;

    list<BookPtr> rangedBooks_;
    using RangeIt = list<BookPtr>::iterator;
    unordered_map<string, RangeIt> bookByName_;
};


unique_ptr<ICache> MakeCache(
        shared_ptr<IBooksUnpacker> books_unpacker,
        const ICache::Settings& settings
        ) {
    // реализуйте функцию
    auto cache = make_unique<LruCache>(move(books_unpacker), settings);
    return cache;
}
