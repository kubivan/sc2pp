#pragma once

#include <sc2pp/Common.h>

#include <utils/UnitTraits.h>
#include <concepts>

namespace sc2
{

constexpr auto alliance(Alliance alliance)
{
    return [alliance](const auto& u) constexpr { return u.alliance == alliance; };
}

constexpr auto self = alliance(Alliance::Self);

constexpr auto ally = alliance(Alliance::Ally);

constexpr auto neutral = alliance(Alliance::Neutral);

constexpr auto enemy = alliance(Alliance::Enemy);

constexpr auto type(UNIT_TYPEID type)
{
    return [type](const auto& u) constexpr { return u.unit_type == type; };
}

constexpr auto building = [](const Unit& u) constexpr {return sc2::utils::is_building_type(u.unit_type); };

//buiild_progress Range: [0.0, 1.0]. 1.0 == finished.
constexpr auto built = [](const auto& u) constexpr { return u.build_progress == 1.f; };
constexpr auto progress_gt(float value)
{
    assert(value <= 1);
    assert(value >= 0);
    return [value](const auto& u) constexpr { return u.build_progress > value; };
}

//constexpr auto in_radius(const Point2D& center, int radius)
//{
//    return [center, radius](auto u) constexpr {
//        return DistanceSquared2D(center, u.pos) <= radius * radius;
//    };
//}

//constexpr auto target(const ObservationInterface& obs, sc2::UNIT_TYPEID target_type)
//{
//    return [&obs, target_type](const auto& u) constexpr
//    {
//        if (u.orders.empty())
//        {
//            return false;
//        }
//        auto target_unit = obs.GetUnit(u.orders.front().target_unit_tag);
//        return target_unit && target_unit->unit_type == target_type;
//    };
//}

inline bool is_geyser(const Unit& u)
{
    return u.unit_type == UNIT_TYPEID::NEUTRAL_VESPENEGEYSER
        || u.unit_type == UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER
        || u.unit_type == UNIT_TYPEID::NEUTRAL_PROTOSSVESPENEGEYSER
        || u.unit_type == UNIT_TYPEID::NEUTRAL_PURIFIERVESPENEGEYSER
        || u.unit_type == UNIT_TYPEID::NEUTRAL_SHAKURASVESPENEGEYSER;
}

inline bool harvester(const sc2::Unit& u)
{
    return !u.orders.empty() && (u.orders.front().ability_id == AbilityID::HARVEST_GATHER
        || (u.orders.front().ability_id == AbilityID::HARVEST_RETURN)
        );
}

inline bool idle(const sc2::Unit& u)
{
    return u.orders.empty();
}

template<typename Pred1, typename Pred2>
requires std::predicate<Pred1, const Unit&> &&
         std::predicate<Pred2, const Unit&>
constexpr auto operator&&(Pred1 a, Pred2 b)
{
    return [a, b](const sc2::Unit& u) constexpr {
        return a(u) && b(u);
    };
}

template<typename Pred1, typename Pred2>
requires std::predicate<Pred1, const Unit&> &&
         std::predicate<Pred2, const Unit&>
constexpr auto operator||(Pred1 a, Pred2 b)
{
    return [a, b](const Unit& u) constexpr {
        return a(u) || b(u);
    };
}

template<typename P>
requires std::predicate<P, const Unit&>
constexpr auto not_a(P p)
{
    return [p](const Unit& u) constexpr {
        return !p(u);
    };
}

const Unit* closest(const sc2::Unit* unit, const std::vector<const sc2::Unit*>& objects);
const Unit* closest(const sc2::Point2D& pos, const std::vector<const sc2::Unit*>& objects);
}
