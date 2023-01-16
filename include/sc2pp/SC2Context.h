#pragma once

#include <sc2pp/Actions.h>
#include <sc2pp/Observation.h>

#include "Converters.h"

namespace sc2
{

class SC2Context
{
public:
    explicit SC2Context(GameInfo game_info);

    void update(const proto::ResponseObservation& response_obs);

    Actions& act();
    const Observation& obs() const;

private:
    Observation m_obs;
    Actions m_act;
};

} // namespace sc2
