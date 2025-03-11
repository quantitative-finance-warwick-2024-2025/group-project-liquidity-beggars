#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "../src/trader.hpp"
#include "../src/exchange.hpp"
#include "../src/order_book.hpp"

using namespace trading;

// Helper function to create a limit order
std::shared_ptr<LimitOrder> createLimitOrderTest(const std::string& traderId, double price, double quantity, bool isBuy) {
    return std::make_shared<LimitOrder>(traderId, price, quantity, isBuy);
}

// Helper function to create a market order
std::shared_ptr<MarketOrder> createMarketOrderTest(const std::string& traderId, double quantity, bool isBuy) {
    return std::make_shared<MarketOrder>(traderId, quantity, isBuy);
}

// Helper function to compare the orders
bool compare_orders(std::shared_ptr<Order> order1, std::shared_ptr<Order> order2) {
    if (order1->getTraderId() == order2->getTraderId() &&
        order1->getPrice() == order2->getPrice() &&
        order1->getQuantity() == order2->getQuantity() &&
        order1->getType() == order2->getType() && 
        order1->isBuyOrder() == order2->isBuyOrder()){
        return true;
    }
    else {
        return false;
    }
}

TEST_CASE("Trader", "[trader]") {

    SECTION("Constructor") {
        Exchange exchange;
        Trader trader1(&exchange);
        REQUIRE (trader1.getId() == "TRD-1");
    }
    
    SECTION("Create Buy Limit Order") {
        Exchange exchange;
        Trader trader1(&exchange);

        auto order1 = trader1.createLimitOrder(99.0, 50, true);
        auto test_order1 = createLimitOrderTest(trader1.getId(), 99.0, 50, true);

        CAPTURE(order1->getTraderId(),
                order1->getPrice(),
                order1->getQuantity(),
                order1->getType(),
                order1->isBuyOrder());
        
        CAPTURE(test_order1->getTraderId(),
                test_order1->getPrice(),
                test_order1->getQuantity(),
                test_order1->getType(),
                test_order1->isBuyOrder());

        REQUIRE (compare_orders(order1, test_order1) == true);
    }

    SECTION("Create Sell Limit Order") {
        Exchange exchange;
        Trader trader1(&exchange);

        auto order1 = trader1.createLimitOrder(99.0, 50, false);
        auto test_order1 = createLimitOrderTest(trader1.getId(), 99.0, 50, false);

        REQUIRE (compare_orders(order1, test_order1) == true);
    }

    SECTION("Create Buy Market Order") {
        Exchange exchange;
        Trader trader1(&exchange);

        auto order1 = trader1.createMarketOrder(50, true);
        auto test_order1 = createMarketOrderTest(trader1.getId(), 50, true);

        REQUIRE (compare_orders(order1, test_order1) == true);
    }

    SECTION("Create Sell Market Order") {
        Exchange exchange;
        Trader trader1(&exchange);

        auto order1 = trader1.createMarketOrder(50, false);
        auto test_order1 = createMarketOrderTest(trader1.getId(), 50, false);

        REQUIRE (compare_orders(order1, test_order1) == true);
    }
}
    