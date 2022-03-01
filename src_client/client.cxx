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

using boost::asio::ip::tcp;

const size_t max_length = 1024;


class Client{
    public:
    Client(const std::string& host, const std::string& port)
            : socket_(this->io_context), resolver(this->io_context), stopped_(true)
            {
                serverHost = host;
                serverPort = port;
            }
    ~Client(){}

    void connect(){
        tcp::resolver::query query(serverHost, serverPort);
        tcp::resolver::iterator endpointIter = resolver.resolve(query);

        auto completedHandler = 
        boost::bind(&Client::handle_connected, this, boost::asio::placeholders::error);

        boost::asio::async_connect(socket_, endpointIter, completedHandler);
    }

    void run(){
        io_context.run();
    }

    void stop(){
        stopped_ = true;
        socket_.close();
        //deadline_.cancel();
    }

    bool isConnected(){
        return !stopped_;
    }

    void handle_connected(const boost::system::error_code& e){

        if(!socket_.is_open())
        {
            std::cout << "Connect timed out\n";

            // Try the next available endpoint.
            //start_connect(++endpoint_iter);
        }
        else if(e){
            std::cerr<<"connection failed: "<<e<<std::endl;
            stopped_ = true;
        }
        else{
             std::cout<<"connected to server!"<<std::endl;
             stopped_ = false;
        }
    }

    void write(std::shared_ptr<char> packet){

        //pushPacket(packet);
        PacketHeader* pHeader = reinterpret_cast<PacketHeader*>(packet.get());
        const size_t length = sizeof(PacketHeader) + pHeader->dataSize;
        //std::cout<<"data length: "<<pHeader->dataSize<<std::endl;
        
        const auto& buf = boost::asio::buffer(packet.get(), length);
        auto completedHandler = 
        boost::bind(&Client::handle_write, this, boost::asio::placeholders::error);

        boost::asio::async_write(socket_, buf, completedHandler);
    }

    private:

        void pushPacket(std::shared_ptr<char> packet){
            lockObj.lock();
            queuePacket.push(packet);
            lockObj.unlock();
        }

        void popPacket(){
            lockObj.lock();
            queuePacket.pop();
            lockObj.unlock();
        }
        
        void handle_write(const boost::system::error_code& e){
            if(e){
                std::cerr<<"socket write error: "<<e<<std::endl;
            }
            else{
                std::cout<<"write done"<<std::endl;
            }
            
            //popPacket();
        }

        bool stopped_;
        boost::asio::io_service io_context;
        tcp::socket socket_;
        tcp::resolver resolver;
        std::string serverHost;
        std::string serverPort;
        std::queue<std::shared_ptr<char>> queuePacket;
        std::mutex lockObj;
};

int main(int argc, char* argv[])
{

    try{
        if( argc != 3){
            std::cerr<<"Usage: client <host> <port>"<<std::endl;
            std::cerr<<"Example: ./client 127.0.0.1 8080"<<std::endl;
            return 1;
        }

        Client client(argv[1], argv[2]);
        client.connect();
        client.run();

        
        size_t cnt = 0;

        while(true){

            const int cameraID = 0;
            const int timeCode = 0;
            
            cv::Mat img = cv::Mat::zeros(200, 200, CV_8UC3);
            cv::putText(img, std::to_string(cnt), cv::Point(20, 50), cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(200, 70, 125), 4);

            auto packet = makeImagePacket(cameraID, timeCode, img);

            if(packet == nullptr)
                continue;

            client.write(packet);

            cnt++;
            //std::this_thread::sleep_for(std::chrono::seconds(2));
            cv::imshow("client", img);
            cv::waitKey(100);
        }

    }catch(std::exception& e){
        std::cerr<<e.what()<<std::endl;
    }

}