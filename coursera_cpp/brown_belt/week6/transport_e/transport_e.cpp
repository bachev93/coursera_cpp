#include <cmath>
#include <exception>
#include <iostream>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "json.h"
#include "test_runner.h"

using namespace std;

namespace {
const int EARTH_RADIUS = 6371e3;  // meters
const double PI = 3.1415926535;

enum class JsonKeys {
  ERR_MSG,
  ROUTE_LNGTH,
  CURV,
  STOP_CNT,
  UNIQ_STOP_CNT,
  BUSES,
  ID,
  BASE_REQS,
  STAT_REQS,
  TYPE,
  ROAD_DIST,
  NAME,
  IS_ROUND,
  LAT,
  LON,
  STOPS,
  REQ_ID,
};

const unordered_map<JsonKeys, string> JSON_KEYS = {
    {JsonKeys::ERR_MSG, "error_message"},
    {JsonKeys::ROUTE_LNGTH, "route_length"},
    {JsonKeys::CURV, "curvature"},
    {JsonKeys::STOP_CNT, "stop_count"},
    {JsonKeys::UNIQ_STOP_CNT, "unique_stop_count"},
    {JsonKeys::BUSES, "buses"},
    {JsonKeys::ID, "id"},
    {JsonKeys::BASE_REQS, "base_requests"},
    {JsonKeys::STAT_REQS, "stat_requests"},
    {JsonKeys::TYPE, "type"},
    {JsonKeys::ROAD_DIST, "road_distances"},
    {JsonKeys::NAME, "name"},
    {JsonKeys::IS_ROUND, "is_roundtrip"},
    {JsonKeys::LAT, "latitude"},
    {JsonKeys::LON, "longitude"},
    {JsonKeys::STOPS, "stops"},
    {JsonKeys::REQ_ID, "request_id"},
};

struct Stop {
  string name_ = "";
  double lat_ = 0.0;
  double lon_ = 0.0;
  set<string> buses_ = {};
  unordered_map<string, size_t> distanceToStops_ = {};
};

double degToRad(double deg) {
  return deg * PI / 180;
}

double computeDistance(const Stop& lhs, const Stop& rhs) {
  auto lhsRadLat = degToRad(lhs.lat_);
  auto lhsRadLon = degToRad(lhs.lon_);
  auto rhsRadLat = degToRad(rhs.lat_);
  auto rhsRadLon = degToRad(rhs.lon_);

  return acos(sin(lhsRadLat) * sin(rhsRadLat) +
              cos(lhsRadLat) * cos(rhsRadLat) *
                  cos(abs(lhsRadLon - rhsRadLon))) *
         EARTH_RADIUS;
}

class BusRoute {
 public:
  BusRoute(const string& routeNumber, bool isRing, vector<string> stopNames)
      : number_(routeNumber),
        isRing_(isRing),
        stopNames_(move(stopNames)),
        lengthByData_(),
        curvature_() {}

  size_t stopsCount() const {
    return stopNames_.size() + (isRing_ ? 0 : stopNames_.size() - 1);
  }

  size_t uniqueStopsCount() const {
    set<string> uniqueStops(stopNames_.begin(), stopNames_.end());
    return uniqueStops.size();
  }

  optional<size_t> lengthByData() const { return lengthByData_; }

  optional<double> curvature() const { return curvature_; }

  const vector<string>& stopsNames() const { return stopNames_; }

  bool isRing() const { return isRing_; }

  string routeNumber() const { return number_; }

  void setLengthByData(size_t length) { lengthByData_.emplace(length); }

  void setCurvature(double curvature) { curvature_.emplace(curvature); }

 private:
  string number_;
  bool isRing_;
  vector<string> stopNames_;
  optional<size_t> lengthByData_;
  optional<double> curvature_;
};

using Stops = unordered_map<string, Stop>;
using Routes = unordered_map<string, BusRoute>;

class BusManager {
 public:
  BusManager(Stops stops, Routes routes)
      : stops_(move(stops)), routes_(move(routes)) {}

