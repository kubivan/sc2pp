#include "Planner.h"

#include <sc2pp/Observation.h>
#include <sc2pp/Query.h>
#include <sc2pp/Actions.h>
#include <sc2pp/utils/UnitTraits.h>
#include <sc2pp/utils/UnitQuery.h>
#include <sc2pp/utils/GridUtils.h>

#include <ranges>

using namespace sc2;

constexpr auto pylon_radius = 6.5f;

static const std::set<AbilityID> build_abilities = {
    AbilityID::BUILD_ARMORY,   // Target: Point.
    AbilityID::BUILD_ASSIMILATOR,   // Target: Unit.
    AbilityID::BUILD_BANELINGNEST,  // Target: Point.
    AbilityID::BUILD_BARRACKS,   // Target: Point.
    AbilityID::BUILD_BUNKER,   // Target: Point.
    AbilityID::BUILD_COMMANDCENTER,   // Target: Point.
    AbilityID::BUILD_CREEPTUMOR,  // Target: Point.
    AbilityID::BUILD_CREEPTUMOR_QUEEN,  // Target: Point.
    AbilityID::BUILD_CREEPTUMOR_TUMOR,  // Target: Point.
    AbilityID::BUILD_CYBERNETICSCORE,   // Target: Point.
    AbilityID::BUILD_DARKSHRINE,   // Target: Point.
    AbilityID::BUILD_ENGINEERINGBAY,   // Target: Point.
    AbilityID::BUILD_EVOLUTIONCHAMBER,  // Target: Point.
    AbilityID::BUILD_EXTRACTOR,  // Target: Unit.
    AbilityID::BUILD_FACTORY,   // Target: Point.
    AbilityID::BUILD_FLEETBEACON,   // Target: Point.
    AbilityID::BUILD_FORGE,   // Target: Point.
    AbilityID::BUILD_FUSIONCORE,   // Target: Point.
    AbilityID::BUILD_GATEWAY,   // Target: Point.
    AbilityID::BUILD_GHOSTACADEMY,   // Target: Point.
    AbilityID::BUILD_HATCHERY,  // Target: Point.
    AbilityID::BUILD_HYDRALISKDEN,  // Target: Point.
    AbilityID::BUILD_INFESTATIONPIT,  // Target: Point.
    AbilityID::BUILD_INTERCEPTORS,  // Target: None.
    AbilityID::BUILD_MISSILETURRET,   // Target: Point.
    AbilityID::BUILD_NEXUS,   // Target: Point.
    AbilityID::BUILD_NUKE,   // Target: None.
    AbilityID::BUILD_NYDUSNETWORK,  // Target: Point.
    AbilityID::BUILD_NYDUSWORM,  // Target: Point.
    AbilityID::BUILD_PHOTONCANNON,   // Target: Point.
    AbilityID::BUILD_PYLON,   // Target: Point.
    AbilityID::BUILD_REACTOR,  // Target: None.
    AbilityID::BUILD_REACTOR_BARRACKS,   // Target: None.
    AbilityID::BUILD_REACTOR_FACTORY,   // Target: None.
    AbilityID::BUILD_REACTOR_STARPORT,   // Target: None.
    AbilityID::BUILD_REFINERY,   // Target: Unit.
    AbilityID::BUILD_ROACHWARREN,  // Target: Point.
    AbilityID::BUILD_ROBOTICSBAY,   // Target: Point.
    AbilityID::BUILD_ROBOTICSFACILITY,   // Target: Point.
    AbilityID::BUILD_SENSORTOWER,   // Target: Point.
    AbilityID::BUILD_SHIELDBATTERY,   // Target: Point.
    AbilityID::BUILD_SPAWNINGPOOL,  // Target: Point.
    AbilityID::BUILD_SPINECRAWLER,  // Target: Point.
    AbilityID::BUILD_SPIRE,  // Target: Point.
    AbilityID::BUILD_SPORECRAWLER,  // Target: Point.
    AbilityID::BUILD_STARGATE,   // Target: Point.
    AbilityID::BUILD_STARPORT,   // Target: Point.
    AbilityID::BUILD_STASISTRAP,  // Target: Point.
    AbilityID::BUILD_SUPPLYDEPOT,   // Target: Point.
    AbilityID::BUILD_TECHLAB,  // Target: None.
    AbilityID::BUILD_TECHLAB_BARRACKS,   // Target: None.
    AbilityID::BUILD_TECHLAB_FACTORY,   // Target: None.
    AbilityID::BUILD_TECHLAB_STARPORT,   // Target: None.
    AbilityID::BUILD_TEMPLARARCHIVE,   // Target: Point.
    AbilityID::BUILD_TWILIGHTCOUNCIL,   // Target: Point.
    AbilityID::BUILD_ULTRALISKCAVERN  // Target: Point.
};

