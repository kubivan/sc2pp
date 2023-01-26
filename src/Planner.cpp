#include "Planner.h"
#include "Observation.h"
#include "Query.h"
#include "UnitTraits.h"
#include "UnitQuery.h"
#include "Actions.h"

#include <ranges>

using namespace sc2;

constexpr auto pylon_radius = 5.f;

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

Point2D
find_buildpos_near(Query& query
    , const Point2D& center
    , float radius
    , AbilityID building)
{
    const auto max_iterations = 10000;
    auto pos = rand_point_near(center, radius);
    int i = 0;
    while (!query.placement(building, pos))
    {
        if (i++ > max_iterations)
            break;
        pos = rand_point_near(center, radius);
    }
    return pos;
}

Point2D
build_near(Query& query
    , Actions& act
    , const Unit& probe
    , const Point2D& center
    , float radius
    , AbilityID building
    , bool queued)
{
    const auto max_iterations = 10;
    auto pos = rand_point_near(center, radius);
    int i = 0;
    while (!query.placement(building, pos))
    {
        if (i++ > max_iterations)
            break;
        pos = rand_point_near(center, radius);
    }

    act.command(probe, building, pos, queued);
    return pos;
}

Point2D
find_anchor_point(Query& query
    , const Observation& obs
    , AbilityID building)
{
    if (building == AbilityID::BUILD_PYLON)
    {
        return find_buildpos_near(query, obs.gameInfo().start_locations.front(), pylon_radius, building);
    }
    using namespace std::views;

    auto pylons = to_units(obs.unitsSelf() | filter(type(UNIT_TYPEID::PROTOSS_PYLON)));
    auto center = pylons[rand() % (pylons.size() - 1)].pos;

    return find_buildpos_near(query, center, pylon_radius, building);
}


} //namespace 

Planner::Planner(const Observation& obs, Query& query)
    : m_obs(obs)
    , m_query(query)
{
}

auto Planner::possibleActions() -> std::vector<Task> const
{
    using namespace std::views;
    std::vector<Task> res;
    const auto& units = m_obs.unitsSelf();

    auto buildings = units | filter(building && built);

    auto nexuses = buildings | filter(type(UNIT_TYPEID::PROTOSS_NEXUS));

    auto nexus_abilities = m_query.abilities(to_vector<Unit>(nexuses));

    for (auto nexus_ab : nexus_abilities)
    {
        auto unit = to_units(nexuses | filter(unit_tag(nexus_ab.unit_tag))).front();
        for (auto ability : nexus_ab.abilities)
        {
            std::cout << "Nexus ability: " << (int)ability.ability_id << " " << ability.requires_point << std::endl;

            res.push_back([this, nexus_ab, unit, ability](Actions& act) {
                act.command(unit, ability.ability_id, rand);
                });
        }
    }

    const auto probes = to_units(m_obs.unitsSelf() | filter(type(UNIT_TYPEID::PROTOSS_PROBE)));
    if (probes.empty())
    {
        return res;
    }

    auto probe_abilities = m_query.abilities(probes.front()).abilities | filter([](auto a) { return build_abilities.contains(a.ability_id); });

    for (auto x : probe_abilities)
    {
        std::cout << "Probes ability: " << (int)x.ability_id  << " " << x.requires_point << std::endl;
        if (x.ability_id == AbilityID::BUILD_PYLON)
        {
            auto anchor_point = find_anchor_point(this->m_query, this->m_obs, x.ability_id);
            res.push_back([this, probe = probes.front(), ability = x.ability_id, anchor_point ](Actions& act) {
                act.command(probe, ability, anchor_point);
                });
        }


    }


    return res;
}
