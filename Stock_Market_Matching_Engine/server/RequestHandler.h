#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H

#include "../engine/PersistentMatchingEngine.h"
#include "SessionManager.h"
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <random>

using json = nlohmann::json;

class RequestHandler {
private:
    PersistentMatchingEngine& engine;
    SessionManager sessions;

public:
    explicit RequestHandler(PersistentMatchingEngine& eng) : engine(eng) {}
    void cleanupConnection(int client_fd) {
        sessions.cleanupConnection(client_fd);
    }

    json handleRequest(int client_fd, const json& request) {
        std::string type = request.value("type", "");
        json data = request.value("data", json::object());
        json response;

        try {
            if(type == "SIGN_IN") {
                response = handleSignIn(client_fd, data);
            } else if(type == "LOG_IN") {
                response = handleLogIn(client_fd, data);
            } else {
                // Get userID from connection
                std::string userID = sessions.getUserForConnection(client_fd);
                if(userID.empty())
                    return { {"status", "error"}, {"message", "Not logged in"} };

                data["userID"] = userID;

                if(type == "PLACE_ORDER") response = handlePlaceOrder(data);
                else if(type == "CANCEL_ORDER") response = handleCancelOrder(data);
                else if(type == "GET_PORTFOLIO") response = handleGetPortfolio(data);
                else if(type == "GET_ORDERBOOK") response = handleGetOrderBook(data);
                else if(type == "GET_TRADES") response = handleGetTrades(data);
                else if(type == "ADD_STOCK") response = handleAddStock(data);
                else response = { {"status", "error"}, {"message", "Unknown request type"} };
            }
        } catch(const std::exception& e) {
            response = { {"status", "error"}, {"message", e.what()} };
        }

        return response;
    }

private:
    // ----------------- SIGN_IN -----------------
    json handleSignIn(int client_fd, const json& data) {
        std::string userID = data.value("userID", "");
        double balance = data.value("balance", 0.0);
        if(userID.empty()) return { {"status","error"},{"message","Invalid userID"} };

        // ✅ Check if user already exists
        User* existingUser = engine.getUser(userID);
        if (existingUser) {
            // User exists, just create a new session
            std::string token = sessions.createSession(userID);
            sessions.bindConnection(client_fd, token);
            return { {"status","success"}, {"userID", userID}, {"message", "User already exists, logged in"} };
        }

        // Create new user
        engine.createUser(userID, balance);

        std::string token = sessions.createSession(userID);
        sessions.bindConnection(client_fd, token);

        return { {"status","success"}, {"userID", userID}, {"balance", balance} };
    }

    json handleLogIn(int client_fd, const json& data) {
        std::string userID = data.value("userID", "");
        if(userID.empty()) return { {"status","error"},{"message","Invalid userID"} };
        User* user = engine.getUser(userID);
        if(!user)
            return { {"status","error"}, {"message","User does not exist"} };

        std::string token = sessions.createSession(userID);
        sessions.bindConnection(client_fd, token); // bind connection

        return { {"status","success"}, {"userID", userID} };
    }




    json handlePlaceOrder(const json& data) {
        std::string symbol = data.value("symbol", "");
        std::string side = data.value("side", "");
        double price = data.value("price", 0.0);
        int quantity = data.value("quantity", 0);

        if (symbol.empty() || side.empty() || quantity <= 0) {
            return { {"status", "error"}, {"message", "Invalid order data"} };
        }

        Order* order = engine.placeOrder(data["userID"], symbol, side, price, quantity);
        if(!order)
            return { {"status", "error"}, {"message", "Failed to place order"} };
        return {
            {"status", "success"},
            {"data", {
                {"orderID", order->orderID},
                {"userID", order->userID},
                {"symbol", order->symbol},
                {"side", order->side},
                {"price", order->price},
                {"quantity", order->quantity},
                {"remaining", order->remainingQty},
                {"status", order->status}
            }}
        };
    }

    json handleCancelOrder(const json& data) {
        int orderID = data.value("orderID", 0);
        if (orderID <= 0) return { {"status", "error"}, {"message", "Invalid orderID"} };

        bool refunded = engine.cancelOrder(orderID, data["userID"]);
        return { {"status", "success"}, {"refunded", refunded} };
    }

    json handleGetPortfolio(const json& data) {
    std::string userID = data.value("userID", "");
    if (userID.empty())
        return { {"status","error"}, {"message","Missing userID"} };

    json portfolio = engine.getPortfolio(userID);
    if (portfolio.contains("error")) {
        return { {"status", "error"}, {"message", portfolio["error"]} };
    }
    return { {"status", "success"}, {"data", portfolio} };
}


    json handleGetOrderBook(const json& data) {
        std::string symbol = data.value("symbol", "");
        if (symbol.empty()) return { {"status","error"}, {"message","Invalid symbol"} };

        json orderBook = engine.getOrderBook(symbol);
        if (orderBook.contains("error")) {
            return { {"status","error"}, {"message", orderBook["error"]} };
        }
        return { {"status","success"}, {"data", orderBook} };
    }

    json handleGetTrades(const json& data) {
        std::string symbol = data.value("symbol", "");
        std::vector<Trade> trades = engine.getUserTrades(data["userID"]);
        if(trades.empty()) return { {"status", "success"}, {"data", json::array()} };
        
        json tradesJSON = json::array();
        for (auto& t : trades) tradesJSON.push_back(t.toJSON());

        return { {"status", "success"}, {"data", tradesJSON} };
    }

    json handleAddStock(const json& data) {
        std::string symbol = data.value("symbol", "");
        int qty = data.value("quantity", 0);

        if (symbol.empty()) return { {"status", "error"}, {"message", "Invalid symbol"} };
        if (qty <= 0)
    return { {"status","error"}, {"message","Invalid quantity"} };

        bool result = engine.addStock(symbol,data["userID"],qty);
        if(result)
            return { {"status", "success"}, {"symbol", symbol} };
        else
            return { {"status", "error"}, {"message", "Failed to add stock"} };
    }
    // In RequestHandler.h
    
};

#endif // REQUESTHANDLER_H
