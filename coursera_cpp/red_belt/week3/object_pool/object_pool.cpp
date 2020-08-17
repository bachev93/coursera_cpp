#include "test_runner.h"

#include <algorithm>
#include <iostream>
#include <queue>
#include <set>
#include <stdexcept>
#include <string>
using namespace std;

template <class T>
class ObjectPool {
 public:
  T* Allocate() {
    if (!releasedObjects_.empty()) {
      auto object = releasedObjects_.front();
      allocatedObjects_.insert(object);
      releasedObjects_.pop();
      return object;
    }

    auto object = new T;
    allocatedObjects_.insert(object);
    return object;
  }

  T* TryAllocate() {
    if (!releasedObjects_.empty()) {
      auto object = releasedObjects_.front();
      allocatedObjects_.insert(object);
      releasedObjects_.pop();
      return object;
    }

    return nullptr;
  }

  void Deallocate(T* object) {
    if (allocatedObjects_.count(object)) {
      auto objectIter = allocatedObjects_.find(object);
      releasedObjects_.push(*objectIter);
      allocatedObjects_.erase(objectIter);
    } else {
      throw invalid_argument("no object to deallocate");
    }
  }

  ~ObjectPool() {
    for (auto& elem : allocatedObjects_) {
      delete elem;
    }
    allocatedObjects_.clear();

    while (!releasedObjects_.empty()) {
      auto object = releasedObjects_.front();
      delete object;
      releasedObjects_.pop();
    }
  }

 private:
  set<T*> allocatedObjects_;
  queue<T*> releasedObjects_;
  // Добавьте сюда поля
};

void TestObjectPool() {
  ObjectPool<string> pool;

  auto p1 = pool.Allocate();
  auto p2 = pool.Allocate();
  auto p3 = pool.Allocate();

  *p1 = "first";
  *p2 = "second";
  *p3 = "third";

  pool.Deallocate(p2);
  ASSERT_EQUAL(*pool.Allocate(), "second");

  pool.Deallocate(p3);
  pool.Deallocate(p1);
  ASSERT_EQUAL(*pool.Allocate(), "third");
  ASSERT_EQUAL(*pool.Allocate(), "first");

  pool.Deallocate(p1);
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestObjectPool);
  return 0;
}
