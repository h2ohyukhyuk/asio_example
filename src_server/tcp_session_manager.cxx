#include "tcp_session.h"
#include "tcp_session_manager.h"

void SessionManager::add(std::shared_ptr<Session> s){
    std::lock_guard<std::mutex> lk(_mx);
    sessions_.insert(s);
}

/// Stop the specified connection.
void SessionManager::stop(std::shared_ptr<Session> s){
    std::lock_guard<std::mutex> lk(_mx);
    sessions_.erase(s);
    s->stop();
}

/// Stop all connections.
void SessionManager::stop_all(){
    std::for_each(sessions_.begin(), sessions_.end(),
    boost::bind(&Session::stop, _1));
    sessions_.clear();
}

void SessionManager::send_all(std::shared_ptr<char>& packet){
    std::vector<std::shared_ptr<Session>> active;
    {
        std::lock_guard<std::mutex> lk(_mx);
        for (auto& s : sessions_)
            active.push_back(s);
        //std::cout<<"sess size: "<<sessions_.size()<<std::endl;
    }

    for( auto& a : active){
        a->write(packet);
    }
}