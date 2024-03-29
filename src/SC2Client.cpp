#include <sc2pp/Agent.h>
#include <sc2pp/SC2Client.h>

#include "Sc2Session.h"

#include "Converters.h"

#include <boost/process/args.hpp>
#include <boost/pfr.hpp>

using namespace sc2;

auto get_game_info(const std::shared_ptr<SC2Session>& session) -> GameInfo
{
    auto request = std::make_unique<proto::Request>();
    auto request_create_game = request->mutable_create_game();
    request->mutable_game_info();

    auto game_info = session->send(std::move(request));
    if (!game_info)
    {
        throw std::runtime_error("get_game_info failed");
    }

    return from_proto(game_info->game_info());
}

auto get_unit_type_data(const std::shared_ptr<SC2Session>& session) -> std::vector<UnitTypeData>
{
    auto request = std::make_unique<proto::Request>();
    auto request_data = request->mutable_data();
    request_data->set_unit_type_id(true);

    auto response = session->send(std::move(request));
    if (!response)
    {
        throw std::runtime_error("get_unit_type_data failed");
    }

    auto response_data = response->data();

    return to_vector<proto::UnitTypeData>(response_data.units());
}

//TODO: merge functions
auto get_ability_data(const std::shared_ptr<SC2Session>& session) -> std::vector<AbilityData>
{
    auto request = std::make_unique<proto::Request>();
    auto request_data = request->mutable_data();
    request_data->set_ability_id(true);

    auto response = session->send(std::move(request));
    if (!response)
    {
        throw std::runtime_error("get_unit_type_data failed");
    }

    auto response_data = response->data();

    return to_vector<proto::AbilityData>(response_data.abilities());
}

SC2Client::~SC2Client()
{
    m_ioc.stop();
    m_game_proc.terminate();
}

SC2Client::SC2Client(net::io_context& ioc)
    : m_session(std::make_shared<SC2Session>(ioc))
    , m_ioc(ioc)
{
}

auto SC2Client::start() -> void
{
    auto process_path = "C:\\Program Files (x86)\\StarCraft II\\Versions\\Base87702\\SC2_x64.exe";

    namespace bp = boost::process; //we will assume this for all further examples
    m_game_proc = bp::child(process_path
        , bp::args = { "-listen","127.0.0.1"
        , "-port" , "8167"
        , "-displayMode" , "0"
        , "-windowwidth" , "1024"
        , "-windowheight" , "768"
        , "-windowx" , "100"
        , "-windowy" , "200" }
        , bp::std_in.close(),
        //bp::std_out > bp::null, //so it can be written without anything
        //bp::std_err > data,
        bp::start_dir = "C:\\Program Files (x86)\\StarCraft II\\Support64"
    );

    //////////////////////////////////////////////////////
    std::cout << "sc2.exe launched" << std::endl;
    for (int retry = 1; retry < 10; ++retry)
    {
        auto ec = m_session->connect("127.0.0.1", "8167");
        if (!ec)
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

auto SC2Client::createGame() -> bool
{
    std::cout << "creating game" << std::endl;
    auto request = std::make_unique<proto::Request>();
    auto request_create_game = request->mutable_create_game();
    //setup map
    auto map = request_create_game->mutable_local_map();
    map->set_map_path("ai/2000AtmospheresAIE.SC2Map");

    //players
    //TODO: for(auto p: players)
    //DOC: should be at least one participant
    for (auto type : { proto::PlayerType::Computer, proto::PlayerType::Participant })
    {
        auto player_setup = request_create_game->add_player_setup();
        player_setup->set_type(type);
        player_setup->set_race(proto::Race::Protoss); //TODO: type
        player_setup->set_difficulty(proto::Difficulty::Hard);
    }

    //realtime
    request_create_game->set_realtime(false);

    auto response = m_session->send(std::move(request));
    if (!response && !response->has_create_game())
    {
        std::cout << "Recieved invalid response" << std::endl;
        return false;
    }
    std::cout << "Resp " << response->create_game().DebugString() << std::endl;
    //assert(response.has_create_game());
    auto create_game = response->create_game();
    if (create_game.has_error())
    {
        std::cout << "Create game error : "
            << ResponseCreateGame_Error_Name(create_game.error()) << std::endl;
        return false;
    }
    return true;
}

auto SC2Client::joinGame(Race race) -> uint32_t
{
    auto request = std::make_unique<sc2::proto::Request>();
    auto request_join_game = request->mutable_join_game();
    request_join_game->set_race(race);

    auto options = request_join_game->mutable_options();
    options->set_raw(true);
    options->set_score(true);
    //TODO options->mutable_feature_layer()

    auto response = m_session->send(std::move(request));
    if (!response)
    {
        return -1;
    }
    if (!response->has_join_game())
    {
        return -1;
    }
    auto response_join_game = response->join_game();
    if (response_join_game.has_error())
    {
        return -1;
    }
    return response_join_game.player_id();
}

auto sc2::SC2Client::createContext() -> SC2Context 
{
    return SC2Context(
          Observation(get_game_info(m_session), get_unit_type_data(m_session), get_ability_data(m_session))
        , Query(m_session)
    );
}

auto SC2Client::step() -> std::pair<int, proto::ResponseObservation>
{
    //std::cout << "SC2Client::step()" << std::endl;
    static const int step_duration = 1;
    auto request = std::make_unique<proto::Request>();
    auto request_step = request->mutable_step();
    request_step->set_count(step_duration);

    auto step_response = m_session->send(std::move(request));
    if (!step_response || !step_response->has_step())
    {
        throw std::runtime_error("step filed!: " + (step_response ? step_response->DebugString(): ""));
    }
    auto simulation_loop = step_response->step().simulation_loop();

    auto request_observation = std::make_unique<proto::Request>();
    request_observation->mutable_observation();
    auto response = m_session->send(std::move(request_observation));
    if (!response || !response->has_observation())
    {
        std::cout << response->DebugString() << std::endl;
        throw std::runtime_error("no obs in response: " + (response? response->DebugString(): ""));
    }
    return { simulation_loop, response->observation() };
}

auto SC2Client::update() -> std::optional<proto::PlayerResult>
{
    const auto [simulation_loop, response_observation] = this->step();

    //for (auto& u : units)
    //{
    //    std::cout << "unit: " << boost::pfr::io(u) << std::endl;
    //}
    //std::cout << "============================================" << std::endl;
    //const auto [created, damaged, self] = m_unit_pool.update(units);
    //if(created.size() + damaged.size() > 0)
    //    std::cout << "SC2Client::update: " << simulation_loop << std::endl;
    //for(auto& c: created)
    //{
    //    std::cout << "unit created: " << c.tag << " " << c.shield << " " << c.health << std::endl;
    //}

    //for(auto& d: damaged)
    //{
    //    std::cout << "unit damaged: " << d.tag << " " << d.shield << " " << d.health << std::endl;
    //}

    //for (auto& e : errors)
    //{
    //    std::cout << "ERROR: " << e.res_str << std::endl;
    //}

    auto actions = m_agent->step(response_observation);
    m_session->send(std::move(actions));

    if (!response_observation.player_result_size())
    {
        return {};
    }
    for(int i = 0; i < response_observation.player_result_size(); ++i)
    {
        const auto& res = response_observation.player_result(i);
        if (res.player_id() == m_agent->id())
        {
            return res;
        }
    }

    m_prev_loop = simulation_loop;

    assert(false);
    return {};
}

auto SC2Client::ping() -> bool
{
    auto request = std::make_unique<proto::Request>();
    request->mutable_ping();
    auto response = m_session->send(std::move(request));
    return response->has_ping();
}

