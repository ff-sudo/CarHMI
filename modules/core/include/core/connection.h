#pragma once

#include <functional>
#include <vector>
#include <memory>

namespace CarHMI::Core {

class Connection {
public:
    Connection() = default;

    explicit Connection(std::function<void()> disconnect)
        : m_disconnect(std::make_shared<DisconnectData>(std::move(disconnect))) {}

    Connection(Connection&& other) noexcept = default;
    Connection& operator=(Connection&& other) noexcept {
        if (this != &other) {
            Disconnect();
            m_disconnect = std::move(other.m_disconnect);
        }
        return *this;
    }

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    ~Connection() { Disconnect(); }

    void Disconnect() {
        if (m_disconnect && m_disconnect->fn) {
            m_disconnect->fn();
            m_disconnect->fn = nullptr;
        }
    }

    bool IsConnected() const {
        return m_disconnect && m_disconnect->fn != nullptr;
    }

    Connection Release() {
        Connection released;
        released.m_disconnect = std::move(m_disconnect);
        return released;
    }

private:
    struct DisconnectData {
        std::function<void()> fn;
        explicit DisconnectData(std::function<void()> f) : fn(std::move(f)) {}
    };
    std::shared_ptr<DisconnectData> m_disconnect;
};

class ConnectionGroup {
public:
    ConnectionGroup() = default;
    ~ConnectionGroup() { DisconnectAll(); }

    ConnectionGroup(ConnectionGroup&&) noexcept = default;
    ConnectionGroup& operator=(ConnectionGroup&&) noexcept = default;
    ConnectionGroup(const ConnectionGroup&) = delete;
    ConnectionGroup& operator=(const ConnectionGroup&) = delete;

    void Add(Connection&& c) {
        if (c.IsConnected())
            m_connections.push_back(std::move(c));
    }

    void DisconnectAll() {
        for (auto& c : m_connections)
            c.Disconnect();
        m_connections.clear();
    }

    size_t Size() const { return m_connections.size(); }

private:
    std::vector<Connection> m_connections;
};

} // namespace CarHMI::Core
