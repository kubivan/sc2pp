#include "SC2Session.h"

#include <boost/asio/strand.hpp>
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

SC2Session::SC2Session(net::io_context& ioc)
    : m_resolver(ioc)
    , m_ws(ioc)
    , m_connected(false)
{
}

const std::atomic<bool>&
SC2Session::connected()
{
    return m_connected;
}

std::unique_ptr<proto::Response>
SC2Session::send(std::unique_ptr<proto::Request> request)
{
    if (!m_connected)
    {
        return {};
    }

    beast::error_code ec;
    std::vector<uint8_t> buf(request->ByteSizeLong());
    request->SerializeToArray(buf.data(), buf.size());
    // Send the message
    m_ws.write(
        net::buffer(buf),
        ec
    );
    if (ec)
    {
        fail(ec, "send failed");
        return {};
    }

    // Turn off the timeout on the tcp_stream, because
    // the websocket stream has its own timeout system.
    beast::get_lowest_layer(m_ws).expires_never();

    // Set suggested timeout settings for the websocket
    //TODO: sc2 recommended timeouts
    m_ws.set_option(
        websocket::stream_base::timeout::suggested(
        beast::role_type::client));

    beast::flat_buffer response_buffer;

    m_ws.read(response_buffer, ec);
    if (ec)
    {
        fail(ec, "read failed");
        return {};
    }

    auto response = std::make_unique<proto::Response>();
    response->ParseFromArray(response_buffer.data().data(), response_buffer.size());

    return response;
}

sc2::SC2Session::~SC2Session()
{
    close();
}

beast::error_code
SC2Session::connect(std::string host, std::string port)
{
    beast::error_code ec;
    if (!m_connected)
    {
        const auto endpoint = m_resolver.resolve(host, port, ec);
        if (ec)
        {
            fail(ec, "resolve");
            return ec;
        }
        m_host = host;

        //auto ep = net::connect(m_ws.next_layer(), endpoint, ec);
        auto ep = beast::get_lowest_layer(m_ws).connect(endpoint, ec);
        if (ec)
        {
            fail(ec, "connect");
            return ec;
        }

        m_host += ':' + port;
        m_connected = true;
    }

    m_ws.handshake(m_host, "/sc2api", ec);

    return ec;
}

void
sc2::SC2Session::close()
{
    if (m_ws.is_open())
    {
        m_ws.close(websocket::close_code::normal);
    }
}
