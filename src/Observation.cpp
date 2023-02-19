#include "Observation.h"

#include "Converters.h"

using namespace sc2;

GameInfo sc2::from_proto(const proto::ResponseGameInfo& x)
{
    return GameInfo{
    .map_name = x.map_name(),
    .local_map_path = x.local_map_path(),
    .map_size = { x.start_raw().map_size().x(), x.start_raw().map_size().y() },

    .pathing_grid = x.start_raw().pathing_grid(),
    .terrain_height = x.start_raw().terrain_height(),
    .placement_grid = x.start_raw().placement_grid(),
    .start_locations = to_vector<Point2D>(x.start_raw().start_locations() | std::views::transform(BOOST_HOF_LIFT(from_proto)))
    };
}

Observation::Observation(GameInfo game_info
    , std::vector<UnitTypeData> unit_type_data
    , std::vector<AbilityData> ability_data)
    : m_game_info(std::move(game_info))
    , m_unit_type_data(std::move(unit_type_data))
    , m_ability_data(std::move(ability_data))
{

}

void Observation::update(const proto::ResponseObservation& response_obs)
{
    const auto [units, unit_commands, errors] = convert_observation(response_obs);

    std::tie(m_created, m_damaged, m_self) = m_units.update(units);
    m_raw = response_obs;
}

auto Observation::raw() const -> const proto::Observation&
{
    return m_raw.observation();
}

auto Observation::unitsSelf() const -> const Units&
{
    return m_self;
}

auto Observation::unitsCreated() const -> const Units&
{
    return m_created;
}

const GameInfo& Observation::gameInfo() const
{
    return m_game_info;
}

auto Observation::units() const -> const Units&
{
    return m_units.units();
}

const std::vector<AbilityData>& Observation::abilityData() const
{
    return m_ability_data;
}

const std::vector<UnitTypeData>& Observation::unitsTypeData() const
{
    return m_unit_type_data;
}
