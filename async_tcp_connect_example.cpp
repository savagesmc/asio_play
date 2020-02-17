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

void OnConnect(const boost::system::error_code &ec, shared_ptr<ba::ip::tcp::socket> sock)
{
   if (ec)
   {
      std::cout << "[" << this_thread::get_id() << "] Error: " << ec << std::endl;
   }
   else
   {
      std::cout << "[" << this_thread::get_id() << "] Connected!" << std::endl;
   }
}

int main(int argc, char *argv[])
{

   ThreadPool worker_threads(3);
   std::cout << "Press [enter] to continue." << std::endl;

   auto sock = make_shared<ba::ip::tcp::socket>(*worker_threads.get_io_context());

   try
   {
      ba::ip::tcp::resolver resolver(*worker_threads.get_io_context());
      ba::ip::tcp::resolver::query query(
            "www.google.com",
            lexical_cast<std::string>(80));
      ba::ip::tcp::resolver::iterator iterator = resolver.resolve(query);
      ba::ip::tcp::endpoint endpoint = *iterator;
      std::cout << "Connecting to: " << endpoint << std::endl;
      sock->async_connect(endpoint, bind(OnConnect, _1, sock));
   }
   catch (const std::exception& e)
   {
      std::cerr << "[" << this_thread::get_id() << "] Exception: " << e.what() << std::endl;
   }

   std::cin.get();

   boost::system::error_code ec;
   sock->shutdown(ba::ip::tcp::socket::shutdown_both, ec);
   sock->close(ec);

   worker_threads.stop();
   worker_threads.reset();
   worker_threads.join_all();

   return 0;
}