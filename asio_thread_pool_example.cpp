#include <iostream>
#include <iomanip>
#include <memory>
#include <thread>
#include <vector>
#include <boost/asio.hpp>
#include <mutex>

using namespace std;
namespace ba = boost::asio;

mutex global_stream_lock;

void doWork(shared_ptr<ba::io_context> io_context) {
   global_stream_lock.lock();
   std::cout << "[" << this_thread::get_id() << "] Thread Start" << endl;
   global_stream_lock.unlock();

   io_context->run();

   global_stream_lock.lock();
   std::cout << "[" << this_thread::get_id() << "] Thread Stop" << endl;
   global_stream_lock.unlock();
}

int main( int argc, char * argv[] ) {
   auto io_context = make_shared<ba::io_context>();
   auto work = make_shared<ba::io_context::work>(*io_context);

   global_stream_lock.lock();
   std::cout << "Press [return] to exit." << std::endl;
   global_stream_lock.unlock();

   vector<thread> threadPool;
   for( int x = 0; x < 4; ++x ) {
      threadPool.emplace_back(bind(doWork, io_context));
   }

   std::cin.get();

   io_context->stop();

   for (auto& thread : threadPool) {
      if (thread.joinable()) {
         thread.join();
      }
   }

   return 0;
}