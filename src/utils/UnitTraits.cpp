#include "UnitTraits.h"

#include "Observation.h"

namespace sc2::utils
{

auto is_training_ability(AbilityID id) -> bool
{
    struct Lazy
    {
        Lazy(AbilityID id)
            : value(magic_enum::enum_name(id).starts_with("TRAIN_"))
        {
        }
        bool value;
    };
    static std::unordered_map<AbilityID, Lazy> res_cache;
    auto [is_training, is_new] = res_cache.try_emplace(id, id);

    return is_training->second.value;
}

auto is_building_ability(AbilityID id) -> bool
{
    struct Lazy
    {
        Lazy(AbilityID id)
            : value(magic_enum::enum_name(id).starts_with("BUILD_"))
        {
        }
        bool value;
    };
    //TODO: make constexpr
    static std::unordered_map<AbilityID, Lazy> res_cache;
    auto [is_training, is_new] = res_cache.try_emplace(id, id);

    return is_training->second.value;
}


TechTree make_tech_tree(const Observation& obs)
{
    TechTree res;
    //res[UNIT_TYPEID::PROTOSS_PYLONOVERCHARGED] = { Race::Protoss, 0, 0, 0, 0, AbilityID::EFFECT_PHOTONOVERCHARGE, 0, { UNIT_TYPEID::PROTOSS_MOTHERSHIPCORE, UNIT_TYPEID::PROTOSS_PYLON }, {}, {} };
    res[UNIT_TYPEID::PROTOSS_PYLON] = { Race::Protoss, 0, 0, 0, 0, AbilityID::BUILD_PYLON, AbilityID::INVALID, {}, {} };
    res[UNIT_TYPEID::PROTOSS_NEXUS] = { Race::Protoss, 0, 0, 0, 0, AbilityID::BUILD_NEXUS, AbilityID::INVALID, {}, {} };
    res[UNIT_TYPEID::PROTOSS_ASSIMILATOR] = { Race::Protoss, 0, 0, 0, 0, AbilityID::BUILD_ASSIMILATOR, AbilityID::INVALID, {} };
    res[UNIT_TYPEID::PROTOSS_CYBERNETICSCORE] = { Race::Protoss, 0, 0, 0, 0, AbilityID::BUILD_CYBERNETICSCORE, AbilityID::INVALID, { UNIT_TYPEID::PROTOSS_GATEWAY, UNIT_TYPEID::PROTOSS_WARPGATE }, {} };
    res[UNIT_TYPEID::PROTOSS_DARKSHRINE] = { Race::Protoss, 0, 0, 0, 0, AbilityID::BUILD_DARKSHRINE, AbilityID::INVALID, { UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL }, {} };
    res[UNIT_TYPEID::PROTOSS_FLEETBEACON] = { Race::Protoss, 0, 0, 0, 0, AbilityID::BUILD_FLEETBEACON, AbilityID::INVALID, { UNIT_TYPEID::PROTOSS_STARGATE }, {} };
    res[UNIT_TYPEID::PROTOSS_FORGE] = { Race::Protoss, 0, 0, 0, 0, AbilityID::BUILD_FORGE, AbilityID::INVALID, { UNIT_TYPEID::PROTOSS_NEXUS }, {} };
    res[UNIT_TYPEID::PROTOSS_GATEWAY] = { Race::Protoss, 0, 0, 0, 0, AbilityID::BUILD_GATEWAY, AbilityID::INVALID, { UNIT_TYPEID::PROTOSS_NEXUS }, {} };
    res[UNIT_TYPEID::PROTOSS_STARGATE] = { Race::Protoss, 0, 0, 0, 0, AbilityID::BUILD_STARGATE, AbilityID::INVALID, { UNIT_TYPEID::PROTOSS_CYBERNETICSCORE }, {} };
    res[UNIT_TYPEID::PROTOSS_PHOTONCANNON] = { Race::Protoss, 0, 0, 0, 0, AbilityID::BUILD_PHOTONCANNON, AbilityID::INVALID, { UNIT_TYPEID::PROTOSS_FORGE }, {} };
    res[UNIT_TYPEID::PROTOSS_ROBOTICSBAY] = { Race::Protoss, 0, 0, 0, 0, AbilityID::BUILD_ROBOTICSBAY, AbilityID::INVALID, { UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY }, {} };
    res[UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY] = { Race::Protoss, 0, 0, 0, 0, AbilityID::BUILD_ROBOTICSFACILITY, AbilityID::INVALID, { UNIT_TYPEID::PROTOSS_CYBERNETICSCORE }, {} };
    res[UNIT_TYPEID::PROTOSS_TEMPLARARCHIVE] = { Race::Protoss, 0, 0, 0, 0, AbilityID::BUILD_TEMPLARARCHIVE, AbilityID::INVALID, { UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL }, {} };
    res[UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL] = { Race::Protoss, 0, 0, 0, 0, AbilityID::BUILD_TWILIGHTCOUNCIL, AbilityID::INVALID, { UNIT_TYPEID::PROTOSS_CYBERNETICSCORE }, {} };
    res[UNIT_TYPEID::PROTOSS_WARPGATE] = { Race::Protoss, 0, 0, 0, 0, AbilityID::MORPH_WARPGATE, AbilityID::INVALID, { UNIT_TYPEID::PROTOSS_GATEWAY }, { UPGRADE_ID::WARPGATERESEARCH } };

    res[UNIT_TYPEID::PROTOSS_PROBE] = { Race::Protoss, 0, 0, 1, 0, AbilityID::TRAIN_PROBE, AbilityID::INVALID, { UNIT_TYPEID::PROTOSS_NEXUS }, {} };
    res[UNIT_TYPEID::PROTOSS_STALKER] = { Race::Protoss, 0, 0, 1, 0, AbilityID::TRAIN_STALKER, AbilityID::INVALID, { UNIT_TYPEID::PROTOSS_NEXUS }, {} };

    for (auto& [id, traits] : res)
    {
        auto& unit_type_data = obs.unitsTypeData();
        auto data = std::find_if(unit_type_data.begin()
            , unit_type_data.end()
            , [id](const auto& u) { return u.unit_id() == (uint32_t)id; });

        traits.mineral_cost = data->mineral_cost();
        traits.gas_cost = data->vespene_cost();
        auto abilities = obs.abilityData();
        auto ability = std::find_if(abilities.begin()
            , abilities.end()
            , [&traits](const auto& a) { return a.ability_id() == (uint32_t)traits.build_ability; });

        traits.tile_width = ability->footprint_radius() * 2;
    }

    return res;
}


AbilityID command(UNIT_TYPEID unit)
{
    switch (unit)
    {
    case UNIT_TYPEID::PROTOSS_ADEPT:
        return AbilityID::TRAIN_ADEPT;
    case UNIT_TYPEID::PROTOSS_ARCHON:
        return AbilityID::MORPH_ARCHON;
    case UNIT_TYPEID::PROTOSS_ASSIMILATORRICH:
    case UNIT_TYPEID::PROTOSS_ASSIMILATOR:
        return AbilityID::BUILD_ASSIMILATOR;
    case UNIT_TYPEID::PROTOSS_CARRIER:
        return AbilityID::TRAIN_CARRIER;
    case UNIT_TYPEID::PROTOSS_COLOSSUS:
        return AbilityID::TRAIN_COLOSSUS;
    case UNIT_TYPEID::PROTOSS_CYBERNETICSCORE:
        return AbilityID::BUILD_CYBERNETICSCORE;
    case UNIT_TYPEID::PROTOSS_DARKSHRINE:
        return AbilityID::BUILD_DARKSHRINE;
    case UNIT_TYPEID::PROTOSS_DARKTEMPLAR:
        return AbilityID::TRAIN_DARKTEMPLAR;
    case UNIT_TYPEID::PROTOSS_DISRUPTOR:
        return AbilityID::TRAIN_DISRUPTOR;
    //case UNIT_TYPEID::PROTOSS_DISRUPTORPHASED:
    //    return AbilityID::TRAIN_;
    case UNIT_TYPEID::PROTOSS_FLEETBEACON:
        return AbilityID::BUILD_FLEETBEACON;
    case UNIT_TYPEID::PROTOSS_FORGE:
        return AbilityID::BUILD_FORGE;
    case UNIT_TYPEID::PROTOSS_GATEWAY:
        return AbilityID::BUILD_GATEWAY;
    case UNIT_TYPEID::PROTOSS_HIGHTEMPLAR:
        return AbilityID::TRAIN_HIGHTEMPLAR;
    case UNIT_TYPEID::PROTOSS_IMMORTAL:
        return AbilityID::TRAIN_IMMORTAL;
    //case UNIT_TYPEID::PROTOSS_INTERCEPTOR:
    //    return AbilityID::TRAIN_INTE;
    case UNIT_TYPEID::PROTOSS_MOTHERSHIP:
        return AbilityID::TRAIN_MOTHERSHIP;
    case UNIT_TYPEID::PROTOSS_MOTHERSHIPCORE:
        return AbilityID::TRAIN_MOTHERSHIPCORE;
    case UNIT_TYPEID::PROTOSS_NEXUS:
        return AbilityID::BUILD_NEXUS;
    case UNIT_TYPEID::PROTOSS_OBSERVER:
        return AbilityID::TRAIN_OBSERVER;
    case UNIT_TYPEID::PROTOSS_ORACLE:
        return AbilityID::TRAIN_ORACLE;
    //case UNIT_TYPEID::PROTOSS_ORACLESTASISTRAP:
    //    return AbilityID::;
    case UNIT_TYPEID::PROTOSS_PHOENIX:
        return AbilityID::TRAIN_PHOENIX;
    case UNIT_TYPEID::PROTOSS_PHOTONCANNON:
        return AbilityID::BUILD_PHOTONCANNON;
    case UNIT_TYPEID::PROTOSS_PROBE:
        return AbilityID::TRAIN_PROBE;
    case UNIT_TYPEID::PROTOSS_PYLON:
    case UNIT_TYPEID::PROTOSS_PYLONOVERCHARGED: //TODO: check
        return AbilityID::BUILD_PYLON;
    case UNIT_TYPEID::PROTOSS_ROBOTICSBAY:
        return AbilityID::BUILD_ROBOTICSBAY;
    case UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY:
        return AbilityID::BUILD_ROBOTICSFACILITY;
    case UNIT_TYPEID::PROTOSS_SENTRY:
        return AbilityID::TRAIN_SENTRY;
    case UNIT_TYPEID::PROTOSS_SHIELDBATTERY:
        return AbilityID::BUILD_SHIELDBATTERY;
    case UNIT_TYPEID::PROTOSS_STALKER:
        return AbilityID::TRAINWARP_STALKER;
    case UNIT_TYPEID::PROTOSS_STARGATE:
        return AbilityID::BUILD_STARGATE;
    case UNIT_TYPEID::PROTOSS_TEMPEST:
        return AbilityID::TRAIN_TEMPEST;
    case UNIT_TYPEID::PROTOSS_TEMPLARARCHIVE:
        return AbilityID::BUILD_TEMPLARARCHIVE;
    case UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL:
        return AbilityID::BUILD_TWILIGHTCOUNCIL;
    case UNIT_TYPEID::PROTOSS_VOIDRAY:
        return AbilityID::TRAIN_VOIDRAY;
    case UNIT_TYPEID::PROTOSS_WARPGATE:
        return AbilityID::MORPH_WARPGATE;
    }

    throw std::runtime_error("TechTree::command undefined for UNIT_ID: " + std::to_string((int)unit));
}

AbilityID command(UPGRADE_ID unit)
{
    switch (unit)
    {
    case UPGRADE_ID::INVALID:
        return AbilityID::INVALID;
    case UPGRADE_ID::CARRIERLAUNCHSPEEDUPGRADE:
        break;
    case UPGRADE_ID::GLIALRECONSTITUTION:
        break;
    case UPGRADE_ID::TUNNELINGCLAWS:
        break;
    case UPGRADE_ID::CHITINOUSPLATING:
        break;
    case UPGRADE_ID::HISECAUTOTRACKING:
        break;
    case UPGRADE_ID::TERRANBUILDINGARMOR:
        break;
    case UPGRADE_ID::TERRANINFANTRYWEAPONSLEVEL1:
        break;
    case UPGRADE_ID::TERRANINFANTRYWEAPONSLEVEL2:
        break;
    case UPGRADE_ID::TERRANINFANTRYWEAPONSLEVEL3:
        break;
    case UPGRADE_ID::NEOSTEELFRAME:
        break;
    case UPGRADE_ID::TERRANINFANTRYARMORSLEVEL1:
        break;
    case UPGRADE_ID::TERRANINFANTRYARMORSLEVEL2:
        break;
    case UPGRADE_ID::TERRANINFANTRYARMORSLEVEL3:
        break;
    case UPGRADE_ID::STIMPACK:
        break;
    case UPGRADE_ID::SHIELDWALL:
        break;
    case UPGRADE_ID::PUNISHERGRENADES:
        break;
    case UPGRADE_ID::HIGHCAPACITYBARRELS:
        break;
    case UPGRADE_ID::BANSHEECLOAK:
        break;
    case UPGRADE_ID::RAVENCORVIDREACTOR:
        break;
    case UPGRADE_ID::PERSONALCLOAKING:
        break;
    case UPGRADE_ID::TERRANVEHICLEWEAPONSLEVEL1:
        break;
    case UPGRADE_ID::TERRANVEHICLEWEAPONSLEVEL2:
        break;
    case UPGRADE_ID::TERRANVEHICLEWEAPONSLEVEL3:
        break;
    case UPGRADE_ID::TERRANSHIPWEAPONSLEVEL1:
        break;
    case UPGRADE_ID::TERRANSHIPWEAPONSLEVEL2:
        break;
    case UPGRADE_ID::TERRANSHIPWEAPONSLEVEL3:
        break;
    case UPGRADE_ID::PROTOSSGROUNDWEAPONSLEVEL1:
        break;
    case UPGRADE_ID::PROTOSSGROUNDWEAPONSLEVEL2:
        break;
    case UPGRADE_ID::PROTOSSGROUNDWEAPONSLEVEL3:
        break;
    case UPGRADE_ID::PROTOSSGROUNDARMORSLEVEL1:
        break;
    case UPGRADE_ID::PROTOSSGROUNDARMORSLEVEL2:
        break;
    case UPGRADE_ID::PROTOSSGROUNDARMORSLEVEL3:
        break;
    case UPGRADE_ID::PROTOSSSHIELDSLEVEL1:
        break;
    case UPGRADE_ID::PROTOSSSHIELDSLEVEL2:
        break;
    case UPGRADE_ID::PROTOSSSHIELDSLEVEL3:
        break;
    case UPGRADE_ID::OBSERVERGRAVITICBOOSTER:
        break;
    case UPGRADE_ID::GRAVITICDRIVE:
        break;
    case UPGRADE_ID::EXTENDEDTHERMALLANCE:
        break;
    case UPGRADE_ID::PSISTORMTECH:
        break;
    case UPGRADE_ID::ZERGMELEEWEAPONSLEVEL1:
        break;
    case UPGRADE_ID::ZERGMELEEWEAPONSLEVEL2:
        break;
    case UPGRADE_ID::ZERGMELEEWEAPONSLEVEL3:
        break;
    case UPGRADE_ID::ZERGGROUNDARMORSLEVEL1:
        break;
    case UPGRADE_ID::ZERGGROUNDARMORSLEVEL2:
        break;
    case UPGRADE_ID::ZERGGROUNDARMORSLEVEL3:
        break;
    case UPGRADE_ID::ZERGMISSILEWEAPONSLEVEL1:
        break;
    case UPGRADE_ID::ZERGMISSILEWEAPONSLEVEL2:
        break;
    case UPGRADE_ID::ZERGMISSILEWEAPONSLEVEL3:
        break;
    case UPGRADE_ID::OVERLORDSPEED:
        break;
    case UPGRADE_ID::BURROW:
        break;
    case UPGRADE_ID::ZERGLINGATTACKSPEED:
        break;
    case UPGRADE_ID::ZERGLINGMOVEMENTSPEED:
        break;
    case UPGRADE_ID::ZERGFLYERWEAPONSLEVEL1:
        break;
    case UPGRADE_ID::ZERGFLYERWEAPONSLEVEL2:
        break;
    case UPGRADE_ID::ZERGFLYERWEAPONSLEVEL3:
        break;
    case UPGRADE_ID::ZERGFLYERARMORSLEVEL1:
        break;
    case UPGRADE_ID::ZERGFLYERARMORSLEVEL2:
        break;
    case UPGRADE_ID::ZERGFLYERARMORSLEVEL3:
        break;
    case UPGRADE_ID::INFESTORENERGYUPGRADE:
        break;
    case UPGRADE_ID::CENTRIFICALHOOKS:
        break;
    case UPGRADE_ID::BATTLECRUISERENABLESPECIALIZATIONS:
        break;
    case UPGRADE_ID::PROTOSSAIRWEAPONSLEVEL1:
        break;
    case UPGRADE_ID::PROTOSSAIRWEAPONSLEVEL2:
        break;
    case UPGRADE_ID::PROTOSSAIRWEAPONSLEVEL3:
        break;
    case UPGRADE_ID::PROTOSSAIRARMORSLEVEL1:
        break;
    case UPGRADE_ID::PROTOSSAIRARMORSLEVEL2:
        break;
    case UPGRADE_ID::PROTOSSAIRARMORSLEVEL3:
        break;
    case UPGRADE_ID::WARPGATERESEARCH:
        return AbilityID::RESEARCH_WARPGATE;
    case UPGRADE_ID::CHARGE:
        break;
    case UPGRADE_ID::BLINKTECH:
        break;
    case UPGRADE_ID::PHOENIXRANGEUPGRADE:
        break;
    case UPGRADE_ID::NEURALPARASITE:
        break;
    case UPGRADE_ID::TERRANVEHICLEANDSHIPARMORSLEVEL1:
        break;
    case UPGRADE_ID::TERRANVEHICLEANDSHIPARMORSLEVEL2:
        break;
    case UPGRADE_ID::TERRANVEHICLEANDSHIPARMORSLEVEL3:
        break;
    case UPGRADE_ID::DRILLCLAWS:
        break;
    case UPGRADE_ID::ADEPTPIERCINGATTACK:
        break;
    case UPGRADE_ID::MAGFIELDLAUNCHERS:
        break;
    case UPGRADE_ID::EVOLVEGROOVEDSPINES:
        break;
    case UPGRADE_ID::EVOLVEMUSCULARAUGMENTS:
        break;
    case UPGRADE_ID::BANSHEESPEED:
        break;
    case UPGRADE_ID::RAVENRECALIBRATEDEXPLOSIVES:
        break;
    case UPGRADE_ID::MEDIVACINCREASESPEEDBOOST:
        break;
    case UPGRADE_ID::LIBERATORAGRANGEUPGRADE:
        break;
    case UPGRADE_ID::DARKTEMPLARBLINKUPGRADE:
        break;
    case UPGRADE_ID::CYCLONELOCKONDAMAGEUPGRADE:
        break;
    case UPGRADE_ID::SMARTSERVOS:
        break;
    case UPGRADE_ID::RAPIDFIRELAUNCHERS:
        break;
    case UPGRADE_ID::ENHANCEDMUNITIONS:
        break;
    default:
        break;

    }
    throw std::runtime_error("command undefined for UNIT_ID: " + std::to_string((int)unit));
}

UNIT_TYPEID producer(AbilityID command)
{
    switch (command)
    {
    case AbilityID::INVALID:
        break;
    case AbilityID::SMART:
        break;
    case AbilityID::ATTACK:
        break;
    case AbilityID::ATTACK_ATTACK:
        break;
    case AbilityID::ATTACK_ATTACKBUILDING:
        break;
    case AbilityID::ATTACK_REDIRECT:
        break;
    case AbilityID::BEHAVIOR_BUILDINGATTACKOFF:
        break;
    case AbilityID::BEHAVIOR_BUILDINGATTACKON:
        break;
    case AbilityID::BEHAVIOR_CLOAKOFF:
        break;
    case AbilityID::BEHAVIOR_CLOAKOFF_BANSHEE:
        break;
    case AbilityID::BEHAVIOR_CLOAKOFF_GHOST:
        break;
    case AbilityID::BEHAVIOR_CLOAKON:
        break;
    case AbilityID::BEHAVIOR_CLOAKON_BANSHEE:
        break;
    case AbilityID::BEHAVIOR_CLOAKON_GHOST:
        break;
    case AbilityID::BEHAVIOR_GENERATECREEPOFF:
        break;
    case AbilityID::BEHAVIOR_GENERATECREEPON:
        break;
    case AbilityID::BEHAVIOR_HOLDFIREOFF:
        break;
    case AbilityID::BEHAVIOR_HOLDFIREOFF_LURKER:
        break;
    case AbilityID::BEHAVIOR_HOLDFIREON:
        break;
    case AbilityID::BEHAVIOR_HOLDFIREON_GHOST:
        break;
    case AbilityID::BEHAVIOR_HOLDFIREON_LURKER:
        break;
    case AbilityID::BEHAVIOR_PULSARBEAMOFF:
        break;
    case AbilityID::BEHAVIOR_PULSARBEAMON:
        break;
    case AbilityID::BUILD_ARMORY:
        break;
    case AbilityID::BUILD_ASSIMILATOR:
        break;
    case AbilityID::BUILD_BANELINGNEST:
        break;
    case AbilityID::BUILD_BARRACKS:
        break;
    case AbilityID::BUILD_BUNKER:
        break;
    case AbilityID::BUILD_COMMANDCENTER:
        break;
    case AbilityID::BUILD_CREEPTUMOR:
        break;
    case AbilityID::BUILD_CREEPTUMOR_QUEEN:
        break;
    case AbilityID::BUILD_CREEPTUMOR_TUMOR:
        break;
    case AbilityID::BUILD_CYBERNETICSCORE:
        break;
    case AbilityID::BUILD_DARKSHRINE:
        break;
    case AbilityID::BUILD_ENGINEERINGBAY:
        break;
    case AbilityID::BUILD_EVOLUTIONCHAMBER:
        break;
    case AbilityID::BUILD_EXTRACTOR:
        break;
    case AbilityID::BUILD_FACTORY:
        break;
    case AbilityID::BUILD_FLEETBEACON:
        break;
    case AbilityID::BUILD_FORGE:
        break;
    case AbilityID::BUILD_FUSIONCORE:
        break;
    case AbilityID::BUILD_GATEWAY:
        break;
    case AbilityID::BUILD_GHOSTACADEMY:
        break;
    case AbilityID::BUILD_HATCHERY:
        break;
    case AbilityID::BUILD_HYDRALISKDEN:
        break;
    case AbilityID::BUILD_INFESTATIONPIT:
        break;
    case AbilityID::BUILD_INTERCEPTORS:
        break;
    case AbilityID::BUILD_MISSILETURRET:
        break;
    case AbilityID::BUILD_NEXUS:
        break;
    case AbilityID::BUILD_NUKE:
        break;
    case AbilityID::BUILD_NYDUSNETWORK:
        break;
    case AbilityID::BUILD_NYDUSWORM:
        break;
    case AbilityID::BUILD_PHOTONCANNON:
        break;
    case AbilityID::BUILD_PYLON:
        break;
    case AbilityID::BUILD_REACTOR:
        break;
    case AbilityID::BUILD_REACTOR_BARRACKS:
        break;
    case AbilityID::BUILD_REACTOR_FACTORY:
        break;
    case AbilityID::BUILD_REACTOR_STARPORT:
        break;
    case AbilityID::BUILD_REFINERY:
        break;
    case AbilityID::BUILD_ROACHWARREN:
        break;
    case AbilityID::BUILD_ROBOTICSBAY:
        break;
    case AbilityID::BUILD_ROBOTICSFACILITY:
        break;
    case AbilityID::BUILD_SENSORTOWER:
        break;
    case AbilityID::BUILD_SHIELDBATTERY:
        break;
    case AbilityID::BUILD_SPAWNINGPOOL:
        break;
    case AbilityID::BUILD_SPINECRAWLER:
        break;
    case AbilityID::BUILD_SPIRE:
        break;
    case AbilityID::BUILD_SPORECRAWLER:
        break;
    case AbilityID::BUILD_STARGATE:
        break;
    case AbilityID::BUILD_STARPORT:
        break;
    case AbilityID::BUILD_STASISTRAP:
        break;
    case AbilityID::BUILD_SUPPLYDEPOT:
        break;
    case AbilityID::BUILD_TECHLAB:
        break;
    case AbilityID::BUILD_TECHLAB_BARRACKS:
        break;
    case AbilityID::BUILD_TECHLAB_FACTORY:
        break;
    case AbilityID::BUILD_TECHLAB_STARPORT:
        break;
    case AbilityID::BUILD_TEMPLARARCHIVE:
        break;
    case AbilityID::BUILD_TWILIGHTCOUNCIL:
        break;
    case AbilityID::BUILD_ULTRALISKCAVERN:
        break;
    case AbilityID::BURROWDOWN:
        break;
    case AbilityID::BURROWDOWN_BANELING:
        break;
    case AbilityID::BURROWDOWN_DRONE:
        break;
    case AbilityID::BURROWDOWN_HYDRALISK:
        break;
    case AbilityID::BURROWDOWN_INFESTOR:
        break;
    case AbilityID::BURROWDOWN_LURKER:
        break;
    case AbilityID::BURROWDOWN_QUEEN:
        break;
    case AbilityID::BURROWDOWN_RAVAGER:
        break;
    case AbilityID::BURROWDOWN_ROACH:
        break;
    case AbilityID::BURROWDOWN_SWARMHOST:
        break;
    case AbilityID::BURROWDOWN_WIDOWMINE:
        break;
    case AbilityID::BURROWDOWN_ZERGLING:
        break;
    case AbilityID::BURROWUP:
        break;
    case AbilityID::BURROWUP_BANELING:
        break;
    case AbilityID::BURROWUP_DRONE:
        break;
    case AbilityID::BURROWUP_HYDRALISK:
        break;
    case AbilityID::BURROWUP_INFESTOR:
        break;
    case AbilityID::BURROWUP_LURKER:
        break;
    case AbilityID::BURROWUP_QUEEN:
        break;
    case AbilityID::BURROWUP_RAVAGER:
        break;
    case AbilityID::BURROWUP_ROACH:
        break;
    case AbilityID::BURROWUP_SWARMHOST:
        break;
    case AbilityID::BURROWUP_WIDOWMINE:
        break;
    case AbilityID::BURROWUP_ZERGLING:
        break;
    case AbilityID::CANCEL:
        break;
    case AbilityID::CANCELSLOT_ADDON:
        break;
    case AbilityID::CANCELSLOT_QUEUE1:
        break;
    case AbilityID::CANCELSLOT_QUEUE5:
        break;
    case AbilityID::CANCELSLOT_QUEUECANCELTOSELECTION:
        break;
    case AbilityID::CANCELSLOT_QUEUEPASSIVE:
        break;
    case AbilityID::CANCEL_ADEPTPHASESHIFT:
        break;
    case AbilityID::CANCEL_ADEPTSHADEPHASESHIFT:
        break;
    case AbilityID::CANCEL_BARRACKSADDON:
        break;
    case AbilityID::CANCEL_BUILDINPROGRESS:
        break;
    case AbilityID::CANCEL_CREEPTUMOR:
        break;
    case AbilityID::CANCEL_FACTORYADDON:
        break;
    case AbilityID::CANCEL_GRAVITONBEAM:
        break;
    case AbilityID::CANCEL_LAST:
        break;
    case AbilityID::CANCEL_MORPHBROODLORD:
        break;
    case AbilityID::CANCEL_MORPHLAIR:
        break;
    case AbilityID::CANCEL_MORPHLURKER:
        break;
    case AbilityID::CANCEL_MORPHLURKERDEN:
        break;
    case AbilityID::CANCEL_MORPHMOTHERSHIP:
        break;
    case AbilityID::CANCEL_MORPHORBITAL:
        break;
    case AbilityID::CANCEL_MORPHOVERLORDTRANSPORT:
        break;
    case AbilityID::CANCEL_MORPHOVERSEER:
        break;
    case AbilityID::CANCEL_MORPHPLANETARYFORTRESS:
        break;
    case AbilityID::CANCEL_MORPHRAVAGER:
        break;
    case AbilityID::CANCEL_QUEUE1:
        break;
    case AbilityID::CANCEL_QUEUE5:
        break;
    case AbilityID::CANCEL_QUEUEADDON:
        break;
    case AbilityID::CANCEL_QUEUECANCELTOSELECTION:
        break;
    case AbilityID::CANCEL_QUEUEPASIVE:
        break;
    case AbilityID::CANCEL_QUEUEPASSIVECANCELTOSELECTION:
        break;
    case AbilityID::CANCEL_SPINECRAWLERROOT:
        break;
    case AbilityID::CANCEL_STARPORTADDON:
        break;
    case AbilityID::EFFECT_ABDUCT:
        break;
    case AbilityID::EFFECT_ADEPTPHASESHIFT:
        break;
    case AbilityID::EFFECT_AUTOTURRET:
        break;
    case AbilityID::EFFECT_BLINDINGCLOUD:
        break;
    case AbilityID::EFFECT_BLINK:
        break;
    case AbilityID::EFFECT_BLINK_STALKER:
        break;
    case AbilityID::EFFECT_CALLDOWNMULE:
        break;
    case AbilityID::EFFECT_CAUSTICSPRAY:
        break;
    case AbilityID::EFFECT_CHARGE:
        break;
    case AbilityID::EFFECT_CHRONOBOOST:
        break;
    case AbilityID::EFFECT_CONTAMINATE:
        break;
    case AbilityID::EFFECT_CORROSIVEBILE:
        break;
    case AbilityID::EFFECT_EMP:
        break;
    case AbilityID::EFFECT_EXPLODE:
        break;
    case AbilityID::EFFECT_FEEDBACK:
        break;
    case AbilityID::EFFECT_FORCEFIELD:
        break;
    case AbilityID::EFFECT_FUNGALGROWTH:
        break;
    case AbilityID::EFFECT_GHOSTSNIPE:
        break;
    case AbilityID::EFFECT_GRAVITONBEAM:
        break;
    case AbilityID::EFFECT_GUARDIANSHIELD:
        break;
    case AbilityID::EFFECT_HEAL:
        break;
    case AbilityID::EFFECT_HUNTERSEEKERMISSILE:
        break;
    case AbilityID::EFFECT_IMMORTALBARRIER:
        break;
    case AbilityID::EFFECT_INFESTEDTERRANS:
        break;
    case AbilityID::EFFECT_INJECTLARVA:
        break;
    case AbilityID::EFFECT_KD8CHARGE:
        break;
    case AbilityID::EFFECT_LOCKON:
        break;
    case AbilityID::EFFECT_LOCUSTSWOOP:
        break;
    case AbilityID::EFFECT_MASSRECALL:
        break;
    case AbilityID::EFFECT_MASSRECALL_MOTHERSHIP:
        break;
    case AbilityID::EFFECT_MASSRECALL_MOTHERSHIPCORE:
        break;
    case AbilityID::EFFECT_MEDIVACIGNITEAFTERBURNERS:
        break;
    case AbilityID::EFFECT_NEURALPARASITE:
        break;
    case AbilityID::EFFECT_NUKECALLDOWN:
        break;
    case AbilityID::EFFECT_ORACLEREVELATION:
        break;
    case AbilityID::EFFECT_PARASITICBOMB:
        break;
    case AbilityID::EFFECT_PHOTONOVERCHARGE:
        break;
    case AbilityID::EFFECT_POINTDEFENSEDRONE:
        break;
    case AbilityID::EFFECT_PSISTORM:
        break;
    case AbilityID::EFFECT_PURIFICATIONNOVA:
        break;
    case AbilityID::EFFECT_REPAIR:
        break;
    case AbilityID::EFFECT_REPAIR_MULE:
        break;
    case AbilityID::EFFECT_REPAIR_SCV:
        break;
    case AbilityID::EFFECT_RESTORE:
        break;
    case AbilityID::EFFECT_SALVAGE:
        break;
    case AbilityID::EFFECT_SCAN:
        break;
    case AbilityID::EFFECT_SHADOWSTRIDE:
        break;
    case AbilityID::EFFECT_SPAWNCHANGELING:
        break;
    case AbilityID::EFFECT_SPAWNLOCUSTS:
        break;
    case AbilityID::EFFECT_SPRAY:
        break;
    case AbilityID::EFFECT_SPRAY_PROTOSS:
        break;
    case AbilityID::EFFECT_SPRAY_TERRAN:
        break;
    case AbilityID::EFFECT_SPRAY_ZERG:
        break;
    case AbilityID::EFFECT_STIM:
        break;
    case AbilityID::EFFECT_STIM_MARAUDER:
        break;
    case AbilityID::EFFECT_STIM_MARINE:
        break;
    case AbilityID::EFFECT_STIM_MARINE_REDIRECT:
        break;
    case AbilityID::EFFECT_SUPPLYDROP:
        break;
    case AbilityID::EFFECT_TACTICALJUMP:
        break;
    case AbilityID::EFFECT_TEMPESTDISRUPTIONBLAST:
        break;
    case AbilityID::EFFECT_TIMEWARP:
        break;
    case AbilityID::EFFECT_TRANSFUSION:
        break;
    case AbilityID::EFFECT_VIPERCONSUME:
        break;
    case AbilityID::EFFECT_VOIDRAYPRISMATICALIGNMENT:
        break;
    case AbilityID::EFFECT_WIDOWMINEATTACK:
        break;
    case AbilityID::EFFECT_YAMATOGUN:
        break;
    case AbilityID::HALLUCINATION_ADEPT:
        break;
    case AbilityID::HALLUCINATION_ARCHON:
        break;
    case AbilityID::HALLUCINATION_COLOSSUS:
        break;
    case AbilityID::HALLUCINATION_DISRUPTOR:
        break;
    case AbilityID::HALLUCINATION_HIGHTEMPLAR:
        break;
    case AbilityID::HALLUCINATION_IMMORTAL:
        break;
    case AbilityID::HALLUCINATION_ORACLE:
        break;
    case AbilityID::HALLUCINATION_PHOENIX:
        break;
    case AbilityID::HALLUCINATION_PROBE:
        break;
    case AbilityID::HALLUCINATION_STALKER:
        break;
    case AbilityID::HALLUCINATION_VOIDRAY:
        break;
    case AbilityID::HALLUCINATION_WARPPRISM:
        break;
    case AbilityID::HALLUCINATION_ZEALOT:
        break;
    case AbilityID::HALT:
        break;
    case AbilityID::HALT_BUILDING:
        break;
    case AbilityID::HALT_TERRANBUILD:
        break;
    case AbilityID::HARVEST_GATHER:
        break;
    case AbilityID::HARVEST_GATHER_DRONE:
        break;
    case AbilityID::HARVEST_GATHER_PROBE:
        break;
    case AbilityID::HARVEST_GATHER_SCV:
        break;
    case AbilityID::HARVEST_RETURN:
        break;
    case AbilityID::HARVEST_RETURN_DRONE:
        break;
    case AbilityID::HARVEST_RETURN_MULE:
        break;
    case AbilityID::HARVEST_RETURN_PROBE:
        break;
    case AbilityID::HARVEST_RETURN_SCV:
        break;
    case AbilityID::HOLDPOSITION:
        break;
    case AbilityID::LAND:
        break;
    case AbilityID::LAND_BARRACKS:
        break;
    case AbilityID::LAND_COMMANDCENTER:
        break;
    case AbilityID::LAND_FACTORY:
        break;
    case AbilityID::LAND_ORBITALCOMMAND:
        break;
    case AbilityID::LAND_STARPORT:
        break;
    case AbilityID::LIFT:
        break;
    case AbilityID::LIFT_BARRACKS:
        break;
    case AbilityID::LIFT_COMMANDCENTER:
        break;
    case AbilityID::LIFT_FACTORY:
        break;
    case AbilityID::LIFT_ORBITALCOMMAND:
        break;
    case AbilityID::LIFT_STARPORT:
        break;
    case AbilityID::LOAD:
        break;
    case AbilityID::LOADALL:
        break;
    case AbilityID::LOADALL_COMMANDCENTER:
        break;
    case AbilityID::LOAD_BUNKER:
        break;
    case AbilityID::LOAD_MEDIVAC:
        break;
    case AbilityID::MORPH_ARCHON:
        break;
    case AbilityID::MORPH_BROODLORD:
        break;
    case AbilityID::MORPH_GATEWAY:
        break;
    case AbilityID::MORPH_GREATERSPIRE:
        break;
    case AbilityID::MORPH_HELLBAT:
        break;
    case AbilityID::MORPH_HELLION:
        break;
    case AbilityID::MORPH_HIVE:
        break;
    case AbilityID::MORPH_LAIR:
        break;
    case AbilityID::MORPH_LIBERATORAAMODE:
        break;
    case AbilityID::MORPH_LIBERATORAGMODE:
        break;
    case AbilityID::MORPH_LURKER:
        break;
    case AbilityID::MORPH_LURKERDEN:
        break;
    case AbilityID::MORPH_MOTHERSHIP:
        break;
    case AbilityID::MORPH_ORBITALCOMMAND:
        break;
    case AbilityID::MORPH_OVERLORDTRANSPORT:
        break;
    case AbilityID::MORPH_OVERSEER:
        break;
    case AbilityID::MORPH_PLANETARYFORTRESS:
        break;
    case AbilityID::MORPH_RAVAGER:
        break;
    case AbilityID::MORPH_ROOT:
        break;
    case AbilityID::MORPH_SIEGEMODE:
        break;
    case AbilityID::MORPH_SPINECRAWLERROOT:
        break;
    case AbilityID::MORPH_SPINECRAWLERUPROOT:
        break;
    case AbilityID::MORPH_SPORECRAWLERROOT:
        break;
    case AbilityID::MORPH_SPORECRAWLERUPROOT:
        break;
    case AbilityID::MORPH_SUPPLYDEPOT_LOWER:
        break;
    case AbilityID::MORPH_SUPPLYDEPOT_RAISE:
        break;
    case AbilityID::MORPH_THOREXPLOSIVEMODE:
        break;
    case AbilityID::MORPH_THORHIGHIMPACTMODE:
        break;
    case AbilityID::MORPH_UNSIEGE:
        break;
    case AbilityID::MORPH_UPROOT:
        break;
    case AbilityID::MORPH_VIKINGASSAULTMODE:
        break;
    case AbilityID::MORPH_VIKINGFIGHTERMODE:
        break;
    case AbilityID::MORPH_WARPGATE:
        break;
    case AbilityID::MORPH_WARPPRISMPHASINGMODE:
        break;
    case AbilityID::MORPH_WARPPRISMTRANSPORTMODE:
        break;
    case AbilityID::MOVE:
        break;
    case AbilityID::PATROL:
        break;
    case AbilityID::RALLY_BUILDING:
        break;
    case AbilityID::RALLY_COMMANDCENTER:
        break;
    case AbilityID::RALLY_HATCHERY_UNITS:
        break;
    case AbilityID::RALLY_HATCHERY_WORKERS:
        break;
    case AbilityID::RALLY_MORPHING_UNIT:
        break;
    case AbilityID::RALLY_NEXUS:
        break;
    case AbilityID::RALLY_UNITS:
        break;
    case AbilityID::RALLY_WORKERS:
        break;
    case AbilityID::RESEARCH_ADEPTRESONATINGGLAIVES:
        break;
    case AbilityID::RESEARCH_ADVANCEDBALLISTICS:
        break;
    case AbilityID::RESEARCH_BANSHEECLOAKINGFIELD:
        break;
    case AbilityID::RESEARCH_BANSHEEHYPERFLIGHTROTORS:
        break;
    case AbilityID::RESEARCH_BATTLECRUISERWEAPONREFIT:
        break;
    case AbilityID::RESEARCH_BLINK:
        break;
    case AbilityID::RESEARCH_BURROW:
        break;
    case AbilityID::RESEARCH_CENTRIFUGALHOOKS:
        break;
    case AbilityID::RESEARCH_CHARGE:
        break;
    case AbilityID::RESEARCH_CHITINOUSPLATING:
        break;
    case AbilityID::RESEARCH_COMBATSHIELD:
        break;
    case AbilityID::RESEARCH_CONCUSSIVESHELLS:
        break;
    case AbilityID::RESEARCH_CYCLONELOCKONDAMAGE:
        break;
    case AbilityID::RESEARCH_DRILLINGCLAWS:
        break;
    case AbilityID::RESEARCH_ENHANCEDMUNITIONS:
        break;
    case AbilityID::RESEARCH_EXTENDEDTHERMALLANCE:
        break;
    case AbilityID::RESEARCH_GLIALREGENERATION:
        break;
    case AbilityID::RESEARCH_GRAVITICBOOSTER:
        break;
    case AbilityID::RESEARCH_GRAVITICDRIVE:
        break;
    case AbilityID::RESEARCH_GROOVEDSPINES:
        break;
    case AbilityID::RESEARCH_HIGHCAPACITYFUELTANKS:
        break;
    case AbilityID::RESEARCH_HISECAUTOTRACKING:
        break;
    case AbilityID::RESEARCH_INFERNALPREIGNITER:
        break;
    case AbilityID::RESEARCH_INTERCEPTORGRAVITONCATAPULT:
        break;
    case AbilityID::RESEARCH_MAGFIELDLAUNCHERS:
        break;
    case AbilityID::RESEARCH_MUSCULARAUGMENTS:
        break;
    case AbilityID::RESEARCH_NEOSTEELFRAME:
        break;
    case AbilityID::RESEARCH_NEURALPARASITE:
        break;
    case AbilityID::RESEARCH_PATHOGENGLANDS:
        break;
    case AbilityID::RESEARCH_PERSONALCLOAKING:
        break;
    case AbilityID::RESEARCH_PHOENIXANIONPULSECRYSTALS:
        break;
    case AbilityID::RESEARCH_PNEUMATIZEDCARAPACE:
        break;
    case AbilityID::RESEARCH_PROTOSSAIRARMOR:
        break;
    case AbilityID::RESEARCH_PROTOSSAIRARMORLEVEL1:
        break;
    case AbilityID::RESEARCH_PROTOSSAIRARMORLEVEL2:
        break;
    case AbilityID::RESEARCH_PROTOSSAIRARMORLEVEL3:
        break;
    case AbilityID::RESEARCH_PROTOSSAIRWEAPONS:
        break;
    case AbilityID::RESEARCH_PROTOSSAIRWEAPONSLEVEL1:
        break;
    case AbilityID::RESEARCH_PROTOSSAIRWEAPONSLEVEL2:
        break;
    case AbilityID::RESEARCH_PROTOSSAIRWEAPONSLEVEL3:
        break;
    case AbilityID::RESEARCH_PROTOSSGROUNDARMOR:
        break;
    case AbilityID::RESEARCH_PROTOSSGROUNDARMORLEVEL1:
        break;
    case AbilityID::RESEARCH_PROTOSSGROUNDARMORLEVEL2:
        break;
    case AbilityID::RESEARCH_PROTOSSGROUNDARMORLEVEL3:
        break;
    case AbilityID::RESEARCH_PROTOSSGROUNDWEAPONS:
        break;
    case AbilityID::RESEARCH_PROTOSSGROUNDWEAPONSLEVEL1:
        break;
    case AbilityID::RESEARCH_PROTOSSGROUNDWEAPONSLEVEL2:
        break;
    case AbilityID::RESEARCH_PROTOSSGROUNDWEAPONSLEVEL3:
        break;
    case AbilityID::RESEARCH_PROTOSSSHIELDS:
        break;
    case AbilityID::RESEARCH_PROTOSSSHIELDSLEVEL1:
        break;
    case AbilityID::RESEARCH_PROTOSSSHIELDSLEVEL2:
        break;
    case AbilityID::RESEARCH_PROTOSSSHIELDSLEVEL3:
        break;
    case AbilityID::RESEARCH_PSISTORM:
        break;
    case AbilityID::RESEARCH_RAPIDFIRELAUNCHERS:
        break;
    case AbilityID::RESEARCH_RAVENCORVIDREACTOR:
        break;
    case AbilityID::RESEARCH_RAVENRECALIBRATEDEXPLOSIVES:
        break;
    case AbilityID::RESEARCH_SHADOWSTRIKE:
        break;
    //case AbilityID::RESEARCH_SMARTSERVOS:
    //    break;
    case AbilityID::RESEARCH_STIMPACK:
        break;
    case AbilityID::RESEARCH_TERRANINFANTRYARMOR:
        break;
    case AbilityID::RESEARCH_TERRANINFANTRYARMORLEVEL1:
        break;
    case AbilityID::RESEARCH_TERRANINFANTRYARMORLEVEL2:
        break;
    case AbilityID::RESEARCH_TERRANINFANTRYARMORLEVEL3:
        break;
    case AbilityID::RESEARCH_TERRANINFANTRYWEAPONS:
        break;
    case AbilityID::RESEARCH_TERRANINFANTRYWEAPONSLEVEL1:
        break;
    case AbilityID::RESEARCH_TERRANINFANTRYWEAPONSLEVEL2:
        break;
    case AbilityID::RESEARCH_TERRANINFANTRYWEAPONSLEVEL3:
        break;
    case AbilityID::RESEARCH_TERRANSHIPWEAPONS:
        break;
    case AbilityID::RESEARCH_TERRANSHIPWEAPONSLEVEL1:
        break;
    case AbilityID::RESEARCH_TERRANSHIPWEAPONSLEVEL2:
        break;
    case AbilityID::RESEARCH_TERRANSHIPWEAPONSLEVEL3:
        break;
    case AbilityID::RESEARCH_TERRANSTRUCTUREARMORUPGRADE:
        break;
    case AbilityID::RESEARCH_TERRANVEHICLEANDSHIPPLATING:
        break;
    case AbilityID::RESEARCH_TERRANVEHICLEANDSHIPPLATINGLEVEL1:
        break;
    case AbilityID::RESEARCH_TERRANVEHICLEANDSHIPPLATINGLEVEL2:
        break;
    case AbilityID::RESEARCH_TERRANVEHICLEANDSHIPPLATINGLEVEL3:
        break;
    case AbilityID::RESEARCH_TERRANVEHICLEWEAPONS:
        break;
    case AbilityID::RESEARCH_TERRANVEHICLEWEAPONSLEVEL1:
        break;
    case AbilityID::RESEARCH_TERRANVEHICLEWEAPONSLEVEL2:
        break;
    case AbilityID::RESEARCH_TERRANVEHICLEWEAPONSLEVEL3:
        break;
    case AbilityID::RESEARCH_TUNNELINGCLAWS:
        break;
    case AbilityID::RESEARCH_WARPGATE:
        return UNIT_TYPEID::PROTOSS_CYBERNETICSCORE;
    case AbilityID::RESEARCH_ZERGFLYERARMOR:
        break;
    case AbilityID::RESEARCH_ZERGFLYERARMORLEVEL1:
        break;
    case AbilityID::RESEARCH_ZERGFLYERARMORLEVEL2:
        break;
    case AbilityID::RESEARCH_ZERGFLYERARMORLEVEL3:
        break;
    case AbilityID::RESEARCH_ZERGFLYERATTACK:
        break;
    case AbilityID::RESEARCH_ZERGFLYERATTACKLEVEL1:
        break;
    case AbilityID::RESEARCH_ZERGFLYERATTACKLEVEL2:
        break;
    case AbilityID::RESEARCH_ZERGFLYERATTACKLEVEL3:
        break;
    case AbilityID::RESEARCH_ZERGGROUNDARMOR:
        break;
    case AbilityID::RESEARCH_ZERGGROUNDARMORLEVEL1:
        break;
    case AbilityID::RESEARCH_ZERGGROUNDARMORLEVEL2:
        break;
    case AbilityID::RESEARCH_ZERGGROUNDARMORLEVEL3:
        break;
    case AbilityID::RESEARCH_ZERGLINGADRENALGLANDS:
        break;
    case AbilityID::RESEARCH_ZERGLINGMETABOLICBOOST:
        break;
    case AbilityID::RESEARCH_ZERGMELEEWEAPONS:
        break;
    case AbilityID::RESEARCH_ZERGMELEEWEAPONSLEVEL1:
        break;
    case AbilityID::RESEARCH_ZERGMELEEWEAPONSLEVEL2:
        break;
    case AbilityID::RESEARCH_ZERGMELEEWEAPONSLEVEL3:
        break;
    case AbilityID::RESEARCH_ZERGMISSILEWEAPONS:
        break;
    case AbilityID::RESEARCH_ZERGMISSILEWEAPONSLEVEL1:
        break;
    case AbilityID::RESEARCH_ZERGMISSILEWEAPONSLEVEL2:
        break;
    case AbilityID::RESEARCH_ZERGMISSILEWEAPONSLEVEL3:
        break;
    case AbilityID::SCAN_MOVE:
        break;
    case AbilityID::STOP:
        break;
    case AbilityID::STOP_BUILDING:
        break;
    case AbilityID::STOP_CHEER:
        break;
    case AbilityID::STOP_DANCE:
        break;
    case AbilityID::STOP_REDIRECT:
        break;
    case AbilityID::STOP_STOP:
        break;
    case AbilityID::TRAINWARP_ADEPT:
        break;
    case AbilityID::TRAINWARP_DARKTEMPLAR:
        break;
    case AbilityID::TRAINWARP_HIGHTEMPLAR:
        break;
    case AbilityID::TRAINWARP_SENTRY:
        break;
    case AbilityID::TRAINWARP_STALKER:
        break;
    case AbilityID::TRAINWARP_ZEALOT:
        break;
    case AbilityID::TRAIN_ADEPT:
        break;
    case AbilityID::TRAIN_BANELING:
        break;
    case AbilityID::TRAIN_BANSHEE:
        break;
    case AbilityID::TRAIN_BATTLECRUISER:
        break;
    case AbilityID::TRAIN_CARRIER:
        break;
    case AbilityID::TRAIN_COLOSSUS:
        break;
    case AbilityID::TRAIN_CORRUPTOR:
        break;
    case AbilityID::TRAIN_CYCLONE:
        break;
    case AbilityID::TRAIN_DARKTEMPLAR:
        break;
    case AbilityID::TRAIN_DISRUPTOR:
        break;
    case AbilityID::TRAIN_DRONE:
        break;
    case AbilityID::TRAIN_GHOST:
        break;
    case AbilityID::TRAIN_HELLBAT:
        break;
    case AbilityID::TRAIN_HELLION:
        break;
    case AbilityID::TRAIN_HIGHTEMPLAR:
        break;
    case AbilityID::TRAIN_HYDRALISK:
        break;
    case AbilityID::TRAIN_IMMORTAL:
        break;
    case AbilityID::TRAIN_INFESTOR:
        break;
    case AbilityID::TRAIN_LIBERATOR:
        break;
    case AbilityID::TRAIN_MARAUDER:
        break;
    case AbilityID::TRAIN_MARINE:
        break;
    case AbilityID::TRAIN_MEDIVAC:
        break;
    case AbilityID::TRAIN_MOTHERSHIP:
        break;
    case AbilityID::TRAIN_MOTHERSHIPCORE:
        break;
    case AbilityID::TRAIN_MUTALISK:
        break;
    case AbilityID::TRAIN_OBSERVER:
        break;
    case AbilityID::TRAIN_ORACLE:
        break;
    case AbilityID::TRAIN_OVERLORD:
        break;
    case AbilityID::TRAIN_PHOENIX:
        break;
    case AbilityID::TRAIN_PROBE:
        break;
    case AbilityID::TRAIN_QUEEN:
        break;
    case AbilityID::TRAIN_RAVEN:
        break;
    case AbilityID::TRAIN_REAPER:
        break;
    case AbilityID::TRAIN_ROACH:
        break;
    case AbilityID::TRAIN_SCV:
        break;
    case AbilityID::TRAIN_SENTRY:
        break;
    case AbilityID::TRAIN_SIEGETANK:
        break;
    case AbilityID::TRAIN_STALKER:
        return UNIT_TYPEID::PROTOSS_GATEWAY;
    case AbilityID::TRAIN_SWARMHOST:
        break;
    case AbilityID::TRAIN_TEMPEST:
        break;
    case AbilityID::TRAIN_THOR:
        break;
    case AbilityID::TRAIN_ULTRALISK:
        break;
    case AbilityID::TRAIN_VIKINGFIGHTER:
        break;
    case AbilityID::TRAIN_VIPER:
        break;
    case AbilityID::TRAIN_VOIDRAY:
        break;
    case AbilityID::TRAIN_WARPPRISM:
        break;
    case AbilityID::TRAIN_WIDOWMINE:
        break;
    case AbilityID::TRAIN_ZEALOT:
        break;
    case AbilityID::TRAIN_ZERGLING:
        break;
    case AbilityID::UNLOADALL:
        break;
    case AbilityID::UNLOADALLAT:
        break;
    case AbilityID::UNLOADALLAT_MEDIVAC:
        break;
    case AbilityID::UNLOADALLAT_OVERLORD:
        break;
    case AbilityID::UNLOADALLAT_WARPPRISM:
        break;
    case AbilityID::UNLOADALL_BUNKER:
        break;
    case AbilityID::UNLOADALL_COMMANDCENTER:
        break;
    case AbilityID::UNLOADALL_NYDASNETWORK:
        break;
    case AbilityID::UNLOADALL_NYDUSWORM:
        break;
    case AbilityID::UNLOADUNIT_BUNKER:
        break;
    case AbilityID::UNLOADUNIT_COMMANDCENTER:
        break;
    case AbilityID::UNLOADUNIT_MEDIVAC:
        break;
    case AbilityID::UNLOADUNIT_NYDASNETWORK:
        break;
    case AbilityID::UNLOADUNIT_OVERLORD:
        break;
    case AbilityID::UNLOADUNIT_WARPPRISM:
        break;
    default:
        break;
    }
    throw std::runtime_error("produce undefined for UNIT_ID: " + std::to_string((int)command));
}

UNIT_TYPEID producer(UNIT_TYPEID unit)
{
    switch (unit)
    {
    case UNIT_TYPEID::INVALID:
        UNIT_TYPEID::INVALID;
    case UNIT_TYPEID::TERRAN_ARMORY:
        break;
    case UNIT_TYPEID::TERRAN_AUTOTURRET:
        break;
    case UNIT_TYPEID::TERRAN_BANSHEE:
        break;
    case UNIT_TYPEID::TERRAN_BARRACKS:
        break;
    case UNIT_TYPEID::TERRAN_BARRACKSFLYING:
        break;
    case UNIT_TYPEID::TERRAN_BARRACKSREACTOR:
        break;
    case UNIT_TYPEID::TERRAN_BARRACKSTECHLAB:
        break;
    case UNIT_TYPEID::TERRAN_BATTLECRUISER:
        break;
    case UNIT_TYPEID::TERRAN_BUNKER:
        break;
    case UNIT_TYPEID::TERRAN_COMMANDCENTER:
        break;
    case UNIT_TYPEID::TERRAN_COMMANDCENTERFLYING:
        break;
    case UNIT_TYPEID::TERRAN_CYCLONE:
        break;
    case UNIT_TYPEID::TERRAN_ENGINEERINGBAY:
        break;
    case UNIT_TYPEID::TERRAN_FACTORY:
        break;
    case UNIT_TYPEID::TERRAN_FACTORYFLYING:
        break;
    case UNIT_TYPEID::TERRAN_FACTORYREACTOR:
        break;
    case UNIT_TYPEID::TERRAN_FACTORYTECHLAB:
        break;
    case UNIT_TYPEID::TERRAN_FUSIONCORE:
        break;
    case UNIT_TYPEID::TERRAN_GHOST:
        break;
    case UNIT_TYPEID::TERRAN_GHOSTACADEMY:
        break;
    case UNIT_TYPEID::TERRAN_HELLION:
        break;
    case UNIT_TYPEID::TERRAN_HELLIONTANK:
        break;
    case UNIT_TYPEID::TERRAN_LIBERATOR:
        break;
    case UNIT_TYPEID::TERRAN_LIBERATORAG:
        break;
    case UNIT_TYPEID::TERRAN_MARAUDER:
        break;
    case UNIT_TYPEID::TERRAN_MARINE:
        break;
    case UNIT_TYPEID::TERRAN_MEDIVAC:
        break;
    case UNIT_TYPEID::TERRAN_MISSILETURRET:
        break;
    case UNIT_TYPEID::TERRAN_MULE:
        break;
    case UNIT_TYPEID::TERRAN_ORBITALCOMMAND:
        break;
    case UNIT_TYPEID::TERRAN_ORBITALCOMMANDFLYING:
        break;
    case UNIT_TYPEID::TERRAN_PLANETARYFORTRESS:
        break;
    case UNIT_TYPEID::TERRAN_RAVEN:
        break;
    case UNIT_TYPEID::TERRAN_REAPER:
        break;
    case UNIT_TYPEID::TERRAN_REFINERY:
        break;
    case UNIT_TYPEID::TERRAN_REFINERYRICH:
        break;
    case UNIT_TYPEID::TERRAN_SCV:
        break;
    case UNIT_TYPEID::TERRAN_SENSORTOWER:
        break;
    case UNIT_TYPEID::TERRAN_SIEGETANK:
        break;
    case UNIT_TYPEID::TERRAN_SIEGETANKSIEGED:
        break;
    case UNIT_TYPEID::TERRAN_STARPORT:
        break;
    case UNIT_TYPEID::TERRAN_STARPORTFLYING:
        break;
    case UNIT_TYPEID::TERRAN_STARPORTREACTOR:
        break;
    case UNIT_TYPEID::TERRAN_STARPORTTECHLAB:
        break;
    case UNIT_TYPEID::TERRAN_SUPPLYDEPOT:
        break;
    case UNIT_TYPEID::TERRAN_SUPPLYDEPOTLOWERED:
        break;
    case UNIT_TYPEID::TERRAN_THOR:
        break;
    case UNIT_TYPEID::TERRAN_THORAP:
        break;
    case UNIT_TYPEID::TERRAN_VIKINGASSAULT:
        break;
    case UNIT_TYPEID::TERRAN_VIKINGFIGHTER:
        break;
    case UNIT_TYPEID::TERRAN_WIDOWMINE:
        break;
    case UNIT_TYPEID::TERRAN_WIDOWMINEBURROWED:
        break;
    case UNIT_TYPEID::TERRAN_KD8CHARGE:
        break;
    case UNIT_TYPEID::TERRAN_NUKE:
        break;
    case UNIT_TYPEID::TERRAN_POINTDEFENSEDRONE:
        break;
    case UNIT_TYPEID::TERRAN_REACTOR:
        break;
    case UNIT_TYPEID::TERRAN_TECHLAB:
        break;
    case UNIT_TYPEID::ZERG_BANELING:
        break;
    case UNIT_TYPEID::ZERG_BANELINGBURROWED:
        break;
    case UNIT_TYPEID::ZERG_BANELINGCOCOON:
        break;
    case UNIT_TYPEID::ZERG_BANELINGNEST:
        break;
    case UNIT_TYPEID::ZERG_BROODLING:
        break;
    case UNIT_TYPEID::ZERG_BROODLORD:
        break;
    case UNIT_TYPEID::ZERG_BROODLORDCOCOON:
        break;
    case UNIT_TYPEID::ZERG_CHANGELING:
        break;
    case UNIT_TYPEID::ZERG_CHANGELINGMARINE:
        break;
    case UNIT_TYPEID::ZERG_CHANGELINGMARINESHIELD:
        break;
    case UNIT_TYPEID::ZERG_CHANGELINGZEALOT:
        break;
    case UNIT_TYPEID::ZERG_CHANGELINGZERGLING:
        break;
    case UNIT_TYPEID::ZERG_CHANGELINGZERGLINGWINGS:
        break;
    case UNIT_TYPEID::ZERG_CORRUPTOR:
        break;
    case UNIT_TYPEID::ZERG_CREEPTUMOR:
        break;
    case UNIT_TYPEID::ZERG_CREEPTUMORBURROWED:
        break;
    case UNIT_TYPEID::ZERG_CREEPTUMORQUEEN:
        break;
    case UNIT_TYPEID::ZERG_DRONE:
        break;
    case UNIT_TYPEID::ZERG_DRONEBURROWED:
        break;
    case UNIT_TYPEID::ZERG_EGG:
        break;
    case UNIT_TYPEID::ZERG_EVOLUTIONCHAMBER:
        break;
    case UNIT_TYPEID::ZERG_EXTRACTOR:
        break;
    case UNIT_TYPEID::ZERG_EXTRACTORRICH:
        break;
    case UNIT_TYPEID::ZERG_GREATERSPIRE:
        break;
    case UNIT_TYPEID::ZERG_HATCHERY:
        break;
    case UNIT_TYPEID::ZERG_HIVE:
        break;
    case UNIT_TYPEID::ZERG_HYDRALISK:
        break;
    case UNIT_TYPEID::ZERG_HYDRALISKBURROWED:
        break;
    case UNIT_TYPEID::ZERG_HYDRALISKDEN:
        break;
    case UNIT_TYPEID::ZERG_INFESTATIONPIT:
        break;
    case UNIT_TYPEID::ZERG_INFESTEDTERRANSEGG:
        break;
    case UNIT_TYPEID::ZERG_INFESTOR:
        break;
    case UNIT_TYPEID::ZERG_INFESTORBURROWED:
        break;
    case UNIT_TYPEID::ZERG_INFESTORTERRAN:
        break;
    case UNIT_TYPEID::ZERG_LAIR:
        break;
    case UNIT_TYPEID::ZERG_LARVA:
        break;
    case UNIT_TYPEID::ZERG_LOCUSTMP:
        break;
    case UNIT_TYPEID::ZERG_LOCUSTMPFLYING:
        break;
    case UNIT_TYPEID::ZERG_LURKERDENMP:
        break;
    case UNIT_TYPEID::ZERG_LURKERMP:
        break;
    case UNIT_TYPEID::ZERG_LURKERMPBURROWED:
        break;
    case UNIT_TYPEID::ZERG_LURKERMPEGG:
        break;
    case UNIT_TYPEID::ZERG_MUTALISK:
        break;
    case UNIT_TYPEID::ZERG_NYDUSCANAL:
        break;
    case UNIT_TYPEID::ZERG_NYDUSNETWORK:
        break;
    case UNIT_TYPEID::ZERG_OVERLORD:
        break;
    case UNIT_TYPEID::ZERG_OVERLORDCOCOON:
        break;
    case UNIT_TYPEID::ZERG_OVERLORDTRANSPORT:
        break;
    case UNIT_TYPEID::ZERG_OVERSEER:
        break;
    case UNIT_TYPEID::ZERG_QUEEN:
        break;
    case UNIT_TYPEID::ZERG_QUEENBURROWED:
        break;
    case UNIT_TYPEID::ZERG_RAVAGER:
        break;
    case UNIT_TYPEID::ZERG_RAVAGERCOCOON:
        break;
    case UNIT_TYPEID::ZERG_ROACH:
        break;
    case UNIT_TYPEID::ZERG_ROACHBURROWED:
        break;
    case UNIT_TYPEID::ZERG_ROACHWARREN:
        break;
    case UNIT_TYPEID::ZERG_SPAWNINGPOOL:
        break;
    case UNIT_TYPEID::ZERG_SPINECRAWLER:
        break;
    case UNIT_TYPEID::ZERG_SPINECRAWLERUPROOTED:
        break;
    case UNIT_TYPEID::ZERG_SPIRE:
        break;
    case UNIT_TYPEID::ZERG_SPORECRAWLER:
        break;
    case UNIT_TYPEID::ZERG_SPORECRAWLERUPROOTED:
        break;
    case UNIT_TYPEID::ZERG_SWARMHOSTBURROWEDMP:
        break;
    case UNIT_TYPEID::ZERG_SWARMHOSTMP:
        break;
    case UNIT_TYPEID::ZERG_TRANSPORTOVERLORDCOCOON:
        break;
    case UNIT_TYPEID::ZERG_ULTRALISK:
        break;
    case UNIT_TYPEID::ZERG_ULTRALISKCAVERN:
        break;
    case UNIT_TYPEID::ZERG_VIPER:
        break;
    case UNIT_TYPEID::ZERG_ZERGLING:
        break;
    case UNIT_TYPEID::ZERG_ZERGLINGBURROWED:
        break;
    case UNIT_TYPEID::ZERG_PARASITICBOMBDUMMY:
        break;
    case UNIT_TYPEID::PROTOSS_ADEPT:
        break;
    case UNIT_TYPEID::PROTOSS_ADEPTPHASESHIFT:
        break;
    case UNIT_TYPEID::PROTOSS_ARCHON:
        break;
    case UNIT_TYPEID::PROTOSS_ASSIMILATOR:
        break;
    case UNIT_TYPEID::PROTOSS_ASSIMILATORRICH:
        break;
    case UNIT_TYPEID::PROTOSS_CARRIER:
        break;
    case UNIT_TYPEID::PROTOSS_COLOSSUS:
        break;
    case UNIT_TYPEID::PROTOSS_CYBERNETICSCORE:
        break;
    case UNIT_TYPEID::PROTOSS_DARKSHRINE:
        break;
    case UNIT_TYPEID::PROTOSS_DARKTEMPLAR:
        break;
    case UNIT_TYPEID::PROTOSS_DISRUPTOR:
        break;
    case UNIT_TYPEID::PROTOSS_DISRUPTORPHASED:
        break;
    case UNIT_TYPEID::PROTOSS_FLEETBEACON:
        break;
    case UNIT_TYPEID::PROTOSS_FORGE:
        break;
    case UNIT_TYPEID::PROTOSS_GATEWAY:
        break;
    case UNIT_TYPEID::PROTOSS_HIGHTEMPLAR:
        break;
    case UNIT_TYPEID::PROTOSS_IMMORTAL:
        break;
    case UNIT_TYPEID::PROTOSS_INTERCEPTOR:
        break;
    case UNIT_TYPEID::PROTOSS_MOTHERSHIP:
        break;
    case UNIT_TYPEID::PROTOSS_MOTHERSHIPCORE:
        break;
    case UNIT_TYPEID::PROTOSS_NEXUS:
        return UNIT_TYPEID::PROTOSS_PROBE;
    case UNIT_TYPEID::PROTOSS_OBSERVER:
        break;
    case UNIT_TYPEID::PROTOSS_ORACLE:
        break;
    case UNIT_TYPEID::PROTOSS_ORACLESTASISTRAP:
        break;
    case UNIT_TYPEID::PROTOSS_PHOENIX:
        break;
    case UNIT_TYPEID::PROTOSS_PHOTONCANNON:
        break;
    case UNIT_TYPEID::PROTOSS_PROBE:
        return UNIT_TYPEID::PROTOSS_NEXUS;
    case UNIT_TYPEID::PROTOSS_PYLON:
        return UNIT_TYPEID::PROTOSS_PROBE;
    case UNIT_TYPEID::PROTOSS_PYLONOVERCHARGED:
        break;
    case UNIT_TYPEID::PROTOSS_ROBOTICSBAY:
        break;
    case UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY:
        break;
    case UNIT_TYPEID::PROTOSS_SENTRY:
        break;
    case UNIT_TYPEID::PROTOSS_SHIELDBATTERY:
        break;
    case UNIT_TYPEID::PROTOSS_STALKER:
        return UNIT_TYPEID::PROTOSS_WARPGATE;
    case UNIT_TYPEID::PROTOSS_STARGATE:
        return UNIT_TYPEID::PROTOSS_PROBE;
    case UNIT_TYPEID::PROTOSS_TEMPEST:
        break;
    case UNIT_TYPEID::PROTOSS_TEMPLARARCHIVE:
        break;
    case UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL:
        break;
    case UNIT_TYPEID::PROTOSS_VOIDRAY:
        break;
    case UNIT_TYPEID::PROTOSS_WARPGATE:
        return UNIT_TYPEID::PROTOSS_GATEWAY;
    case UNIT_TYPEID::PROTOSS_WARPPRISM:
        break;
    case UNIT_TYPEID::PROTOSS_WARPPRISMPHASING:
        break;
    case UNIT_TYPEID::PROTOSS_ZEALOT:
    case UNIT_TYPEID::NEUTRAL_BATTLESTATIONMINERALFIELD:
        break;
    case UNIT_TYPEID::NEUTRAL_BATTLESTATIONMINERALFIELD750:
        break;
    case UNIT_TYPEID::NEUTRAL_COLLAPSIBLEROCKTOWERDEBRIS:
        break;
    case UNIT_TYPEID::NEUTRAL_COLLAPSIBLEROCKTOWERDIAGONAL:
        break;
    case UNIT_TYPEID::NEUTRAL_COLLAPSIBLEROCKTOWERPUSHUNIT:
        break;
    case UNIT_TYPEID::NEUTRAL_COLLAPSIBLETERRANTOWERDEBRIS:
        break;
    case UNIT_TYPEID::NEUTRAL_COLLAPSIBLETERRANTOWERDIAGONAL:
        break;
    case UNIT_TYPEID::NEUTRAL_COLLAPSIBLETERRANTOWERPUSHUNIT:
        break;
    case UNIT_TYPEID::NEUTRAL_COLLAPSIBLETERRANTOWERPUSHUNITRAMPLEFT:
        break;
    case UNIT_TYPEID::NEUTRAL_COLLAPSIBLETERRANTOWERPUSHUNITRAMPRIGHT:
        break;
    case UNIT_TYPEID::NEUTRAL_COLLAPSIBLETERRANTOWERRAMPLEFT:
        break;
    case UNIT_TYPEID::NEUTRAL_COLLAPSIBLETERRANTOWERRAMPRIGHT:
        break;
    case UNIT_TYPEID::NEUTRAL_DEBRISRAMPLEFT:
        break;
    case UNIT_TYPEID::NEUTRAL_DEBRISRAMPRIGHT:
        break;
    case UNIT_TYPEID::NEUTRAL_DESTRUCTIBLEDEBRIS6X6:
        break;
    case UNIT_TYPEID::NEUTRAL_DESTRUCTIBLEDEBRISRAMPDIAGONALHUGEBLUR:
        break;
    case UNIT_TYPEID::NEUTRAL_DESTRUCTIBLEDEBRISRAMPDIAGONALHUGEULBR:
        break;
    case UNIT_TYPEID::NEUTRAL_DESTRUCTIBLEROCK6X6:
        break;
    case UNIT_TYPEID::NEUTRAL_DESTRUCTIBLEROCKEX1DIAGONALHUGEBLUR:
        break;
    case UNIT_TYPEID::NEUTRAL_FORCEFIELD:
        break;
    case UNIT_TYPEID::NEUTRAL_KARAKFEMALE:
        break;
    case UNIT_TYPEID::NEUTRAL_LABMINERALFIELD:
        break;
    case UNIT_TYPEID::NEUTRAL_LABMINERALFIELD750:
        break;
    case UNIT_TYPEID::NEUTRAL_MINERALFIELD:
        break;
    case UNIT_TYPEID::NEUTRAL_MINERALFIELD750:
        break;
    case UNIT_TYPEID::NEUTRAL_PROTOSSVESPENEGEYSER:
        break;
    case UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD:
        break;
    case UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD750:
        break;
    case UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD:
        break;
    case UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD750:
        break;
    case UNIT_TYPEID::NEUTRAL_PURIFIERVESPENEGEYSER:
        break;
    case UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD:
        break;
    case UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD750:
        break;
    case UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER:
        break;
    case UNIT_TYPEID::NEUTRAL_SCANTIPEDE:
        break;
    case UNIT_TYPEID::NEUTRAL_SHAKURASVESPENEGEYSER:
        break;
    case UNIT_TYPEID::NEUTRAL_SPACEPLATFORMGEYSER:
        break;
    case UNIT_TYPEID::NEUTRAL_UNBUILDABLEBRICKSDESTRUCTIBLE:
        break;
    case UNIT_TYPEID::NEUTRAL_UNBUILDABLEPLATESDESTRUCTIBLE:
        break;
    case UNIT_TYPEID::NEUTRAL_UTILITYBOT:
        break;
    case UNIT_TYPEID::NEUTRAL_VESPENEGEYSER:
        break;
    case UNIT_TYPEID::NEUTRAL_XELNAGATOWER:
        break;
    default:
        break;

    }

    throw std::runtime_error("TechTree::command undefined for UNIT_ID: " + std::to_string((int)unit));
}

UNIT_TYPEID producer(UPGRADE_ID upgrade)
{
    switch (upgrade)
    {
    case UPGRADE_ID::INVALID:
        return UNIT_TYPEID::INVALID;
    case UPGRADE_ID::CARRIERLAUNCHSPEEDUPGRADE:
        break;
    case UPGRADE_ID::GLIALRECONSTITUTION:
        break;
    case UPGRADE_ID::TUNNELINGCLAWS:
        break;
    case UPGRADE_ID::CHITINOUSPLATING:
        break;
    case UPGRADE_ID::HISECAUTOTRACKING:
        break;
    case UPGRADE_ID::TERRANBUILDINGARMOR:
        break;
    case UPGRADE_ID::TERRANINFANTRYWEAPONSLEVEL1:
        break;
    case UPGRADE_ID::TERRANINFANTRYWEAPONSLEVEL2:
        break;
    case UPGRADE_ID::TERRANINFANTRYWEAPONSLEVEL3:
        break;
    case UPGRADE_ID::NEOSTEELFRAME:
        break;
    case UPGRADE_ID::TERRANINFANTRYARMORSLEVEL1:
        break;
    case UPGRADE_ID::TERRANINFANTRYARMORSLEVEL2:
        break;
    case UPGRADE_ID::TERRANINFANTRYARMORSLEVEL3:
        break;
    case UPGRADE_ID::STIMPACK:
        break;
    case UPGRADE_ID::SHIELDWALL:
        break;
    case UPGRADE_ID::PUNISHERGRENADES:
        break;
    case UPGRADE_ID::HIGHCAPACITYBARRELS:
        break;
    case UPGRADE_ID::BANSHEECLOAK:
        break;
    case UPGRADE_ID::RAVENCORVIDREACTOR:
        break;
    case UPGRADE_ID::PERSONALCLOAKING:
        break;
    case UPGRADE_ID::TERRANVEHICLEWEAPONSLEVEL1:
        break;
    case UPGRADE_ID::TERRANVEHICLEWEAPONSLEVEL2:
        break;
    case UPGRADE_ID::TERRANVEHICLEWEAPONSLEVEL3:
        break;
    case UPGRADE_ID::TERRANSHIPWEAPONSLEVEL1:
        break;
    case UPGRADE_ID::TERRANSHIPWEAPONSLEVEL2:
        break;
    case UPGRADE_ID::TERRANSHIPWEAPONSLEVEL3:
        break;
    case UPGRADE_ID::PROTOSSGROUNDWEAPONSLEVEL1:
        break;
    case UPGRADE_ID::PROTOSSGROUNDWEAPONSLEVEL2:
        break;
    case UPGRADE_ID::PROTOSSGROUNDWEAPONSLEVEL3:
        break;
    case UPGRADE_ID::PROTOSSGROUNDARMORSLEVEL1:
        break;
    case UPGRADE_ID::PROTOSSGROUNDARMORSLEVEL2:
        break;
    case UPGRADE_ID::PROTOSSGROUNDARMORSLEVEL3:
        break;
    case UPGRADE_ID::PROTOSSSHIELDSLEVEL1:
        break;
    case UPGRADE_ID::PROTOSSSHIELDSLEVEL2:
        break;
    case UPGRADE_ID::PROTOSSSHIELDSLEVEL3:
        break;
    case UPGRADE_ID::OBSERVERGRAVITICBOOSTER:
        break;
    case UPGRADE_ID::GRAVITICDRIVE:
        break;
    case UPGRADE_ID::EXTENDEDTHERMALLANCE:
        break;
    case UPGRADE_ID::PSISTORMTECH:
        break;
    case UPGRADE_ID::ZERGMELEEWEAPONSLEVEL1:
        break;
    case UPGRADE_ID::ZERGMELEEWEAPONSLEVEL2:
        break;
    case UPGRADE_ID::ZERGMELEEWEAPONSLEVEL3:
        break;
    case UPGRADE_ID::ZERGGROUNDARMORSLEVEL1:
        break;
    case UPGRADE_ID::ZERGGROUNDARMORSLEVEL2:
        break;
    case UPGRADE_ID::ZERGGROUNDARMORSLEVEL3:
        break;
    case UPGRADE_ID::ZERGMISSILEWEAPONSLEVEL1:
        break;
    case UPGRADE_ID::ZERGMISSILEWEAPONSLEVEL2:
        break;
    case UPGRADE_ID::ZERGMISSILEWEAPONSLEVEL3:
        break;
    case UPGRADE_ID::OVERLORDSPEED:
        break;
    case UPGRADE_ID::BURROW:
        break;
    case UPGRADE_ID::ZERGLINGATTACKSPEED:
        break;
    case UPGRADE_ID::ZERGLINGMOVEMENTSPEED:
        break;
    case UPGRADE_ID::ZERGFLYERWEAPONSLEVEL1:
        break;
    case UPGRADE_ID::ZERGFLYERWEAPONSLEVEL2:
        break;
    case UPGRADE_ID::ZERGFLYERWEAPONSLEVEL3:
        break;
    case UPGRADE_ID::ZERGFLYERARMORSLEVEL1:
        break;
    case UPGRADE_ID::ZERGFLYERARMORSLEVEL2:
        break;
    case UPGRADE_ID::ZERGFLYERARMORSLEVEL3:
        break;
    case UPGRADE_ID::INFESTORENERGYUPGRADE:
        break;
    case UPGRADE_ID::CENTRIFICALHOOKS:
        break;
    case UPGRADE_ID::BATTLECRUISERENABLESPECIALIZATIONS:
        break;
    case UPGRADE_ID::PROTOSSAIRWEAPONSLEVEL1:
        break;
    case UPGRADE_ID::PROTOSSAIRWEAPONSLEVEL2:
        break;
    case UPGRADE_ID::PROTOSSAIRWEAPONSLEVEL3:
        break;
    case UPGRADE_ID::PROTOSSAIRARMORSLEVEL1:
        break;
    case UPGRADE_ID::PROTOSSAIRARMORSLEVEL2:
        break;
    case UPGRADE_ID::PROTOSSAIRARMORSLEVEL3:
        break;
    case UPGRADE_ID::WARPGATERESEARCH:
        return UNIT_TYPEID::PROTOSS_CYBERNETICSCORE;
    case UPGRADE_ID::CHARGE:
        break;
        break;
    case UPGRADE_ID::BLINKTECH:
        break;
    case UPGRADE_ID::PHOENIXRANGEUPGRADE:
        break;
    case UPGRADE_ID::NEURALPARASITE:
        break;
    case UPGRADE_ID::TERRANVEHICLEANDSHIPARMORSLEVEL1:
        break;
    case UPGRADE_ID::TERRANVEHICLEANDSHIPARMORSLEVEL2:
        break;
    case UPGRADE_ID::TERRANVEHICLEANDSHIPARMORSLEVEL3:
        break;
    case UPGRADE_ID::DRILLCLAWS:
        break;
    case UPGRADE_ID::ADEPTPIERCINGATTACK:
        break;
    case UPGRADE_ID::MAGFIELDLAUNCHERS:
        break;
    case UPGRADE_ID::EVOLVEGROOVEDSPINES:
        break;
    case UPGRADE_ID::EVOLVEMUSCULARAUGMENTS:
        break;
    case UPGRADE_ID::BANSHEESPEED:
        break;
    case UPGRADE_ID::RAVENRECALIBRATEDEXPLOSIVES:
        break;
    case UPGRADE_ID::MEDIVACINCREASESPEEDBOOST:
        break;
    case UPGRADE_ID::LIBERATORAGRANGEUPGRADE:
        break;
    case UPGRADE_ID::DARKTEMPLARBLINKUPGRADE:
        break;
    case UPGRADE_ID::CYCLONELOCKONDAMAGEUPGRADE:
        break;
    case UPGRADE_ID::SMARTSERVOS:
        break;
    case UPGRADE_ID::RAPIDFIRELAUNCHERS:
        break;
    case UPGRADE_ID::ENHANCEDMUNITIONS:
        break;
    default:
        break;
    }
    throw std::runtime_error("TechTree::command undefined for UNIT_ID: " + std::to_string((int)upgrade));
}

}
