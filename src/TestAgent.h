#pragma once

#include <sc2pp/Agent.h>

#include "Converters.h"
#include <string>

struct TestAgent : sc2::Agent
{
    TestAgent(sc2::SC2Context sc2);

    void update() override;

    sc2::proto::Race race();
};

