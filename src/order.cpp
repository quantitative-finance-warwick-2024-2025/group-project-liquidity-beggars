#include "order.hpp"
#include <random>
#include <sstream>

namespace trading {

// Order parent class implementation

// Forward definition
std::string generateOrderId();

// Order constructor
Order::Order(std::string traderId, double quantity, bool isBuy): 
    traderId(std::move(traderId)), quantity(quantity), isBuy(isBuy) {
        if (quantity <= 0) {
            throw std::invalid_argument("Order quantity must be greater than zero.");
        }
        // Generate sequential order ID
        static unsigned int nextOrderId = 1;
        id = "ORD-" + std::to_string(nextOrderId++);
}

// Getters
const std::string& Order::getId() const {
    return id;
}
const std::string& Order::getTraderId() const {
    return traderId;
}
double Order::getQuantity() const {
    return quantity;
}
bool Order::isBuyOrder() const {
    return isBuy;
}

// LimitOrder implementation

// LimitOrder constructor
LimitOrder::LimitOrder(std::string traderId, double price, double quantity, bool isBuy): 
    Order(std::move(traderId), quantity, isBuy), price(price) {
        if (quantity <= 0) {
            throw std::invalid_argument("Order quantity must be greater than zero.");
        }
        
        if (price <= 0) {
            throw std::invalid_argument("Limit price must be greater than zero.");
        }
}

// Get order type
OrderType LimitOrder::getType() const{
    return OrderType::LIMIT;
}

// Get order price
double LimitOrder::getPrice() const {
    return price;
}

// Display Limit Order
std::string LimitOrder::toString() const {
    std::stringstream ss;
    ss << "Order " << id << " (" << (isBuy ? "BUY" : "SELL") << "): "
       << "Trader " << traderId << " | "
       << quantity << " units @ $" << price;
    return ss.str();
}

// Modify price
void LimitOrder::setPrice(double newPrice) {
    price = newPrice;
}

// Modify quantity
void Order::setQuantity(double newQuantity) {
    quantity = newQuantity;
}

// MarketOrder implementation

// MarketOrder constructor
MarketOrder::MarketOrder(std::string traderId, double quantity, bool isBuy): 
    Order(std::move(traderId), quantity, isBuy) {
        if (quantity <= 0) {
            throw std::invalid_argument("Order quantity must be greater than zero.");
        }
}

// Get order type
OrderType MarketOrder::getType() const {
    return OrderType::MARKET;
}

// Get order price
double MarketOrder::getPrice() const {
    return 0.0;
}

// Display Market Order
std::string MarketOrder::toString() const{
    std::stringstream ss;
    ss << "Order " << id << " (" << (isBuy ? "BUY" : "SELL") << "): "
       << "Trader " << traderId << " | "
       << quantity << " units @ MARKET";
    return ss.str();
}
}