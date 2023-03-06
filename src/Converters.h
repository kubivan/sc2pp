#pragma once

#include <sc2pp/Common.h>
#include <sc2pp/utils/Grid.h>

#include <vector>
#include <ranges>
#include <variant>
#include <optional>

#include <s2clientprotocol/error.pb.h>
#include <google/protobuf/descriptor.h>

#include <boost/hof.hpp>
#include <boost/hof/proj.hpp>

//TODO: move types to Types.h
namespace sc2 {
namespace proto = SC2APIProtocol;

auto from_proto(const proto::Point2D& input) -> Point2D;

struct UnitCommand
{
    int ability_id;
    std::variant<Point2D, std::uint64_t> target;
};

auto from_proto(const proto::ActionRawUnitCommand& unit_command) -> UnitCommand;

//message ResponseObservation {
//  repeated Action actions = 1;                  // Actions this player did since the last Observation.
//  repeated ActionError action_errors = 2;       // Equivalent of UI "red text" errors.
//  optional Observation observation = 3;
//  repeated PlayerResult player_result = 4;      // Only populated if the game ended during this step.
//  repeated ChatReceived chat = 5;
//}
//message ActionError {
//  optional uint64 unit_tag = 1;                             // Only populated when using raw interface.
//  optional uint64 ability_id = 2;
//  optional ActionResult result = 3;
//}

struct ActionError
{
    uint64_t unit_tag;
    uint64_t ability_id;
    proto::ActionResult result;
    std::string res_str;
};

auto from_proto(const proto::ActionError& x) -> ActionError;

auto from_proto(const proto::Point& x) -> Point3D;

auto from_proto(const proto::UnitOrder& x);

auto from_proto(const proto::PassengerUnit& x);

auto from_proto(const proto::RallyTarget& x);

Unit from_proto(const proto::Unit& x);

auto convert_observation(const proto::ResponseObservation& response_observation) -> std::tuple< std::vector<Unit>, std::vector<UnitCommand>, std::vector<ActionError> >;

}