  unordered_map<string, Json::Node> routeInfo(const string& routeNumber,
                                              int id) {
    unordered_map<string, Json::Node> result;
    result[JSON_KEYS.at(JsonKeys::TYPE)] = Json::Node("Bus"s);
    result[JSON_KEYS.at(JsonKeys::REQ_ID)] = Json::Node(id);

    if (routes_.count(routeNumber)) {
      auto stops = routes_.at(routeNumber).stopsCount();
      auto uniqueStops = routes_.at(routeNumber).uniqueStopsCount();

      auto lengthByData = routes_.at(routeNumber).lengthByData();
      if (!lengthByData) {
        auto routeLength = computeLengthByData(routeNumber);
        routes_.at(routeNumber).setLengthByData(routeLength);

        auto lengthByCoordinates = computeLengthByCoordinates(routeNumber);
        auto curvature = routeLength / lengthByCoordinates;
        routes_.at(routeNumber).setCurvature(curvature);
      }

      result[JSON_KEYS.at(JsonKeys::ROUTE_LNGTH)] =
          Json::Node(static_cast<int>(*routes_.at(routeNumber).lengthByData()));
      result[JSON_KEYS.at(JsonKeys::CURV)] =
          Json::Node(*routes_.at(routeNumber).curvature());
      result[JSON_KEYS.at(JsonKeys::STOP_CNT)] =
          Json::Node(static_cast<int>(stops));
      result[JSON_KEYS.at(JsonKeys::UNIQ_STOP_CNT)] =
          Json::Node(static_cast<int>(uniqueStops));
    } else {
      result[JSON_KEYS.at(JsonKeys::ERR_MSG)] = Json::Node("not found"s);
    }

    return result;
  }

  unordered_map<string, Json::Node> stopInfo(const string& stopName, int id) {
    unordered_map<string, Json::Node> result;
    result[JSON_KEYS.at(JsonKeys::TYPE)] = Json::Node("Stop"s);
    result[JSON_KEYS.at(JsonKeys::REQ_ID)] = Json::Node(id);

    if (stops_.count(stopName)) {
      const auto& buses = stops_.at(stopName).buses_;
      vector<Json::Node> busesForStop;
      for (const auto& bus : buses) {
        busesForStop.push_back(Json::Node(bus));
      }

      result[JSON_KEYS.at(JsonKeys::BUSES)] = Json::Node(busesForStop);
    } else {
      result[JSON_KEYS.at(JsonKeys::ERR_MSG)] = Json::Node("not found"s);
    }

    return result;
  }

 private:
  Stops stops_;
  Routes routes_;

  double computeLengthByCoordinates(const string& routeNumber) const {
    auto stopsNames = routes_.at(routeNumber).stopsNames();
    double length = 0.0;
    for (size_t i = 0; i < stopsNames.size() - 1; ++i) {
      const auto& first = stops_.at(stopsNames[i]);
      const auto& second = stops_.at(stopsNames[i + 1]);
      length += computeDistance(first, second);
    }

    if (!routes_.at(routeNumber).isRing()) {
      length *= 2;
    }

    return length;
  }

  size_t computeLengthByData(const string& routeNumber) const {
    const auto stopsNames = routes_.at(routeNumber).stopsNames();
    size_t length = 0;
    for (size_t i = 0; i < stopsNames.size() - 1; ++i) {
      const auto& first = stops_.at(stopsNames[i]);
      const auto& second = stops_.at(stopsNames[i + 1]);

      if (first.distanceToStops_.count(second.name_)) {
        length += first.distanceToStops_.at(second.name_);
      } else {
        length += second.distanceToStops_.at(first.name_);
      }
    }

    if (!routes_.at(routeNumber).isRing()) {
      for (size_t i = stopsNames.size() - 1; i > 0; --i) {
        const auto& first = stops_.at(stopsNames[i]);
        const auto& second = stops_.at(stopsNames[i - 1]);
        if (first.distanceToStops_.count(second.name_)) {
          length += first.distanceToStops_.at(second.name_);
        } else {
          length += second.distanceToStops_.at(first.name_);
        }
      }
    }

    return length;
  }
};
}  // namespace

