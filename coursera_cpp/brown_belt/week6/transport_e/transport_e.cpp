#include <cmath>
#include <exception>
#include <iostream>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "graph.h"
#include "json.h"
#include "router.h"
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
  ROUTE_SETTS,
  TYPE,
  ROAD_DIST,
  NAME,
  IS_ROUND,
  LAT,
  LON,
  STOPS,
  REQ_ID,
  WAIT_T,
  VELOCITY,
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
    {JsonKeys::ROUTE_SETTS, "routing_settings"},
    {JsonKeys::TYPE, "type"},
    {JsonKeys::ROAD_DIST, "road_distances"},
    {JsonKeys::NAME, "name"},
    {JsonKeys::IS_ROUND, "is_roundtrip"},
    {JsonKeys::LAT, "latitude"},
    {JsonKeys::LON, "longitude"},
    {JsonKeys::STOPS, "stops"},
    {JsonKeys::REQ_ID, "request_id"},
    {JsonKeys::WAIT_T, "bus_wait_time"},
    {JsonKeys::VELOCITY, "bus_velocity"},
};

struct Stop {
  string name_ = "";
  double lat_ = 0.0;
  double lon_ = 0.0;
  set<string> buses_ = {};
  unordered_map<string, size_t> distanceToStops_ = {};
  Graph::VertexId from_ = 0;
  Graph::VertexId to_ = 0;
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

class Bus {
 public:
  Bus(const string& routeNumber, bool isRing, vector<string> stopNames)
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
using Buses = unordered_map<string, Bus>;
using FloatGraph = Graph::DirectedWeightedGraph<float>;
using FloatRouter = Graph::Router<float>;

class TransportCatalog {
 public:
  TransportCatalog(Stops stops, Buses buses, int waitTime, int velocity)
      : stops_(move(stops)),
        buses_(move(buses)),
        waitTime_(waitTime),
        velocity_(velocity),
        graphPtr_(),
        routerPtr_(),
        waitActs_(),
        moveActs_() {
    graphPtr_ = make_unique<FloatGraph>(stops_.size() * 2);

    for (const auto& [busNames, bus] : buses_) {
      const auto& stopsNames = bus.stopsNames();
      for (size_t i = 0; i < stopsNames.size() - 1; ++i) {
        const auto& first = stops_.at(stopsNames[i]);
        const auto& second = stops_.at(stopsNames[i + 1]);

        Graph::Edge<float> firstWaitEdge{first.from_, first.to_, waitTime_};
        const auto firstWaitEdgeId = graphPtr_->AddEdge(firstWaitEdge);
        waitActs_[firstWaitEdgeId] = firstWaitEdge;

        float time;
        if (first.distanceToStops_.count(second.name_)) {
          time = first.distanceToStops_.at(second.name_) / velocity_;
        } else {
          time = second.distanceToStops_.at(first.name_) / velocity_;
        }
        Graph::Edge<float> moveEdge{first.to_, second.from_, time};
        const auto moveEdgeId = graphPtr_->AddEdge(moveEdge);
        moveActs_[moveEdgeId] = moveEdge;

        Graph::Edge<float> secondWaitEdge{second.from_, second.to_, waitTime_};
        const auto secondWaitEdgeId = graphPtr_->AddEdge(secondWaitEdge);
        waitActs_[secondWaitEdgeId] = secondWaitEdge;
      }

      if (!bus.isRing()) {
        for (size_t i = stopsNames.size() - 1; i > 0; --i) {
          const auto& first = stops_.at(stopsNames[i]);
          const auto& second = stops_.at(stopsNames[i - 1]);

          Graph::Edge<float> firstWaitEdge{first.from_, first.to_, waitTime_};
          const auto firstWaitEdgeId = graphPtr_->AddEdge(firstWaitEdge);
          waitActs_[firstWaitEdgeId] = firstWaitEdge;

          float time;
          if (first.distanceToStops_.count(second.name_)) {
            time = first.distanceToStops_.at(second.name_) / velocity_;
          } else {
            time = second.distanceToStops_.at(first.name_) / velocity_;
          }
          Graph::Edge<float> moveEdge{first.to_, second.from_, time};
          const auto moveEdgeId = graphPtr_->AddEdge(moveEdge);
          moveActs_[moveEdgeId] = moveEdge;

          Graph::Edge<float> secondWaitEdge{second.from_, second.to_,
                                            waitTime_};
          const auto secondWaitEdgeId = graphPtr_->AddEdge(secondWaitEdge);
          waitActs_[secondWaitEdgeId] = secondWaitEdge;
        }
      }
    }
  }

