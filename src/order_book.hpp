#pragma once

#include "order.hpp"
#include <map>
#include <list>
#include <memory>
#include <unordered_map>

namespace trading {

// PriceLevel struct
struct PriceLevel {
    explicit PriceLevel(double p);

    double price;
    std::list<std::shared_ptr<Order>> orders;  // Doubly-linked list
    
    void addOrder(std::shared_ptr<Order> order);
    bool removeOrder(const std::string& orderId);
    std::shared_ptr<Order> findOrder(const std::string& orderId) const;
};

// OrderBook class
class OrderBook {
private:
    // Binary trees for price levels
    std::map<double, PriceLevel, std::greater<>> bids;  // Highest bid
    std::map<double, PriceLevel> asks;                  // Lowest ask
    std::unordered_map<std::string, std::pair<bool, double>> orderMap;  // orderId -> (isBuy, price)

    
public:
    // Add order
    void addOrder(std::shared_ptr<Order> order);
    
    // Remove order
    bool removeOrder(const std::string& orderId);
    
    // Find order
    std::shared_ptr<Order> findOrder(const std::string& orderId) const;
    
    // Get highest bid
    std::shared_ptr<Order> getHighestBid() const;

    // Get lowest ask
    std::shared_ptr<Order> getLowestAsk() const;
    
    // Check if book is empty
    bool isEmpty() const;
    
    // Display Order Book
    std::string toString() const;
};

}