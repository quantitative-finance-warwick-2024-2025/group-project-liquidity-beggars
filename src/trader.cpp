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
std::shared_ptr<LimitOrder> Trader::createLimitOrder(double quantity, double price, bool isBuy){
    // TODO
    return std::make_shared<LimitOrder>(id, quantity, price, isBuy); 
}

// Create market order
std::shared_ptr<MarketOrder> Trader::createMarketOrder(double quantity, bool isBuy) {
    // TODO
    return std::make_shared<MarketOrder>(id, quantity, isBuy);
}

// Cancel limit order
bool Trader::cancelOrder(const std::string& orderId) {
    // TODO
    return true;
}

// Modify limit order
bool Trader::modifyOrder(const std::string& orderId, double newQuantity, double newPrice) {
    // TODO
    return true;
}

// Get id 
const std::string& Trader::getId() const {
    return id;
}

}