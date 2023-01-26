#pragma once

#include <sc2pp/Actions.h>
#include <sc2pp/Observation.h>
#include <sc2pp/Query.h>

#include <sc2pp/Map.h>

namespace sc2
{

class SC2Context
{
public:
    explicit SC2Context(Observation obs, Query query);

    void update(const proto::ResponseObservation& response_obs);

    auto act() -> Actions&;
    auto obs() const -> const Observation&;
    auto query() -> Query&;

private:
    Observation m_obs;
    Map m_map;
    Actions m_act;
    Query m_query;
};

} // namespace sc2
