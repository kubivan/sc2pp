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

GameInfo get_game_info(const std::shared_ptr<SC2Session>& session);

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

    void start();

    bool createGame();

    template<class T>
    bool joinGame()
    {
        const auto id = joinGame(Race::Protoss);
        if (id == -1)
            return false;
        m_agent = std::make_unique<T>(id, SC2Context(get_game_info(m_session)));

        return m_agent.get();
    }

    std::optional<proto::PlayerResult> update();

    bool ping();

private:
    uint32_t joinGame(Race race);

    std::pair<int, proto::ResponseObservation> step();

    boost::process::child m_game_proc;
    boost::asio::io_context& m_ioc;
    std::shared_ptr<SC2Session> m_session;
    std::unique_ptr<Agent> m_agent;
    int m_prev_loop;
};

}


