#include "Agent.h"

using namespace sc2;

Agent::Agent(SC2Context sc2)
    : m_sc2(std::move(sc2))
{
}

std::unique_ptr<proto::Request> Agent::step(const proto::ResponseObservation& response_obs)
{
    m_sc2.update(response_obs);
    this->update();
    return m_sc2.act().reset();
}
