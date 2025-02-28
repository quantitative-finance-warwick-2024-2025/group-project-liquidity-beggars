#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "../src/trader.hpp"
#include "../src/exchange.hpp"
#include "../src/order_book.hpp"

using namespace trading;

// Trader class testing

TEST_CASE("Trader", "[trader]") {

    SECTION("Constructor") {
        Exchange* exchange;
        std::string traderid = "trader1";
        Trader trader1(traderid, exchange);
        REQUIRE (trader1.getId() == "trader1");
    }
    
    SECTION("Create Buy Limit Order") {
        Exchange* exchange;
        std::string traderid = "trader1";
        Trader trader1(traderid, exchange);

        std::shared_ptr<LimitOrder> order1 = trader1.createLimitOrder(99.0, 50, true);
        const OrderBook& orderBook = exchange->getOrderBook();
        std::shared_ptr<Order> foundOrder = orderBook.findOrder(order1->getId());

        REQUIRE (foundOrder == order1);
    }

    SECTION("Create Sell Limit Order") {
        Exchange* exchange;
        std::string traderid = "trader1";
        Trader trader1(traderid, exchange);

        std::shared_ptr<LimitOrder> order1 = trader1.createLimitOrder(101.0, 50, false);
        const OrderBook& orderBook = exchange->getOrderBook();
        std::shared_ptr<Order> foundOrder = orderBook.findOrder(order1->getId());

        REQUIRE (foundOrder == order1);
    }

    SECTION("Create Buy Market Order") {
        Exchange* exchange;
        std::string traderid = "trader1";
        Trader trader1(traderid, exchange);

        std::shared_ptr<MarketOrder> order1 = trader1.createMarketOrder(50, true);
        const OrderBook& orderBook = exchange->getOrderBook();
        std::shared_ptr<Order> foundOrder = orderBook.findOrder(order1->getId());

        REQUIRE (foundOrder == order1);
    }

    SECTION("Create Sell Market Order") {
        Exchange* exchange;
        std::string traderid = "trader1";
        Trader trader1(traderid, exchange);

        std::shared_ptr<MarketOrder> order1 = trader1.createMarketOrder(50, false);
        const OrderBook& orderBook = exchange->getOrderBook();
        std::shared_ptr<Order> foundOrder = orderBook.findOrder(order1->getId());

        REQUIRE (foundOrder == order1);
    }

    SECTION("Cancel Order"){
        Exchange* exchange;
        std::string traderid = "trader1";
        Trader trader1(traderid, exchange);
        std::shared_ptr<LimitOrder> order1 = trader1.createLimitOrder(101.0, 50, false);

        trader1.cancelOrder(order1->getId());
    
        const OrderBook& orderBook = exchange->getOrderBook();
        std::shared_ptr<Order> foundOrder = orderBook.findOrder(order1->getId());

        REQUIRE (foundOrder==nullptr);
    }

    SECTION("Modify Order"){
        Exchange* exchange;
        std::string traderid = "trader1";
        Trader trader1(traderid, exchange);
        std::shared_ptr<LimitOrder> order1 = trader1.createLimitOrder(101.0, 50, false);

        trader1.modifyOrder(order1->getId(), 55, 105.0);
        
        const OrderBook& orderBook = exchange->getOrderBook();
        std::shared_ptr<Order> foundOrder = orderBook.findOrder(order1->getId());

        REQUIRE (foundOrder->getPrice() == 105.0);
        REQUIRE (foundOrder->getQuantity() == 55);
    }

}
    