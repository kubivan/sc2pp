#pragma once

#include <unordered_map>

#include "Converters.h"

namespace sc2
{

inline auto get_health(const Unit& u)
{
    return u.shield + u.health;
}

class UnitPool
{
public:
    auto update(const std::vector<Unit>& units)
    {
        m_units = units;

        using namespace std::views;
        auto self = units | filter([](auto& x) { return x.alliance == sc2::proto::Self; });

        auto existing = to_vector<Unit>(self | filter([this](const Unit& u) { return m_prev_units.contains(u.tag); }));
        auto new_units = to_vector<Unit>(self | filter([this](const Unit& u) { return !m_prev_units.contains(u.tag); }));

        auto damaged = to_vector<Unit>(existing | filter([this](const Unit& existing) {
            const auto& prev = m_prev_units.at(existing.tag);
            return get_health(existing) < get_health(prev);
            }));

        auto prev_range = self | transform([](auto& x) {return std::make_pair(x.tag, x); });
        m_prev_units = std::unordered_map<uint64_t, Unit>{prev_range.begin(), prev_range.end()};

        //return std::make_tuple(std::move(new_units), to_vector<Unit>(damaged), to_vector<Unit>(self));
        return std::make_tuple(std::move(new_units), std::move(damaged), to_vector<Unit>(self));
    }

    const auto& units() const { return m_units; }

private:
    std::vector<Unit> m_units;
    std::unordered_map<uint64_t, Unit> m_prev_units;


};

}
