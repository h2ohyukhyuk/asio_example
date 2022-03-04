
#include "tcp_session.h"
#include "tcp_session_manager.h"
#include <chrono>
#include <thread>
#include <opencv2/opencv.hpp>

class Server
{
    public:
        Server(short port, std::shared_ptr<PacketReceiveBuffer>& receiveBuffPtr)
            : acceptor_(io_service_, tcp::endpoint(tcp::v4(), port)),
            sess_manager(),
            receiveBuffPtr_(receiveBuffPtr){
            start_accept();
        }

        void run(){
            io_service_.run();
        }

        void stop(){
            io_service_.post(boost::bind(&Server::handle_stop, this));
        }

    private:
        void start_accept(){
            std::shared_ptr<Session> new_session(new Session(io_service_, sess_manager, receiveBuffPtr_));

            acceptor_.async_accept(new_session->socket(),
                boost::bind(&Server::handle_accept, this, new_session,
                    boost::asio::placeholders::error));
        }

        void handle_accept(std::shared_ptr<Session> new_session,
            const boost::system::error_code& error){
            if (error.value() == 125){
                // closed
                new_session.reset();
            }
            else if(error)
            {
                //std::cerr<<"socket accept error: "<<error<<std::endl;
                new_session.reset();
            }
            else
            {
                std::string address = new_session->endpoint();
                std::cout<<"connected: "<<address<<std::endl;

                new_session->set_start();
                new_session->print_bufsize();
                new_session->do_read();
                sess_manager.add(new_session);
                start_accept();
            }
        }

        void handle_stop(){
            acceptor_.close();
            sess_manager.stop_all();
        }

        //boost 1.66 after (Ubuntu 18.10 after) version: io_service -> io_context
        boost::asio::io_service io_service_;
        tcp::acceptor acceptor_;
        std::shared_ptr<PacketReceiveBuffer> receiveBuffPtr_;
    public:
        SessionManager sess_manager;
};

#if 1
int main(int argc, char* argv[])
{
    try{
        if (argc != 2){
            std::cerr << "Usage: async_tcp_echo_server <port>\n";
            return 1;
        }

        std::shared_ptr<PacketReceiveBuffer> receiveBuffPtr = std::make_shared<PacketReceiveBuffer>(100);
        const short port = atoi(argv[1]);
        std::shared_ptr<Server> serverPtr = std::make_shared<Server>(port, receiveBuffPtr);

        std::thread socketThred([](std::shared_ptr<Server> serverPtr){
                serverPtr->run();

            }, serverPtr);

        while(true){
            packet_type packet = receiveBuffPtr->dequeue();
            if(packet != nullptr){
                PacketHeader* pH = reinterpret_cast<PacketHeader*>(packet.get());

                if(pH->purpose == PacketPurpose::ImageJpeg){
                cv::Mat img;
                int64_t camID = 0;
                int64_t timeCode = 0;
                parseJpgImagePacket(packet.get()+sizeof(PacketHeader), img, camID, timeCode);
                cv::imshow("server receive" + std::to_string(camID), img);
                cv::waitKey(1);

                serverPtr->sess_manager.send_all(packet);
                }
            }
        }

        serverPtr->stop();
        socketThred.join();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
#else
#include <boost/format.hpp>

using boost::asio::ip::tcp;
using boost::format;
using namespace std;

int main()
{
    try
    {
        boost::asio::io_service io_service;
        tcp::socket socket(io_service, tcp::endpoint(tcp::v4(), 0));
        tcp::resolver resolver(io_service);
        tcp::resolver::query query(tcp::v4(), "localhost", "7770");
        tcp::resolver::iterator iterator = resolver.resolve(query);

        boost::system::error_code error_code;
        socket.set_option(boost::asio::socket_base::send_buffer_size(128*1024*1024), error_code);
        cout << error_code << endl;
        boost::asio::socket_base::send_buffer_size send_buffer_size;
        socket.get_option(send_buffer_size);
        cout << format("send_buffer_size=%s") % send_buffer_size.value() << endl;

        socket.set_option(boost::asio::socket_base::receive_buffer_size(128*1024*1024), error_code);
        cout << error_code << endl;
        boost::asio::socket_base::receive_buffer_size receive_buffer_size;
        socket.get_option(receive_buffer_size);
        cout << format("receive_buffer_size=%s") % receive_buffer_size.value() << endl;
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
#endif