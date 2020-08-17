#include "game_object.h"
#include "geo2d.h"

#include "test_runner.h"

#include <memory>
#include <vector>

using namespace std;

// Определите классы Unit, Building, Tower и Fence так, чтобы они наследовались
// от GameObject и реализовывали его интерфейс.

// идиома CRTP — Curiously recurring template pattern
// Создаём шаблон класса Collider, только в нём будет переопределяться метод
// Collide
template <typename T>
class Collider : public GameObject {
 public:
  bool Collide(const GameObject& that) const final {
    // Статически приводим тип *this к типу const T&, потому что мы знаем,
    // что T — наш наследник (см. ниже)
    return that.CollideWith(static_cast<const T&>(*this));
  }
};

// Наследуем класс Unit от класса Collider<Unit>, который в свою очередь
// наследуется от GameObject
class Unit final : public Collider<Unit> {
 public:
  // Переопределения методов CollideWith — метод Collide переопределять уже не
  // нужно
  explicit Unit(geo2d::Point position);
  geo2d::Point getPosition() const;
  bool CollideWith(const Unit& that) const override;
  bool CollideWith(const Building& that) const override;
  bool CollideWith(const Tower& that) const override;
  bool CollideWith(const Fence& that) const override;

 private:
  geo2d::Point point_;
};

class Building final : public Collider<Building> {
 public:
  explicit Building(geo2d::Rectangle geometry);
  geo2d::Rectangle getRectangle() const;
  bool CollideWith(const Unit& that) const override;
  bool CollideWith(const Building& that) const override;
  bool CollideWith(const Tower& that) const override;
  bool CollideWith(const Fence& that) const override;

 private:
  geo2d::Rectangle rect_;
};

class Tower final : public Collider<Tower> {
 public:
  explicit Tower(geo2d::Circle geometry);
  geo2d::Circle getCircle() const;
  bool CollideWith(const Unit& that) const override;
  bool CollideWith(const Building& that) const override;
  bool CollideWith(const Tower& that) const override;
  bool CollideWith(const Fence& that) const override;

 private:
  geo2d::Circle circle_;
};

class Fence final : public Collider<Fence> {
 public:
  explicit Fence(geo2d::Segment geometry);
  geo2d::Segment getSegment() const;
  bool CollideWith(const Unit& that) const override;
  bool CollideWith(const Building& that) const override;
  bool CollideWith(const Tower& that) const override;
  bool CollideWith(const Fence& that) const override;

 private:
  geo2d::Segment segment_;
};

Unit::Unit(geo2d::Point position) : point_(position) {}

geo2d::Point Unit::getPosition() const {
  return point_;
}

bool Unit::CollideWith(const Unit& that) const {
  return geo2d::Collide(point_, that.getPosition());
}

bool Unit::CollideWith(const Building& that) const {
  return geo2d::Collide(point_, that.getRectangle());
}

bool Unit::CollideWith(const Tower& that) const {
  return geo2d::Collide(point_, that.getCircle());
}

bool Unit::CollideWith(const Fence& that) const {
  return geo2d::Collide(point_, that.getSegment());
}

Building::Building(geo2d::Rectangle geometry) : rect_(geometry) {}

geo2d::Rectangle Building::getRectangle() const {
  return rect_;
}

bool Building::CollideWith(const Unit& that) const {
  return geo2d::Collide(rect_, that.getPosition());
}

bool Building::CollideWith(const Building& that) const {
  return geo2d::Collide(rect_, that.getRectangle());
}

bool Building::CollideWith(const Tower& that) const {
  return geo2d::Collide(rect_, that.getCircle());
}

bool Building::CollideWith(const Fence& that) const {
  return geo2d::Collide(rect_, that.getSegment());
}

Tower::Tower(geo2d::Circle geometry) : circle_(geometry) {}

geo2d::Circle Tower::getCircle() const {
  return circle_;
}

bool Tower::CollideWith(const Unit& that) const {
  return geo2d::Collide(circle_, that.getPosition());
}

bool Tower::CollideWith(const Building& that) const {
  return geo2d::Collide(circle_, that.getRectangle());
}

bool Tower::CollideWith(const Tower& that) const {
  return geo2d::Collide(circle_, that.getCircle());
}

bool Tower::CollideWith(const Fence& that) const {
  return geo2d::Collide(circle_, that.getSegment());
}

Fence::Fence(geo2d::Segment geometry) : segment_(geometry) {}

geo2d::Segment Fence::getSegment() const {
  return segment_;
}

bool Fence::CollideWith(const Unit& that) const {
  return geo2d::Collide(segment_, that.getPosition());
}

bool Fence::CollideWith(const Building& that) const {
  return geo2d::Collide(segment_, that.getRectangle());
}

bool Fence::CollideWith(const Tower& that) const {
  return geo2d::Collide(segment_, that.getCircle());
}

bool Fence::CollideWith(const Fence& that) const {
  return geo2d::Collide(segment_, that.getSegment());
}

// Реализуйте функцию Collide из файла GameObject.h
bool Collide(const GameObject& first, const GameObject& second) {
  return first.Collide(second);
}

void TestAddingNewObjectOnMap() {
  // Юнит-тест моделирует ситуацию, когда на игровой карте уже есть какие-то
  // объекты, и мы хотим добавить на неё новый, например, построить новое здание
  // или башню. Мы можем его добавить, только если он не пересекается ни с одним
  // из существующих.
  using namespace geo2d;

  const vector<shared_ptr<GameObject>> game_map = {
      make_shared<Unit>(Point{3, 3}),
      make_shared<Unit>(Point{5, 5}),
      make_shared<Unit>(Point{3, 7}),
      make_shared<Fence>(Segment{{7, 3}, {9, 8}}),
      make_shared<Tower>(Circle{Point{9, 4}, 1}),
      make_shared<Tower>(Circle{Point{10, 7}, 1}),
      make_shared<Building>(Rectangle{{11, 4}, {14, 6}})};

  for (size_t i = 0; i < game_map.size(); ++i) {
    Assert(Collide(*game_map[i], *game_map[i]),
           "An object doesn't collide with itself: " + to_string(i));

    for (size_t j = 0; j < i; ++j) {
      Assert(!Collide(*game_map[i], *game_map[j]),
             "Unexpected collision found " + to_string(i) + ' ' + to_string(j));
    }
  }

  auto new_warehouse = make_shared<Building>(Rectangle{{4, 3}, {9, 6}});
  ASSERT(!Collide(*new_warehouse, *game_map[0]));
  ASSERT(Collide(*new_warehouse, *game_map[1]));
  ASSERT(!Collide(*new_warehouse, *game_map[2]));
  ASSERT(Collide(*new_warehouse, *game_map[3]));
  ASSERT(Collide(*new_warehouse, *game_map[4]));
  ASSERT(!Collide(*new_warehouse, *game_map[5]));
  ASSERT(!Collide(*new_warehouse, *game_map[6]));

  auto new_defense_tower = make_shared<Tower>(Circle{{8, 2}, 2});
  ASSERT(!Collide(*new_defense_tower, *game_map[0]));
  ASSERT(!Collide(*new_defense_tower, *game_map[1]));
  ASSERT(!Collide(*new_defense_tower, *game_map[2]));
  ASSERT(Collide(*new_defense_tower, *game_map[3]));
  ASSERT(Collide(*new_defense_tower, *game_map[4]));
  ASSERT(!Collide(*new_defense_tower, *game_map[5]));
  ASSERT(!Collide(*new_defense_tower, *game_map[6]));
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestAddingNewObjectOnMap);
  return 0;
}
