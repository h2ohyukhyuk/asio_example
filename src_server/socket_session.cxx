
#include "socket_session.h"
#include "socket_session_manager.h"

using boost::asio::ip::tcp;

std::string Session::endpoint(){
    if(socket_.is_open()){                
        std::string address =  socket_.remote_endpoint().address().to_string();
        std::string port = std::to_string(socket_.remote_endpoint().port());
        return address + ":" + port;;
    }
    
    return "";
}
void Session::stop(){
    started_.store(false);
    socket_.close();
    shared_from_this().reset();
}

void Session::write(packet_type& packet){
    if(!started())
        return;

    if(sendPacketBuffer_.is_first(packet)){
        PacketHeader* pH = reinterpret_cast<PacketHeader*>(sendPacketBuffer_.front().get());
        auto data = reinterpret_cast<char*>(sendPacketBuffer_.front().get());
        auto length = sizeof(PacketHeader) + pH->dataSize;

        const auto& buf = boost::asio::buffer(data, length);
        auto completedHandler = 
        boost::bind(&Session::handle_write, shared_from_this(), boost::asio::placeholders::error);

        boost::asio::async_write(socket_, buf, completedHandler);
    }
}

void Session::do_read(){
    PacketHeader* pHeader = reinterpret_cast<PacketHeader*>(read_buffer_);
    pHeader->reset(); // set begin to zeros
    const auto& buf = boost::asio::buffer(read_buffer_, sizeof(PacketHeader));
    auto completedHandler = 
    boost::bind(&Session::handle_read_header, shared_from_this(), boost::asio::placeholders::error);

    boost::asio::async_read(socket_, buf, completedHandler);
}

void Session::handle_read_header(const boost::system::error_code& e){
    if((boost::asio::error::eof == e) ||
        (boost::asio::error::connection_reset == e)){
        std::cerr<<"disconnected:"<< e << std::endl;
        sess_manager_.stop(shared_from_this());
    }
    else if(e){
        std::cerr<<"read header error: "<< e << std::endl;
        sess_manager_.stop(shared_from_this());
    }
    else{
        PacketHeader* pHeader = reinterpret_cast<PacketHeader*>(read_buffer_);
        
        if( strncmp(pHeader->begin, "begin", 5) == 0){
            const auto& buf = boost::asio::buffer(read_buffer_ + sizeof(PacketHeader), pHeader->dataSize);
            auto completedHandler = 
            boost::bind(&Session::handle_read_body, shared_from_this(), boost::asio::placeholders::error);

            boost::asio::async_read(socket_, buf, completedHandler);
        }
        else{
            std::cerr<<"not begin"<<std::endl;
            // restart read header
            do_read();
        }
    }
}

void Session::handle_read_body(const boost::system::error_code& e){
    
    if((boost::asio::error::eof == e) ||
        (boost::asio::error::connection_reset == e)){
        std::cerr<<"disconnected:"<< e << std::endl;
        started_.store(false);
        sess_manager_.stop(shared_from_this());
    }
    else if(e){
        std::cerr<<"read header error: "<< e << std::endl;
        sess_manager_.stop(shared_from_this());
    }
    else
    {
        //queue_receive_packet
        PacketHeader* pHeader = reinterpret_cast<PacketHeader*>(read_buffer_);
        
        char* pEnd = read_buffer_ + sizeof(PacketHeader) + pHeader->dataSize - 3;
        if( strncmp(pEnd, "end", 3) == 0){
            const size_t packetSize = sizeof(PacketHeader) + pHeader->dataSize;                        
            packet_type new_packet;
            new_packet.reset(new char[packetSize], charArrayDeletor);
            memcpy(new_packet.get(), read_buffer_,  packetSize);
            receivePacketBuffer_->enqueue(new_packet);                        
        }
        else{
            std::cerr<<"not end"<<std::endl;
        }
        
        do_read(); // wait for another message
    }
}

void Session::handle_write(const boost::system::error_code& e){
    if(e){
        std::cerr<<"socket write error: "<<e<<std::endl;
        sess_manager_.stop(shared_from_this());
    }
    else if(e.value() == 125){
        // socket closed
        sess_manager_.stop(shared_from_this());
    }
    else{
        packet_type packet = sendPacketBuffer_.next_packet();
        if(packet != nullptr){
            PacketHeader* pH = reinterpret_cast<PacketHeader*>(packet.get());
            auto data = reinterpret_cast<char*>(packet.get());
            auto length = sizeof(PacketHeader) + pH->dataSize;

            const auto& buf = boost::asio::buffer(data, length);
            auto completedHandler = 
            boost::bind(&Session::handle_write, shared_from_this(), boost::asio::placeholders::error);

            boost::asio::async_write(socket_, buf, completedHandler);
        }
    }
}