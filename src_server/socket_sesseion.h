#pragma once

#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/basic_stream_socket.hpp>
#include <opencv2/opencv.hpp>

#include "../common/packet.h"

using boost::asio::ip::tcp;
 
class session
{
public:
    enum { max_length = 256*1024*1024 };

    session(boost::asio::io_service& io_service)
    //boost 1.66이후 (Ubuntu 18.10 이후) 버전의 경우 io_context를 사용
    //session(boost::asio::io_context& io_service)
    : socket_(io_service)
    {
    }

    void set_bufsize(int a){
        boost::asio::socket_base::receive_buffer_size option(a);
        socket_.set_option(option);
    }

    int print_bufsize(){
        boost::asio::socket_base::receive_buffer_size option;
        socket_.get_option(option);
        std::cout<<"receive buf size: "<<option.value()/(1024*1024)<<"MB"<<std::endl;
    }
 
    tcp::socket& socket()
    {
        return socket_;
    }
 
    void read()
    {
        auto completedHandler = 
            boost::bind(&session::handleRead, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred);

        socket_.async_read_some(boost::asio::buffer(data, max_length),
                                completedHandler);
    }
 
private:

    void handleRead(const boost::system::error_code& error,
        size_t bytes_transferred)
    {
        if ((boost::asio::error::eof == error) ||
        (boost::asio::error::connection_reset == error)){
            std::cerr<<"socket disconnected:"<< error << std::endl;
            delete this;
        }
        else if(error){
            std::cerr<<"socket read header error: "<< error << std::endl;
            delete this;
        }
        else
        {
            PacketHeader* pHeader = reinterpret_cast<PacketHeader*>(data);
            if( strncmp(pHeader->begin, "begin", 5) == 0){

                // std::cout<<"purpose: "<< pHeader->purpose <<std::endl;
                // std::cout<<"dataSize: "<< pHeader->dataSize <<std::endl;
                char* pEnd = data + sizeof(PacketHeader) + pHeader->dataSize - 3;
                //std::cout<<pEnd[0]<<pEnd[1]<<pEnd[2]<<std::endl;
                //std::cout<<"PacketHeader size: "<<sizeof(PacketHeader)<<std::endl;
                static int sum = 0;
                static int cnt = 0;
                sum += bytes_transferred;
                cnt++;

                
                //std::cout<<"bytes_transferred size: "<<bytes_transferred<<" total: "<<sum<<std::endl;                
                

                if( strncmp(pEnd, "end", 3) == 0){
                    //std::cout<<"find end"<<std::endl;

                    if( purpose == PacketPurpose::Image){
                        //std::cout<<"parsing image"<<std::endl;
                        const char* pImageData = data + sizeof(PacketHeader);
                        cv::Mat img;
                        int cameraID = -1;
                        int timeCode = -1;

                        //parseImagePacket(pImageData, img, cameraID, timeCode);
                        //cv::imwrite("./image.jpg", img);
                        
                        // cv::imshow("server", img);
                        // cv::waitKey(1);
                    }
                }
                else{
                    std::cout<<"not end"<<std::endl;
                }
            }
            else{
                std::cout<<"not begin"<<std::endl;
            }
            
            read();
        }
    }

  tcp::socket socket_;
  char data[max_length];
  unsigned int purpose;
  unsigned int dataSize;
};