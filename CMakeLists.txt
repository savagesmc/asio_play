cmake_minimum_required(VERSION 3.0.0)
project(asio_test VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)
enable_testing()

find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread)

set(Boost_USE_STATIC_LIBS        OFF)  # only find static libs
set(Boost_USE_DEBUG_LIBS         OFF) # ignore debug libs and
set(Boost_USE_RELEASE_LIBS       ON)  # only find release libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME     OFF)

set(APPS 
  asio_async_client
  asio_async_server
  asio_test_client
  asio_test_server
  asio_thread_pool_example
)

set(LIBS 
  pthread
  boost_system
  boost_thread
)

if (Boost_FOUND)
  foreach(app ${APPS})
    add_executable(${app} ${app}.cpp)
    target_link_libraries(${app} ${LIBS})
  endforeach()
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
