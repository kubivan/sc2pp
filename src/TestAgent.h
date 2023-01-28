#pragma once

#include <sc2pp/Agent.h>
#include <sc2pp/Planner.h>

#include <string>
#include <random>

struct TestAgent : sc2::Agent
{
    TestAgent(uint32_t id, sc2::SC2Context sc2);

    void update() override;

    sc2::proto::Race race();

    sc2::Planner m_planner;
    
    std::random_device rd;
    std::mt19937 g;

};

