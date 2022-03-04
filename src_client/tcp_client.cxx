#include <cstdlib>
#include <cstring>
#include <iostream>
#include <chrono>
#include <thread>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <opencv2/opencv.hpp>
#include "../common/tcp_packet.h"

using namespace boost::asio;
using boost::asio::ip::tcp;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::system_clock;

class Client : public std::enable_shared_from_this<Client>{
    public:
    typedef boost::system::error_code error_code;

    Client(const std::string& host, const std::string& port,
            std::shared_ptr<PacketReceiveBuffer>& receivePacketBuffer)
            : socket_(service_), resolver(service_), serverHost(host), serverPort(port),
            receivePacketBuffer_(receivePacketBuffer), sendPacketBuffer_(50), started_(false)
            {}
    ~Client(){
    }

    void start(){
        if(started())
            return;

        tcp::resolver::query query(serverHost, serverPort);
        tcp::resolver::iterator endpointIter = resolver.resolve(query);

        auto completedHandler =
        boost::bind(&Client::handle_connected, shared_from_this(), boost::asio::placeholders::error);

        boost::asio::async_connect(socket_, endpointIter, completedHandler);
    }

    void stop(){
        if(!started())
            return;

        service_.post(boost::bind(&Client::do_close, shared_from_this()));
    }

    void run(){
        service_.run();
    }

    bool started(){
        return started_.load();
    }

    std::string address(){
        if(socket_.is_open()){
            return socket_.local_endpoint().address().to_string();
        }

        return "";
    }

    std::string port(){
        if(socket_.is_open()){
            //std::string address =  socket_.local_endpoint().address().to_string();
            std::string port = std::to_string(socket_.local_endpoint().port());
            return port;
        }

        return "";
    }

    void write(const packet_type& packet){
        if(!started())
            return;

        service_.post(boost::bind(&Client::do_write, shared_from_this(), packet));
    }

