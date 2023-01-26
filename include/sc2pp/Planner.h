#pragma once

#include <sc2pp/Common.h>

namespace sc2
{
class Actions;
class Observation;
class Query;

class Planner
{
public: 
    using Task = std::function<void(Actions& act)>;

    explicit Planner(const Observation& obs, Query& query);

    auto possibleActions() -> std::vector<Task> const;
private:
    const Observation& m_obs;
    Query& m_query;
};

}