  unordered_map<string, Json::Node> busInfo(const string& routeNumber, int id) {
    unordered_map<string, Json::Node> result;
    result[JSON_KEYS.at(JsonKeys::TYPE)] = Json::Node("Bus"s);
    result[JSON_KEYS.at(JsonKeys::REQ_ID)] = Json::Node(id);

    if (buses_.count(routeNumber)) {
      auto stops = buses_.at(routeNumber).stopsCount();
      auto uniqueStops = buses_.at(routeNumber).uniqueStopsCount();

      auto lengthByData = buses_.at(routeNumber).lengthByData();
      if (!lengthByData) {
        auto routeLength = computeLengthByData(routeNumber);
        buses_.at(routeNumber).setLengthByData(routeLength);

        auto lengthByCoordinates = computeLengthByCoordinates(routeNumber);
        auto curvature = routeLength / lengthByCoordinates;
        buses_.at(routeNumber).setCurvature(curvature);
      }

      result[JSON_KEYS.at(JsonKeys::ROUTE_LNGTH)] =
          Json::Node(static_cast<int>(*buses_.at(routeNumber).lengthByData()));
      result[JSON_KEYS.at(JsonKeys::CURV)] =
          Json::Node(*buses_.at(routeNumber).curvature());
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

  //  unordered_map<string, Json::Node> routeInfo(const pair<string, string>& p,
  //                                              int id) {
  //    unordered_map<string, Json::Node> result;
  //    result[JSON_KEYS.at(JsonKeys::TYPE)] = Json::Node("Route"s);
  //    result[JSON_KEYS.at(JsonKeys::REQ_ID)] = Json::Node(id);

  //    if (routes_.count(p)) {
  //      // TODO: return existing route
  //    } else {
  //      // TODO: try to calculate route, add to routes_, and return route.
  //      // If it's impossible to calculate route, return error message
  //      result[JSON_KEYS.at(JsonKeys::ERR_MSG)] = Json::Node("not found"s);
  //    }

  //    return result;
  //  }

 private:
  using RouteActions = unordered_map<Graph::EdgeId, Graph::Edge<float>>;

  struct WaitAction {
    string stopName_;
    float time_;
  };

  Stops stops_;
  Buses buses_;
  const float waitTime_;
  const float velocity_;
  unique_ptr<FloatGraph> graphPtr_;
  unique_ptr<FloatRouter> routerPtr_;
  RouteActions waitActs_;
  RouteActions moveActs_;

  double computeLengthByCoordinates(const string& routeNumber) const {
    auto stopsNames = buses_.at(routeNumber).stopsNames();
    double length = 0.0;
    for (size_t i = 0; i < stopsNames.size() - 1; ++i) {
      const auto& first = stops_.at(stopsNames[i]);
      const auto& second = stops_.at(stopsNames[i + 1]);
      length += computeDistance(first, second);
    }

    if (!buses_.at(routeNumber).isRing()) {
      length *= 2;
    }

    return length;
  }

  size_t computeLengthByData(const string& routeNumber) const {
    const auto stopsNames = buses_.at(routeNumber).stopsNames();
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

    if (!buses_.at(routeNumber).isRing()) {
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

Stop buildStop(const map<string, Json::Node>& reqMap, Graph::VertexId& ctr) {
  Stop stop;
  stop.name_ = reqMap.at(JSON_KEYS.at(JsonKeys::NAME)).AsString();
  stop.lat_ = reqMap.at(JSON_KEYS.at(JsonKeys::LAT)).AsDouble();
  stop.lon_ = reqMap.at(JSON_KEYS.at(JsonKeys::LON)).AsDouble();
  stop.from_ = ++ctr;
  stop.to_ = ++ctr;
  const auto& distancesToStops =
      reqMap.at(JSON_KEYS.at(JsonKeys::ROAD_DIST)).AsMap();
  for (const auto& elem : distancesToStops) {
    stop.distanceToStops_.emplace(elem.first, elem.second.AsInt());
  }

  return stop;
}

Bus buildBus(const map<string, Json::Node>& reqMap) {
  auto routeNumber = reqMap.at(JSON_KEYS.at(JsonKeys::NAME)).AsString();
  auto isRing = reqMap.at(JSON_KEYS.at(JsonKeys::IS_ROUND)).AsBool();
  const auto& stops = reqMap.at(JSON_KEYS.at(JsonKeys::STOPS)).AsArray();
  vector<string> stopNames;
  for (const auto& stop : stops) {
    stopNames.push_back(stop.AsString());
  }

  return Bus(routeNumber, isRing, move(stopNames));
}

TransportCatalog buildCatalog(const vector<Json::Node>& requests,
                              const map<string, Json::Node>& routingParams) {
  Stops stops;
  Buses buses;

  Graph::VertexId ctr = 0;
  for (const auto& req : requests) {
    const auto& reqMap = req.AsMap();
    const auto& type = reqMap.at(JSON_KEYS.at(JsonKeys::TYPE)).AsString();
    if (type == "Stop") {
      auto stop = buildStop(reqMap, ctr);
      if (stops.count(stop.name_)) {
        stops[stop.name_].lat_ = stop.lat_;
        stops[stop.name_].lon_ = stop.lon_;
        stops[stop.name_].distanceToStops_ = move(stop.distanceToStops_);
        stops[stop.name_].name_ = move(stop.name_);
      } else {
        stops[stop.name_] = move(stop);
      }
    } else if (type == "Bus") {
      auto bus = buildBus(reqMap);
      auto routeNumber = bus.routeNumber();
      for (const auto& stopName : bus.stopsNames()) {
        stops[stopName].buses_.insert(routeNumber);
      }
      // TODO: почему не работает operator[]
      // routes[move(routeNumber)] = move(busRoute);
      buses.insert({routeNumber, move(bus)});
    }
  }

  auto waitTime = routingParams.at(JSON_KEYS.at(JsonKeys::WAIT_T)).AsInt();
  auto velocity = routingParams.at(JSON_KEYS.at(JsonKeys::VELOCITY)).AsInt();
  return {stops, buses, waitTime, velocity};
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
    } else if (type == "Route") {
      // TODO: fill outStream
    }
  }

  outStream << "}";
  return outStream.str();
}

void processRequests(TransportCatalog& manager,
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
      const auto busInfo = manager.busInfo(
          reqMap.at(JSON_KEYS.at(JsonKeys::NAME)).AsString(), id);
      outStream << printReq(busInfo);
    } else if (type == "Stop") {
      const auto stopInfo = manager.stopInfo(
          reqMap.at(JSON_KEYS.at(JsonKeys::NAME)).AsString(), id);
      outStream << printReq(stopInfo);
    } else if (type == "Route") {
      //      const auto& from = reqMap.at("from").AsString();
      //      const auto& to = reqMap.at("to").AsString();
      //      const auto p = make_pair(from, to);
      // TODO: print route info
    }
  }
  outStream << "]";
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
  auto catalog =
      buildCatalog(rootMap.at(JSON_KEYS.at(JsonKeys::BASE_REQS)).AsArray(),
                   rootMap.at(JSON_KEYS.at(JsonKeys::ROUTE_SETTS)).AsMap());
  ostringstream out_stream;
  processRequests(catalog,
                  rootMap.at(JSON_KEYS.at(JsonKeys::STAT_REQS)).AsArray(),
                  out_stream);

  cout << out_stream.str() << endl;
}

int main() {
  auto doc = Json::Load(cin);
  const auto& rootMap = doc.GetRoot().AsMap();
  auto catalog =
      buildCatalog(rootMap.at(JSON_KEYS.at(JsonKeys::BASE_REQS)).AsArray(),
                   rootMap.at(JSON_KEYS.at(JsonKeys::ROUTE_SETTS)).AsMap());

  processRequests(catalog,
                  rootMap.at(JSON_KEYS.at(JsonKeys::STAT_REQS)).AsArray());

  //  TestRunner tr;
  //  RUN_TEST(tr, testTransportDPart);

  return 0;
}