    private:
        void handle_connected(const boost::system::error_code& e){
            if(e){
                std::cerr<<"connection failed: "<<e<<std::endl;
                std::cerr<<"retry connection: "<<std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(5));
                start(); // retry connect
            }
            else{
                std::cout<<"connected to server!"<<std::endl;
                started_.store(true);
                do_read();
            }
        }

        void do_close(){
            started_.store(false);
            socket_.close();
        }

        void do_write(packet_type packet){
            if( sendPacketBuffer_.is_first(packet)){

                PacketHeader* pH = reinterpret_cast<PacketHeader*>(sendPacketBuffer_.front().get());
                auto data = reinterpret_cast<char*>(sendPacketBuffer_.front().get());
                auto length = sizeof(PacketHeader) + pH->dataSize;

                const auto& buf = boost::asio::buffer(data, length);
                auto completedHandler =
                boost::bind(&Client::handle_write, shared_from_this(), boost::asio::placeholders::error);

                boost::asio::async_write(socket_, buf, completedHandler);
            }
        }

        void handle_write(const boost::system::error_code& e){
            if(e){
                std::cerr<<"socket write error: "<<e<<std::endl;
                // close?
            }
            else{
                packet_type packet = sendPacketBuffer_.next_packet();
                if(packet != nullptr){
                    PacketHeader* pH = reinterpret_cast<PacketHeader*>(packet.get());
                    auto data = reinterpret_cast<char*>(packet.get());
                    auto length = sizeof(PacketHeader) + pH->dataSize;

                    const auto& buf = boost::asio::buffer(data, length);
                    auto completedHandler =
                    boost::bind(&Client::handle_write, shared_from_this(), boost::asio::placeholders::error);

                    boost::asio::async_write(socket_, buf, completedHandler);
                }
            }
        }

        void do_read(){
            PacketHeader* pHeader = reinterpret_cast<PacketHeader*>(read_buffer_);
            pHeader->reset(); // set begin to zeros
            const auto& buf = boost::asio::buffer(read_buffer_, sizeof(PacketHeader));
            auto completedHandler =
            boost::bind(&Client::handle_read_header, shared_from_this(), boost::asio::placeholders::error);

            boost::asio::async_read(socket_, buf, completedHandler);
        }

        void handle_read_header(const boost::system::error_code& e){
            if((boost::asio::error::eof == e) ||
                (boost::asio::error::connection_reset == e)){
                std::cerr<<"disconnected:"<< e << std::endl;
                started_.store(false);
                start(); // retry connect
            }
            else if(e){
                std::cerr<<"read header error: "<< e << std::endl;
            }
            else{
                PacketHeader* pHeader = reinterpret_cast<PacketHeader*>(read_buffer_);

                if( strncmp(pHeader->begin, "begin", 5) == 0){
                    const auto& buf = boost::asio::buffer(read_buffer_ + sizeof(PacketHeader), pHeader->dataSize);
                    auto completedHandler =
                    boost::bind(&Client::handle_read_body, shared_from_this(), boost::asio::placeholders::error);

                    boost::asio::async_read(socket_, buf, completedHandler);
                }
                else{
                    std::cerr<<"not begin"<<std::endl;
                    // restart read header
                    do_read();
                }
            }
        }

        void handle_read_body(const boost::system::error_code& e){
            if((boost::asio::error::eof == e) ||
                (boost::asio::error::connection_reset == e)){
                std::cerr<<"disconnected:"<< e << std::endl;
                started_.store(false);
                start();
            }
            else if(e){
                std::cerr<<"read header error: "<< e << std::endl;
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

    private:
        boost::asio::io_service service_;
        tcp::socket socket_;
        tcp::resolver resolver;
        std::string serverHost;
        std::string serverPort;
        std::atomic<bool> started_;
        enum { max_length = 12*1024*1024}; // 12MB
        char read_buffer_[max_length];

        PacketSendBuffer sendPacketBuffer_;
        std::shared_ptr<PacketReceiveBuffer> receivePacketBuffer_;
};

int main(int argc, char* argv[]){
    try{
        if( argc != 3){
            std::cerr<<"Usage: client <host> <port>"<<std::endl;
            std::cerr<<"Example: ./client 127.0.0.1 8080"<<std::endl;
            return 1;
        }

        const std::string host(argv[1]);
        const std::string port(argv[2]);
        std::shared_ptr<PacketReceiveBuffer> receivePacketBuf = std::make_shared<PacketReceiveBuffer>(100);

        std::shared_ptr<Client> clientPtr = std::make_shared<Client>(host, port, receivePacketBuf);

        std::thread socketThred([](std::shared_ptr<Client> clientPtr){
            clientPtr->start();
            clientPtr->run();

        }, clientPtr);

        std::thread sendThred([](std::shared_ptr<Client> clientPtr){
            // 49152 = 128*128*3 = 48KB
            // 230400 = 320x240 = 225KB
            // 23082 = jpeg = 22KB

            cv::Mat imgTest = cv::imread("../images/test.bmp");
            cv::resize(imgTest, imgTest, cv::Size(320, 240));
            std::vector<int> qualityType;
            qualityType.push_back(CV_IMWRITE_JPEG_QUALITY);
            qualityType.push_back(80);
            size_t cnt = 0;

            while(true){
                for( size_t i = 0; i < 4; ++i){
                    cv::Mat img = imgTest.clone();
                    cv::putText(img, std::to_string(cnt), cv::Point(20, 50), cv::FONT_HERSHEY_COMPLEX, 1.0, cv::Scalar(200, 70, 125), 4);

                    std::vector<uchar> bufJPEG;
                    cv::imencode(".jpg", img, bufJPEG, qualityType);

                    //std::cout<<"size jpeg buf: "<< bufJPEG.size() << std::endl;
                    //std::cout<<"size img: "<< img.rows * img.cols * img.channels() <<std::endl;
                    int64_t camID = atoll(clientPtr->port().c_str()) * 10 + i;
                    auto packet = makeJpgImagePacket(camID, 0, bufJPEG);
                    //std::cout << camID + i << std::endl;

                    if(packet == nullptr)
                        continue;

                    clientPtr->write(packet);
                    cnt++;
                    // cv::imshow("client send " + address, img);
                    // int key = cv::waitKey(1);
                    // if(key == 113)
                    //     break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(50)); // 20 FPS
            }
        }, clientPtr);

        std::string address;

        bool once = true;
        while(true){
            auto packet = receivePacketBuf->dequeue();
            if(packet != nullptr){
                if(once){
                    once = false;
                    address = clientPtr->port() + ":" + clientPtr->address();
                }
                PacketHeader* header = reinterpret_cast<PacketHeader*>(packet.get());
                if( header->purpose == PacketPurpose::ImageJpeg){
                    cv::Mat img;
                    int64_t cameraID, timeCode;
                    const char* pData = reinterpret_cast<const char*>(packet.get() + sizeof(PacketHeader));
                    parseJpgImagePacket(pData, img, cameraID, timeCode);

                    cv::imshow("client receive " + address, img);
                    cv::waitKey(1);
                }
            }
        }

        clientPtr->stop();
        socketThred.join();
        sendThred.join();

    }catch(std::exception& e){
        std::cerr<<e.what()<<std::endl;
    }
}