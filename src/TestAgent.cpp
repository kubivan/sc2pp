#include "TestAgent.h"

TestAgent::TestAgent()
    : Agent(std::make_unique<sc2::SC2Context>())
{
}

void TestAgent::update()
{
    //std::cout << std::this_thread::get_id() << " : TestAgent::step()" << std::endl;

    using namespace std::ranges;
    using namespace std::views;

    auto nexuses = sc2().obs().self_units() | filter([](auto u) { return u.unit_type == 59; });

    if (!sc2().obs().created_units().empty())
    {
        std::string s;
        for (auto& x : sc2().obs().created_units())
        {
            s += "Unit: " + std::to_string(x.tag) + ": " + std::to_string(x.build_progress) + "\n";
        }

        std::cout << s << std::endl;
        sc2().act().chat(s);
    }

    for (auto& n : nexuses)
    {
        sc2().act().command(n, sc2::AbilityID::TRAIN_PROBE);
        //sc2().act().chat("HEY! creating a probe on");
    }

}

sc2::proto::Race TestAgent::race()
{
    return sc2::proto::Protoss;
}
