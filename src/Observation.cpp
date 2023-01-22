#include "Observation.h"


using namespace sc2;

Observation::Observation(GameInfo game_info)
    : m_game_info(std::move(game_info))
{
}

void Observation::update(const proto::ResponseObservation& response_obs)
{
    const auto [units, unit_commands, errors] = convert_observation(response_obs);

    std::tie(m_created, m_damaged, m_self) = m_units.update(units);
}

const proto::Observation& Observation::raw() const
{
    return m_raw.observation();
}

const std::vector<Unit>& Observation::self_units() const
{
    return m_self;
}

const std::vector<Unit>& Observation::created_units() const
{
    return m_created;
}

const GameInfo& Observation::game_info() const
{
    return m_game_info;
}

const std::vector<Unit>& Observation::units() const
{
    return m_units.units();
}