Stop buildStop(const map<string, Json::Node>& reqMap) {
  Stop stop;
  stop.name_ = reqMap.at(JSON_KEYS.at(JsonKeys::NAME)).AsString();
  stop.lat_ = reqMap.at(JSON_KEYS.at(JsonKeys::LAT)).AsDouble();
  stop.lon_ = reqMap.at(JSON_KEYS.at(JsonKeys::LON)).AsDouble();
  const auto& distancesToStops =
      reqMap.at(JSON_KEYS.at(JsonKeys::ROAD_DIST)).AsMap();
  for (const auto& elem : distancesToStops) {
    stop.distanceToStops_.emplace(elem.first, elem.second.AsInt());
  }

  return stop;
}

BusRoute buildRoute(const map<string, Json::Node>& reqMap) {
  auto routeNumber = reqMap.at(JSON_KEYS.at(JsonKeys::NAME)).AsString();
  auto isRing = reqMap.at(JSON_KEYS.at(JsonKeys::IS_ROUND)).AsBool();
  const auto& stops = reqMap.at(JSON_KEYS.at(JsonKeys::STOPS)).AsArray();
  vector<string> stopNames;
  for (const auto& stop : stops) {
    stopNames.push_back(stop.AsString());
  }

  return BusRoute(routeNumber, isRing, move(stopNames));
}

BusManager buildDataBase(const vector<Json::Node>& requests) {
  Stops stops;
  Routes routes;

  for (const auto& req : requests) {
    const auto& reqMap = req.AsMap();
    const auto& type = reqMap.at(JSON_KEYS.at(JsonKeys::TYPE)).AsString();
    if (type == "Stop") {
      auto stop = buildStop(reqMap);
      if (stops.count(stop.name_)) {
        stops[stop.name_].lat_ = stop.lat_;
        stops[stop.name_].lon_ = stop.lon_;
        stops[stop.name_].distanceToStops_ = move(stop.distanceToStops_);
        stops[stop.name_].name_ = move(stop.name_);
      } else {
        stops[stop.name_] = move(stop);
      }
    } else if (type == "Bus") {
      auto busRoute = buildRoute(reqMap);
      auto routeNumber = busRoute.routeNumber();
      for (const auto& stopName : busRoute.stopsNames()) {
        stops[stopName].buses_.insert(routeNumber);
      }
      // TODO: почему не работает operator[]
      // routes[move(routeNumber)] = move(busRoute);
      routes.insert({routeNumber, move(busRoute)});
    }
  }

  return {stops, routes};
}

string printReq(const unordered_map<string, Json::Node>& req) {
  ostringstream outStream;
  outStream.precision(6);

  outStream << "{";
  const auto& type = req.at(JSON_KEYS.at(JsonKeys::TYPE)).AsString();
  outStream << "\"" << JSON_KEYS.at(JsonKeys::REQ_ID)
            << "\": " << req.at(JSON_KEYS.at(JsonKeys::REQ_ID)).AsInt() << ", ";

  if (req.count(JSON_KEYS.at(JsonKeys::ERR_MSG))) {
    outStream << "\"" << JSON_KEYS.at(JsonKeys::ERR_MSG) << "\": "
              << "\"" << req.at(JSON_KEYS.at(JsonKeys::ERR_MSG)).AsString()
              << "\"";
  } else {
    if (type == "Bus") {
      outStream << "\"" << JSON_KEYS.at(JsonKeys::ROUTE_LNGTH)
                << "\": " << req.at(JSON_KEYS.at(JsonKeys::ROUTE_LNGTH)).AsInt()
                << ", ";
      outStream << "\"" << JSON_KEYS.at(JsonKeys::CURV)
                << "\": " << req.at(JSON_KEYS.at(JsonKeys::CURV)).AsDouble()
                << ", ";
      outStream << "\"" << JSON_KEYS.at(JsonKeys::STOP_CNT)
                << "\": " << req.at(JSON_KEYS.at(JsonKeys::STOP_CNT)).AsInt()
                << ", ";
      outStream << "\"" << JSON_KEYS.at(JsonKeys::UNIQ_STOP_CNT) << "\": "
                << req.at(JSON_KEYS.at(JsonKeys::UNIQ_STOP_CNT)).AsInt();
    } else if (type == "Stop") {
      outStream << "\"" << JSON_KEYS.at(JsonKeys::BUSES) << "\": [";
      const auto& buses = req.at(JSON_KEYS.at(JsonKeys::BUSES)).AsArray();

      bool isFirst = true;
      for (const auto& bus : buses) {
        if (isFirst) {
          isFirst = false;
        } else {
          outStream << ", ";
        }

        outStream << "\"" << bus.AsString() << "\"";
      }

      outStream << "]";
    }
  }

  outStream << "}";
  return outStream.str();
}

