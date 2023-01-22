#pragma once

#include <sc2pp/Common.h>
#include <UnitPool.h>

#include <vector>

namespace sc2
{

class Observation
{
public:
    Observation(GameInfo game_info, std::vector<UnitTypeData> unit_type_data, std::vector<AbilityData> ability_data);

    Observation(Observation&&) = default;

    void update(const proto::ResponseObservation& response_obs);

    const proto::Observation& raw() const;

    const std::vector<Unit>& unitsSelf() const;

    const std::vector<Unit>& unitsCreated() const;

    const GameInfo& gameInfo() const;

    const std::vector<Unit>& units() const;

    const std::vector<UnitTypeData>& unitsTypeData() const;
    const std::vector<AbilityData>& abilityData() const;

private:
    proto::ResponseObservation m_raw;
    UnitPool m_units;

    std::vector<Unit> m_created;
    std::vector<Unit> m_damaged;
    std::vector<Unit> m_self;

    GameInfo m_game_info;
    std::vector<UnitTypeData> m_unit_type_data;
    std::vector<AbilityData> m_ability_data;
};

} // namspace sc2
