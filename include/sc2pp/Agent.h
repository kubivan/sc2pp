#pragma once

#include <sc2pp/SC2Context.h>

#include <memory>

namespace sc2
{

class Agent
{
public:
    explicit Agent(uint32_t id, SC2Context sc2);

    virtual proto::Race race() = 0;

    uint32_t id() const;

    std::unique_ptr<proto::Request> step(const proto::ResponseObservation& response_obs);

protected:
    SC2Context m_sc2;

private:
    virtual void update() = 0;
    uint32_t m_id;
};

}