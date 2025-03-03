#include "trader.hpp"
#include "exchange.hpp"
#include <iostream>

namespace trading {

// Trader class implementation

// Constructor
Trader::Trader(std::string id, Exchange* exchange):
    id(std::move(id)), exchange(exchange) {
        // TODO
}

// Create limit order
std::shared_ptr<LimitOrder> Trader::createLimitOrder(double price, double quantity, bool isBuy){
    try{
        if (price <= 0){
            throw std::invalid_argument("Limit price must be greater than zero.");
        }
        if (quantity <= 0){
            throw std::invalid_argument("Order quantity must be greater than zero.");
        }
        auto order = std::make_shared<LimitOrder>(id, price, quantity, isBuy);
        return order;
    }
    catch (std::invalid_argument& exception){
        std::cerr << "Exception caught: " << exception.what() << "\n";
        return nullptr;
    }
}

// Create market order
std::shared_ptr<MarketOrder> Trader::createMarketOrder(double quantity, bool isBuy) {
    try{
        if (quantity <= 0){
            throw std::invalid_argument("Order quantity must be greater than zero.");
        }
        auto order = std::make_shared<MarketOrder>(id, quantity, isBuy);
        return order;
    }
    catch (std::invalid_argument& exception){
        std::cerr << "Exception caught: " << exception.what() << "\n";
        return nullptr;
    }
}

// Cancel limit order
bool Trader::cancelOrder(const std::string& orderId) {
    return exchange->cancelOrder(orderId);
}

// Modify limit order
bool Trader::modifyOrder(const std::string& orderId, double newPrice, double newQuantity) {
    try{
        if (newPrice <= 0){
            throw std::invalid_argument("Limit price must be greater than zero.");
        }
        if (newQuantity <= 0){
            throw std::invalid_argument("Order quantity must be greater than zero.");
        }
        return exchange->modifyOrder(orderId, newPrice, newQuantity);
    }
    catch (std::invalid_argument& exception){
        std::cerr << "Exception caught: " << exception.what() << "\n";
        return false;
    }
}

// Get id 
const std::string& Trader::getId() const {
    return id;
}

}