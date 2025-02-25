add_executable(order_tests ${SRC_FILES} order_tests.cpp)
target_include_directories(order_tests PUBLIC ${CMAKE_SOURCE_DIR}/extern/catch2 ${CMAKE_SOURCE_DIR}/src)
add_test(NAME order_tests COMMAND order_tests)

add_executable(trader_tests ${SRC_FILES} trader_tests.cpp)
target_include_directories(trader_tests PUBLIC ${CMAKE_SOURCE_DIR}/extern/catch2 ${CMAKE_SOURCE_DIR}/src)
add_test(NAME trader_tests COMMAND trader_tests)

add_executable(order_book_tests ${SRC_FILES} order_book_tests.cpp)
target_include_directories(order_book_tests PUBLIC ${CMAKE_SOURCE_DIR}/extern/catch2 ${CMAKE_SOURCE_DIR}/src)
add_test(NAME order_book_tests COMMAND order_book_tests)

add_executable(exchange_tests ${SRC_FILES} exchange_tests.cpp)
target_include_directories(exchange_tests PUBLIC ${CMAKE_SOURCE_DIR}/extern/catch2 ${CMAKE_SOURCE_DIR}/src)
add_test(NAME exchange_tests COMMAND exchange_tests)
