#include "Converters.h"

namespace sc2
{

auto from_proto(const proto::Point2D& input) -> Point2D
{
    return Point2D{ input.x(), input.y() };
}

auto from_proto(const proto::ActionRawUnitCommand& unit_command) -> UnitCommand
{
    UnitCommand result;
    result.ability_id = unit_command.ability_id();
    switch (unit_command.target_case())
    {
    case proto::ActionRawUnitCommand::kTargetUnitTag:
        result.target = unit_command.target_unit_tag();
    case proto::ActionRawUnitCommand::kTargetWorldSpacePos:
        result.target = from_proto(unit_command.target_world_space_pos());
    default:
        break;
    }

    return result;
}

auto from_proto(const proto::ActionError& x) -> ActionError
{
    const google::protobuf::EnumDescriptor* descriptor = proto::ActionResult_descriptor();

    return ActionError{
        .unit_tag = x.unit_tag(),
        .ability_id = x.ability_id(),
        .result = x.result(),
        .res_str = descriptor->full_name()
    };
}

auto from_proto(const proto::Point& x) -> Point3D
{
    return Point3D{ .x = x.x(), .y = x.y(), .z = x.z() };
}

auto from_proto(const proto::UnitOrder& x)
{
    assert(x.ability_id());
    return UnitOrder{
        .ability_id = AbilityID(x.ability_id()),
        .target = x.has_target_world_space_pos() ?
        std::variant<Point3D, uint64_t>{from_proto(x.target_world_space_pos())}
    : std::variant<Point3D, uint64_t>{ x.target_unit_tag() }

    };
}

auto from_proto(const proto::PassengerUnit& x)
{
    return PassengerUnit{
        .tag = x.tag(),
        .health = x.has_health() ? x.health() : 0,
        .health_max = x.has_health_max() ? x.health_max() : 0,
        .shield = x.has_shield() ? x.shield() : -1,
        .shield_max = x.has_shield_max() ? x.shield_max() : -1,
        .energy = x.has_energy() ? x.energy() : -1,
        .energy_max = x.energy_max() ? x.energy_max() : -1,
        .unit_type = x.has_unit_type() ? x.unit_type() : -1,
    };
}

auto from_proto(const proto::RallyTarget& x)
{
    return RallyTarget{
        .point = from_proto(x.point()),
        .tag = x.has_tag() ? std::optional<int64_t>{x.tag()} : std::nullopt
    };
}

auto from_proto(const proto::Unit& x) -> Unit
{
    using namespace std::views;
    return Unit{
        .display_type = x.display_type(),
        .alliance = x.alliance(),
        .tag = x.tag(),                  // Unique identifier for a unit
        .unit_type = UNIT_TYPEID(x.unit_type()),
        .owner = x.owner(),
        .pos = from_proto(x.pos()),
        .facing = x.facing(),
        .radius = x.radius(),
        .build_progress = x.build_progress(),        // Range: [0.0, 1.0]
        .cloak = x.cloak(),
        .buff_ids = to_vector<uint32_t>(x.buff_ids()),

        .detect_range = x.detect_range(),
        .radar_range = x.radar_range(),

        .is_selected = x.is_selected(),
        .is_on_screen = x.is_on_screen(),          // Visible and within the camera frustrum.
        .is_blip = x.is_blip(),                    // Detected by sensor tower
        .is_powered = x.is_powered(),
        .is_active = x.is_active(),                // Building is training/researching (ie animated).
        .attack_upgrade_level = x.attack_upgrade_level(),
        .armor_upgrade_level = x.armor_upgrade_level(),
        .shield_upgrade_level = x.shield_upgrade_level(),

        .health = x.health(),
        .health_max = x.health_max(),
        .shield = x.shield(),
        .shield_max = x.shield_max(),
        .energy = x.energy(),
        .energy_max = x.energy_max(),
        .mineral_contents = x.mineral_contents(),
        .vespene_contents = x.vespene_contents(),
        .is_flying = x.is_flying(),
        .is_burrowed = x.is_burrowed(),
        .is_hallucination = x.is_hallucination(),      // Unit is your own or detected as a hallucination.

        .orders = to_vector<UnitOrder>(all(x.orders()) | transform(BOOST_HOF_LIFT(from_proto))),
        .add_on_tag = x.has_add_on_tag() ? x.add_on_tag() : -1,
        .passengers = to_vector<PassengerUnit>(all(x.passengers()) | transform(BOOST_HOF_LIFT(from_proto))),
        .cargo_space_taken = x.has_cargo_space_taken() ? x.cargo_space_taken() : 0,
        .cargo_space_max = x.has_cargo_space_max() ? x.cargo_space_max() : 0,
        .assigned_harvesters = x.has_assigned_harvesters() ? x.assigned_harvesters() : -1,
        .ideal_harvesters = x.has_ideal_harvesters() ? x.ideal_harvesters() : -1,
        .weapon_cooldown = x.has_weapon_cooldown() ? x.weapon_cooldown() : -1,
        .engaged_target_tag = x.has_engaged_target_tag() ? x.engaged_target_tag() : -1,
        .buff_duration_remain = x.has_buff_duration_remain() ? x.buff_duration_remain() : -1,  // How long a buff or unit is still around (eg mule, broodling, chronoboost).
        .buff_duration_max = x.has_buff_duration_max() ? x.buff_duration_max() : -1,    // How long the buff or unit is still around (eg mule, broodling, chronoboost). 
        .rally_targets = to_vector<RallyTarget>(all(x.rally_targets()) | transform(BOOST_HOF_LIFT(from_proto)))
    };
}


auto convert_observation(const proto::ResponseObservation& response_observation) -> std::tuple< 
    std::vector<Unit>, std::vector<UnitCommand>, std::vector<ActionError> >
{
    using namespace std::views;
    using namespace boost;
    using namespace proto;

    auto actions_raw = all(response_observation.actions())
        | filter(&Action::has_action_raw)
        | transform(&Action::action_raw)
        | filter(&ActionRaw::has_unit_command)
        | transform(hof::proj(&ActionRaw::unit_command, BOOST_HOF_LIFT(from_proto)));

    const auto actions_chat = filter(&Action::has_action_chat) | transform(&Action::action_chat);

    auto units = all(response_observation.observation().raw_data().units()) | transform(BOOST_HOF_LIFT(from_proto));

    const auto errors = all(response_observation.action_errors()) | transform(BOOST_HOF_LIFT(from_proto));

    return std::make_tuple(to_vector<Unit>(units)
        , to_vector<UnitCommand>(actions_raw)
        , to_vector<ActionError>(errors)
    );
}

}
