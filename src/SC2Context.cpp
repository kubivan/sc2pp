#include <sc2pp/SC2Context.h>

using namespace sc2;

SC2Context::SC2Context(Observation obs, Query query)
    : m_obs(std::move(obs))
    , m_map(m_obs)
    , m_query(std::move(query))
{
}

void SC2Context::update(const proto::ResponseObservation& response_obs)
{
    m_obs.update(response_obs);
}

Actions& SC2Context::act()
{ 
    return m_act; 
}

const Observation& SC2Context::obs() const
{
    return m_obs; 
}

auto SC2Context::query() -> Query&
{
    return m_query;
}