void processRequests(BusManager& manager,
                     const vector<Json::Node>& requests,
                     ostream& outStream = cout) {
  outStream.precision(6);
  outStream << "[";

  bool isFirst = true;
  for (const auto& request : requests) {
    if (isFirst) {
      isFirst = false;
    } else {
      outStream << ", ";
    }

    const auto reqMap = request.AsMap();
    const auto& type = reqMap.at(JSON_KEYS.at(JsonKeys::TYPE)).AsString();
    const auto id = reqMap.at(JSON_KEYS.at(JsonKeys::ID)).AsInt();
    if (type == "Bus") {
      const auto routeInfo = manager.routeInfo(
          reqMap.at(JSON_KEYS.at(JsonKeys::NAME)).AsString(), id);
      outStream << printReq(routeInfo);
    } else {
      const auto stopInfo = manager.stopInfo(
          reqMap.at(JSON_KEYS.at(JsonKeys::NAME)).AsString(), id);
      outStream << printReq(stopInfo);
    }
  }
  outStream << "]";
}

void testJson() {
  string str = R"({
               "type": "Bus",
               "is_roundtrip": true,
               "name": "256",
               "stops": [
                 "Biryulyovo Zapadnoye",
                 "Biryusinka",
                 "Universam",
                 "Biryulyovo Tovarnaya",
                 "Biryulyovo Passazhirskaya",
                 "Biryulyovo Zapadnoye"
               ]
             })";

  istringstream in_stream(str);
  auto doc = Json::Load(in_stream);
  const auto& root = doc.GetRoot();
  auto parsedResult = root.AsMap();

  cout << "name: " << parsedResult["name"].AsString() << endl;

  auto route = buildRoute(parsedResult);
  cout << "route num: " << route.routeNumber() << endl;
}

void testLoadVal() {
  string str =
      "{"
      "\"value\": -15.42,"
      "\"id\": 21,"
      "\"latitude\": 55.61"
      "}";
  istringstream in_stream(str);
  auto doc = Json::Load(in_stream);
  const auto& root = doc.GetRoot();
  auto parsedRes = root.AsMap();
  cout << "value:_" << parsedRes["value"].AsDouble() << endl;
  cout << "id:_" << parsedRes["id"].AsInt() << endl;
  cout << "latitude:_" << parsedRes["latitude"].AsDouble() << endl;
}

