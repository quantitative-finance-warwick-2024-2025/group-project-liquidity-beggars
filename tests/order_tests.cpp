#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "../src/order.hpp"

using namespace trading;

// Helper function to create a limit order
std::shared_ptr<LimitOrder> createLimitOrderTest(const std::string& traderId, double price, double quantity, bool isBuy) {
    return std::make_shared<LimitOrder>(traderId, price, quantity, isBuy);
}

// Helper function to create a market order
std::shared_ptr<MarketOrder> createMarketOrderTest(const std::string& traderId, double quantity, bool isBuy) {
    return std::make_shared<MarketOrder>(traderId, quantity, isBuy);
}

TEST_CASE("LimitOrder Creation", "[LimitOrder]") {

    SECTION("Valid Limit Order") {
        REQUIRE_NOTHROW(LimitOrder("trader123", 50.5, 100, true));

        auto order = createLimitOrderTest("trader123", 50.5, 100, true);
        REQUIRE(order->getPrice() == 50.5);
        REQUIRE(order->getQuantity() == 100);
        REQUIRE(order->isBuyOrder() == true);
    }

    SECTION("Invalid Limit Orders") {
        REQUIRE_THROWS_AS(LimitOrder("trader123", 50.5, -100, true), std::invalid_argument);
        REQUIRE_THROWS_AS(LimitOrder("trader123", -10.0, 100, true), std::invalid_argument);
        REQUIRE_THROWS_AS(LimitOrder("trader123", 50.5, 0, true), std::invalid_argument);
        REQUIRE_THROWS_AS(LimitOrder("trader123", 0, 100, true), std::invalid_argument);
    }
}

TEST_CASE("MarketOrder Creation", "[MarketOrder]") {

    SECTION("Valid Market Order") {
        REQUIRE_NOTHROW(MarketOrder("trader123", 100, true));

        auto order = createMarketOrderTest("trader123", 200, false);
        REQUIRE(order->getQuantity() == 200);
        REQUIRE(order->isBuyOrder() == false);
    }
}
