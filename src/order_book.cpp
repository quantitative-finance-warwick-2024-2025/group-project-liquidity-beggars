#include "order_book.hpp"
#include <sstream>
#include <algorithm>

namespace trading {

// PriceLevel struct implementation

// Constructor
PriceLevel::PriceLevel(double p): price(p) {
    // TODO
}

// Add order
void PriceLevel::addOrder(std::shared_ptr<Order> order) {
    // TODO
}

// Remove order
bool PriceLevel::removeOrder(const std::string& orderId) {
    // TODO
    return true;
}

// Find order
std::shared_ptr<Order> PriceLevel::findOrder(const std::string& orderId) {
    // TODO
  return std::make_shared<Order>(orderId);
}

// OrderBook class implementation

// Add order
void OrderBook::addOrder(std::shared_ptr<Order> order) {
    // TODO
}

// Remove order
bool OrderBook::removeOrder(const std::string& orderId) {
    // TODO
    return true;
}

// Find order
std::shared_ptr<Order> OrderBook::findOrder(const std::string& orderId) {
    // TODO
    return std::make_shared<Order>(orderId);
}

// Get highest bid
std::shared_ptr<Order> OrderBook::getHighestBid() const {
    // TODO
    return std::make_shared<Order>();
}

// Get lowest ask
std::shared_ptr<Order> OrderBook::getLowestAsk() const {
    // TODO
    return std::make_shared<Order>();
}

// Check if book is empty
bool OrderBook::isEmpty() const {
    // TODO
    return true;
}

// For display
std::string OrderBook::toString() const {
    // TODO
    return "";
}

}