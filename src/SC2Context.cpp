#include <sc2pp/SC2Context.h>

using namespace sc2;

SC2Context::SC2Context(GameInfo game_info)
    : m_obs(std::move(game_info))
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
