#include <iostream>
#include <iomanip>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>

using namespace std;
namespace ba = boost::asio;

using ba::ip::tcp;

class con_handler : public boost::enable_shared_from_this<con_handler>
{
private:
  tcp::socket sock;
  std::string message="Hello From Server!";
  enum { max_length = 1024 };
  char data[max_length];

public:
  typedef boost::shared_ptr<con_handler> pointer;
  con_handler(boost::asio::io_context& io_context): sock(io_context){}

  // creating the pointer
  static pointer create(boost::asio::io_context& io_context)
  {
    return pointer(new con_handler(io_context));
  }

  //socket creation
  tcp::socket& socket()
  {
    return sock;
  }

  void start()
  {
    sock.async_read_some(
        boost::asio::buffer(data, max_length),
        boost::bind(&con_handler::handle_read,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
  
    sock.async_write_some(
        boost::asio::buffer(message, max_length),
        boost::bind(&con_handler::handle_write,
                  shared_from_this(),
                  boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));
  }

  void handle_read(const boost::system::error_code& err, size_t bytes_transferred)
  {
    if (!err) {
         cout << data << endl;
    } else {
         std::cerr << "error: " << err.message() << std::endl;
         sock.close();
    }
  }
  void handle_write(const boost::system::error_code& err, size_t bytes_transferred)
  {
    if (!err) {
       cout << "Server sent Hello message!"<< endl;
    } else {
       std::cerr << "error: " << err.message() << endl;
       sock.close();
    }
  }
};

class Server 
{
private:
   ba::io_context& io_context_;
   tcp::acceptor acceptor_;

   void start_accept()
   {
    // socket
     con_handler::pointer connection = con_handler::create(io_context_);

    // asynchronous accept operation and wait for a new connection.
     acceptor_.async_accept(connection->socket(),
        boost::bind(&Server::handle_accept, this, connection,
        boost::asio::placeholders::error));
  }

public:

  //constructor for accepting connection from client
  Server(boost::asio::io_context& io_context)
  : io_context_(io_context)
  , acceptor_(io_context_, tcp::endpoint(tcp::v4(), 1234))
  {
     start_accept();
  }

  void handle_accept(con_handler::pointer connection, const boost::system::error_code& err)
  {
    if (!err) {
      connection->start();
    }
    start_accept();
  }

};

int main(int argc, char *argv[])
{
  try
  {
    boost::asio::io_context io_context;  
    Server server(io_context);
    io_context.run();
  }
  catch(std::exception& e)
  {
    std::cerr << e.what() << endl;
  }
  return 0;
}