auto is_training_ability(AbilityID id)
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

namespace
{

float
random()
{
    return static_cast <float> (std::rand()) / static_cast <float> (RAND_MAX);
}

float
random0(float x)
{
    return static_cast <float> (std::rand()) / (static_cast <float> (RAND_MAX / x));
}

sc2::Point2D
rand_point_near(const sc2::Point2D& center
    , float radius)
{
    float theta = random0(2 * 3.14159f);
    float distance = random0(1.0) * radius;

    float px = distance * cos(theta) + center.x;
    float py = distance * sin(theta) + center.y;
    return sc2::Point2D{ px, py };
}

sc2::Point2D
rand_point_around(const sc2::Point2D& center
    , float radius
    , float divisor /*= 1.f*/)
{
    float theta = random0(2 * 3.14159f) / divisor;

    float px = radius * cos(theta) + center.x;
    float py = radius * sin(theta) + center.y;
    return sc2::Point2D{ px, py };
}

Point2D
enemy_base_location(const Observation& obs)
{
    static Point2D res = [&obs]() {
        using namespace std::views;
        const auto nexus = to_vector<Unit>(obs.unitsSelf() | filter(type(UNIT_TYPEID::PROTOSS_NEXUS))).front();

        for (auto enemy_pos : obs.gameInfo().start_locations)
        {
            if (dist_squared(enemy_pos, nexus.pos) > 1.)
            {
                continue;
            }
            return enemy_pos;
        }
        assert(false);
        return Point2D{ -1, -1 };
    }();

    return res;
}

std::optional<Point2D>
find_buildpos_near(Query& query
    , const Point2D& center
    , float radius
    , AbilityID building)
{
    std::queue<Point2DI> fringe;
    fringe.push(utils::tile_pos(center));

    auto point_comparator = [](const Point2DI& a, const Point2DI& b) {
        return std::tie(a.x, a.y) < std::tie(b.x, b.y);
    };
    std::set<Point2DI, decltype(point_comparator)> marked(point_comparator);
    while (!fringe.empty())
    {
        auto pixel = fringe.front();
        fringe.pop();

        if (query.placement(building, { float(pixel.x), float(pixel.y) }))
        {
            return Point2D{ float(pixel.x), float(pixel.y) };
        }

        const auto variants = std::array<Point2DI, 6>{ Point2DI{ pixel.x + 1, pixel.y }
            , Point2DI{ pixel.x, pixel.y + 1 }
            , Point2DI{ pixel.x - 1, pixel.y }
            , Point2DI{ pixel.x, pixel.y - 1 }
            , Point2DI{ pixel.x + 1, pixel.y + 1 }
            , Point2DI{ pixel.x - 1, pixel.y - 1 }
        };

        for (const auto& v : variants)
        {
            if (dist_squared({ (float)v.x, (float)v.y }, center) > (radius + 3) * (radius + 3))
                continue;

            if (!marked.insert(v).second)
            {
                continue;
            }
            fringe.push(v);
        }
    }
    return { };
}

std::optional<Point2D>
find_anchor_point(Query& query
    , const Observation& obs
    , AbilityID building)
{
    using namespace std::views;

    auto pylons = to_units(obs.unitsSelf() | filter(type(UNIT_TYPEID::PROTOSS_PYLON)));
    auto pylons_built = pylons | filter(built);

    if (!pylons.empty() && pylons_built.empty())
    {
        std::cout << "waiting pylons to complete" << std::endl;
        return {};
    }

    if (pylons.empty())
    {
        return {};
    }

    for (const auto& center : pylons_built)
    {
        auto found = find_buildpos_near(query, center.pos, pylon_radius, building);
        if (found)
            return found;
    }

    return {};
}


} //namespace 

