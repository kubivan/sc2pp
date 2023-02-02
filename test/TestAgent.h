#pragma once

#include <sc2pp/SC2IDs.h>
#include <sc2pp/Agent.h>

#include "Planner.h"

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
    
    std::unordered_map<sc2::AbilityID, int> taken_actions;
    const std::unordered_map<sc2::AbilityID, int> max_taken_actions = { 
        {sc2::AbilityID::BUILD_FORGE, 1},
        {sc2::AbilityID::BUILD_CYBERNETICSCORE, 1},
        {sc2::AbilityID::BUILD_NEXUS, 0},
        {sc2::AbilityID::BUILD_SHIELDBATTERY, 0},
        {sc2::AbilityID::BUILD_TWILIGHTCOUNCIL, 0},

        {sc2::AbilityID::TRAIN_SENTRY, 2},
        {sc2::AbilityID::TRAIN_WARPPRISM, 0},
        {sc2::AbilityID::TRAIN_OBSERVER, 1},
        {sc2::AbilityID::TRAIN_PHOENIX, 0},
        {sc2::AbilityID::TRAIN_PROBE, 64},
    };

    const int max_taken_actions_default = 5;

};

