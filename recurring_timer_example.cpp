#include <iostream>
#include <mutex>
#include <chrono>
#include <functional>
#include <boost/asio.hpp>
#include "ThreadPool.h"

using namespace std;
using namespace std::chrono;
namespace ba = boost::asio;
using asio_utils::ThreadPool;
using namespace std::placeholders;

typedef lock_guard<mutex> Guard;

mutex global_stream_lock;

void TimerHandler(const boost::system::error_code &error,
                  shared_ptr<ba::deadline_timer> timer)
{
   if (error)
   {
      std::cout << "[" << this_thread::get_id() << "] Error: " << error << std::endl;
   }
   else
   {
      std::cout << "[" << this_thread::get_id() << "] TimerHandler " << std::endl;
      timer->expires_from_now(boost::posix_time::seconds(5));
      timer->async_wait(bind(&TimerHandler, _1, timer));
   }
}

int main(int argc, char *argv[])
{

   ThreadPool worker_threads(3);
   auto timer = make_shared<ba::deadline_timer>(*worker_threads.get_io_context());
   timer->expires_from_now(boost::posix_time::seconds(5));
   timer->async_wait(bind(TimerHandler, _1, timer));
   std::cout << "Press [enter] to continue." << std::endl;
   std::cin.get();
   worker_threads.stop();
   worker_threads.reset();
   worker_threads.join_all();

   return 0;
}