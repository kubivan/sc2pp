#pragma once

#include <sc2pp/Agent.h>

#include "Converters.h"
#include <string>

struct TestAgent : sc2::Agent
{
    TestAgent();

    void update() override;

    sc2::proto::Race race();
};

