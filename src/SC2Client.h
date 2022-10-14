#pragma once
#include "Sc2Session.h"

#include <iostream>
#include <algorithm>
#include <boost/asio.hpp>

#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/process.hpp>

#include <rxcpp/rx.hpp>
#include <rxcpp/operators/rx-all.hpp>

#include "Actions.h"

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

class SC2Client;

class SC2Context : public std::enable_shared_from_this<SC2Context>
{
public:
    ~SC2Context();
    SC2Context();
    SC2Context(SC2Context&&) = default;

    void init(const rxcpp::observable<int>& input, SC2Client* client);

    rxcpp::synchronize_in_one_worker worker;

    rxcpp::observable<int> game_step;

    rxcpp::subjects::replay<proto::ResponseObservation, rxcpp::synchronize_in_one_worker> obs;

    rxcpp::observable<std::vector<proto::Unit>> units;
    rxcpp::observable<std::vector<proto::Unit>> units_created;
private: 
    rxcpp::composite_subscription obs_subsctiption;
    std::set<proto::Unit> m_units;
};

struct Agent
{
    virtual std::unique_ptr<proto::Request> step() = 0;
    virtual proto::Race race() = 0;
};

auto make_set_adapter(const proto::ObservationRaw& obs)
{
    auto cmp_prev = [&obs](const auto& a, const auto& b) {return obs.units(a).tag() < obs.units(b).tag(); };
    std::set<int, decltype(cmp_prev)> res(cmp_prev);
    for (int i = 0; i < obs.units_size(); ++i)
        res.insert(i);
    return res;
}

struct TestAgent : public Agent
{
    rxcpp::subjects::subject<proto::Unit> m_units_created;
    TestAgent(std::shared_ptr<SC2Context> context)
        : m_sc2(std::move(context))
        , m_actions(std::make_unique<Actions>())
    {
        //m_sc2.obs.subscribe_on(m_sc2.worker).subscribe([this](const auto& obs) { 
        //    std::cout << std::this_thread::get_id() << "TestAgent subscriptions got " << obs.observation().game_loop() << std::endl;
        //    const auto& units = obs.observation().raw_data().units();
        //    auto nexus = std::find_if(units.cbegin(), units.cend(), 
        //        [](const auto& u) {return u.unit_type() == (int)sc2::UNIT_TYPEID::PROTOSS_NEXUS; });
        //    if (nexus != units.cend() && nexus->orders_size() == 0)
        //    {
        //        m_actions->command(*nexus, sc2::AbilityID::TRAIN_PROBE );
        //        m_actions->chat("HEY! creating a probe on" + std::to_string(obs.observation().game_loop())); 
        //        cv.notify_one();
        //    }
        //});
    }

    rxcpp::observable<proto::Request> getActionsObs()
    {
        return m_sc2->obs.get_observable().map([this](const auto& obs) {
            std::cout << std::this_thread::get_id() << "TestAgent subscriptions got " << obs.observation().game_loop() << std::endl;
            const auto& units = obs.observation().raw_data().units();
            auto nexus = std::find_if(units.cbegin(), units.cend(),
                [](const auto& u) {return u.unit_type() == (int)sc2::UNIT_TYPEID::PROTOSS_NEXUS; });
            if (nexus != units.cend() && nexus->orders_size() == 0)
            {
                m_actions->command(*nexus, sc2::AbilityID::TRAIN_PROBE);
                m_actions->chat("HEY! creating a probe on" + std::to_string(obs.observation().game_loop()));
            }
            return *m_actions->reset();
            });
    }

        //m_units_created.get_observable().observe_on(m_sc2.worker).subscribe([this](const auto& created) {
        //    std::cout << "created: " << created.tag() << std::endl;
        //    m_actions->chat("Probe created"); 
        //    cv.notify_one();
        //    });

        //auto obs_diff = m_sc2.obs.observe_on(m_sc2.worker).map([](const auto obs_resp) {return obs_resp.observation().raw_data(); }) | rxcpp::operators::pairwise();
        //obs_diff.subscribe_on(m_sc2.worker).subscribe([this](const auto& obs_pair) {
        //    const auto& [prev, current] = obs_pair;
        //    const auto prev_units = make_set_adapter(prev);
        //    const auto current_units = make_set_adapter(current);
        //    for (const auto& c : current_units)
        //    {
        //        if (!prev_units.contains(c))
        //        {
        //            this->m_units_created.get_subscriber().on_next(current.units(c));
        //        }
        //    }
        //    }, [](std::exception_ptr ep)
        //    {
        //        std::cout << "subscribe error: " <<  rxcpp::util::what(ep) << std::endl;
        //    });

    std::unique_ptr<proto::Request> step() override;

    proto::Race race() override
    {
        return proto::Race::Protoss;
    }

    std::shared_ptr<SC2Context> m_sc2;
    std::mutex m;
    std::condition_variable cv;
    rxcpp::observable<std::unique_ptr<Actions>> m_actions_obs;

    std::unique_ptr<Actions> m_actions;
};

class SC2Client
{
public:
    ~SC2Client();

    explicit SC2Client(net::io_context& ioc);

    void start();

    bool createGame();

    bool joinGame(std::unique_ptr<TestAgent> agent);

    int m_steps = 0;
    void startGame()
    {
        std::cout << "starting the bot" << std::endl;
        static auto actions = this->m_agent->getActionsObs();

        actions.subscribe([&](auto acts)
            {
                std::cout << std::this_thread::get_id() << " actions.subscribe() " << std::endl;
                if (acts.has_action())
                {
                    auto actions_response = this->m_session->send(std::make_unique<proto::Request>(acts));
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                std::cout << std::this_thread::get_id() << " stepping " << std::endl;
                this->m_game_step.get_subscriber().on_next(++m_steps);
            });

    }

    bool update()
    {
        return true;
    }

    proto::ResponseObservation step();

    bool ping();

    std::shared_ptr<SC2Context> createContext();

private:
    boost::process::child m_game_proc;
    net::io_context m_ioc;
    std::shared_ptr<SC2Session> m_session;
    rxcpp::subjects::behavior<int> m_game_step = rxcpp::subjects::behavior<int>(-1);
public:
    std::unique_ptr<TestAgent> m_agent;
};
}


