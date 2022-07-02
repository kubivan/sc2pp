#include "Sc2Session.h"


#include <iostream>

namespace http = beast::http;

using namespace sc2;
namespace proto = SC2APIProtocol;

// Report a failure
void
fail(beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

Sc2Session::Sc2Session(net::io_context& ioc)
    : m_resolver(net::make_strand(ioc))
    , m_ws(net::make_strand(ioc))
    , m_connected(false)
{
}

const std::atomic<bool>&
Sc2Session::connected()
{
    return m_connected;
}

std::future<bool>
Sc2Session::send(std::shared_ptr<SC2APIProtocol::Request> request, ResponseHandler handler)
{
    std::promise<bool> res_promise;
    auto fut = res_promise.get_future();
    if (!m_connected)
    {
        res_promise.set_value(false);
        return fut;
    }
    using namespace std::placeholders;

    std::vector<uint8_t> buf(request->ByteSizeLong());
    request->SerializeToArray(buf.data(), buf.size());
    // Send the message
    m_ws.async_write(
        net::buffer(buf),
        beast::bind_front_handler(
        &Sc2Session::on_write,
        shared_from_this(),
        std::move(handler),
        std::move(res_promise)
    ));

    return fut;
}

//start async operation
std::future<bool> 
Sc2Session::run(std::string host, std::string port)
{
    std::promise<bool> res_promise;
    std::future<bool> res_future = res_promise.get_future();

    beast::error_code ec;
    const auto endpoint = m_resolver.resolve(host, port, ec);
    if (ec)
    {
        fail(ec, "run");
        res_promise.set_value(false);
        return res_future;
    }
    m_host = host;

    //beast::get_lowest_layer(m_ws).expires_after(std::chrono::seconds(300));
    beast::get_lowest_layer(m_ws).async_connect(
        endpoint,
        beast::bind_front_handler(
        &Sc2Session::on_connect,
        shared_from_this(),
        std::move(res_promise)
    ));

    return res_future;
}

void 
sc2::Sc2Session::close()
{
    //// Close the WebSocket connection
    m_ws.async_close(websocket::close_code::normal,
        beast::bind_front_handler(
        &Sc2Session::on_close,
        shared_from_this()));
}

void 
Sc2Session::on_connect(std::promise<bool> res
    , beast::error_code ec
    , tcp::resolver::results_type::endpoint_type ep)
{
    std::cout << "on_connect: " << ec << std::endl;
    if (ec)
    {
        res.set_value(false);
        return fail(ec, "connect");
    }
    // Turn off the timeout on the tcp_stream, because
    // the websocket stream has its own timeout system.
    beast::get_lowest_layer(m_ws).expires_never();

    // Set suggested timeout settings for the websocket
    //TODO: sc2 recommended timeouts
    m_ws.set_option(
        websocket::stream_base::timeout::suggested(
        beast::role_type::client));

    //TODO: port
    //m_host += ':' + std::to_string(ep.port());

    // Perform the websocket handshake
    m_ws.async_handshake(m_host, "/sc2api",
        beast::bind_front_handler(
        &Sc2Session::on_handshake,
        shared_from_this(),
        std::move(res)
    ));

    //m_ws.handshake(m_host, "/sc2api", ec);
}

void 
Sc2Session::on_handshake(std::promise<bool> res, beast::error_code ec)
{
    std::cout << "on_handshake: " << ec << std::endl;
    if (ec)
    {
        res.set_value(false);
        return fail(ec, "handshake");
    }

    m_connected = true;
    res.set_value(true);
}

void Sc2Session::on_write(
    ResponseHandler handler,
    std::promise<bool> resp_promise,
    beast::error_code ec,
    std::size_t bytes_transferred
)
{
    //std::cout << "on_write" << std::endl;
    boost::ignore_unused(bytes_transferred);

    if (ec)
    {
        resp_promise.set_value(false);
        return fail(ec, "write");
    }

    // Read a message into our buffer
    m_ws.async_read(
        m_buffer,
        beast::bind_front_handler(
        &Sc2Session::on_read,
        shared_from_this(),
        std::move(handler),
        std::move(resp_promise)
    ));
}

void Sc2Session::on_read(
    ResponseHandler handler,
    std::promise<bool> resp_promise,
    beast::error_code ec,
    std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);
    if (ec)
    {
        resp_promise.set_value(false);
        return fail(ec, "read");
    }

    auto response = std::make_unique<sc2::proto::Response>();
    if (!response->ParseFromArray(m_buffer.cdata().data(), m_buffer.size()))
    {
        resp_promise.set_value(false);
        return fail(ec, "read: failed to parse response");
    }

    if (!handler(std::move(response)))
    {
        resp_promise.set_value(false);
        return fail(ec, "read");
    }

    resp_promise.set_value(true);
}

void Sc2Session::on_close(beast::error_code ec)
{
    if (ec)
        return fail(ec, "close");

    // If we get here then the connection is closed gracefully

    // The make_printable() function helps print a ConstBufferSequence
    std::cout << beast::make_printable(m_buffer.data()) << std::endl;
}
