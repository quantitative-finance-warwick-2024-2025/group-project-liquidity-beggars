#pragma once

#include <string>
#include <memory>

namespace trading {

// Enumator class for Order types
enum class OrderType {
    LIMIT,
    MARKET
};

// Parent Order class
class Order {
protected:
    std::string id;
    std::string traderId;
    double quantity;
    bool isBuy;
    
    // Constructor
    Order(std::string traderId, double quantity, bool isBuy);

public:
    virtual ~Order() = default;
    
    // Virtual methods

    // Get order type
    virtual OrderType getType() const = 0;

    // Get order price
    virtual double getPrice() const = 0;

    // For display
    virtual std::string toString() const = 0;
    
    // Getters
    const std::string& getId() const;
    const std::string& getTraderId() const;
    double getQuantity() const;
    bool isBuyOrder() const;
    
    // Modify quantity 
    void setQuantity(double newQuantity);
};

// Child LimitOrder class
class LimitOrder : public Order {
private:
    double price;

public:
    // Flag for limit order
    bool isValid;

    LimitOrder(std::string traderId, double price, double quantity, bool isBuy);
    
    // Abstract methods

    // Get order type
    OrderType getType() const override;

    // Get order price
    double getPrice() const override;

    // For display
    std::string toString() const override;
    
    // Modify price
    void setPrice(double newPrice);
};

// Child MarketOrder class
class MarketOrder : public Order {
public:
    MarketOrder(std::string traderId, double quantity, bool isBuy);
    
    // Abstract methods

    // Get order type
    OrderType getType() const override;

    // Get order price
    double getPrice() const override; 

    // For display
    std::string toString() const override;
};

}