#include <future>
#include <string>

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>

#include <s2clientprotocol/sc2api.pb.h>

namespace beast = boost::beast;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;
namespace websocket = beast::websocket;

namespace sc2
{
namespace proto = SC2APIProtocol;

class Sc2Session : public std::enable_shared_from_this<Sc2Session>
{
public:
    using ResponseHandler = std::function<bool(std::unique_ptr<sc2::proto::Response>)>;

    explicit Sc2Session(net::io_context& ioc);

    const std::atomic<bool>& connected();

    std::future<bool>
        send(std::shared_ptr<proto::Request> request, ResponseHandler handler);

    //start async operation
    std::future<bool> run(std::string host, std::string port);
    void close();

private:
    void on_connect(std::promise<bool> res, beast::error_code ec, tcp::resolver::results_type::endpoint_type ep);

    void on_handshake(std::promise<bool> res, beast::error_code ec);

    void on_write(
        ResponseHandler handler,
        std::promise<bool> resp_promise,
        beast::error_code ec,
        std::size_t bytes_transferred
    );

    void on_read(
        ResponseHandler handler,
        std::promise<bool> resp_promise,
        beast::error_code ec,
        std::size_t bytes_transferred);

    void on_close(beast::error_code ec);
    
private:
    tcp::resolver m_resolver;
    websocket::stream<beast::tcp_stream> m_ws;
    beast::flat_buffer m_buffer;
    std::string m_host;
    std::atomic<bool> m_connected;
};

}