Planner::Planner(const Observation& obs, Query& query)
    : m_obs(obs)
    , m_query(query)
{
}

auto Planner::possibleActions() -> std::vector<Task>
{
    using namespace std::views;
    std::vector<Task> res;
    const auto& units = m_obs.unitsSelf();

    auto buildings = units | filter(building && built);

    for (const auto& building_abilities : m_query.abilities(to_units(buildings | filter(idle))))
    {
        for (const auto& ability : building_abilities.abilities)
        {
            if (!is_training_ability(ability.ability_id))
            {
                continue;
            }

            //TODO: add target finding
            if (ability.requires_point)
            {
                continue;
            }

            res.push_back(Task{ .executor = building_abilities.unit_tag, .action = ability.ability_id });
        }
    }

    const auto probes = to_units(m_obs.unitsSelf() | filter(type(UNIT_TYPEID::PROTOSS_PROBE)));
    if (probes.empty())
    {
        return res;
    }

    auto probe_build_abilities = m_query.abilities(probes.front()).abilities | filter([](auto a) { return build_abilities.contains(a.ability_id); });

    for (auto x : probe_build_abilities)
    {
        std::cout << "Probes ability: " << magic_enum::enum_name(x.ability_id) << " " << x.requires_point << std::endl;

        switch (x.ability_id)
        {
        case AbilityID::BUILD_ASSIMILATOR:
        {
            auto task = buildAssimilator(probes.front().tag);
            if (task) 
                res.push_back(*task);
        }
        break;
        case AbilityID::BUILD_PYLON:
        {
            auto task = buildPylon(probes.front().tag);
            if (task)
                res.push_back(*task);
            break;
        }
        default: 
            auto anchor_point = find_anchor_point(this->m_query, this->m_obs, x.ability_id);
            if (anchor_point)
            {
                res.push_back(Task{ .executor = probes.front().tag, .action = x.ability_id, .target = anchor_point });
            }
            else if (auto pylon_task = buildPylon(probes.front().tag, true))
            {
                std::cout << "!!! placement failed: no pylons!!!" << std::endl;
                res.push_back(*pylon_task);
            }
            else
            {
                std::cout << "!!! placement failed: " << std::endl;
            }
            break;
        }

    }

    return res;
}

auto Planner::buildAssimilator(Tag builder) -> std::optional<Task>
{
    using namespace std::views;
    auto nexuses = to_units(m_obs.unitsSelf() | filter(type(UNIT_TYPEID::PROTOSS_NEXUS)));
    auto assimilators = to_units(m_obs.unitsSelf() | filter(type(UNIT_TYPEID::PROTOSS_ASSIMILATOR)));
    if (assimilators.size() >= nexuses.size() * 2)
        return {};

    auto geysers = m_obs.units()
        | filter(is_geyser)
        | filter([&assimilators](const auto& g)
        {
            for (auto& a : assimilators)
            {
                if (dist_squared(g.pos, a.pos) == 0)
                {
                    return false;
                }
            }
            return true;
        });

    if (geysers.empty())
        return {};

    return Task{ .executor = builder,
                 .action = AbilityID::BUILD_ASSIMILATOR,
                 .target = closest(nexuses.front(), to_units(geysers)).tag };
}

auto Planner::buildPylon(Tag builder, bool force) -> std::optional<Task>
{
    using namespace std::views;
    const auto pylons = to_units(m_obs.unitsSelf() | filter(type(UNIT_TYPEID::PROTOSS_PYLON)));
    if (force && 2 * m_obs.raw().player_common().food_used() < (m_obs.raw().player_common().food_cap() + pylons.size() * 5))
    {
        return {};
    }

    const auto nexus = to_units(m_obs.unitsSelf() | filter(type(UNIT_TYPEID::PROTOSS_NEXUS))).front().pos;

    for (int i = 0; i < 100; ++i)
    {
        //auto target = rand_point_around(nexus, pylon_radius*3, 1);
        auto target = rand_point_near(nexus, pylon_radius*3);
        if (m_query.placement(AbilityID::BUILD_PYLON, target))
        {
            return Task{ .executor = builder, .action = AbilityID::BUILD_PYLON, .target = target };
        }
    }

    return {};

}
