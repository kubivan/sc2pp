#pragma once

#include <future>
#include <string>

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <s2clientprotocol/sc2api.pb.h>

namespace beast = boost::beast;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;
namespace websocket = beast::websocket;

namespace sc2
{
namespace proto = SC2APIProtocol;

class SC2Session
{
public:
    using ResponseHandler = std::function<bool(std::unique_ptr<sc2::proto::Response>)>;

    ~SC2Session();

    explicit SC2Session(net::io_context& ioc);

    const std::atomic<bool>& connected();


    std::unique_ptr<proto::Response> send(std::unique_ptr<proto::Request> request);

    beast::error_code connect(std::string host, std::string port);
    void close();

private:
    tcp::resolver m_resolver;
    websocket::stream<beast::tcp_stream> m_ws;
    std::string m_host;
    std::atomic<bool> m_connected;
};

}
