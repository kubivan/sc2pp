#include "SC2Client.h"

#include <boost/process.hpp>
#include <boost/process/args.hpp>
#include <iostream>

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

    if (!client.joinGame(std::make_unique<sc2::TestAgent>(client.createContext())))
    {
        std::cout << "join failed" << std::endl;
        return -1;
    }
    //context.obs.subscribe(
    //    [](const auto& response_observation) {
    //        std::cout << "observe: " << response_observation.observation().game_loop() << std::endl; 

    //    },
    //    []() {std::cout << "onComplete" << std::endl; }
    //);
    //auto game_loop = response_observation.map([](const sc2::proto::ResponseObservation& resp) {return resp.observation().game_loop(); });
    //auto game_loop_10 = game_loop | rxcpp::operators::filter([](const auto& loop) { return loop % 10 == 0; });
    //game_loop_10.subscribe([](const auto& loop) { std::cout << "loop: " << loop << std::endl; });

    client.startGame();
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

err:
    std::cout << "stopping ioc" << std::endl;
    ioc.stop();
    return 0;
}
catch (std::exception& e)
{
    std::cout << "EXCEPTION: " << e.what() << std::endl;
}
#endif
