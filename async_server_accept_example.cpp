#include <iostream>
#include <mutex>
#include <chrono>
#include <functional>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include "ThreadPool.h"

using namespace std;
using namespace std::chrono;
namespace ba = boost::asio;
using asio_utils::ThreadPool;
using namespace std::placeholders;
using boost::lexical_cast;

typedef lock_guard<mutex> Guard;

mutex global_stream_lock;

void OnAccept(const boost::system::error_code &ec, shared_ptr<ba::ip::tcp::socket> sock)
{
    if (ec)
    {
        std::cout << "[" << this_thread::get_id() << "] Error: " << ec << std::endl;
    }
    else
    {
        std::cout << "[" << this_thread::get_id() << "] Accepted!" << std::endl;
    }
}

int main(int argc, char *argv[])
{
   ThreadPool worker_threads(3);

   std::cout << "Press [return] to exit." << std::endl;

   auto acceptor = make_shared<ba::ip::tcp::acceptor>(*worker_threads.get_io_context());
   auto sock = make_shared<ba::ip::tcp::socket>(*worker_threads.get_io_context());

   try
   {
      ba::ip::tcp::resolver resolver(*worker_threads.get_io_context());
      ba::ip::tcp::resolver::query query(
         "127.0.0.1",
         boost::lexical_cast<std::string>(7777));
      ba::ip::tcp::endpoint endpoint = *resolver.resolve(query);
      acceptor->open(endpoint.protocol());
      acceptor->set_option(ba::ip::tcp::acceptor::reuse_address(false));
      acceptor->bind(endpoint);
      acceptor->listen(boost::asio::socket_base::max_connections);
      acceptor->async_accept(*sock, bind(OnAccept, _1, sock));

      std::cout << "Listening on: " << endpoint << std::endl;
   }
   catch (std::exception &ex)
   {
      std::cout << "[" << this_thread::get_id() << "] Exception: " << ex.what() << std::endl;
   }

   std::cin.get();

   boost::system::error_code ec;
   acceptor->close(ec);
   sock->shutdown(ba::ip::tcp::socket::shutdown_both, ec);
   sock->close(ec);

   worker_threads.stop();
   worker_threads.reset();
   worker_threads.join_all();

   return 0;
}