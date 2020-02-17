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

typedef lock_guard<mutex> Guard;

// Thread pool size
const int numThreads = 1;
mutex global_stream_lock;

void doWork(shared_ptr<ba::io_context> io_context)
{
   {
      Guard locked(global_stream_lock);
      std::cout << "[" << this_thread::get_id() << "] Thread Start" << endl;
   }

   io_context->run();

   {
      Guard locked(global_stream_lock);
      std::cout << "[" << this_thread::get_id() << "] Thread Stop" << endl;
   }
}

void Dispatch(int x)
{
   {
      Guard locked(global_stream_lock);
      std::cout << "[" << this_thread::get_id() << "] "
               << __FUNCTION__ << " x = " << x << std::endl;
   }
}

void Post(int x)
{
   {
      Guard locked(global_stream_lock);
      std::cout << "[" << this_thread::get_id() << "] "
               << __FUNCTION__ << " x = " << x << std::endl;
   }
}

void Run3(shared_ptr<ba::io_context> io_context)
{
   for (int x = 0; x < 3; ++x)
   {
      io_context->dispatch(bind(&Dispatch, x * 2));
      io_context->post(bind(&Post, x * 2 + 1));
      this_thread::sleep_for(milliseconds(1000));
   }
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
   {
      Guard locked(global_stream_lock);
      std::cout << "[" << this_thread::get_id()
               << "] Now calculating fib( " << n << " ) " << std::endl;
   }

   size_t f = fib(n);

   {
      Guard locked(global_stream_lock);
      std::cout << "[" << this_thread::get_id()
               << "] fib( " << n << " ) = " << f << std::endl;
   }
}

int main(int argc, char *argv[])
{
   auto io_context = make_shared<ba::io_context>();
   auto work = make_shared<ba::io_context::work>(*io_context);

   {
      Guard locked(global_stream_lock);
      std::cout << "Press [return] to exit." << std::endl;
   }

   vector<thread> threadPool;
   for (int x = 0; x < numThreads; ++x)
   {
      threadPool.emplace_back(bind(doWork, io_context));
   }

   #if COMMENT
   io_context->post(bind(CalculateFib, 3));
   io_context->post(bind(CalculateFib, 4));
   io_context->post(bind(CalculateFib, 5));
   #endif

   io_context->post(bind(Run3, io_context));

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