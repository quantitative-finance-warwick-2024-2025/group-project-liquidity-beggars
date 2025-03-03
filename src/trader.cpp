#include "trader.hpp"
#include "exchange.hpp"

namespace trading {

// Trader class implementation

// Constructor
Trader::Trader(std::string id, Exchange* exchange):
    id(std::move(id)), exchange(exchange) {
        // TODO
}

// Create limit order
std::shared_ptr<LimitOrder> Trader::createLimitOrder(double price, double quantity, bool isBuy){
    auto order = std::make_shared<LimitOrder>(id, price, quantity, isBuy);
    return order;
}

// Create market order
std::shared_ptr<MarketOrder> Trader::createMarketOrder(double quantity, bool isBuy) {
    auto order = std::make_shared<MarketOrder>(id, quantity, isBuy);
    return order;
}

// Cancel limit order
bool Trader::cancelOrder(const std::string& orderId) {
    return exchange->cancelOrder(orderId);
}

// Modify limit order
bool Trader::modifyOrder(const std::string& orderId, double newPrice, double newQuantity) {
    return exchange->modifyOrder(orderId, newPrice, newQuantity);
}

// Get id 
const std::string& Trader::getId() const {
    return id;
}

}