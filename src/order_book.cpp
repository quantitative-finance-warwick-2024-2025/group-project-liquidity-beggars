#include "order_book.hpp"
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <iostream>

namespace trading {

// PriceLevel struct implementation

// Constructor
PriceLevel::PriceLevel(double p): price(p) {
}

// Add order
void PriceLevel::addOrder(std::shared_ptr<Order> order) {
    try{
        if (!order){
            throw std::invalid_argument("Cannot add null order.");
        }
        orders.push_back(order);
    }
    catch(const std::invalid_argument& exception){
        std::cerr << "Exception caught: " << exception.what() << "\n";
    }
}

// Remove order
bool PriceLevel::removeOrder(const std::string& orderId) {
    try{
        for (auto it = orders.begin(); it != orders.end(); ++it) {
            if ((*it)->getId() == orderId) {
                orders.erase(it);
                return true;
            }
        }
        throw std::runtime_error("Order " + orderId + " not found.");
    }
    catch (const std::runtime_error& exception){
        std::cerr << "Exception caught: " << exception.what() << "\n";
        return false;
    }
}

// Find order
std::shared_ptr<Order> PriceLevel::findOrder(const std::string& orderId) const {
    try{
        for (const auto& order : orders) {
            if (order->getId() == orderId) {
                return order;
            }
        }
        throw std::runtime_error("Order " + orderId + " not found.");
    }
    catch (const std::runtime_error& exception){
        std::cerr << "Exception caught: " << exception.what() << "\n";
        return nullptr;
    }
}

// OrderBook class implementation

// Add order to respective ask/bid tree and to hash map
void OrderBook::addOrder(std::shared_ptr<Order> order) {
    try{
        if (!order){
            throw std::invalid_argument("Cannot add null order.");
        }
        if (order->getType() != OrderType::LIMIT) {
            throw std::invalid_argument("Order " + order->getId() + "not a Limit order.");
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
    catch (std::invalid_argument& exception){
        std::cerr << "Exception caught: " << exception.what() << "\n";
    }
}

// Remove order by ID
bool OrderBook::removeOrder(const std::string& orderId) {
    try{
        auto it = orderMap.find(orderId);
        if (it == orderMap.end()) {
            throw std::runtime_error("Order " + orderId + " not found.");
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
        }else {
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
    catch (std::runtime_error& exception){
        std::cerr << "Exception caught: " << exception.what() << "\n";
        return false;
    }
}

// Find order
std::shared_ptr<Order> OrderBook::findOrder(const std::string& orderId) const {
    try{
        auto it = orderMap.find(orderId);
        if (it == orderMap.end()) {
            throw std::runtime_error("Order " + orderId + " not found.");
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
        return nullptr;
    }
    catch (std::runtime_error& exception){
        std::cerr << "Exception caught: " << exception.what() << "\n";
        return nullptr;
    }
}

// Get highest bid function
std::shared_ptr<Order> OrderBook::getHighestBid() const {
    try{
        // get first price level (highest - greater operator)
        const auto& level = bids.begin() -> second;
        if (bids.empty() || level.orders.empty()) {
            throw std::runtime_error("No bids.");
        }
        // get first order (time priority)
        return level.orders.front();
    }
    catch (std::runtime_error& exception){
        std::cerr << "Exception caught: " << exception.what() << "\n";
        return nullptr;
    }
}

// Get lowest ask
std::shared_ptr<Order> OrderBook::getLowestAsk() const {
    try{
        // get first price level (lowest)
        const auto& level = asks.begin() -> second;

        if (asks.empty() || level.orders.empty()) {
            throw std::runtime_error("No asks.");
        }
        
        // get first order (time priority - default operator)
        return level.orders.front();
    }
    catch (std::runtime_error& exception){
        std::cerr << "Exception caught: " << exception.what() << "\n";
        return nullptr;
    }
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