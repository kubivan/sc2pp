#include "TestAgent.h"

#include <sc2pp/utils/UnitTraits.h>
#include <sc2pp/utils/UnitQuery.h>
#include <sc2pp/utils/GridUtils.h>

#include <sc2pp/SC2Context.h>

#include <ranges>
#include <queue>

using namespace sc2;

struct PossibleAction
{
    AbilityID action;
    Tag executor;
};

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
        auto found = find_buildpos_near(query, center.pos, utils::pylon_radius, building);
        if (found)
            return found;
    }

    return {};
}

auto buildAssimilator(const Observation& m_obs, Tag builder) -> std::optional<Task>
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

auto buildPylon(const Observation& m_obs, Query& m_query, Tag builder, bool force = false) -> std::optional<Task>
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
        auto target = rand_point_near(nexus, utils::pylon_radius*3);
        if (m_query.placement(AbilityID::BUILD_PYLON, target))
        {
            return Task{ .executor = builder, .action = AbilityID::BUILD_PYLON, .target = target };
        }
    }

    return {};

}

auto find_possible_tasks(const Observation& obs, Query& query, const std::vector<PossibleAction>& actions)
{
    std::vector<Task> tasks;

    using namespace std::views;
    const auto& units = obs.unitsSelf();

    for (auto& x : actions)
    {
        if (utils::is_training_ability(x.action))
        {
            tasks.push_back(Task{ .executor = x.executor, .action = x.action });
            continue;
        }

        if (utils::is_building_ability(x.action))
        switch (x.action)
        {
        case AbilityID::BUILD_ASSIMILATOR:
        {
            if (auto task = buildAssimilator(obs, x.executor)) 
                tasks.push_back(*task);
        }
        break;
        case AbilityID::BUILD_PYLON:
        {
            if (auto task = buildPylon(obs, query, x.executor))
                tasks.push_back(*task);
            break;
        }
        default: 
            if (auto anchor_point = find_anchor_point(query, obs, x.action))
            {
                tasks.push_back(Task{ .executor = x.executor, .action = x.action, .target = anchor_point });
            }
            else if (auto pylon_task = buildPylon(obs, query, x.executor, true))
            {
                std::cout << "!!! placement of " << magic_enum::enum_name(x.action) << " failed: no pylons, building pylon instead" << std::endl;
                tasks.push_back(*pylon_task);
            }
            else
            {
                std::cout << "!!! placement failed: " << std::endl;
            }
            break;
        }

    }

    return tasks;
}

auto find_possible_actions(const Observation& obs, Query& query) -> std::vector<PossibleAction>
{
    using namespace std::views;

    const auto& units = obs.unitsSelf();

    auto idle_buildings = to_units(units | filter(building && built && idle));
    auto probes = to_units(units | filter(type(UNIT_TYPEID::PROTOSS_PROBE)));

    auto to_possible_action = [](AvailableAbilities& a) -> std::vector<PossibleAction> {
        auto res_range =  a.abilities | transform([&a](auto& avail_ability) { return PossibleAction{ .action = avail_ability.ability_id, .executor = a.unit_tag }; });
        return to_vector<PossibleAction>(res_range);
    };

    auto buildings_actions = query.abilities(idle_buildings) | transform(to_possible_action) | join;

    if (probes.empty())
    {
        std::vector<PossibleAction> res;
        for (auto x : buildings_actions)
            res.push_back(x);
        return res;
    }

    const auto builder = probes.front();
    auto possible_build_actions = query.abilities(builder).abilities 
        | filter([](auto a) { return utils::is_building_ability(a.ability_id); })
        | transform([tag = builder.tag](auto& x) -> PossibleAction { return PossibleAction{ .action = x.ability_id, .executor = tag }; });

    //TODO: implement std::views::concat
    std::vector<PossibleAction> res;
    for (auto& x : buildings_actions)
    {
        res.push_back(x);
    }

    for (auto x : possible_build_actions)
    {
        res.push_back(x);
    }

    return res;
}

