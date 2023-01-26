#include "TestAgent.h"

using namespace sc2;

TestAgent::TestAgent(uint32_t id, sc2::SC2Context sc2)
    : Agent(id, std::move(sc2))
    , m_planner(m_sc2.obs(), m_sc2.query())
    //, m_map(m_sc2.obs())
{
}

void TestAgent::update()
{
    using namespace std::ranges;
    using namespace std::views;

    auto nexuses = m_sc2.obs().unitsSelf() | filter([](auto u) { return u.unit_type == UNIT_TYPEID::PROTOSS_NEXUS; });

    if (!m_sc2.obs().unitsCreated().empty())
    {
        std::string s;
        for (auto& x : m_sc2.obs().unitsCreated())
        {
            s += "Unit: " + std::to_string(x.tag) + ": " + std::to_string(x.build_progress) + "\n";
        }

        std::cout << s << std::endl;
        m_sc2.act().chat(s);
    }

    m_planner.possibleActions();

    //for (auto& n : nexuses)
    //{
    //    m_sc2.act().command(n, sc2::AbilityID::TRAIN_PROBE);
    //    //sc2().act().chat("HEY! creating a probe on");
    //}

}

sc2::proto::Race TestAgent::race()
{
    return sc2::proto::Protoss;
}
