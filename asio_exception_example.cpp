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

mutex global_stream_lock;

void RaiseAnException(shared_ptr<ba::io_context> io_context)
{
   {
      Guard locked(global_stream_lock);
      std::cout << "[" << this_thread::get_id()
               << "] " << __FUNCTION__ << std::endl;
   }

   io_context->post(bind(&RaiseAnException, io_context));

   throw(std::runtime_error("Oops!"));
}

int main(int argc, char *argv[])
{
   {
      Guard locked(global_stream_lock);
      std::cout << "[" << this_thread::get_id()
               << "] The program will exit when all work has finished." << std::endl;
   }
   ThreadPool worker_threads(3);
   worker_threads.post(bind(&RaiseAnException, worker_threads.get_io_context()));
   worker_threads.reset();
   worker_threads.join_all();
   return 0;
}