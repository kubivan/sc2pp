#pragma once

#include <sc2pp/Common.h>

#include <memory>

namespace sc2
{
class SC2Session;

struct AvailableAbility
{
    AbilityID ability_id;
    bool requires_point;
};

struct AvailableAbilities
{
  std::vector<AvailableAbility> abilities;
  Tag unit_tag;
  UNIT_TYPEID unit_type_id;
};

class Query
{
public:
    explicit Query(std::shared_ptr<SC2Session> session);
    Query(Query&&) = default;

    auto abilities(const Units& units, bool ignore_resource_requirements = false) const -> std::vector<AvailableAbilities>;
    auto abilities(const Unit& unit, bool ignore_resource_requirements = false) const -> AvailableAbilities;

    auto placement(AbilityID building, const Point2D& pos, bool ignore_resource_requirements = false) const -> bool;

private:
    std::shared_ptr<SC2Session> m_session;

};

} // namespace sc2

