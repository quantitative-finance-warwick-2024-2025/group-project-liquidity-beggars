#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "../src/exchange.hpp"
#include "../src/trader.hpp"
#include "../src/order.hpp"
#include "../src/order_book.hpp"

using namespace trading;

// Helper function to create a limit order
std::shared_ptr<LimitOrder> createTestLimitOrder(const std::string& traderId,
                                                 double price,
                                                 double quantity,
                                                 bool isBuy)
{
    return std::make_shared<LimitOrder>(traderId, price, quantity, isBuy);
}

// Helper function to create a market order
std::shared_ptr<MarketOrder> createTestMarketOrder(const std::string& traderId,
                                                   double quantity,
                                                   bool isBuy)
{
    return std::make_shared<MarketOrder>(traderId, quantity, isBuy);
}

TEST_CASE("Exchange - Basic Functionality", "[Exchange]") 
{
    Exchange exchange;

    SECTION("Initial state") {
        REQUIRE(exchange.getTrades().empty());
        REQUIRE(exchange.getOrderBook().isEmpty());
    }

    SECTION("Register trader") {
        auto trader1 = exchange.registerTrader();
        REQUIRE(trader1 -> getId() == "TRD-1");

        // Register another trader
        auto trader2 = exchange.registerTrader();
        REQUIRE(trader2 -> getId() == "TRD-2");
        REQUIRE(&trader2 != &trader1);

        // The exchange must store multiple traders
        REQUIRE_FALSE(&trader2 == &trader1);
    }
}

TEST_CASE("Exchange - Submit Orders & Match Logic", "[Exchange]")
{
    Exchange exchange;
    auto trader1 = exchange.registerTrader();
    auto trader2 = exchange.registerTrader();

    SECTION("Submitting a single limit order - no match") {
        auto buyOrder = trader1 -> createLimitOrder(100.0, 10.0, true); 
        std::vector<Trade> trades = exchange.submitOrder(buyOrder);

        REQUIRE(trades.empty()); // No opposing orders to match with
        REQUIRE_FALSE(exchange.getOrderBook().isEmpty());

        auto highestBid = exchange.getOrderBook().getHighestBid();
        REQUIRE(highestBid);
        REQUIRE(highestBid->getId() == buyOrder->getId());

        REQUIRE(exchange.getTrades().empty());
    }

    SECTION("Limit order match - full fill") {
        // Submit a sell limit order
        auto sellOrder = trader2 -> createLimitOrder(105.0, 10.0, false);
        exchange.submitOrder(sellOrder);

        // Submit a buy limit order with quantity 10, crossing the ask
        // Price crosses if buy price >= sell price
        auto buyOrder = trader1 -> createLimitOrder(105.0, 10.0, true);
        std::vector<Trade> trades = exchange.submitOrder(buyOrder);

        // Should fully match, so 1 trade
        REQUIRE(trades.size() == 1);
        REQUIRE(trades[0].buyOrderId == buyOrder->getId());
        REQUIRE(trades[0].sellOrderId == sellOrder->getId());
        REQUIRE(trades[0].price == Approx(105.0));
        REQUIRE(trades[0].quantity == Approx(10.0));

        // Order book should be empty because both orders fully filled
        REQUIRE(exchange.getOrderBook().isEmpty());
        REQUIRE(exchange.getTrades().size() == 1);
    }

    SECTION("Partial fill scenario") {
        // Trader2 places a sell order (quantity 20)
        auto sellOrder = trader2 -> createLimitOrder(101.0, 20.0, false);
        exchange.submitOrder(sellOrder);

        // Trader1 places a buy order with quantity 10, same price 101
        auto buyOrder = trader1 -> createLimitOrder(101.0, 10.0, true);
        auto resultTrades = exchange.submitOrder(buyOrder);

        // Should produce a single trade (10 units)
        REQUIRE(resultTrades.size() == 1);
        REQUIRE(resultTrades[0].quantity == Approx(10.0));
        REQUIRE(resultTrades[0].price == Approx(101.0));

        // The sell order was partially filled -> 10 out of 20 left
        auto remainingSell = exchange.getOrderBook().getLowestAsk();
        REQUIRE(remainingSell);
        REQUIRE(remainingSell->getQuantity() == Approx(10.0));

        // The buy order is fully filled, so no leftover in the book
        REQUIRE(exchange.getOrderBook().getHighestBid() == nullptr);

        // The exchange's trades vector should also have recorded it
        REQUIRE(exchange.getTrades().size() == 1);
    }

    SECTION("Market order scenario") {
        // Place a sell limit order in the book
        auto sellOrder1 = trader2 -> createLimitOrder(100.0, 15.0, false);
        auto sellOrder2 = trader2 ->createLimitOrder(99.0, 10.0,  false);
        exchange.submitOrder(sellOrder1);
        exchange.submitOrder(sellOrder2);

        // Trader1 places a MARKET buy order with quantity 20
        auto marketBuy = trader1 -> createMarketOrder(20.0, true);
        auto executed = exchange.submitOrder(marketBuy);

        // We expect the market order to match from the best ask upwards
        // The best ask is 99.0 (sellOrder2), then next is 100.0 (sellOrder1)
        REQUIRE(executed.size() == 2);

        // First trade: buy 10 @ 99
        REQUIRE(executed[0].price == Approx(99.0));
        REQUIRE(executed[0].quantity == Approx(10.0));
        REQUIRE(executed[0].sellOrderId == sellOrder2->getId());

        // Second trade: buy 10 @ 100 (because the total buy quantity was 20)
        REQUIRE(executed[1].price == Approx(100.0));
        REQUIRE(executed[1].quantity == Approx(10.0));
        REQUIRE(executed[1].sellOrderId == sellOrder1->getId());

        // The buy order had quantity 20, so 10 filled from the 99.0 order,
        // and 10 from the 100.0 order, the second sell order still has 5 left (15 - 10)
        auto remainingSell = exchange.getOrderBook().findOrder(sellOrder1->getId());
        REQUIRE(remainingSell);
        REQUIRE(remainingSell->getQuantity() == Approx(5.0));

        // The 99.0 ask (sellOrder2) should be fully consumed and removed
        REQUIRE(exchange.getOrderBook().findOrder(sellOrder2->getId()) == nullptr);
    }
}

