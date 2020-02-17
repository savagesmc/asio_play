#include <ios>
#include <memory>
#include <thread>
#include <vector>
#include <boost/asio.hpp>

namespace asio_utils
{

namespace ba = boost::asio;

// The ThreadPool class holds a collection of threads that service their own
// IO context.
class ThreadPool
{
   int num_threads_;
   bool playThrough_;
   std::shared_ptr<ba::io_context> io_context_;
   ba::executor_work_guard<ba::io_context::executor_type> work_guard_;
   std::vector<std::thread> threads_;
public:
   ThreadPool(int num_threads, bool playThroughExceptions=false) 
   : num_threads_(num_threads)
   , playThrough_(playThroughExceptions)
   , io_context_(std::make_shared<ba::io_context>())
   , work_guard_(ba::make_work_guard(*io_context_))
   , threads_()
   {
      for (int x = 0; x < num_threads_; ++x)
      {
         threads_.emplace_back(std::bind(&ThreadPool::worker, this));
      }
   }

   ~ThreadPool()
   {
      reset();
      join_all();
   }

   // Get the io context used by this thread pool
   std::shared_ptr<ba::io_context> get_io_context()
   {
      return io_context_;
   }

   // Create a new strand that is tied to this thread pool's 
   // io context.
   std::shared_ptr<ba::io_context::strand> get_strand()
   {
      return std::make_shared<ba::io_context::strand>(*io_context_);
   }

   // Posts task to the io_context 'queue'
   template<class Task>
   void post(Task task) 
   {
      io_context_->post(task);
   }

   // Dispatch task - if this function is being run from 
   // an io_context, then it may run immediately
   template<class Task>
   void dipatch(Task task) 
   {
      io_context_->dispatch(task);
   }

   // Force stop the io context
   void stop()
   {
      io_context_->stop();
   }

   // Allow the io_context to complete once all current work is 
   // finished
   void reset()
   {
      work_guard_.reset();
   }

   // Wait for all of the threads in the pool to complete
   void join_all()
   {
      for (auto &thread : threads_)
      {
         if (thread.joinable())
         {
            thread.join();
         }
      }
   }

private:
 
   // 'work' wrapper for each of the threads
   void worker() 
   { 
      while(true)
      {
         try
         {
            boost::system::error_code ec;
            io_context_->run(ec);
            if (ec)
            {
               std::cerr << "ThreadPool::error: " << ec << "\n";
            }
            else
            {
               break;
            }
         }
         catch (const std::exception& e)
         {
            std::cerr << "ThreadPool::exception - " << e.what() << "\n";
         }
         if (!playThrough_)
         {
            break;
         }
      }
   }

};

}