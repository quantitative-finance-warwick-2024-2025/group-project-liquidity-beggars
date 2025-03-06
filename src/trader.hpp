#pragma once

#include "order.hpp"
#include <string>
#include <memory>

namespace trading {

// Forward declaration 
class Exchange;

// Trader class
class Trader {
private:
    std::string id; 
    Exchange* exchange; 

public:
    // Constructor
    Trader(Exchange* exchange);
    
    // Create limit order
    std::shared_ptr<LimitOrder> createLimitOrder(double price, double quantity, bool isBuy);

    // Create market order
    std::shared_ptr<MarketOrder> createMarketOrder(double quantity, bool isBuy);
    
    // Cancel limit order
    bool cancelOrder(const std::string& orderId);

    // Modify limit order
    bool modifyOrder(const std::string& orderId, double newQuantity, double newPrice);
    
    // Get id 
    const std::string& getId() const;
};

} 