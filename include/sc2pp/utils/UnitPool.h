#pragma once

#include <sc2pp/Common.h>

#include <unordered_map>

namespace sc2
{

inline auto get_health(const Unit& u)
{
    return u.shield + u.health;
}

class UnitPool
{
public:
    auto update(const std::vector<Unit>& units) -> std::tuple<Units, Units, Units>;

    const auto& units() const { return m_units; }

private:
    std::vector<Unit> m_units;
    std::unordered_map<uint64_t, Unit> m_prev_units;
};

}
