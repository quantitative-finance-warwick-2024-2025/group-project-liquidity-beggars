#pragma once

#include "order_book.hpp"
#include "trader.hpp"
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

namespace trading {

// Trade struct
struct Trade {
    std::string buyOrderId;
    std::string sellOrderId;
    std::string buyTraderId;   
    std::string sellTraderId;  
    double price;
    double quantity;

    std::string toString() const;
};

// Exchange class
class Exchange {
private:
    OrderBook orderBook;
    std::vector<Trade> trades;
    std::unordered_map<std::string, std::shared_ptr<Trader>> traders;

    // Match order
    std::vector<Trade> matchOrder(std::shared_ptr<Order> order);
    
public:
    
    // Register a new trader
    std::shared_ptr<Trader> registerTrader();

    // Display all registered traders
    void displayTraders() const;
    
    // Submit order
    std::vector<Trade> submitOrder(std::shared_ptr<Order> order);
    
    // Cancel order
    bool cancelOrder(const std::string& orderId);
    
    // Modify order
    bool modifyOrder(const std::string& orderId, double newPrice, double newQuantity);
    
    // Get order book
    const OrderBook& getOrderBook() const;
    
    // Get trades
    const std::vector<Trade>& getTrades() const;
};

// Helper function to get current timestamp 
std::string getCurrentTimestamp();

}