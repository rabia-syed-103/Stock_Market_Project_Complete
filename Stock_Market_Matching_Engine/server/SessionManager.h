#pragma once

#include <unordered_map>
#include <string>
#include <random>
#include <mutex>
using namespace std;

class SessionManager {
private:
    unordered_map<std::string, std::string> tokenToUser; // sessionToken -> userID
    unordered_map<int, std::string> fdToUser;            // client_fd -> userID
    mutex mtx;

    string generateToken() {
        static const char chars[] =
            "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(0, sizeof(chars) - 2);

        string token;
        for (int i = 0; i < 32; ++i) token += chars[dis(gen)];
        return token;
    }

public:
    string createSession(const std::string& userID) {
        lock_guard<std::mutex> lock(mtx);
        string token = generateToken();
        tokenToUser[token] = userID;
        return token;
    }

    bool isValid(const std::string& token) {
        std::lock_guard<std::mutex> lock(mtx);
        return tokenToUser.find(token) != tokenToUser.end();
    }

    std::string getUserID(const std::string& token) {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = tokenToUser.find(token);
        return it != tokenToUser.end() ? it->second : "";
    }

    void invalidate(const std::string& token) {
        std::lock_guard<std::mutex> lock(mtx);
        tokenToUser.erase(token);
    }

    // ----- New functions for connection-bound sessions -----
    void bindConnection(int fd, const std::string& token) {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = tokenToUser.find(token);
        if(it != tokenToUser.end()) {
            fdToUser[fd] = it->second;
        }
    }

    std::string getUserForConnection(int fd) {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = fdToUser.find(fd);
        return it != fdToUser.end() ? it->second : "";
    }

    void unbindConnection(int fd) {
        std::lock_guard<std::mutex> lock(mtx);
        fdToUser.erase(fd);
    }

    void cleanupConnection(int fd) {
    std::lock_guard<std::mutex> lock(mtx);
    fdToUser.erase(fd);
}
};
