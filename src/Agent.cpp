#include "Agent.h"

using namespace sc2;

Agent::Agent(uint32_t id, SC2Context sc2)
    : m_sc2(std::move(sc2))
    , m_id(id)
{
}

uint32_t Agent::id() const
{
    return m_id;
}

std::unique_ptr<proto::Request> Agent::step(const proto::ResponseObservation& response_obs)
{
    m_sc2.update(response_obs);
    this->update();
    return m_sc2.act().reset();
}
