#include "order.hpp"
#include <random>
#include <sstream>

namespace trading {

// Order parent class implementation

// Forward definition
std::string generateOrderId();

// Order constructor
Order::Order(std::string traderId, double quantity, bool isBuy): 
    id(generateOrderId()), traderId(std::move(traderId)), quantity(quantity), isBuy(isBuy) {
        //TODO
}
// Generate order ID
std::string generateOrderId() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(1000, 9999);
    return std::to_string(dis(gen));
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

// LimiOrder constructor
LimitOrder::LimitOrder(std::string traderId, double price, double quantity, bool isBuy): 
    Order(std::move(traderId), quantity, isBuy), price(price) {
        //TODO
}

// Get order type
OrderType LimitOrder::getType() const{
    return OrderType::LIMIT;
}

// Get order price
double LimitOrder::getPrice() const {
    return price;
}

// For display
std::string LimitOrder::toString() const{
    std::stringstream ss;
    ss << (isBuy ? "BUY" : "SELL") << " " << quantity << " @ MARKET";
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
        // TODO
}

// Get order type
OrderType MarketOrder::getType() const {
    return OrderType::MARKET;
}

// Get order price
double MarketOrder::getPrice() const {
    return 0.0;
}

// For display
std::string MarketOrder::toString() const{
    std::stringstream ss;
    ss << (isBuy ? "BUY" : "SELL") << " " << quantity << " @ MARKET";
    return ss.str();
}
}