#pragma once

#include <iostream>
#include <algorithm>
#include <boost/asio.hpp>

#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/process.hpp>

#include "Actions.h"

#include "SC2Context.h"

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
        return joinGame(std::make_unique<T>(SC2Context(get_game_info(m_session))));
    }

    std::optional<proto::PlayerResult> update();

    bool ping();

private:
    bool joinGame(std::unique_ptr<Agent> agent);

    std::pair<int, proto::ResponseObservation> step();

    boost::process::child m_game_proc;
    boost::asio::io_context& m_ioc;
    std::shared_ptr<SC2Session> m_session;
    std::unique_ptr<Agent> m_agent;
    int m_agent_id;
    int m_prev_loop;
};

}


