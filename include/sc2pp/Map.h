#pragma once

#include <sc2pp/utils/Grid.h>

namespace sc2
{
class Observation;

class Map
{
public:
    Map(const Observation& obs);
    void place_building(const Unit& u, char mark = 'b');

private:
    utils::Grid<bool> m_pathing_grid;
    utils::Grid<uint8_t> m_placement_grid;
    utils::Grid<char> m_topology;
};

} // namespace sc2
