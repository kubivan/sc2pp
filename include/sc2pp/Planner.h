#pragma once

#include <sc2pp/Common.h>
#include <sc2pp/Actions.h>

#include <optional>

namespace sc2
{
class Observation;
class Query;

class Planner
{
public: 

    explicit Planner(const Observation& obs, Query& query);

    auto possibleActions() -> std::vector<Task>;
private:
    const Observation& m_obs;
    Query& m_query;
};

}

