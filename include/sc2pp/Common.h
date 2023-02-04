#pragma once 

#include <sc2pp/SC2IDs.h>

#include <optional>
#include <set>
#include <variant>
#include <vector>

#include <s2clientprotocol/sc2api.pb.h>

namespace sc2 {
namespace proto = SC2APIProtocol;

template <class R, class Value>
concept range_over = std::ranges::range<R> &&
std::same_as<std::ranges::range_value_t<R>, Value>;

template <class T, range_over<T> Range>
auto to_vector(Range r)
{
    return std::vector<T>(std::ranges::begin(r), std::ranges::end(r));
}

template <class T, range_over<T> Range>
auto to_set(Range r)
{
    return std::set<T>{std::ranges::begin(r), std::ranges::end(r)};
}

struct Point3D
{
    float x, y, z;
};

struct Point2D
{
    constexpr Point2D(const Point3D& p)
        : x(p.x)
        , y(p.y)
    {}

    constexpr Point2D(float x = 0, float y = 0)
        : x(x)
        , y(y)
    {}

    float x;
    float y;
};

struct Point2DI
{
    int x;
    int y;
};

constexpr inline bool operator==(const Point2DI& a, const Point2DI& b)
{
    return std::tie(a.x, b.x) == std::tie(a.y, b.y);
}

using Alliance = proto::Alliance;
using DisplayType = proto::DisplayType;
using CloakState = proto::CloakState;
using Race = proto::Race;

struct RallyTarget {
    Point3D point;  // Will always be filled.
    std::optional<int64_t> tag;   // Only if it's targeting a unit.
};

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

struct UnitOrder {
    AbilityID ability_id;
    std::variant<Point3D, uint64_t> target;
    //oneof target{
    //  Point target_world_space_pos = 2;
    //  uint64 target_unit_tag = 3;
    //}
    /*optional*/float progress;              // Progress of train abilities. Range: [0.0, 1.0]
};

struct Unit
{
    DisplayType display_type;
    Alliance alliance;

    uint64_t tag;                  // Unique identifier for a unit
    UNIT_TYPEID unit_type;
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

using Tag = decltype(Unit::tag);
using Units = std::vector<Unit>;

using UnitTypeData = proto::UnitTypeData;
using AbilityData = proto::AbilityData;

inline auto to_units(auto range)
{
    return to_vector<Unit>(range);
}

} //namespace sc2