TEST_CASE("Exchange - Cancel & Modify Orders", "[Exchange]")
{
    Exchange exchange;
    auto trader1 = exchange.registerTrader();
    auto trader2 = exchange.registerTrader();

    SECTION("Cancel a limit order") {
        auto buyOrder = trader1 -> createLimitOrder(100.0, 10.0, true);
        exchange.submitOrder(buyOrder);
        REQUIRE_FALSE(exchange.getOrderBook().isEmpty());

        bool cancelled = exchange.cancelOrder(buyOrder->getId());
        REQUIRE(cancelled);
        REQUIRE(exchange.getOrderBook().isEmpty());

        // Attempt to cancel again should fail
        bool cancelledAgain = exchange.cancelOrder(buyOrder->getId());
        REQUIRE_FALSE(cancelledAgain);
    }

    SECTION("Modify limit order - immediate re‐match if crossing") {
        // Place a sell order at price 105
        auto sellOrder = trader2 -> createLimitOrder(105.0, 10.0, false);
        exchange.submitOrder(sellOrder);

        // Place a buy order at price 100 (no match yet)
        auto buyOrder = trader1 -> createLimitOrder(100.0, 10.0, true);
        exchange.submitOrder(buyOrder);

        // Currently, no trades have occurred
        REQUIRE(exchange.getTrades().empty());

        // Now modify the buy order to price 105 -> should match immediately
        bool modified = exchange.modifyOrder(buyOrder->getId(), 105.0, 10.0);
        REQUIRE(modified);

        // The re-match should have happened, thus producing 1 trade
        REQUIRE(exchange.getTrades().size() == 1);
        auto theTrade = exchange.getTrades().back();
        REQUIRE(theTrade.buyOrderId == buyOrder->getId());
        REQUIRE(theTrade.sellOrderId == sellOrder->getId());
        REQUIRE(theTrade.price == Approx(105.0));
        REQUIRE(theTrade.quantity == Approx(10.0));

        // Both orders fully filled -> order book empty
        REQUIRE(exchange.getOrderBook().isEmpty());
    }

    SECTION("Modify order with invalid new quantity/price") {
        auto buyOrder = trader1 -> createLimitOrder(101.0, 10.0, true);
        exchange.submitOrder(buyOrder);

        // Attempt to set newQuantity = 0, newPrice = 0
        bool modifiedBad = exchange.modifyOrder(buyOrder->getId(), 0.0, 0.0);
        REQUIRE_FALSE(modifiedBad);

        // The original order should remain in the book
        auto highestBid = exchange.getOrderBook().getHighestBid();
        REQUIRE(highestBid);
        REQUIRE(highestBid->getId() == buyOrder->getId());
    }

    SECTION("Modify non-existent order") {
        // Attempt to modify an order that doesn’t exist
        bool mod = exchange.modifyOrder("no_such_id", 110.0, 10.0);
        REQUIRE_FALSE(mod);
    }

    SECTION("Modify a market order - should fail") {
        auto marketBuy = trader1 -> createMarketOrder(10.0, true);
        exchange.submitOrder(marketBuy);

        // If it’s still leftover, we attempt to modify it, the exchange logic should reject this
        bool modMarket = exchange.modifyOrder(marketBuy->getId(), 120.0, 5.0);
        REQUIRE_FALSE(modMarket);
    }
}

