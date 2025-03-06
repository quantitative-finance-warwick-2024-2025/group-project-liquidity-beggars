#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "../src/order.hpp"

using namespace trading;

// Create limit order for testing
std::shared_ptr<LimitOrder> createLimitOrderTest(const std::string& traderId, double price, double quantity, bool isBuy) {
    return std::make_shared<LimitOrder>(traderId, price, quantity, isBuy);
}

// Create market order for testing
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
        LimitOrder o1("trader123", 50.5, -100, true);
        LimitOrder o2("trader123", -10.0, 100, true);
        LimitOrder o3("trader123", 50.5, 0, true);
        LimitOrder o4("trader123", 0, 100, true);
        REQUIRE_FALSE(o1.isValid);
        REQUIRE_FALSE(o2.isValid);
        REQUIRE_FALSE(o3.isValid);
        REQUIRE_FALSE(o4.isValid);
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
