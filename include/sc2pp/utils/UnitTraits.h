#pragma once 

#include <sc2pp/Common.h>
#include <sc2pp/SC2IDs.h>

#include <array>

namespace sc2
{
class Observation;
}

namespace sc2::utils
{

constexpr auto pylon_radius = 6.5f;

constexpr auto is_building_type(UNIT_TYPEID type) -> bool
{
    switch (type)
    {
    // Protoss
    case UNIT_TYPEID::PROTOSS_ASSIMILATOR:
    case UNIT_TYPEID::PROTOSS_ASSIMILATORRICH:
    case UNIT_TYPEID::PROTOSS_CYBERNETICSCORE:
    case UNIT_TYPEID::PROTOSS_DARKSHRINE:
    case UNIT_TYPEID::PROTOSS_FLEETBEACON:
    case UNIT_TYPEID::PROTOSS_FORGE:
    case UNIT_TYPEID::PROTOSS_GATEWAY:
    case UNIT_TYPEID::PROTOSS_NEXUS:
    case UNIT_TYPEID::PROTOSS_PHOTONCANNON:
    case UNIT_TYPEID::PROTOSS_PYLON:
    case UNIT_TYPEID::PROTOSS_PYLONOVERCHARGED:
    case UNIT_TYPEID::PROTOSS_ROBOTICSBAY:
    case UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY:
    case UNIT_TYPEID::PROTOSS_SHIELDBATTERY:
    case UNIT_TYPEID::PROTOSS_STARGATE:
    case UNIT_TYPEID::PROTOSS_TEMPLARARCHIVE:
    case UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL:
    case UNIT_TYPEID::PROTOSS_WARPGATE:
        return true;
    }
    return false;
}

auto is_training_ability(AbilityID id) -> bool;
auto is_building_ability(AbilityID id) -> bool;

struct BuildingTraits
{
    Race race;
    int mineral_cost = 0; //mineral cost of the item
    int gas_cost = 0; //gas cost of the item
    int supply_cost = 0; //supply cost of the item
    int build_time = 0; //build time of the item
    AbilityID build_ability = AbilityID::INVALID; //the ability that creates this item
    AbilityID warp_ability = AbilityID::INVALID; //the ability that creates this item via warp-in
    std::vector<UNIT_TYPEID> required_units; //owning ONE of these is required to make
    std::vector<UPGRADE_ID> required_upgrades; //having ALL of these is required to make
    int tile_width = 0;
};

using TechTree = std::unordered_map<sc2::UNIT_TYPEID, BuildingTraits>;

TechTree make_tech_tree(const Observation& obs);

/*constexpr*/ AbilityID command(UNIT_TYPEID unit);
/*constexpr*/ AbilityID command(UPGRADE_ID unit);

/*constexpr*/ UNIT_TYPEID producer(AbilityID command);
/*constexpr*/ UNIT_TYPEID producer(UNIT_TYPEID unit);
/*constexpr*/ UNIT_TYPEID producer(UPGRADE_ID unit);

//inline bool can_afford(UNIT_TYPEID item, TechTree& tree, const sc2::ObservationInterface& obs)
//{
//    const auto minerals = obs.GetMinerals();
//    const auto vespene = obs.GetVespene();
//    const auto unit_traits = tree[item];
//    return minerals >= unit_traits.mineral_cost && vespene >= unit_traits.gas_cost;
//}

}
