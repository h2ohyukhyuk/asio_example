
#include "socket_sesseion.h"
#include <opencv2/opencv.hpp>

class server
{
public:
  server(boost::asio::io_service& io_service, short port)
  //boost 1.66이후 (Ubuntu 18.10 이후) 버전의 경우 io_context를 사용
  //server(boost::asio::io_context& io_service, short port)
    : io_service_(io_service),
      //PORT 번호 등록
      acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
  {
    start_accept();
  }
 
private:
  void start_accept()
  {
    session* new_session = new session(io_service_);
    //client로부터 접속될 때 까지 대기한다.
    acceptor_.async_accept(new_session->socket(),
        boost::bind(&server::handle_accept, this, new_session,
          boost::asio::placeholders::error));
  }
 
  //client로부터 접속이 되었을 때 해당 handler 함수를 실행한다.
  void handle_accept(session* new_session,
      const boost::system::error_code& error)
  {
    std::cout<<"new client connected"<<std::endl;
    if (error)
    {
        std::cerr<<"socket accept error: "<<error<<std::endl;
        delete new_session;
    }
    else
    {
        new_session->print_bufsize();
        new_session->read();
    }
    //client로부터 접속이 끊겼을 대 다시 대기한다.
    start_accept();
  }
 
  boost::asio::io_service& io_service_;
  //boost 1.66이후 (Ubuntu 18.10 이후) 버전의 경우 io_context를 사용
  //boost::asio::io_context &io_service_;
  tcp::acceptor acceptor_;
};
 
 #if 1
int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: async_tcp_echo_server <port>\n";
      return 1;
    }
 
    boost::asio::io_service io_service;
    //boost 1.66이후 (Ubuntu 18.10 이후) 버전의 경우 io_context를 사용
    //boost::asio::io_context io_service;
 
    server s(io_service, atoi(argv[1]));

    //asio 통신을 시작한다.
    io_service.run();
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