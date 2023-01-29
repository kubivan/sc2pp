#include "TestAgent.h"
#include "UnitQuery.h"

using namespace sc2;

TestAgent::TestAgent(uint32_t id, sc2::SC2Context sc2)
    : Agent(id, std::move(sc2))
    , m_planner(m_sc2.obs(), m_sc2.query())
    , g(rd())
{
}

void TestAgent::update()
{
    using namespace std::ranges;
    using namespace std::views;

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

    if (!m_sc2.obs().unitsCreated().empty())
    {
        std::string s;
        for (auto& x : m_sc2.obs().unitsCreated())
        {
            s += "Unit created: " + std::to_string(x.tag) + ": " + std::to_string(x.build_progress) + "\n";
        }

        std::cout << s << std::endl;
        m_sc2.act().chat(s);
    }

    auto possible_actions = m_planner.possibleActions();
    //filter out probes
    {
        auto nexuses = to_units(m_sc2.obs().unitsSelf() | filter(type(UNIT_TYPEID::PROTOSS_NEXUS) && built));
        auto probes = to_units(m_sc2.obs().unitsSelf() | filter(type(UNIT_TYPEID::PROTOSS_PROBE)));
        std::erase_if(possible_actions, [this, &nexuses, &probes, stop_probes = probes.size() >=nexuses.size() * 24 ](Task& t) {
            return t.action == AbilityID::TRAIN_PROBE && stop_probes;
        });
    }

    //bring new_buildings ot 
    auto partitioned = std::partition(
        possible_actions.begin()
        , possible_actions.end()
        , [&](const Task& t) {
            return !this->taken_actions.contains(t.action);
        });

    std::shuffle(partitioned, possible_actions.end(), g);
    for (auto& task : possible_actions)
    {
        m_sc2.act().command(task);
        taken_actions.insert(task.action);
    }
}

sc2::proto::Race TestAgent::race()
{
    return sc2::proto::Protoss;
}
