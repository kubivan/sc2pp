#pragma once
#include <s2clientprotocol/sc2api.pb.h>

#include "SC2IDs.h"

#include <vector>
#include <string>

namespace sc2 {

namespace proto = SC2APIProtocol;
using Tag = int;
using Units = std::vector<proto::Unit>;

struct Point2D 
{
    float x;
    float y;
};

class Actions
{
public:
    bool has() { return m_request.get(); }

    Actions& command(const proto::Unit& unit, AbilityID ability, bool queued = false);
    Actions& command(const proto::Unit& unit, AbilityID ability, const Point2D& point, bool queued = false);
    Actions& command(const proto::Unit& unit, AbilityID ability, const proto::Unit& target, bool queued = false);

    Actions& command(const Units& units, AbilityID ability, const proto::Unit& target, bool queued = false);
    Actions& command(const Units& units, AbilityID ability, const Point2D& point, bool queued = false);
    Actions& command(const Units& units, AbilityID ability, bool queued = false);

    Actions& toggleAutocast(Tag unit_tag, AbilityID ability);
    Actions& toggleAutocast(const std::vector<Tag>& unit_tags, AbilityID ability);

    Actions& chat(const std::string& message, proto::ActionChat::Channel channel = proto::ActionChat_Channel::ActionChat_Channel_Broadcast);

    std::unique_ptr<proto::Request> reset();

private:
    proto::RequestAction* requestAction();

    std::unique_ptr<proto::Request> m_request;
};

}

