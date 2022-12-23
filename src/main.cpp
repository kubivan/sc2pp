#include "SC2Client.h"

#include <boost/process.hpp>
#include <boost/process/args.hpp>
#include <iostream>


struct DummyAgent : sc2::Agent
{
    DummyAgent(std::unique_ptr<sc2::SC2Context> context)
    {
    }

    std::unique_ptr<sc2::proto::Request> step()
    {
        std::cout << std::this_thread::get_id() << " : TestAgent::step()" << std::endl;
        m_actions.chat("HEY! creating a probe on");

        auto res = m_actions.reset();
        return res;
    }

    sc2::proto::Race race()
    {
        return sc2::proto::Protoss;
    }

    sc2::Actions m_actions;
};

int main(int argc, char** argv)
try{

    std::cout << std::this_thread::get_id() << " :main thread" << std::endl;
    net::io_context ioc;
    auto guard = boost::asio::make_work_guard(ioc);
    auto io_thread = std::jthread{ [&] {ioc.run(); } };

    sc2::SC2Client client(ioc);
    client.start();
    client.ping();

    std::cout << "client started" << std::endl;
    auto created = client.createGame();
    std::cout << "game created: " << created << std::endl;
    if (!created)
    {
        //goto err;
        return -1;
    }

    std::cout << "joining" << std::endl;

    if (!client.joinGame(std::make_unique<DummyAgent>(client.createContext())))
    {
        std::cout << "join failed" << std::endl;
        return -1;
    }

    std::optional<sc2::proto::PlayerResult> res;
    while (!res)
    {
        res = client.update();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    std::cout << "Result " << res->result() << std::endl;

err:
    std::cout << "stopping ioc" << std::endl;
    //ioc.stop();
    return 0;
}
catch (std::exception& e)
{
    std::cout << "EXCEPTION: " << e.what() << std::endl;
}
catch (...)
{
    std::cout << "EXCEPTION: unknown" << std::endl;
}
