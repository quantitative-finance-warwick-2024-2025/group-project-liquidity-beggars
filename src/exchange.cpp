#include "exchange.hpp"
#include <sstream>
#include <algorithm>  // std::min
#include <iostream>   // optional logging

namespace trading {

// ------------------------------------------
// Trade struct
// ------------------------------------------
std::string Trade::toString() const {
    std::stringstream ss;
    ss << "Trade executed: "
       << "BUYER(" << buyOrderId << ") "
       << "SELLER(" << sellOrderId << ") "
       << "QUANTITY(" << quantity << ") "
       << "PRICE(" << price << ")";
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

    // If it's a buy order...
    if (incomingOrder->isBuyOrder()) {
        while (incomingOrder->getQuantity() > 0) {
            // 1. Check if there's a lowest ask
            auto bestAsk = orderBook.getLowestAsk();
            if (!bestAsk || bestAsk->getQuantity() <= 0) {
                // No ask orders left, can't match further
                break;
            }

            // 2. If incoming is limit, ensure crossing with best ask
            if (incomingOrder->getType() == OrderType::LIMIT) {
                double incomingPrice = incomingOrder->getPrice();
                double bestAskPrice  = bestAsk->getPrice();
                if (incomingPrice < bestAskPrice) {
                    // No crossing if incoming buy price is less than best ask price
                    break;
                }
            }

            // For market orders, no price check needed

            // 3. Determine the trade quantity
            double matchedQty = std::min(incomingOrder->getQuantity(), bestAsk->getQuantity());

            // 4. Create a Trade
            Trade trade;
            trade.buyOrderId  = incomingOrder->getId();
            trade.sellOrderId = bestAsk->getId();
            // Execution price is typically the resting order's price
            trade.price       = bestAsk->getPrice();
            trade.quantity    = matchedQty;
            executedTrades.push_back(trade);

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
                    break;
                }
            }
        }
    }
    // If it's a sell order...
    else {
        while (incomingOrder->getQuantity() > 0) {
            // 1. Check if there's a highest bid
            auto bestBid = orderBook.getHighestBid();
            if (!bestBid || bestBid->getQuantity() <= 0) {
                // No bid orders left
                break;
            }

            // 2. If incoming is limit, ensure crossing with best bid
            if (incomingOrder->getType() == OrderType::LIMIT) {
                double incomingPrice = incomingOrder->getPrice();
                double bestBidPrice  = bestBid->getPrice();
                if (bestBidPrice < incomingPrice) {
                    // No crossing if best bid price is less than incoming sell price
                    break;
                }
            }

            // 3. Determine the trade quantity
            double matchedQty = std::min(incomingOrder->getQuantity(), bestBid->getQuantity());

            // 4. Create a Trade
            Trade trade;
            trade.buyOrderId  = bestBid->getId();
            trade.sellOrderId = incomingOrder->getId();
            trade.price       = bestBid->getPrice();
            trade.quantity    = matchedQty;
            executedTrades.push_back(trade);

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
                    break;
                }
            }
        }
    }

    return executedTrades;
}

// ------------------------------------------
// registerTrader
// ------------------------------------------
Trader& Exchange::registerTrader(const std::string& traderId) {
    return traders.try_emplace(traderId, traderId, this).first->second;
}

// ------------------------------------------
// submitOrder
// ------------------------------------------
std::vector<Trade> Exchange::submitOrder(std::shared_ptr<Order> order)
{
    try{
        if (!order){
            throw std::invalid_argument("Cannot add null order.");
        }
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
    catch(const std::invalid_argument& exception){
        std::cerr << "Exception caught: " << exception.what() << "\n";
    }
    
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
    try{
        // 1. Locate the existing order
        auto existingOrder = orderBook.findOrder(orderId);
        if (!existingOrder) {
            throw std::runtime_error("Order " + orderId + " not found.");
        }

        // 2. Only allow modifying limit orders
        if (existingOrder->getType() != OrderType::LIMIT) {
            throw std::runtime_error("Market orders cannot be modified.");
        }

        // 3. Basic validation
        if (newQuantity <= 0) {
            throw std::invalid_argument("Order quantity must be greater than zero.");
        }

        if (newPrice <= 0){
            throw std::invalid_argument("Limit price must be greater than zero.");
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
    catch (std::invalid_argument& exception){
        std::cerr << "Exception caught: " << exception.what() << "\n";
    }
    catch (std::runtime_error& exception){
        std::cerr << "Exception caught: " << exception.what() << "\n";
    }   
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
