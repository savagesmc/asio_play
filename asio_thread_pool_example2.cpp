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
mutex global_stream_lock;

// The ThreadPool class holds a collection of threads that service their own
// IO context.
class ThreadPool
{
   int num_threads_;
   shared_ptr<ba::io_context> io_context_;
   ba::executor_work_guard<ba::io_context::executor_type> work_guard_;
   std::vector<thread> threads_;
public:
   void work()
   {
      {
         Guard locked(global_stream_lock);
         std::cout << "[" << this_thread::get_id() << "] ThreadPool Thread Start" << endl;
      }
      io_context_->run();
      {
         Guard locked(global_stream_lock);
         std::cout << "[" << this_thread::get_id() << "] ThreadPool Thread Stop" << endl;
      }
   }

   ThreadPool(int num_threads) 
   : num_threads_(num_threads)
   , io_context_(make_shared<ba::io_context>())
   , work_guard_(ba::make_work_guard(*io_context_))
   , threads_()
   {
      for (int x = 0; x < num_threads_; ++x)
      {
         threads_.emplace_back(bind(&ThreadPool::work, this));
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

   shared_ptr<ba::io_context::strand> get_strand()
   {
      return make_shared<ba::io_context::strand>(*io_context_);
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

   void reset()
   {
      work_guard_.reset();
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
   }
   else if (example == 1)
   {
      cout << "post vs. dispatch test" << endl;
      ThreadPool pool(1); // 1 thread
      pool.post(bind(Run3, pool.get_io_context()));
      pool.reset();
   }
   else if (example == 2)
   {
      cout << "post test (ordering)" << endl;
      ThreadPool pool(3); // 3 threads
      pool.post(bind(PrintNum, 1));
      pool.post(bind(PrintNum, 2));
      pool.post(bind(PrintNum, 3));
      pool.post(bind(PrintNum, 4));
      pool.post(bind(PrintNum, 5));
      pool.reset();
   }
   else if (example == 3)
   {
      cout << "post + strand test (strands are ordered)" << endl;
      ThreadPool pool(3); // 3 threads
      auto strand = pool.get_strand();
      pool.post(bind(PrintNum, 1));
      strand->post(bind(PrintNum, 6));
      pool.post(bind(PrintNum, 2));
      strand->post(bind(PrintNum, 7));
      pool.post(bind(PrintNum, 3));
      strand->post(bind(PrintNum, 8));
      pool.post(bind(PrintNum, 4));
      strand->post(bind(PrintNum, 9));
      pool.post(bind(PrintNum, 5));
      strand->post(bind(PrintNum, 10));
      pool.reset();
   }

   return 0;
}