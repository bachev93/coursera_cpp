#include <iostream>
#include <list>
#include <map>

using namespace std;

int main() {
  list<int> sportsmen;
  map<int, list<int>::iterator> sportsmenMap;

  int sportsmenCount;
  cin >> sportsmenCount;

  for (int i = 0; i < sportsmenCount; ++i) {
    int playerNumber, otherPlayerNumber;
    cin >> playerNumber >> otherPlayerNumber;

    if (sportsmenMap.count(otherPlayerNumber)) {
      auto it =
          sportsmen.insert(sportsmenMap.at(otherPlayerNumber), playerNumber);
      sportsmenMap[playerNumber] = it;
    } else {
      sportsmen.push_back(playerNumber);
      sportsmenMap[playerNumber] = prev(sportsmen.end());
    }
  }

  for (const auto& elem : sportsmen) {
    cout << elem << " ";
  }
  cout << endl;

  return 0;
}
