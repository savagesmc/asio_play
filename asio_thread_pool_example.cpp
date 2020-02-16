#include <iostream>
#include <iomanip>
#include <memory>
#include <thread>
#include <vector>
#include <boost/asio.hpp>

using namespace std;
namespace ba = boost::asio;

void doWork(shared_ptr<ba::io_context> io_context) {
   std::cout << "Thread Start\n";
   io_context.run();
   std::cout << "Thread Finish\n";
}

int main( int argc, char * argv[] ) {
   auto io_service = make_shared<ba::io_context>();
   auto work = make_shared<ba::io_conext::work>(*io_service);

   std::cout << "Press [return] to exit." << std::endl;

   vector<thread> threadPool;
   for( int x = 0; x < 4; ++x ) {
      threadPool.emplace_back(bind(doWork, io_service));
   }

   std::cin.get();

   io_service.stop();

   for (auto& thread : threadPool) {
      if (thread.joinable()) {
         thread.join();
      }
   }

   return 0;
}