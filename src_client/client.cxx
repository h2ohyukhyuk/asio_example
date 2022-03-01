#include <cstdlib>
#include <cstring>
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <opencv2/opencv.hpp>
#include "../common/packet.h"

using namespace boost::asio;
using boost::asio::ip::tcp;

const size_t max_length = 1024;


class Client : public std::enable_shared_from_this<Client>{
    public:
    typedef boost::system::error_code error_code;
    typedef boost::shared_ptr<Client> ptr;

    Client(const std::string& host, const std::string& port)
            : socket_(service_), resolver(service_), started_(true)
            {
                serverHost = host;
                serverPort = port;
                writingTrigerThreadPtr_ = nullptr;
            }
    ~Client(){
        if(writingTrigerThreadPtr_ != nullptr && writingTrigerThreadPtr_->joinable())
            writingTrigerThreadPtr_->join();

    }

    void start(){
        tcp::resolver::query query(serverHost, serverPort);
        tcp::resolver::iterator endpointIter = resolver.resolve(query);

        auto completedHandler = 
        boost::bind(&Client::handle_connected, shared_from_this(), boost::asio::placeholders::error);

        boost::asio::async_connect(socket_, endpointIter, completedHandler);
    }

    void stop(){
        if(!started_)
            return;
        started_ = false;
        socket_.close();
    }

    void run(){
        service_.run();
    }

    bool started(){
        return started_;
    }

    void queue(std::shared_ptr<char> packet){
        lockObj.lock();
        if(packets_.size() < 100){
            packets_.push(packet);
        }
        else{
            packets_.pop();
            packets_.push(packet);
            std::cout<<"packet queue overflow"<<std::endl;
        }
        //std::cout<<"packet queue size: "<<packets_.size()<<std::endl;
        lockObj.unlock();
    }

    private:
        void do_read(){
            const auto& buf = boost::asio::buffer(read_buffer_);
            auto completedHandler = 
            boost::bind(&Client::handle_read, shared_from_this(), _1, _2);
            socket_.async_read_some(buf, completedHandler);
        }

        void do_write(){
            
            std::shared_ptr<char> packet = dequeue();
            while(packet == nullptr && started()){                
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                packet = dequeue();
            }

            if(!started())
                return;

            PacketHeader* pHeader = reinterpret_cast<PacketHeader*>(packet.get());
            const size_t length = sizeof(PacketHeader) + pHeader->dataSize;

            static int sum = 0;
            sum += length;
            std::cout<<"write len: "<< length <<" total: "<< sum <<std::endl;
            memcpy(write_buffer_, packet.get(), length);        
            
            const auto& buf = boost::asio::buffer(write_buffer_, length);
            auto completedHandler = 
            boost::bind(&Client::handle_write, shared_from_this(), boost::asio::placeholders::error);

            boost::asio::async_write(socket_, buf, completedHandler);            
        }

        void handle_connected(const boost::system::error_code& e){
            if(e){
                std::cerr<<"connection failed: "<<e<<std::endl;
                std::cerr<<"retry connection: "<<std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(5));
                start();
                //stop();
            }
            else{
                std::cout<<"connected to server!"<<std::endl;
                started_ = true;
                do_read();                
                writingTrigerThreadPtr_ = 
                std::make_shared<std::thread>(&Client::do_write, shared_from_this());
            }
        }

        void handle_read(const boost::system::error_code& e, size_t bytes){

            if(!started())
                return;
            //processing(read_buffer_, bytes);
            //do_read();
        }

        void handle_write(const boost::system::error_code& e){
            if(e){
                std::cerr<<"socket write error: "<<e<<std::endl;
            }
            else{
                // std::cout<<"write done"<<std::endl;
                do_write();
            }
        }

        std::shared_ptr<char> dequeue(){
            lockObj.lock();
            if(packets_.empty()){
                lockObj.unlock();
                return nullptr;
            }
            else{
                std::shared_ptr<char> v = packets_.front();
                packets_.pop();
                lockObj.unlock();
                return v;
            }
        }

        boost::asio::io_service service_;
        tcp::socket socket_;
        tcp::resolver resolver;
        std::string serverHost;
        std::string serverPort;
        bool started_;
        enum { max_msg = 32*1024*1024 };
        char read_buffer_[max_msg];
        char write_buffer_[max_msg];

        std::shared_ptr<std::thread> writingTrigerThreadPtr_;
        std::mutex lockObj;
        std::queue<std::shared_ptr<char>> packets_;
};

int main(int argc, char* argv[])
{

    try{
        if( argc != 3){
            std::cerr<<"Usage: client <host> <port>"<<std::endl;
            std::cerr<<"Example: ./client 127.0.0.1 8080"<<std::endl;
            return 1;
        }

        const std::string host(argv[1]);
        const std::string port(argv[2]);
        std::shared_ptr<Client> clientPtr = std::make_shared<Client>(host, port);
        
        std::thread socketThred([](std::shared_ptr<Client> clientPtr){
            clientPtr->start();
            clientPtr->run();

        }, clientPtr);
        
        size_t cnt = 0;

        /*
        125MBPS
        image = 128x128 = 16KB
        12500
        */
        while(true){

            cv::Mat img = cv::Mat::zeros(128, 128, CV_8UC3);
            cv::putText(img, std::to_string(cnt), cv::Point(20, 50), cv::FONT_HERSHEY_COMPLEX, 1.0, cv::Scalar(200, 70, 125), 4);

            auto packet = makeImagePacket(0, 0, img);

            if(packet == nullptr)
                continue;

            clientPtr->queue(packet);

            cnt++;
            //std::this_thread::sleep_for(std::chrono::seconds(2));
            cv::imshow("client", img);
            int key = cv::waitKey(100);
            if(key == 113)
                break;
        }

        clientPtr->stop();
        socketThred.join();

    }catch(std::exception& e){
        std::cerr<<e.what()<<std::endl;
    }

}