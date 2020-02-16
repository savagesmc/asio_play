#include <iostream>
#include <iomanip>
#include <memory>
#include <thread>
#include <vector>
#include <boost/asio.hpp>
#include <mutex>
#include <chrono>

using namespace std;
using namespace std::chrono;
namespace ba = boost::asio;

// Thread pool size
const int numThreads = 2;
mutex global_stream_lock;

void doWork(shared_ptr<ba::io_context> io_context)
{
   global_stream_lock.lock();
   std::cout << "[" << this_thread::get_id() << "] Thread Start" << endl;
   global_stream_lock.unlock();

   io_context->run();

   global_stream_lock.lock();
   std::cout << "[" << this_thread::get_id() << "] Thread Stop" << endl;
   global_stream_lock.unlock();
}

size_t fib(size_t n)
{
   if (n <= 1)
   {
      return n;
   }
   this_thread::sleep_for(milliseconds(1000));
   return fib(n - 1) + fib(n - 2);
}

void CalculateFib(size_t n)
{
   global_stream_lock.lock();
   std::cout << "[" << this_thread::get_id()
             << "] Now calculating fib( " << n << " ) " << std::endl;
   global_stream_lock.unlock();

   size_t f = fib(n);

   global_stream_lock.lock();
   std::cout << "[" << this_thread::get_id()
             << "] fib( " << n << " ) = " << f << std::endl;

   global_stream_lock.unlock();
}

int main(int argc, char *argv[])
{
   auto io_context = make_shared<ba::io_context>();
   auto work = make_shared<ba::io_context::work>(*io_context);

   global_stream_lock.lock();
   std::cout << "Press [return] to exit." << std::endl;
   global_stream_lock.unlock();

   vector<thread> threadPool;
   for (int x = 0; x < numThreads; ++x)
   {
      threadPool.emplace_back(bind(doWork, io_context));
   }

   io_context->post(bind(CalculateFib, 3));
   io_context->post(bind(CalculateFib, 4));
   io_context->post(bind(CalculateFib, 5));

   work.reset();

   for (auto &thread : threadPool)
   {
      if (thread.joinable())
      {
         thread.join();
      }
   }

   return 0;
}