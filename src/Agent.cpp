#include "Agent.h"

using namespace sc2;

Agent::Agent(uint32_t id, SC2Context sc2)
    : m_sc2(std::move(sc2))
    , m_id(id)
{
}

auto Agent::id() const -> uint32_t
{
    return m_id;
}

auto Agent::step(const proto::ResponseObservation& response_obs) -> std::unique_ptr<proto::Request>
{
    m_sc2.update(response_obs);
    this->update();
    return m_sc2.act().reset();
}
