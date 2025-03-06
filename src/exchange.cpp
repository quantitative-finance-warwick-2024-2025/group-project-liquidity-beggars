#include "exchange.hpp"
#include <sstream>
#include <algorithm>
#include <iostream>  
#include <iomanip>   
#include <chrono>     
#include <ctime>     

namespace trading {

// Helper function to get current timestamp
std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

// ------------------------------------------
// Trade struct
// ------------------------------------------
// Trade struct implementation in exchange.cpp
std::string Trade::toString() const {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << "[" << getCurrentTimestamp() << "] TRADE EXECUTED: "
       << quantity << " units at $" << price
       << " | Buyer: " << buyTraderId 
       << " (Order " << buyOrderId << ")"
       << " | Seller: " << sellTraderId
       << " (Order " << sellOrderId << ")";
    return ss.str();
}
// ------------------------------------------
// Exchange class implementation
// ------------------------------------------

// ------------------------------------------
// matchOrder (private helper)
// ------------------------------------------
std::vector<Trade> Exchange::matchOrder(std::shared_ptr<Order> incomingOrder)
{
    std::vector<Trade> executedTrades;
    
    // Record initial quantity for reporting purposes
    double initialQuantity = incomingOrder->getQuantity();
    std::string incomingTraderId = incomingOrder->getTraderId();
    
    // If it's a buy order...
    if (incomingOrder->isBuyOrder()) {
        while (incomingOrder->getQuantity() > 0) {
            // 1. Check if there's a lowest ask
            auto bestAsk = orderBook.getLowestAsk();
            if (!bestAsk || bestAsk->getQuantity() <= 0) {
                // No ask orders left, can't match further
                std::cout << "[" << getCurrentTimestamp() << "] ORDER STATUS: Buy order " << incomingOrder->getId() 
                          << " (Trader " << incomingTraderId << ")"
                          << " - No matching sell orders available in the book" << std::endl;
                break;
            }

            // 2. If incoming is limit, ensure crossing with best ask
            if (incomingOrder->getType() == OrderType::LIMIT) {
                double incomingPrice = incomingOrder->getPrice();
                double bestAskPrice  = bestAsk->getPrice();
                if (incomingPrice < bestAskPrice) {
                    // No crossing if incoming buy price is less than best ask price
                    std::cout << "[" << getCurrentTimestamp() << "] ORDER STATUS: Buy limit order " << incomingOrder->getId()
                              << " (Trader " << incomingTraderId << ")"
                              << " - Price $" << std::fixed << std::setprecision(2) << incomingPrice
                              << " below best ask of $" << bestAskPrice 
                              << " - Order added to book" << std::endl;
                    break;
                }
            }

            // For market orders, no price check needed

            // 3. Determine the trade quantity
            double matchedQty = std::min(incomingOrder->getQuantity(), bestAsk->getQuantity());

            // 4. Create a Trade
            Trade trade;
            trade.buyOrderId    = incomingOrder->getId();
            trade.sellOrderId   = bestAsk->getId();
            trade.buyTraderId   = incomingOrder->getTraderId();
            trade.sellTraderId  = bestAsk->getTraderId();
            // Execution price is typically the resting order's price
            trade.price         = bestAsk->getPrice();
            trade.quantity      = matchedQty;
            executedTrades.push_back(trade);
            
            // Print trade execution details
            std::cout << trade.toString() << std::endl;

            // 5. Update quantities
            double newIncomingQty = incomingOrder->getQuantity() - matchedQty;
            double newAskQty = bestAsk->getQuantity() - matchedQty;
            
            incomingOrder->setQuantity(newIncomingQty);
            bestAsk->setQuantity(newAskQty);

            // 6. If the ask was fully filled, remove it
            if (newAskQty <= 0) {
                bool removed = orderBook.removeOrder(bestAsk->getId());
                if (!removed) {
                    // Failed to remove order - break to avoid infinite loop
                    std::cout << "[" << getCurrentTimestamp() << "] ERROR: Failed to remove fully matched sell order " 
                              << bestAsk->getId() << " (Trader " << bestAsk->getTraderId() << ")" << std::endl;
                    break;
                }
            }
        }
        
        // Report final state of the order
        if (incomingOrder->getQuantity() <= 0) {
            std::cout << "[" << getCurrentTimestamp() << "] ORDER COMPLETE: Buy order " << incomingOrder->getId()
                      << " (Trader " << incomingTraderId << ")"
                      << " fully executed for " << initialQuantity << " units" << std::endl;
        } else if (incomingOrder->getQuantity() < initialQuantity) {
            std::cout << "[" << getCurrentTimestamp() << "] ORDER PARTIAL: Buy order " << incomingOrder->getId()
                      << " (Trader " << incomingTraderId << ")"
                      << " partially executed. " 
                      << (initialQuantity - incomingOrder->getQuantity()) << " units filled, "
                      << incomingOrder->getQuantity() << " units remaining" << std::endl;
        }
    }
    // If it's a sell order...
    else {
        while (incomingOrder->getQuantity() > 0) {
            // 1. Check if there's a highest bid
            auto bestBid = orderBook.getHighestBid();
            if (!bestBid || bestBid->getQuantity() <= 0) {
                // No bid orders left
                std::cout << "[" << getCurrentTimestamp() << "] ORDER STATUS: Sell order " << incomingOrder->getId()
                          << " (Trader " << incomingTraderId << ")"
                          << " - No matching buy orders available in the book" << std::endl;
                break;
            }

            // 2. If incoming is limit, ensure crossing with best bid
            if (incomingOrder->getType() == OrderType::LIMIT) {
                double incomingPrice = incomingOrder->getPrice();
                double bestBidPrice  = bestBid->getPrice();
                if (bestBidPrice < incomingPrice) {
                    // No crossing if best bid price is less than incoming sell price
                    std::cout << "[" << getCurrentTimestamp() << "] ORDER STATUS: Sell limit order " << incomingOrder->getId()
                              << " (Trader " << incomingTraderId << ")"
                              << " - Price $" << std::fixed << std::setprecision(2) << incomingPrice
                              << " above best bid of $" << bestBidPrice 
                              << " - Order added to book" << std::endl;
                    break;
                }
            }

            // 3. Determine the trade quantity
            double matchedQty = std::min(incomingOrder->getQuantity(), bestBid->getQuantity());

            // 4. Create a Trade
            Trade trade;
            trade.buyOrderId    = bestBid->getId();
            trade.sellOrderId   = incomingOrder->getId();
            trade.buyTraderId   = bestBid->getTraderId();
            trade.sellTraderId  = incomingOrder->getTraderId();
            trade.price         = bestBid->getPrice();
            trade.quantity      = matchedQty;
            executedTrades.push_back(trade);
            
            // Print trade execution details
            std::cout << trade.toString() << std::endl;

            // 5. Update quantities
            double newIncomingQty = incomingOrder->getQuantity() - matchedQty;
            double newBidQty = bestBid->getQuantity() - matchedQty;
            
            incomingOrder->setQuantity(newIncomingQty);
            bestBid->setQuantity(newBidQty);

            // 6. If the bid was fully filled, remove it
            if (newBidQty <= 0) {
                bool removed = orderBook.removeOrder(bestBid->getId());
                if (!removed) {
                    // Failed to remove order - break to avoid infinite loop
                    std::cout << "[" << getCurrentTimestamp() << "] ERROR: Failed to remove fully matched buy order " 
                              << bestBid->getId() << " (Trader " << bestBid->getTraderId() << ")" << std::endl;
                    break;
                }
            }
        }
        
        // Report final state of the order
        if (incomingOrder->getQuantity() <= 0) {
            std::cout << "[" << getCurrentTimestamp() << "] ORDER COMPLETE: Sell order " << incomingOrder->getId()
                      << " (Trader " << incomingTraderId << ")"
                      << " fully executed for " << initialQuantity << " units" << std::endl;
        } else if (incomingOrder->getQuantity() < initialQuantity) {
            std::cout << "[" << getCurrentTimestamp() << "] ORDER PARTIAL: Sell order " << incomingOrder->getId()
                      << " (Trader " << incomingTraderId << ")"
                      << " partially executed. " 
                      << (initialQuantity - incomingOrder->getQuantity()) << " units filled, "
                      << incomingOrder->getQuantity() << " units remaining" << std::endl;
        }
    }

    return executedTrades;
}

// ------------------------------------------
// registerTrader
// ------------------------------------------
std::shared_ptr<Trader> Exchange::registerTrader() {

    auto trader = std::make_shared<Trader>(this);
    
    traders[trader->getId()] = trader;
    
    // Log registration
    std::cout << "[" << getCurrentTimestamp() << "] TRADER REGISTERED: " 
              << trader->getId() << std::endl;
              
    return trader;
}

void Exchange::displayTraders() const {
    std::cout << "[" << getCurrentTimestamp() << "] REGISTERED TRADERS:" << std::endl;
    std::cout << "=============================" << std::endl;
    
    if (traders.empty()) {
        std::cout << "No traders registered." << std::endl;
        return;
    }
    
    for (const auto& [traderId, trader] : traders) {
        std::cout << "Trader ID: " << traderId << std::endl;
    }
    
    std::cout << "Total traders: " << traders.size() << std::endl;
}

// ------------------------------------------
// submitOrder
// ------------------------------------------
std::vector<Trade> Exchange::submitOrder(std::shared_ptr<Order> order)
{
    // 1. Match the incoming order
    std::vector<Trade> newTrades = matchOrder(order);

    // 2. If it’s a limit order and there is leftover quantity, add it to the book
    if (order->getType() == OrderType::LIMIT && order->getQuantity() > 0) {
        orderBook.addOrder(order);
    }

    // 3. Record the newly executed trades
    trades.insert(trades.end(), newTrades.begin(), newTrades.end());

    // 4. Return them to caller
    return newTrades;
}

// ------------------------------------------
// cancelOrder
// ------------------------------------------
bool Exchange::cancelOrder(const std::string& orderId)
{
    return orderBook.removeOrder(orderId);
}

// ------------------------------------------
// modifyOrder
// ------------------------------------------
// Removes the target limit order, checks new values, updates, and *resubmits* it
// so that if it crosses the market, it can match immediately.
bool Exchange::modifyOrder(const std::string& orderId, double newPrice, double newQuantity)
{
    // 1. Locate the existing order
    auto existingOrder = orderBook.findOrder(orderId);
    if (!existingOrder) {
        return false; // Not found
    }

    // 2. Only allow modifying limit orders
    if (existingOrder->getType() != OrderType::LIMIT) {
        return false;
    }

    // 3. Basic validation
    if (newQuantity <= 0 || newPrice <= 0) {
        return false;
    }

    // 4. Remove from the book
    if (!orderBook.removeOrder(orderId)) {
        return false;
    }

    // 5. Update the fields
    existingOrder->setQuantity(newQuantity);
    auto limitPtr = std::dynamic_pointer_cast<LimitOrder>(existingOrder);
    if (limitPtr) {
        limitPtr->setPrice(newPrice);
    } else {
        // Should not happen if we confirmed it is limit, but just in case:
        return false;
    }

    // 6. Re-submit (which triggers immediate match if crossing)
    submitOrder(existingOrder);

    return true;
}

// ------------------------------------------
// getOrderBook
// ------------------------------------------
const OrderBook& Exchange::getOrderBook() const
{
    return orderBook;
}

// ------------------------------------------
// getTrades
// ------------------------------------------
const std::vector<Trade>& Exchange::getTrades() const
{
    return trades;
}

} // namespace trading
