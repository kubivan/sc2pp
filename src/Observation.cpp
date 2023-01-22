#include "Observation.h"


using namespace sc2;

Observation::Observation(GameInfo game_info
    , std::vector<UnitTypeData> unit_type_data
    , std::vector<AbilityData> ability_data)
    : m_game_info(std::move(game_info))
    , m_unit_type_data(std::move(unit_type_data))
    , m_ability_data(std::move(ability_data))
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

const std::vector<Unit>& Observation::unitsSelf() const
{
    return m_self;
}

const std::vector<Unit>& Observation::unitsCreated() const
{
    return m_created;
}

const GameInfo& Observation::gameInfo() const
{
    return m_game_info;
}

const std::vector<Unit>& Observation::units() const
{
    return m_units.units();
}

const std::vector<AbilityData>& Observation::abilityData() const
{
    return m_ability_data;
}

const std::vector<UnitTypeData>& Observation::unitsTypeData() const
{
    return m_unit_type_data;
}
