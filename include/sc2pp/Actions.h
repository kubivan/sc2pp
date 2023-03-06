#pragma once

#include <sc2pp/Common.h>
#include <sc2pp/SC2IDs.h>

#include <vector>
#include <string>

namespace sc2 {

struct Task
{
    Tag executor;
    AbilityID action;
    std::optional<std::variant<Point2D, Tag>> target;
};

class Actions
{
public:
    auto has() -> bool;

    auto command(const Unit& unit, AbilityID ability, bool queued = false) -> Actions&;
    auto command(const Unit& unit, AbilityID ability, const Point2D& point, bool queued = false) -> Actions&;
    auto command(const Unit& unit, AbilityID ability, const Unit& target, bool queued = false) -> Actions&;

    auto command(const Units& units, AbilityID ability, const Unit& target, bool queued = false) -> Actions&;
    auto command(const Units& units, AbilityID ability, const Point2D& point, bool queued = false) -> Actions&;
    auto command(const Units& units, AbilityID ability, bool queued = false) -> Actions&;

    auto command(const Task& task, bool queued = false) -> Actions&;

    auto toggleAutocast(Tag unit_tag, AbilityID ability) -> Actions&;
    auto toggleAutocast(const std::vector<Tag>& unit_tags, AbilityID ability) -> Actions&;

    auto chat(const std::string& message, proto::ActionChat::Channel channel = proto::ActionChat_Channel::ActionChat_Channel_Broadcast) -> Actions&;

    auto reset() -> std::unique_ptr<proto::Request>;

private:
    auto requestAction() -> proto::RequestAction*;

    std::unique_ptr<proto::Request> m_request;
};

}

