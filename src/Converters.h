#pragma once

#include <sc2pp/Common.h>
#include <sc2pp/utils/Grid.h>

#include <vector>
#include <ranges>
#include <variant>
#include <optional>

#include <s2clientprotocol/error.pb.h>
#include <google/protobuf/descriptor.h>

#include <boost/hof.hpp>
#include <boost/hof/proj.hpp>

//TODO: move types to Types.h
namespace sc2 {
namespace proto = SC2APIProtocol;

template <class R, class Value>
concept range_over = std::ranges::range<R> &&
std::same_as<std::ranges::range_value_t<R>, Value>;

template <class T, range_over<T> Range>
auto to_vector(Range r)
{
    return std::vector<T>{std::ranges::begin(r), std::ranges::end(r)};
}

Point2D from_proto(const proto::Point2D& input);

struct UnitCommand
{
    int ability_id;
    std::variant<Point2D, std::uint64_t> target;
};

UnitCommand from_proto(const proto::ActionRawUnitCommand& unit_command);

//message ResponseObservation {
//  repeated Action actions = 1;                  // Actions this player did since the last Observation.
//  repeated ActionError action_errors = 2;       // Equivalent of UI "red text" errors.
//  optional Observation observation = 3;
//  repeated PlayerResult player_result = 4;      // Only populated if the game ended during this step.
//  repeated ChatReceived chat = 5;
//}
//message ActionError {
//  optional uint64 unit_tag = 1;                             // Only populated when using raw interface.
//  optional uint64 ability_id = 2;
//  optional ActionResult result = 3;
//}

struct ActionError
{
    uint64_t unit_tag;
    uint64_t ability_id;
    proto::ActionResult result;
    std::string res_str;
};

ActionError from_proto(const proto::ActionError& x);

Point3D from_proto(const proto::Point& x);

auto from_proto(const proto::UnitOrder& x);

auto from_proto(const proto::PassengerUnit& x);

auto from_proto(const proto::RallyTarget& x);

Unit from_proto(const proto::Unit& x);

std::tuple<
    std::vector<Unit>
    , std::vector<UnitCommand>
    , std::vector<ActionError>
> convert_observation(const proto::ResponseObservation& response_observation);

enum Difficulty
{
    VeryEasy = 1,
    Easy = 2,
    Medium = 3,
    MediumHard = 4,
    Hard = 5,
    HardVeryHard = 6,
    VeryHard = 7,
    CheatVision = 8,
    CheatMoney = 9,
    CheatInsane = 10
};

enum PlayerType
{
    Participant = 1,
    Computer = 2,
    Observer = 3
};

struct PlayerInfo
{
    uint32_t player_id;
    PlayerType player_type;
    Race race_requested;
    Race race_actual;
    Difficulty difficulty;
};

/*
* message StartRaw {
  optional Size2DI map_size = 1;            // Width and height of the map.
  optional ImageData pathing_grid = 2;      // 1 bit bitmap of the pathing grid.
  optional ImageData terrain_height = 3;    // 1 byte bitmap of the terrain height.
  optional ImageData placement_grid = 4;    // 1 bit bitmap of the building placement grid.
  optional RectangleI playable_area = 5;    // The playable cells.
  repeated Point2D start_locations = 6;     // Possible start locations for players.
}
*/

struct GameInfo
{
    std::string map_name;
    //Filename of map. Includes the ".SC2Map" file extension.
    std::string local_map_path;
    Point2DI map_size;
    //Grid showing which cells are pathable by units.
    utils::Grid<bool> pathing_grid;
    //Height map of terrain.
    utils::Grid<std::uint8_t> terrain_height;
    //Grid showing which cells can accept placement of structures.
    utils::Grid<std::uint8_t> placement_grid;

    std::vector<Point2D> start_locations;

    //std::vector<PlayerInfo> player_info;
};

GameInfo from_proto(const proto::ResponseGameInfo& x);

}
