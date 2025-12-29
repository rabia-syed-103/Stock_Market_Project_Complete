#include "WebSocketServer.h"
#include <iostream>
#include <map>

WebSocketServer::WebSocketServer(int port)
    : port(port), running(false) {

    server.init_asio();

    server.set_open_handler(
        [this](connection_hdl hdl) { onOpen(hdl); });

    server.set_close_handler(
        [this](connection_hdl hdl) { onClose(hdl); });

    server.set_message_handler(
        [this](connection_hdl hdl, server_t::message_ptr msg) {
            onMessage(hdl, msg);
        });
}

WebSocketServer::~WebSocketServer() {
    stop();
}

void WebSocketServer::setHandler(
    std::function<json(int, const json&)> handler) {
    requestHandler = handler;
}

void WebSocketServer::setSessionCleanup(
    std::function<void(int)> cleanup) {
    sessionCleanup = cleanup;
}

void WebSocketServer::run() {
    running = true;
    server.listen(port);
    server.start_accept();
    server.run();
}

void WebSocketServer::stop() {
    if (!running) return;
    running = false;
    server.stop_listening();
}

void WebSocketServer::onOpen(connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(sessionMutex);
    sessions[hdl] = nextClientId++;
    std::cout << "Client connected, id=" << sessions[hdl] << "\n";
}

void WebSocketServer::onClose(connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(sessionMutex);

    int clientId = sessions[hdl];
    sessions.erase(hdl);

    if (sessionCleanup)
        sessionCleanup(clientId);

    std::cout << "Client disconnected, id=" << clientId << "\n";
}

void WebSocketServer::onMessage(
    connection_hdl hdl, server_t::message_ptr msg) {

    int clientId;
    {
        std::lock_guard<std::mutex> lock(sessionMutex);
        clientId = sessions[hdl];
    }

    try {
        json request = json::parse(msg->get_payload());
        json response;

        if (requestHandler)
            response = requestHandler(clientId, request);
        else
            response = {{"error", "No handler registered"}};

        server.send(hdl, response.dump(),
                    websocketpp::frame::opcode::text);
    }
    catch (const std::exception& e) {
        json err = {{"error", e.what()}};
        server.send(hdl, err.dump(),
                    websocketpp::frame::opcode::text);
    }
}
