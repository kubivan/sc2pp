#include <sc2pp/Actions.h>

#include <variant>

using namespace sc2;

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

std::unique_ptr<proto::Request> Actions::reset()
{
    if (!m_request)
    {
        return std::make_unique<proto::Request>();
    }
    return std::move(m_request);
}

proto::RequestAction* Actions::requestAction() {
    if (!m_request) {
        m_request = std::make_unique<proto::Request>();
    }
    return m_request->mutable_action();
}

Actions& Actions::toggleAutocast(Tag unit_tag, AbilityID ability)
{
    toggleAutocast(std::vector<Tag>{ unit_tag }, ability);
    return *this;
}

Actions& Actions::toggleAutocast(const std::vector<Tag>& unit_tags, AbilityID ability)
{
    auto request_action = requestAction();
    auto action = request_action->add_actions();
    auto action_raw = action->mutable_action_raw();
    auto autocast = action_raw->mutable_toggle_autocast();
    for (const auto& u : unit_tags) {
        autocast->add_unit_tags(u);
    }
    autocast->set_ability_id(static_cast<int>(ability));
    return *this;
}

Actions& Actions::chat(const std::string& message, proto::ActionChat::Channel channel)
{
    auto request_action = requestAction();
    auto action = request_action->add_actions();
    auto action_chat = action->mutable_action_chat();
    action_chat->set_message(message);

    action_chat->set_channel(channel);
    return *this;
}

Actions& Actions::command(const Unit& unit, AbilityID ability, bool queued)
{
    return command(Units{ unit }, ability, queued);
}

Actions& Actions::command(const Unit& unit, AbilityID ability, const Point2D& point, bool queued)
{
    return command(Units{ unit }, ability, point, queued);
}

Actions& Actions::command(const Unit& unit, AbilityID ability, const Unit& target, bool queued)
{
    return command(Units{ unit }, ability, target, queued);
}

Actions& Actions::command(const Units& units, AbilityID ability, bool queued)
{
    auto request_action = requestAction();
    auto action = request_action->add_actions();
    auto action_raw = action->mutable_action_raw();
    auto unit_command = action_raw->mutable_unit_command();

    unit_command->set_ability_id(static_cast<int>(ability));
    unit_command->set_queue_command(queued);

    for (const auto& unit : units)
    {
        unit_command->add_unit_tags(unit.tag);
    }
    return *this;
}

Actions& Actions::command(const Task& task, bool queued)
{
    auto request_action = requestAction();
    auto action = request_action->add_actions();
    auto action_raw = action->mutable_action_raw();
    auto unit_command = action_raw->mutable_unit_command();

    unit_command->set_ability_id(static_cast<int>(task.action));
    unit_command->add_unit_tags(task.executor);

    if (!task.target)
    {
        return *this;
    }

    std::visit(overloaded{
        [&unit_command](Point2D point) {
            auto target_point = unit_command->mutable_target_world_space_pos();
            target_point->set_x(point.x);
            target_point->set_y(point.y);

        },
        [&unit_command](Tag target) {
            unit_command->set_target_unit_tag(target);
        }
    }
    , task.target.value());

    unit_command->set_queue_command(queued);

    return *this;
}

Actions& Actions::command(const Units& units, AbilityID ability, const Point2D& point, bool queued)
{
    auto request_action = requestAction();
    auto action = request_action->add_actions();
    auto action_raw = action->mutable_action_raw();
    auto unit_command = action_raw->mutable_unit_command();

    unit_command->set_ability_id(static_cast<int>(ability));
    auto target_point = unit_command->mutable_target_world_space_pos();
    target_point->set_x(point.x);
    target_point->set_y(point.y);
    unit_command->set_queue_command(queued);

    for (const auto& unit : units)
    {
        unit_command->add_unit_tags(unit.tag);
    }
    return *this;
}

Actions& Actions::command(const Units& units, AbilityID ability, const Unit& target, bool queued) {
    auto request_action = requestAction();
    auto action = request_action->add_actions();
    auto action_raw = action->mutable_action_raw();
    auto unit_command = action_raw->mutable_unit_command();

    unit_command->set_ability_id(static_cast<int>(ability));
    unit_command->set_target_unit_tag(target.tag);
    unit_command->set_queue_command(queued);

    for (const auto& unit : units)
    {
        unit_command->add_unit_tags(unit.tag);
    }

    return *this;
}
