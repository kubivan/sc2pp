#pragma once

#include <sc2pp/Common.h>
#include <UnitPool.h>

#include <vector>

namespace sc2
{

class Observation
{
public:
    Observation(GameInfo game_info);

    void update(const proto::ResponseObservation& response_obs);

    const proto::Observation& raw() const;;

    const std::vector<Unit>& self_units() const;

    const std::vector<Unit>& created_units() const;

    const GameInfo& game_info() const;

    const std::vector<Unit>& units() const;

private:
    proto::ResponseObservation m_raw;
    UnitPool m_units;

    std::vector<Unit> m_created;
    std::vector<Unit> m_damaged;
    std::vector<Unit> m_self;

    GameInfo m_game_info;
};

} // namspace sc2
