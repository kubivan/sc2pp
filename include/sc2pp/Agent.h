#pragma once

#include <sc2pp/SC2Context.h>

#include <memory>

namespace sc2
{

class Agent
{
public:
    explicit Agent(uint32_t id, SC2Context sc2);

    virtual auto race() -> proto::Race = 0;

    auto id() const -> uint32_t;

    auto step(const proto::ResponseObservation& response_obs) -> std::unique_ptr<proto::Request>;

protected:
    SC2Context m_sc2;

private:
    virtual auto update() -> void = 0;
    uint32_t m_id;
};

}
