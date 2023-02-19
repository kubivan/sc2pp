#pragma once

#include <sc2pp/Common.h>
#include <sc2pp/utils/Grid.h>
#include <sc2pp/utils/UnitPool.h>

#include <vector>

namespace sc2
{

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

class Observation
{
public:
    Observation(GameInfo game_info, std::vector<UnitTypeData> unit_type_data, std::vector<AbilityData> ability_data);

    Observation(Observation&&) = default;

    void update(const proto::ResponseObservation& response_obs);

    auto raw() const -> const proto::Observation&;

    auto unitsSelf() const -> const Units&;

    auto unitsCreated() const -> const Units&;

    auto gameInfo() const -> const GameInfo&;

    auto units() const -> const Units&;

    auto unitsTypeData() const -> const std::vector<UnitTypeData>&;
    auto abilityData() const -> const std::vector<AbilityData>&;

private:
    proto::ResponseObservation m_raw;
    UnitPool m_units;

    Units m_created;
    Units m_damaged;
    Units m_self;

    GameInfo m_game_info;
    std::vector<UnitTypeData> m_unit_type_data;
    std::vector<AbilityData> m_ability_data;
};

} // namspace sc2
