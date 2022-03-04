#pragma once

#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <opencv2/opencv.hpp>

#include "../common/tcp_packet.h"

using boost::asio::ip::tcp;

class SessionManager;

class Session : public std::enable_shared_from_this<Session>
{
    public:
        Session(boost::asio::io_service& io_service, SessionManager& manager,
                std::shared_ptr<PacketReceiveBuffer>& receivePacketBuffer)
        : socket_(io_service), sess_manager_(manager),
         receivePacketBuffer_(receivePacketBuffer), started_(false),
         sendPacketBuffer_(50)
        {}

        void set_bufsize(int a){
            boost::asio::socket_base::receive_buffer_size option(a);
            socket_.set_option(option);
        }

        int print_bufsize(){
            boost::asio::socket_base::receive_buffer_size option;
            socket_.get_option(option);
            int buf_size = option.value()/(1024*1024);
            std::cout<<"receive buf size: "<<buf_size<<"MB"<<std::endl;
            return buf_size;
        }

        tcp::socket& socket(){
            return socket_;
        }

        void set_start(){
            started_.store(true);
        }

        bool started(){
            return started_.load();
        }

        std::string endpoint();
        void stop();
        void write(packet_type& packet);
        void do_read();

    private:

        void do_write();
        void handle_write(const boost::system::error_code& e);
        void handle_read_header(const boost::system::error_code& e);
        void handle_read_body(const boost::system::error_code& e);

        tcp::socket socket_;
        std::atomic<bool> started_;
        enum { max_length = 12*1024*1024 }; // 12MB
        char read_buffer_[max_length];

        unsigned int purpose;
        unsigned int dataSize;
        SessionManager& sess_manager_;
        std::shared_ptr<PacketReceiveBuffer> receivePacketBuffer_;
        PacketSendBuffer sendPacketBuffer_;
};
