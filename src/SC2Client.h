#pragma once
#include "Sc2Session.h"

#include <iostream>
#include <algorithm>
#include <boost/asio.hpp>

#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/process.hpp>

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

class SC2Context
{
public:
    ~SC2Context();
    SC2Context();
    SC2Context(SC2Context&&) = default;

private:
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

struct GameResult
{

};

class SC2Client
{
public:
    ~SC2Client();

    explicit SC2Client(net::io_context& ioc);

    void start();

    bool createGame();

    std::unique_ptr<SC2Context> createContext();

    bool joinGame(std::unique_ptr<Agent> agent);

    //auto actions_response = this->m_session->send(std::make_unique<proto::Request>(acts));

    proto::ResponseObservation step();

    std::optional<proto::PlayerResult> update();

    bool ping();

private:
    boost::process::child m_game_proc;
    net::io_context& m_ioc;
    std::shared_ptr<SC2Session> m_session;
    std::unique_ptr<Agent> m_agent;
    int m_agent_id;
};
}


