#include <iostream>
#include <iomanip>
#include <memory>
#include <thread>
#include <vector>
#include <boost/asio.hpp>
#include <mutex>
#include <chrono>
#include "ThreadPool.h"

using namespace std;
using namespace std::chrono;
namespace ba = boost::asio;
using asio_utils::ThreadPool;

typedef lock_guard<mutex> Guard;

// Thread pool size
mutex global_stream_lock;

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

// NOTE: No mutex lock here - so if multiple threads are 
// calling this at the same time, can get mixed up output
void PrintNum(int x)
{
   std::cout << "[" << this_thread::get_id()
             << "] x: " << x << std::endl;
}

int main(int argc, char *argv[])
{
   int example = 0;
   if (argc>=1)
   {
      example = atoi(argv[1]);
   }

   if (example == 0)
   {
      cout << "Fibonacci Test" << endl;
      ThreadPool pool(2); // 2 threads
      pool.post(bind(CalculateFib, 3));
      pool.post(bind(CalculateFib, 4));
      pool.post(bind(CalculateFib, 5));
      pool.reset();
      pool.join_all();
   }
   else if (example == 1)
   {
      cout << "post vs. dispatch test" << endl;
      ThreadPool pool(1); // 1 thread
      pool.post(bind(Run3, pool.get_io_context()));
      pool.reset();
      pool.join_all();
   }
   else if (example == 2)
   {
      cout << "post test (no enforced ordering, and thread exclusion)" << endl;
      ThreadPool pool(3); // 3 threads
      pool.post(bind(PrintNum, 1));
      pool.post(bind(PrintNum, 2));
      pool.post(bind(PrintNum, 3));
      pool.post(bind(PrintNum, 4));
      pool.post(bind(PrintNum, 5));
      pool.reset();
      pool.join_all();
   }
   else if (example == 3)
   {
      cout << "strand test (strands are ordered - and no thread contention)" << endl;
      ThreadPool pool(3); // 3 threads
      auto strand = pool.get_strand();
      strand->post(bind(PrintNum, 1));
      strand->post(bind(PrintNum, 2));
      strand->post(bind(PrintNum, 3));
      strand->post(bind(PrintNum, 4));
      strand->post(bind(PrintNum, 5));
      pool.reset();
      pool.join_all();
   }

   return 0;
}