#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "../src/order_book.hpp"
#include "../src/order.hpp"

using namespace trading;

// Helper function to create a limit order
std::shared_ptr<LimitOrder> createLimitOrderTest(const std::string& traderId, double price, double quantity, bool isBuy) {
    return std::make_shared<LimitOrder>(traderId, price, quantity, isBuy);
}

TEST_CASE("PriceLevel Methods", "[PriceLevel]") {

    SECTION("Constructor") {
        PriceLevel level(100.0);
        REQUIRE(level.price == 100.0);
        REQUIRE(level.orders.empty());
    }
    
    SECTION("Add orders") {
        PriceLevel level(100.0);
        auto order1 = createLimitOrderTest("trader1", 99.0, 10.0, true);
        auto order2 = createLimitOrderTest("trader2", 101.0, 20.0, false);
        
        level.addOrder(order1);
        REQUIRE(level.orders.size() == 1);
        
        level.addOrder(order2);
        REQUIRE(level.orders.size() == 2);
    }
    
    SECTION("Remove orders") {
        PriceLevel level(100.0);
        auto order1 = createLimitOrderTest("trader1", 100.0, 10.0, true);
        auto order2 = createLimitOrderTest("trader2", 100.0, 20.0, true);
        
        level.addOrder(order1);
        level.addOrder(order2);
        
        // Remove existing order
        bool removedExistingOrder = level.removeOrder(order1->getId());
        REQUIRE(removedExistingOrder);
        REQUIRE(level.orders.size() == 1);
        
        // Remove non-existing order
        bool removedOrder = level.removeOrder("12345");
        REQUIRE_FALSE(removedOrder);
        REQUIRE(level.orders.size() == 1);
    }
    
    SECTION("Find orders") {
        PriceLevel level(100.0);
        auto order1 = createLimitOrderTest("trader1", 100.0, 10.0, true);
        auto order2 = createLimitOrderTest("trader2", 100.0, 20.0, true);
        
        level.addOrder(order1);
        level.addOrder(order2);
        
        // Find existing order
        auto foundOrder = level.findOrder(order1->getId());
        REQUIRE(foundOrder != nullptr);
        REQUIRE(foundOrder->getId() == order1->getId());
        
        // Find non-existing order
        auto notFoundOrder = level.findOrder("12345");
        REQUIRE(notFoundOrder == nullptr);
    }
}

TEST_CASE("OrderBook Methods", "[OrderBook]") {
    SECTION("Constructor") {
        OrderBook book;
        REQUIRE(book.isEmpty());
        REQUIRE(book.getHighestBid() == nullptr);
        REQUIRE(book.getLowestAsk() == nullptr);
    }
    
    SECTION("Add orders") {
        OrderBook book;
        
        // Add buy orders
        auto buyOrder1 = createLimitOrderTest("trader1", 98.0, 10.0, true);
        auto buyOrder2 = createLimitOrderTest("trader1", 99.0, 20.0, true);
        
        book.addOrder(buyOrder1);
        book.addOrder(buyOrder2);
        
        // Add sell orders
        auto sellOrder1 = createLimitOrderTest("trader2", 101.0, 15.0, false);
        auto sellOrder2 = createLimitOrderTest("trader2", 102.0, 25.0, false);
        
        book.addOrder(sellOrder1);
        book.addOrder(sellOrder2);
        
        // Check best bid/ask
        REQUIRE(book.getHighestBid()->getPrice() == 99.0);
        REQUIRE(book.getLowestAsk()->getPrice() == 101.0);
        
        // Check if not empty
        REQUIRE_FALSE(book.isEmpty());
    }
    
    SECTION("Find orders") {
        OrderBook book;
        
        auto buyOrder = createLimitOrderTest("trader1", 99.0, 10.0, true);
        auto sellOrder = createLimitOrderTest("trader2", 101.0, 15.0, false);
        
        book.addOrder(buyOrder);
        book.addOrder(sellOrder);
        
        // Find existing orders
        auto foundBuyOrder = book.findOrder(buyOrder->getId());
        REQUIRE(foundBuyOrder != nullptr);
        REQUIRE(foundBuyOrder->getId() == buyOrder->getId());
        
        auto foundSellOrder = book.findOrder(sellOrder->getId());
        REQUIRE(foundSellOrder != nullptr);
        REQUIRE(foundSellOrder->getId() == sellOrder->getId());
        
        // Find non-existing order
        auto notFoundOrder = book.findOrder("12345");
        REQUIRE(notFoundOrder == nullptr);
    }
    
    SECTION("Remove orders") {
        OrderBook book;
        
        auto buyOrder = createLimitOrderTest("trader1", 99.0, 10.0, true);
        auto sellOrder = createLimitOrderTest("trader2", 101.0, 15.0, false);
        
        book.addOrder(buyOrder);
        book.addOrder(sellOrder);
        
        // Remove buy order
        bool removedBuyOrder = book.removeOrder(buyOrder->getId());
        REQUIRE(removedBuyOrder);
        REQUIRE(book.findOrder(buyOrder->getId()) == nullptr);
        REQUIRE(book.getHighestBid() == nullptr);
        REQUIRE(book.getLowestAsk()->getPrice() == 101.0);
        
        // Remove sell order
        bool removedSellOrder = book.removeOrder(sellOrder->getId());
        REQUIRE(removedSellOrder);
        REQUIRE(book.findOrder(sellOrder->getId()) == nullptr);
        REQUIRE(book.getLowestAsk() == nullptr);
        
        // Check if empty
        REQUIRE(book.isEmpty());
        
        // Remove non-existing order
        bool removedOrder = book.removeOrder("12345");
        REQUIRE_FALSE(removedOrder);
    }
    
    SECTION("Multiple orders at the same level") {
        OrderBook book;
        
        // Add multiple buy orders at the same price levels
        auto buyOrder1 = createLimitOrderTest("trader1", 100.0, 10.0, true);
        auto buyOrder2 = createLimitOrderTest("trader1", 100.0, 20.0, true);
        
        book.addOrder(buyOrder1);
        book.addOrder(buyOrder2);
        
        // Test time priority within the price level
        REQUIRE(book.getHighestBid()->getId() == buyOrder1->getId());
        
        // Remove the first order
        book.removeOrder(buyOrder1->getId());
        
        // The second order becomes the best bid
        REQUIRE(book.getHighestBid()->getId() == buyOrder2->getId());
    }
    
    SECTION("Select best bid/ask price") {
        OrderBook book;

        auto buyOrder1 = createLimitOrderTest("trader1", 100.0, 10.0, true);
        auto buyOrder2 = createLimitOrderTest("trader1", 101.0, 20.0, true); 
        
        book.addOrder(buyOrder1);
        book.addOrder(buyOrder2);

        // Check highest bid
        REQUIRE(book.getHighestBid()->getPrice() == 101.0);
        
        auto sellOrder1 = createLimitOrderTest("trader2", 103.0, 15.0, false);
        auto sellOrder2 = createLimitOrderTest("trader2", 102.0, 25.0, false); 
        
        book.addOrder(sellOrder1);
        book.addOrder(sellOrder2);

        // Check lowest ask
        REQUIRE(book.getLowestAsk()->getPrice() == 102.0);
    }
}