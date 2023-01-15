#include "TestAgent.h"

TestAgent::TestAgent()
{
}

void TestAgent::update()
{
    using namespace std::ranges;
    using namespace std::views;

    auto nexuses = obs().self_units() | filter([](auto u) { return u.unit_type == 59; });

    if (!obs().created_units().empty())
    {
        std::string s;
        for (auto& x : obs().created_units())
        {
            s += "Unit: " + std::to_string(x.tag) + ": " + std::to_string(x.build_progress) + "\n";
        }

        std::cout << s << std::endl;
        act().chat(s);
    }

    for (auto& n : nexuses)
    {
        act().command(n, sc2::AbilityID::TRAIN_PROBE);
        //sc2().act().chat("HEY! creating a probe on");
    }

}

sc2::proto::Race TestAgent::race()
{
    return sc2::proto::Protoss;
}
