#include "Query.h"
#include "SC2Session.h"

using namespace sc2;

Query::Query(std::shared_ptr<SC2Session> session)
    : m_session(std::move(session))
{
}

auto Query::abilities(const Unit& unit, bool ignore_resource_requirements) const -> AvailableAbilities
{
    const Units units = { unit };
    return abilities(units, ignore_resource_requirements).front();
}

auto Query::abilities(const Units& units, bool ignore_resource_requirements) const -> std::vector<AvailableAbilities>
{
    std::vector<AvailableAbilities> res;
    if (units.empty())
    {
        return {};
    }

    auto request = std::make_unique<proto::Request>();
    auto request_query = request->mutable_query();
    request_query->set_ignore_resource_requirements(ignore_resource_requirements);
    for (const auto& unit : units)
    {
        auto request_abilities = request_query->add_abilities();
        request_abilities->set_unit_tag(unit.tag);
    }

    auto response = m_session->send(std::move(request));
    if (!response)
    {
        return {};
    }

    if (!response->has_query())
    {
        throw std::logic_error("Query::abilities: wrong response");
    }
    const auto& query = response->query();
    if (!query.abilities_size())
    {
        return {};
    }

    for (const auto& abilities : query.abilities())
    {
        AvailableAbilities available_abilities_unit;
        available_abilities_unit.unit_tag = abilities.unit_tag();
        available_abilities_unit.unit_type_id = UNIT_TYPEID(abilities.unit_type_id());
        for (const auto& ability : abilities.abilities())
        {
            AvailableAbility available_ability;
            available_abilities_unit.abilities.push_back(
                AvailableAbility{
                .ability_id = AbilityID(ability.ability_id()),
                .requires_point = ability.requires_point()
                });
        }

        res.push_back(available_abilities_unit);
    }

    return res;
}

auto Query::placement(AbilityID building, const Point2D& pos, bool ignore_resource_requirements) const -> bool
{
    auto request = std::make_unique<proto::Request>();
    auto request_query = request->mutable_query();
    request_query->set_ignore_resource_requirements(ignore_resource_requirements);
    auto request_placements = request_query->add_placements();
    request_placements->set_ability_id((int32_t)building);

    auto target = request_placements->mutable_target_pos();
    target->set_x(pos.x);
    target->set_y(pos.y);

    auto response = m_session->send(std::move(request));
    if (!response)
    {
        return {};
    }

    if (!response->has_query())
    {
        throw std::logic_error("Query::placements: wrong response");
    }

    auto response_query = response->query();
    assert(response_query.placements_size() == 1);

    auto res = response_query.placements(0).result();
    return res == proto::ActionResult::Success;
}
