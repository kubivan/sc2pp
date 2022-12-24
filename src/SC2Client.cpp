#include "SC2Client.h"
#include "Converters.h"

#include <boost/process/args.hpp>


using namespace sc2;

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

void SC2Client::start()
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

bool SC2Client::createGame()
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

std::unique_ptr<SC2Context> sc2::SC2Client::createContext()
{
    return std::unique_ptr<SC2Context>();
}

bool SC2Client::joinGame(std::unique_ptr<Agent> agent)
{
    auto request = std::make_unique<sc2::proto::Request>();
    auto request_join_game = request->mutable_join_game();
    request_join_game->set_race(agent->race());

    auto options = request_join_game->mutable_options();
    options->set_raw(true);
    options->set_score(true);
    //TODO options->mutable_feature_layer()

    auto response = m_session->send(std::move(request));
    if (!response)
    {
        return false;
    }
    if (!response->has_join_game())
    {
        return false;
    }
    auto response_join_game = response->join_game();
    if (response_join_game.has_error())
    {
        return false;
    }
    m_agent = std::move(agent);
    m_agent_id = response_join_game.player_id();

    return true;
}

proto::ResponseObservation SC2Client::step()
{
    std::cout << "SC2Client::step()" << std::endl;
    static const int step_duration = 1;
    auto request = std::make_unique<proto::Request>();
    auto request_step = request->mutable_step();
    request_step->set_count(step_duration);

    auto step_response = m_session->send(std::move(request));
    if (!step_response || !step_response->has_step())
    {
        throw std::runtime_error("step filed!: " + (step_response ? step_response->DebugString(): ""));
    }

    auto request_observation = std::make_unique<proto::Request>();
    request_observation->mutable_observation();
    auto response = m_session->send(std::move(request_observation));
    if (!response || !response->has_observation())
    {
        std::cout << response->DebugString() << std::endl;
        throw std::runtime_error("no obs in response: " + (response? response->DebugString(): ""));
    }
    return response->observation();
}

std::optional<proto::PlayerResult> SC2Client::update()
{
    const auto response_observation = this->step();
    const auto [units, unit_commands, errors] = convert_observation(response_observation);
    for (auto& u : unit_commands)
    {
        std::cout << "ucomm: " << u.ability_id << std::endl;
    }

    for (auto& e : errors)
    {
        std::cout << "ERROR: " << e.res_str << std::endl;
    }

    auto actions = m_agent->step();
    m_session->send(std::move(actions));

    if (!response_observation.player_result_size())
    {
        return {};
    }
    for(int i = 0; i < response_observation.player_result_size(); ++i)
    {
        const auto& res = response_observation.player_result(i);
        if (res.player_id() == m_agent_id)
        {
            return res;
        }
    }
    assert(false);
    return {};
}

bool SC2Client::ping()
{
    auto request = std::make_unique<proto::Request>();
    request->mutable_ping();
    auto response = m_session->send(std::move(request));
    return response->has_ping();
}

SC2Context::~SC2Context()
{

}
