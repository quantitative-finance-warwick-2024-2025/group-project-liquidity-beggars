#include "exchange.hpp"
#include <sstream>

namespace trading {

// Trade struct implementation

// For display
std::string Trade::toString() const {
    // TODO
    return "";
}

// Exchange class implementation

// Match order
std::vector<Trade> Exchange::matchOrder(std::shared_ptr<Order> order) {
    // TODO
    return {}; 
}

// Register trader
Trader& Exchange::registerTrader(const std::string& traderId) {
    traders[traderId] = Trader(traderId, this);
    return traders[traderId];
}

// Submit order
std::vector<Trade> Exchange::submitOrder(std::shared_ptr<Order> order) {
    // TODO
    return {};
}

// Cancel order
bool Exchange::cancelOrder(const std::string& orderId) {
    // TODO
    return true;
}

// Modify order
bool Exchange::modifyOrder(const std::string& orderId, double newQuantity, double newPrice) {
    // TODO
    return true;
}

// Get order book
const OrderBook& Exchange::getOrderBook() const {
    // TODO
    return orderBook;
}

// Get trades
const std::vector<Trade>& Exchange::getTrades() const {
    // TODO
    return trades;
}

}