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
    auto buildAssimilator(Tag builder) -> std::optional<Task>;
    auto buildPylon(Tag builder, bool force = false) -> std::optional<Task>;
    const Observation& m_obs;
    Query& m_query;
};

}