void testTransportDPart() {
  string str = R"({
          "base_requests": [
            {
              "type": "Stop",
              "road_distances": {
                "Marushkino": 3900
              },
              "longitude": 37.20829,
              "name": "Tolstopaltsevo",
              "latitude": 55.611087
            },
            {
              "type": "Stop",
              "road_distances": {
                "Rasskazovka": 9900
              },
              "longitude": 37.209755,
              "name": "Marushkino",
              "latitude": 55.595884
            },
            {
              "type": "Bus",
              "name": "256",
              "stops": [
                "Biryulyovo Zapadnoye",
                "Biryusinka",
                "Universam",
                "Biryulyovo Tovarnaya",
                "Biryulyovo Passazhirskaya",
                "Biryulyovo Zapadnoye"
              ],
              "is_roundtrip": true
            },
            {
              "type": "Bus",
              "name": "750",
              "stops": [
                "Tolstopaltsevo",
                "Marushkino",
                "Rasskazovka"
              ],
              "is_roundtrip": false
            },
            {
              "type": "Stop",
              "road_distances": {},
              "longitude": 37.333324,
              "name": "Rasskazovka",
              "latitude": 55.632761
            },
            {
              "type": "Stop",
              "road_distances": {
                "Rossoshanskaya ulitsa": 7500,
                "Biryusinka": 1800,
                "Universam": 2400
              },
              "longitude": 37.6517,
              "name": "Biryulyovo Zapadnoye",
              "latitude": 55.574371
            },
            {
              "type": "Stop",
              "road_distances": {
                "Universam": 750
              },
              "longitude": 37.64839,
              "name": "Biryusinka",
              "latitude": 55.581065
            },
            {
              "type": "Stop",
              "road_distances": {
                "Rossoshanskaya ulitsa": 5600,
                "Biryulyovo Tovarnaya": 900
              },
              "longitude": 37.645687,
              "name": "Universam",
              "latitude": 55.587655
            },
            {
              "type": "Stop",
              "road_distances": {
                "Biryulyovo Passazhirskaya": 1300
              },
              "longitude": 37.653656,
              "name": "Biryulyovo Tovarnaya",
              "latitude": 55.592028
            },
            {
              "type": "Stop",
              "road_distances": {
                "Biryulyovo Zapadnoye": 1200
              },
              "longitude": 37.659164,
              "name": "Biryulyovo Passazhirskaya",
              "latitude": 55.580999
            },
            {
              "type": "Bus",
              "name": "828",
              "stops": [
                "Biryulyovo Zapadnoye",
                "Universam",
                "Rossoshanskaya ulitsa",
                "Biryulyovo Zapadnoye"
              ],
              "is_roundtrip": true
            },
            {
              "type": "Stop",
              "road_distances": {},
              "longitude": 37.605757,
              "name": "Rossoshanskaya ulitsa",
              "latitude": 55.595579
            },
            {
              "type": "Stop",
              "road_distances": {},
              "longitude": 37.603831,
              "name": "Prazhskaya",
              "latitude": 55.611678
            }
          ],
          "stat_requests": [
            {
              "type": "Bus",
              "name": "256",
              "id": 1965312327
            },
            {
              "type": "Bus",
              "name": "750",
              "id": 519139350
            },
            {
              "type": "Bus",
              "name": "751",
              "id": 194217464
            },
            {
              "type": "Stop",
              "name": "Samara",
              "id": 746888088
            },
            {
              "type": "Stop",
              "name": "Prazhskaya",
              "id": 65100610
            },
            {
              "type": "Stop",
              "name": "Biryulyovo Zapadnoye",
              "id": 1042838872
            }
          ]
    }
    )";

  istringstream in_stream(str);
  auto doc = Json::Load(in_stream);
  const auto& rootMap = doc.GetRoot().AsMap();
  auto manager =
      buildDataBase(rootMap.at(JSON_KEYS.at(JsonKeys::BASE_REQS)).AsArray());
  ostringstream out_stream;
  processRequests(manager,
                  rootMap.at(JSON_KEYS.at(JsonKeys::STAT_REQS)).AsArray(),
                  out_stream);

  cout << out_stream.str() << endl;
}

int main() {
  auto doc = Json::Load(cin);
  const auto& rootMap = doc.GetRoot().AsMap();
  auto manager = buildDataBase(rootMap.at("base_requests").AsArray());
  processRequests(manager, rootMap.at("stat_requests").AsArray());

  //  TestRunner tr;
  //  RUN_TEST(tr, testTransportDPart);
  //  RUN_TEST(tr, testJson);
  //  RUN_TEST(tr, testLoadVal);

  return 0;
}