TEST_CASE("Exchange - Trade Records", "[Exchange]") 
{
    Exchange exchange;
    auto trader1 = exchange.registerTrader();
    auto trader2 = exchange.registerTrader();

    SECTION("Trade records appended to exchange's trades vector") {
        // Trader1 places a sell order
        auto sellOrder = trader1 -> createLimitOrder(50.0, 10.0, false);
        exchange.submitOrder(sellOrder);
        
        // Trader2 places a buy order crossing Trader1 sell
        auto buyOrder = trader2 -> createLimitOrder(55.0, 10.0, true);
        auto theseTrades = exchange.submitOrder(buyOrder);

        // Should produce exactly 1 trade
        REQUIRE(theseTrades.size() == 1);

        // The Exchange's total trade record should also have size 1
        REQUIRE(exchange.getTrades().size() == 1);

        const auto& recordedTrade = exchange.getTrades().back();
        REQUIRE(recordedTrade.buyOrderId == buyOrder->getId());
        REQUIRE(recordedTrade.sellOrderId == sellOrder->getId());
        REQUIRE(recordedTrade.price == Approx(50.0));
        REQUIRE(recordedTrade.quantity == Approx(10.0));
    }

    SECTION("Multiple trades accumulate") {
        // Sell order 1
        auto sellOrder1 = trader2 -> createLimitOrder(50.0, 10.0, false);
        exchange.submitOrder(sellOrder1);

        // Sell order 2
        auto sellOrder2 = trader2 -> createLimitOrder(51.0, 5.0, false);
        exchange.submitOrder(sellOrder2);

        // Buy order crosses both
        auto buyOrder = trader1 -> createLimitOrder(55.0, 20.0, true);
        auto tradeVec = exchange.submitOrder(buyOrder);

        // We expect two trades: 10 shares at 50, then 5 shares at 51
        REQUIRE(tradeVec.size() == 2);
        REQUIRE(tradeVec[0].price == Approx(50.0));
        REQUIRE(tradeVec[0].quantity == Approx(10.0));
        REQUIRE(tradeVec[1].price == Approx(51.0));
        REQUIRE(tradeVec[1].quantity == Approx(5.0));

        // The exchange's master list of trades should also contain these two
        REQUIRE(exchange.getTrades().size() == 2);

        // The buy order still has leftover quantity (20 - 15 = 5), 
        // so it remains in the order book at price 55, the sell orders are fully filled
        REQUIRE_FALSE(exchange.getOrderBook().isEmpty());
        auto leftoverBuy = exchange.getOrderBook().getHighestBid();
        REQUIRE(leftoverBuy->getQuantity() == Approx(5.0));
        REQUIRE(leftoverBuy->getPrice() == Approx(55.0));
    }
}