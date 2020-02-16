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

typedef std::lock_guard<mutex> Guard;

// Thread pool size
mutex global_stream_lock;

// The ThreadPool class holds a collection of threads that service their own
// IO context.
class ThreadPool
{
   shared_ptr<ba::io_context> io_context_;
   shared_ptr<ba::io_context::work> work_;
   std::vector<thread> threads_;
public:
   void work(shared_ptr<ba::io_context> io_context)
   {
      {
         Guard locked(mutex);
         std::cout << "[" << this_thread::get_id() << "] ThreadPool Thread Start" << endl;
      }
      io_context->run();
      {
         Guard locked(mutex);
         std::cout << "[" << this_thread::get_id() << "] ThreadPool Thread Stop" << endl;
      }
   }
   ThreadPool(int num_threads) 
   : io_context_(make_shared<ba::io_context>())
   , threads_()
   {
      for (int x = 0; x < num_threads; ++x)
      {
         threads_.emplace_back(bind(&ThreadPool::work, this, io_context_));
      }
   }

   ~ThreadPool()
   {
      for (auto &thread : threads_)
      {
         if (thread.joinable())
         {
            thread.join();
         }
      }
   }

   shared_ptr<ba::io_context> get_io_context()
   {
      return io_context_;
   }

   template<class Task>
   void post(Task task) 
   {
      io_context_->post(task);
   }

   template<class Task>
   void dipatch(Task task) 
   {
      io_context_->dispatch(task);
   }

   void stop()
   {
      io_context_->stop();
   }

};

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
      Guard locked(mutex);
      std::cout << "[" << this_thread::get_id()
               << "] Now calculating fib( " << n << " ) " << std::endl;
   }

   size_t f = fib(n);

   {
      Guard locked(mutex);
      std::cout << "[" << this_thread::get_id()
               << "] fib( " << n << " ) = " << f << std::endl;
   }
}

void Dispatch(int x)
{
   {
      Guard locked(mutex);
      std::cout << "[" << this_thread::get_id() << "] "
               << __FUNCTION__ << " x = " << x << std::endl;
   }
}

void Post(int x)
{
   {
      Guard locked(mutex);
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

void PrintNum(int x)
{
   std::cout << "[" << this_thread::get_id()
             << "] x: " << x << std::endl;
}

int main(int argc, char *argv[])
{
   const int example = 2;
   if (example == 0)
   {
      ThreadPool pool(2); // 2 threads
      pool.post(bind(CalculateFib, 3));
      pool.post(bind(CalculateFib, 4));
      pool.post(bind(CalculateFib, 5));
   }
   else if (example == 1)
   {
      ThreadPool pool(1); // 1 thread
      pool.post(bind(Run3, pool.get_io_context()));
   }
   else if (example == 2)
   {
      ThreadPool pool(3); // 1 thread
      pool.post(bind(PrintNum, 1));
      pool.post(bind(PrintNum, 2));
      pool.post(bind(PrintNum, 3));
      pool.post(bind(PrintNum, 4));
      pool.post(bind(PrintNum, 5));
   }
   else if (example == 3)
   {

   }

   return 0;
}