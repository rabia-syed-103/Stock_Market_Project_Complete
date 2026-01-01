#pragma once
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <nlohmann/json.hpp>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <unordered_map>

using json = nlohmann::json;
using websocketpp::connection_hdl;
using namespace std;

class WebSocketServer {
public:
    WebSocketServer(int port = 8080);
    ~WebSocketServer();

    void run();
    void stop();

    void setHandler(std::function<json(int, const json&)> handler);
    void setSessionCleanup(std::function<void(int)> cleanup);

private:
    using server_t = websocketpp::server<websocketpp::config::asio>;

    server_t server;
    int port;
    atomic<bool> running;

    function<json(int, const json&)> requestHandler;
    function<void(int)> sessionCleanup;

    mutex sessionMutex;
    int nextClientId = 1;

    map<connection_hdl, int, std::owner_less<connection_hdl>> sessions;
    void onOpen(connection_hdl hdl);
    void onClose(connection_hdl hdl);
    void onMessage(connection_hdl hdl, server_t::message_ptr msg);
};
