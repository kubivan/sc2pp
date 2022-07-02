#include "Sc2Session.h"

#include <iostream>
#include <algorithm>
#include <boost/process.hpp>
#include <boost/process/args.hpp>
#include <boost/asio.hpp>

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>

#include <rxcpp/rx.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

namespace sc2
{
////! Setup for a player in a game.
//struct PlayerSetup {
//    //! Player can be a Participant (usually an agent), Computer (in-built AI) or Observer.
//    PlayerType type;
//    //! Agent, if one is available.
//    Agent* agent;
//
//    // Only used for Computer
//
//    //! Race: Terran, Zerg or Protoss. Only for playing against the built-in AI.
//    Race race;
//    //! Difficulty: Only for playing against the built-in AI.
//    Difficulty difficulty;
//};

class SC2Client
{
public:
    ~SC2Client()
    {
    }

    rxcpp::subjects::behavior<int> m_game_step = rxcpp::subjects::behavior<int>(0);

    SC2Client(net::io_context& ioc)
        : m_session(std::make_shared<Sc2Session>(ioc))
    {


    }

    void start()
    {
        for (int retry = 1; retry < 10; ++retry)
        {
            auto client_started_future = m_session->run("127.0.0.1", "8167");
            if (client_started_future.get())
            {
                break;
            }
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }

    auto createGame()
    {
        std::cout << "creating game" << std::endl;
        auto request = std::make_shared<proto::Request>();
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

        return m_session->send(request, [](auto response)
            {
                if (!response->has_create_game())
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
            });
    }

    auto joinGame()
    {
        auto request = std::make_shared<sc2::proto::Request>();
        auto request_join_game = request->mutable_join_game();
        request_join_game->set_race(sc2::proto::Race::Protoss);
        
        auto options = request_join_game->mutable_options();
        options->set_raw(true);
        options->set_score(true);
        //TODO options->mutable_feature_layer()

        return m_session->send(request, [](auto response) 
            {
                if (!response->has_create_game())
                {
                    return false;
                }
                auto response_create_game = response->create_game();
                if (response_create_game.has_error())
                {
                    return false;
                }
                return true;
            });

    }

    auto step()
    {
        static const int step_duration = 1;
        auto request = std::make_shared<sc2::proto::Request>();
        auto request_step = request->mutable_step();
        request_step->set_count(step_duration);

        m_session->send(std::move(request), [this](const auto response) {
            return true; 
            }).wait();

        int loop;
        {
            auto request = std::make_shared<sc2::proto::Request>();
            request->mutable_observation();
            m_session->send(std::move(request), [&](auto response_observation) {
                //m_observation = std::move(response_observation->observation());
                loop = response_observation->observation().observation().game_loop();
                return true;
                }).wait();
        }
        return loop;
    }

    //auto updateObservation()
    //{
    //    auto request = std::make_shared<sc2::proto::Request>();
    //    request->mutable_observation();
    //    return m_session->send(std::move(request), [this](auto response_observation) {
    //        //m_observation = std::move(response_observation->observation());
    //        return true;
    //        });
    //}

    void ping()
    {
        auto request = std::make_shared<proto::Request>();
        request->mutable_ping();
        m_session->send(std::move(request), [](auto response) 
            {
                return response->has_ping();
            }).wait();
    }

    auto createObserverable()
    {
        return m_game_step.get_observable().observe_on(rxcpp::synchronize_new_thread()).map([this](const int step) {
            std::cout << std::this_thread::get_id() << "step: "<< step << std::endl;
                return this->step();
                //this->updateObservation().wait();
                //return this->m_observation.observation().game_loop();
            });
    }

private:
    net::io_context m_ioc;
    std::shared_ptr<Sc2Session> m_session;
    //rxcpp::ob
public:
    //sc2::proto::ResponseObservation m_observation;
    int loop;
};
}

struct Observation
{

};

struct SC2Context
{
    Observation obs;
    //Actions act;

};

struct Agent
{
    //input
    rxcpp::observable<sc2::proto::ResponseObservation> obs;

    rxcpp::observable<sc2::proto::Unit> unitsCreated;
    rxcpp::observable<sc2::proto::Unit> unitsDestroyed;


    void step()
    {
        //step_impl();

    }

private:
    virtual void step_impl(SC2Context& sc2) = 0 {};

    //output
    //rxcpp::observable<sc2::proto::RequestAction*> obs;
};


#if 0
int main(int argc, char** argv)
{
    //---------------- Generate a range of values //---------------- Apply Square function
    auto values = rxcpp::observable<>::range(1,4).
        map([](int v){ return v*v;}); //------------- Emit the current thread details
    std::cout << "Main Thread id => "
        << std::this_thread::get_id() << std::endl;

    //---------- observe_on another thread....
    //---------- make it blocking to
    values.observe_on(rxcpp::synchronize_new_thread()).as_blocking().
        subscribe([](int v) {
        std::cout << "Observable Thread id => "
            << std::this_thread::get_id() << " " << v << std::endl; }, []() { std::cout <<
            "OnCompleted" << std::endl; }); //------------------ Print the main thread details
    std::cout << "Main Thread id => "
        << std::this_thread::get_id() << std::endl;


}
#else
int main(int argc, char** argv)
{
    auto process_path = "C:\\Program Files (x86)\\StarCraft II\\Versions\\Base84643\\SC2_x64.exe";

    /// <summary>
    /// Start process
    /// </summary>
    /// <param name="argc"></param>
    /// <param name="argv"></param>
    /// <returns></returns>
    namespace bp = boost::process; //we will assume this for all further examples
    auto sc2_proc = bp::child(process_path
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

    net::io_context ioc;
    auto guard = boost::asio::make_work_guard(ioc);
    auto io_thread = std::jthread{ [&] {ioc.run(); } };

    sc2::SC2Client client(ioc);
    client.start();
    client.ping();

    std::cout << "client started" << std::endl;
    auto created = client.createGame().get();
    std::cout << "game created: " << created << std::endl;
    if (!created)
    {
        //goto err;
        return -1;
    }

    std::cout << "joining" << std::endl;
    client.joinGame().wait();

    client.createObserverable()
        //.observe_on(rxcpp::synchronize_new_thread())
        //.as_blocking()
        .subscribe(
        [](const auto& some) {std::cout << "observe: " << some << std::endl; },
        []() {std::cout << "onComplete" << std::endl; }
    );
    //auto game_loop = response_observation.map([](const sc2::proto::ResponseObservation& resp) {return resp.observation().game_loop(); });
    //auto game_loop_10 = game_loop | rxcpp::operators::filter([](const auto& loop) { return loop % 10 == 0; });
    //game_loop_10.subscribe([](const auto& loop) { std::cout << "loop: " << loop << std::endl; });
    while (true)
    {
        static int steps = 0;
        //client.step().wait();
        //std::cout << "minerals count: " << client.m_observation.observation().player_common().minerals() << std::endl;
        client.m_game_step.get_subscriber().on_next(++steps);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

err:
    std::cout << "stopping ioc" << std::endl;
    ioc.stop();
    std::cout << "waiting for sc2.exe close" << std::endl;
    sc2_proc.wait();
    return 0;
}
#endif
