#include <sc2pp/Map.h>

#include <sc2pp/Observation.h>

#include <utils/GridUtils.h>
#include <utils/UnitQuery.h>

#include <fstream>
#include <functional>

using namespace sc2;
using namespace sc2::utils;

Map::Map(const Observation& obs)
    : m_pathing_grid(obs.game_info().pathing_grid)
    , m_placement_grid(obs.game_info().placement_grid)
    , m_topology(m_pathing_grid.width(), m_pathing_grid.height())
{
    dump_grid(m_pathing_grid, "pathing.txt");
    dump_grid(m_placement_grid, "placement.txt");

    for (int y = 0; y < m_pathing_grid.height(); ++y)
        for (int x = 0; x < m_pathing_grid.width(); ++x)
        {
            m_topology[{x, y}] = m_pathing_grid[{x, y}] ? ' ' : '#';
        }

    //set ramps position
    const auto ramps = unite(m_pathing_grid
        , m_placement_grid
        , [](bool pathing, bool placement) -> bool 
        {
            return pathing && !placement;
        });
    for (const auto& r : ramps)
    {
        m_topology[r] = '/';
    }

    //using namespace sc2::utils;
    //const auto is_mineral = type(UNIT_TYPEID::NEUTRAL_MINERALFIELD) || type(UNIT_TYPEID::NEUTRAL_MINERALFIELD750);
    //const auto resources_around_probe = obs->GetUnits(is_resource && in_radius(probe_pos, 10 ));

    using namespace std::views;

    auto minerals = obs.units() | filter([](const Unit& u) {
        return u.unit_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD
        || u.unit_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD750
        || u.unit_type == UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD;
        });

    for (const auto& m : minerals)
    {
        apply_footprint(m_topology, get_tile_pos(m.pos), get_footprint(m.unit_type), '*');
    }

    for (const auto& g : obs.units() | filter(is_geyser))
    {
        apply_footprint(m_topology, get_tile_pos(g.pos), get_footprint(g.unit_type), '$');
    }

    auto nexuses = obs.units() | filter(type(UNIT_TYPEID::PROTOSS_NEXUS));
    for (const auto& n : nexuses)
    {
        apply_footprint(m_topology, get_tile_pos(n.pos), get_footprint(n.unit_type), 'n');
    }

    dump_grid(m_topology, "topology.txt");
}

void
Map::place_building(const Unit& u, char mark /*= 'b'*/)
{
    if (!sc2::utils::is_building_type(u.unit_type))
    {
        return;
    }
    sc2::utils::place_building(m_topology, u, mark);
    dump_grid(m_topology, "topology.txt");
}