TestAgent::TestAgent(uint32_t id, sc2::SC2Context sc2)
    : Agent(id, std::move(sc2))
    , g(rd())
{
}

void TestAgent::update()
{
    std::cout << "====================STEP: " << m_sc2.obs().raw().game_loop() << "=======================" << std::endl;
    using namespace std::ranges;
    using namespace std::views;

    //created units
    if (!m_sc2.obs().unitsCreated().empty())
    {
        std::string s;
        for (auto& x : m_sc2.obs().unitsCreated())
        {
            s += "Unit created: " + std::to_string(x.tag) + ": " + std::to_string(x.build_progress) + "\n";
        }

        std::cout << s << std::endl;
        m_sc2.act().chat(s);

        for (auto& x : m_sc2.obs().unitsCreated() | filter(type(UNIT_TYPEID::PROTOSS_PROBE))
        {
            probes.insert(x);
        }
    }

    auto probes = to_units(m_sc2.obs().unitsSelf() | filter(type(UNIT_TYPEID::PROTOSS_PROBE)));

    //manage idle unit
    {
        auto nexuses = to_units(m_sc2.obs().unitsSelf() | filter(type(UNIT_TYPEID::PROTOSS_NEXUS)));
        auto minerals = to_units(
            m_sc2.obs().units()
            | filter(type(UNIT_TYPEID::NEUTRAL_MINERALFIELD))
            | filter([&](const auto& m) {
                return std::any_of(nexuses.begin(), nexuses.end(), [&](const auto& n) { return dist_squared(m.pos, n.pos) < 100; });
            }));
        auto idle_probes = m_sc2.obs().unitsSelf() | filter(type(UNIT_TYPEID::PROTOSS_PROBE) && idle);
        for (const auto& idle : idle_probes)
        {
            m_sc2.act().command(idle, AbilityID::HARVEST_GATHER, closest(idle, minerals));
        }

    }

    // assign to assimilators
    {
        auto new_assimilators = m_sc2.obs().unitsCreated() | filter(type(UNIT_TYPEID::PROTOSS_ASSIMILATOR));
        int probe_index = 0;
        for (auto& x : new_assimilators)
        {
            if (probe_index >= probes.size())
                break;
            m_sc2.act().command(probes[probe_index++], AbilityID::HARVEST_GATHER, x);
        }
    }

    ////////////////////////

    auto possible_actions = find_possible_actions(m_sc2.obs(), m_sc2.query());
    //filter out probes
    {
        auto nexuses = to_units(m_sc2.obs().unitsSelf() | filter(type(UNIT_TYPEID::PROTOSS_NEXUS) && built));
        std::erase_if(possible_actions, [this, &nexuses, stop_probes = probes.size() >=nexuses.size() * 24 ](PossibleAction& p) {
            return p.action == AbilityID::TRAIN_PROBE && stop_probes;
        });
    }

    //filter out "duplcate" actions
    {
        std::erase_if(possible_actions, [this](PossibleAction& p) {
            auto contains = this->max_taken_actions.find(p.action);
            auto max = contains == max_taken_actions.end() ? this->max_taken_actions_default : contains->second;
            return this->taken_actions[p.action] >= max;
        });
    }

    //bring new_buildings ot 
    auto partitioned = std::partition(
        possible_actions.begin()
        , possible_actions.end()
        , [&](const PossibleAction& p) {
            return !this->taken_actions.contains(p.action);
        });

    std::shuffle(partitioned, possible_actions.end(), g);
    //for (auto& task : possible_actions | std::views::take(rand() % std::max(size_t(1), possible_actions.size() / 2)))
    for (auto& task : find_possible_tasks(m_sc2.obs(), m_sc2.query(), possible_actions))
    {
        std::cout << "Task: " << magic_enum::enum_name(task.action) << " " << task.executor << std::endl;
        m_sc2.act().command(task);
        taken_actions[task.action]++;
    }
}

sc2::proto::Race TestAgent::race()
{
    return sc2::proto::Protoss;
}
