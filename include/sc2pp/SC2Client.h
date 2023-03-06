#pragma once

#include <iostream>
#include <algorithm>
#include <boost/asio.hpp>

#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/process.hpp>

#include <sc2pp/Actions.h>

#include <sc2pp/SC2Context.h>

namespace sc2
{
class SC2Session;

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

class Agent;

const 

class SC2Client
{
public:
    ~SC2Client();

    explicit SC2Client(boost::asio::io_context& ioc);

    auto start() -> void;

    auto createGame() -> bool;

    template<class T>
    auto joinGame() -> bool
    {
        const auto id = joinGame(Race::Protoss);
        if (id == -1)
            return false;
        m_agent = std::make_unique<T>(id, createContext());

        return m_agent.get();
    }

    auto update() -> std::optional<proto::PlayerResult>;

    auto ping() -> bool;

private:
    auto createContext() -> SC2Context;

    auto joinGame(Race race) -> uint32_t;

    auto step() -> std::pair<int, proto::ResponseObservation>;

    boost::process::child m_game_proc;
    boost::asio::io_context& m_ioc;
    std::shared_ptr<SC2Session> m_session;
    std::unique_ptr<Agent> m_agent;
    int m_prev_loop;
};

}
