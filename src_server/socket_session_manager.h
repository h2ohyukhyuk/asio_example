#pragma once

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>

class Session;

class SessionManager : private boost::noncopyable{
    public:
        
        void add(std::shared_ptr<Session> s);

        /// Stop the specified connection.
        void stop(std::shared_ptr<Session> s);

        /// Stop all connections.
        void stop_all();

        void send_all(std::shared_ptr<char>& packet);

    private:
        std::set<std::shared_ptr<Session>> sessions_;
        std::mutex _mx;
};