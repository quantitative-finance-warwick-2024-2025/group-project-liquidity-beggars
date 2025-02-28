#include "order_book.hpp"
#include <sstream>
#include <algorithm>
#include <iomanip>

namespace trading {

// PriceLevel struct implementation

// Constructor
PriceLevel::PriceLevel(double p): price(p) {
}

// Add order
void PriceLevel::addOrder(std::shared_ptr<Order> order) {
    orders.push_back(order);
}

// Remove order
bool PriceLevel::removeOrder(const std::string& orderId) {
    for (auto it = orders.begin(); it != orders.end(); ++it) {
        if ((*it)->getId() == orderId) {
            orders.erase(it);
            return true;
        }
    }
    return false;
}

// Find order
std::shared_ptr<Order> PriceLevel::findOrder(const std::string& orderId) const {
    for (const auto& order : orders) {
        if (order->getId() == orderId) {
            return order;
        }
    }
    return nullptr; // TODO EXCEPTION
}

// OrderBook class implementation

// Add order to respective ask/bid tree and to hash map
void OrderBook::addOrder(std::shared_ptr<Order> order) {
    
    // only Limit orders can be stored in the book
    if (order->getType() != OrderType::LIMIT) {
        return;
    }

    // get price
    double price = order -> getPrice();
    bool isBuy = order -> isBuyOrder();

    // 
    if (isBuy) {
        auto it = bids.find(price);
        if (it == bids.end()) {
            bids.emplace(price, PriceLevel(price)); 
        }
        bids.at(price).addOrder(order); 
    }
    else {
        auto it = asks.find(price);
        if (it == asks.end()) {
            asks.emplace(price, PriceLevel(price));
        }
        asks.at(price).addOrder(order);
    }

    // store order in hash map for look up by ID
    orderMap[order->getId()] = std::make_pair(isBuy, price);
}

// Remove order by ID
bool OrderBook::removeOrder(const std::string& orderId) {
    
    auto it = orderMap.find(orderId);
    if (it == orderMap.end()) {
        return false;
    }

    // get order price level and tree side from hash map
    double price = it->second.second;
    bool isBuy = it->second.first;

    if (isBuy) {
        auto& tree = bids;
        auto level = tree.find(price);
        
        if (level != tree.end()) {
            bool removed = level->second.removeOrder(orderId);
            
            if (removed) {
                if (level->second.orders.empty()) {
                    tree.erase(level);
                }
                
                orderMap.erase(it);
                return true;
            }
        }
    } else {
        auto& tree = asks;
        auto level = tree.find(price);
    
        if (level != tree.end()) {
            bool removed = level->second.removeOrder(orderId);
            
            if (removed) {
                if (level->second.orders.empty()) {
                    tree.erase(level);
                }
            
                orderMap.erase(it);
                return true;
            }
        }
    }
    return false;
}

// Find order
std::shared_ptr<Order> OrderBook::findOrder(const std::string& orderId) const {

    auto it = orderMap.find(orderId); 
    if (it == orderMap.end()) {
        return nullptr; // TODO EXCEPTION
    }
    
    // get order price level and tree side from hash map
    double price = it->second.second;
    bool isBuy = it->second.first;
    
    if (isBuy) {
        auto& tree = bids;
        auto level = tree.find(price);
        
        if (level != tree.end()) {
            return level->second.findOrder(orderId);
        }
    } else {
        auto& tree = asks;
        auto level = tree.find(price);
        
        if (level != tree.end()) {
            return level->second.findOrder(orderId);
        }
    }
    
    return nullptr; // TODO EXCEPTION
}

// Get highest bid
std::shared_ptr<Order> OrderBook::getHighestBid() const {
    
    if (bids.empty()) {
        return nullptr; // TODO EXCEPTION
    }

    // get first price level (highest - greater operator)
    const auto& level = bids.begin() -> second;

    // get first order (time priority)
    return level.orders.empty() ? nullptr : level.orders.front(); // TODO EXCEPTION
}

// Get lowest ask
std::shared_ptr<Order> OrderBook::getLowestAsk() const {
    
    if (asks.empty()) {
        return nullptr; // TODO EXCEPTION
    }

    // get first price level (lowest)
    const auto& level = asks.begin() -> second;

    // get first order (time priority - default operator)
    return level.orders.empty() ? nullptr : level.orders.front(); // TODO EXCEPTION
}

// Check if book is empty
bool OrderBook::isEmpty() const {

    return bids.empty() && asks.empty();
}

// For display
std::string OrderBook::toString() const {
    std::stringstream ss;
    ss << "ORDER BOOK\n";
    ss << "==========\n";
    
    // Print asks (reverse order)
    ss << "ASKS:\n";
    for (auto it = asks.rbegin(); it != asks.rend(); ++it) {
        ss << std::fixed << std::setprecision(2) << it->first << ": ";
        for (const auto& order : it->second.orders) {
            ss << order->getQuantity() << " ";
        }
        ss << "\n";
    }

    ss << "----------\n";
    
    // Print bids
    ss << "BIDS:\n";
    for (const auto& [price, level] : bids) {
        ss << std::fixed << std::setprecision(2) << price << ": ";
        for (const auto& order : level.orders) {
            ss << order->getQuantity() << " ";
        }
        ss << "\n";
    }
    
    return ss.str();
}

}