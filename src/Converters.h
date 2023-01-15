#pragma once

#include <vector>
#include <ranges>
#include <variant>
#include <optional>

#include <s2clientprotocol/sc2api.pb.h>
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

struct Point2D
{
    float x;
    float y;
};

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

using Alliance = proto::Alliance;
using DisplayType = proto::DisplayType;
using CloakState = proto::CloakState;

struct Point3D
{
    float x, y, z;
};

Point3D from_proto(const proto::Point& x);

struct UnitOrder {
    uint32_t ability_id;
    std::variant<Point3D, uint64_t> target;
    //oneof target{
    //  Point target_world_space_pos = 2;
    //  uint64 target_unit_tag = 3;
    //}
    /*optional*/float progress;              // Progress of train abilities. Range: [0.0, 1.0]
};

auto from_proto(const proto::UnitOrder& x);

struct PassengerUnit {
    uint64_t tag;
    float health;
    float health_max;
    float shield;
    float shield_max;
    float energy;
    float energy_max;
    uint32_t unit_type;
};

auto from_proto(const proto::PassengerUnit& x);

struct RallyTarget {
    Point3D point;  // Will always be filled.
    std::optional<int64_t> tag;   // Only if it's targeting a unit.
};

auto from_proto(const proto::RallyTarget& x);

struct Unit
{
    DisplayType display_type;
    Alliance alliance;

    uint64_t tag;                  // Unique identifier for a unit
    uint32_t unit_type;
    int32_t owner;

    Point3D pos;
    float facing;
    float radius;
    float build_progress;        // Range: [0.0, 1.0]
    CloakState cloak;
    std::vector<uint32_t> buff_ids;

    float detect_range;
    float radar_range;

    bool is_selected;
    bool is_on_screen;          // Visible and within the camera frustrum.
    bool is_blip;               // Detected by sensor tower
    bool is_powered;
    bool is_active;             // Building is training/researching (ie animated).
    int32_t attack_upgrade_level;
    int32_t armor_upgrade_level;
    int32_t shield_upgrade_level;

    float health;
    float health_max;
    float shield;
    float shield_max;
    float energy;
    float energy_max;
    int32_t mineral_contents;
    int32_t vespene_contents;
    bool is_flying;
    bool is_burrowed;
    bool is_hallucination;      // Unit is your own or detected as a hallucination.

    std::vector<UnitOrder> orders;
    uint64_t add_on_tag;
    std::vector<PassengerUnit> passengers;
    int32_t cargo_space_taken;
    int32_t cargo_space_max;
    int32_t assigned_harvesters;
    int32_t ideal_harvesters;
    float weapon_cooldown;
    uint64_t engaged_target_tag;
    int32_t buff_duration_remain;  // How long a buff or unit is still around (eg mule, broodling, chronoboost).
    int32_t buff_duration_max;     // How long the buff or unit is still around (eg mule, broodling, chronoboost).
    std::vector<RallyTarget> rally_targets;
};

Unit from_proto(const proto::Unit& x);

std::tuple<
    std::vector<Unit>
    , std::vector<UnitCommand>
    , std::vector<ActionError>
> convert_observation(const proto::ResponseObservation& response_observation);

}
