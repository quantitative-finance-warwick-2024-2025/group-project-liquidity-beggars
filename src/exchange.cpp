#include "exchange.hpp"
#include <sstream>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>

namespace trading {

// Gets current local time with millisecond detail
std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  now.time_since_epoch()) % 1000;
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S")
       << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

// Formats trade details as a log string
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

// Matches an incoming order against the order book
std::vector<Trade> Exchange::matchOrder(std::shared_ptr<Order> incomingOrder)
{
    std::vector<Trade> executedTrades;
    double initialQuantity = incomingOrder->getQuantity();
    std::string incomingTraderId = incomingOrder->getTraderId();

    // Buy side
    if (incomingOrder->isBuyOrder()) {
        while (incomingOrder->getQuantity() > 0) {
            auto bestAsk = orderBook.getLowestAsk();
            if (!bestAsk || bestAsk->getQuantity() <= 0) {
                std::cout << "[" << getCurrentTimestamp() << "] ORDER STATUS: Buy order "
                          << incomingOrder->getId() << " (Trader " << incomingTraderId
                          << ") - No matching sell orders available" << std::endl;
                break;
            }

            // Check crossing for limit orders
            if (incomingOrder->getType() == OrderType::LIMIT) {
                double incomingPrice = incomingOrder->getPrice();
                double bestAskPrice = bestAsk->getPrice();
                if (incomingPrice < bestAskPrice) {
                    std::cout << "[" << getCurrentTimestamp() << "] ORDER STATUS: Buy limit order "
                              << incomingOrder->getId() << " (Trader " << incomingTraderId
                              << ") - Price $" << incomingPrice
                              << " below best ask $" << bestAskPrice
                              << " - Order added to book" << std::endl;
                    break;
                }
            }

            // Determine matched quantity
            double matchedQty = std::min(incomingOrder->getQuantity(), bestAsk->getQuantity());

            // Create and log trade
            Trade trade;
            trade.buyOrderId = incomingOrder->getId();
            trade.sellOrderId = bestAsk->getId();
            trade.buyTraderId = incomingOrder->getTraderId();
            trade.sellTraderId = bestAsk->getTraderId();
            trade.price = bestAsk->getPrice();
            trade.quantity = matchedQty;
            executedTrades.push_back(trade);
            std::cout << trade.toString() << std::endl;

            // Update quantities
            double newIncomingQty = incomingOrder->getQuantity() - matchedQty;
            double newAskQty = bestAsk->getQuantity() - matchedQty;
            incomingOrder->setQuantity(newIncomingQty);
            bestAsk->setQuantity(newAskQty);

            // Remove fully filled ask
            if (newAskQty <= 0) {
                bool removed = orderBook.removeOrder(bestAsk->getId());
                if (!removed) {
                    std::cout << "[" << getCurrentTimestamp() << "] ERROR: Failed to remove sell order "
                              << bestAsk->getId() << std::endl;
                    break;
                }
            }
        }

        // Log final outcome
        if (incomingOrder->getQuantity() <= 0) {
            std::cout << "[" << getCurrentTimestamp() << "] ORDER COMPLETE: Buy order "
                      << incomingOrder->getId() << " (Trader " << incomingTraderId
                      << ") fully executed for " << initialQuantity << " units" << std::endl;
        } else if (incomingOrder->getQuantity() < initialQuantity) {
            std::cout << "[" << getCurrentTimestamp() << "] ORDER PARTIAL: Buy order "
                      << incomingOrder->getId() << " (Trader " << incomingTraderId
                      << ") " << (initialQuantity - incomingOrder->getQuantity())
                      << " filled, " << incomingOrder->getQuantity() << " remaining" << std::endl;
        }
    }
    // Sell side
    else {
        while (incomingOrder->getQuantity() > 0) {
            auto bestBid = orderBook.getHighestBid();
            if (!bestBid || bestBid->getQuantity() <= 0) {
                std::cout << "[" << getCurrentTimestamp() << "] ORDER STATUS: Sell order "
                          << incomingOrder->getId() << " (Trader " << incomingTraderId
                          << ") - No matching buy orders available" << std::endl;
                break;
            }

            // Check crossing for limit orders
            if (incomingOrder->getType() == OrderType::LIMIT) {
                double incomingPrice = incomingOrder->getPrice();
                double bestBidPrice = bestBid->getPrice();
                if (bestBidPrice < incomingPrice) {
                    std::cout << "[" << getCurrentTimestamp() << "] ORDER STATUS: Sell limit order "
                              << incomingOrder->getId() << " (Trader " << incomingTraderId
                              << ") - Price $" << incomingPrice
                              << " above best bid $" << bestBidPrice
                              << " - Order added to book" << std::endl;
                    break;
                }
            }

            // Determine matched quantity
            double matchedQty = std::min(incomingOrder->getQuantity(), bestBid->getQuantity());

            // Create and log trade
            Trade trade;
            trade.buyOrderId = bestBid->getId();
            trade.sellOrderId = incomingOrder->getId();
            trade.buyTraderId = bestBid->getTraderId();
            trade.sellTraderId = incomingOrder->getTraderId();
            trade.price = bestBid->getPrice();
            trade.quantity = matchedQty;
            executedTrades.push_back(trade);
            std::cout << trade.toString() << std::endl;

            // Update quantities
            double newIncomingQty = incomingOrder->getQuantity() - matchedQty;
            double newBidQty = bestBid->getQuantity() - matchedQty;
            incomingOrder->setQuantity(newIncomingQty);
            bestBid->setQuantity(newBidQty);

            // Remove fully filled bid
            if (newBidQty <= 0) {
                bool removed = orderBook.removeOrder(bestBid->getId());
                if (!removed) {
                    std::cout << "[" << getCurrentTimestamp() << "] ERROR: Failed to remove buy order "
                              << bestBid->getId() << std::endl;
                    break;
                }
            }
        }

        // Log final outcome
        if (incomingOrder->getQuantity() <= 0) {
            std::cout << "[" << getCurrentTimestamp() << "] ORDER COMPLETE: Sell order "
                      << incomingOrder->getId() << " (Trader " << incomingTraderId
                      << ") fully executed for " << initialQuantity << " units" << std::endl;
        } else if (incomingOrder->getQuantity() < initialQuantity) {
            std::cout << "[" << getCurrentTimestamp() << "] ORDER PARTIAL: Sell order "
                      << incomingOrder->getId() << " (Trader " << incomingTraderId
                      << ") " << (initialQuantity - incomingOrder->getQuantity())
                      << " filled, " << incomingOrder->getQuantity() << " remaining" << std::endl;
        }
    }

    return executedTrades;
}

// Registers a new Trader and logs the event
std::shared_ptr<Trader> Exchange::registerTrader() {
    auto trader = std::make_shared<Trader>(this);
    traders[trader->getId()] = trader;
    std::cout << "[" << getCurrentTimestamp() << "] TRADER REGISTERED: "
              << trader->getId() << std::endl;
    return trader;
}

// Prints all registered traders
void Exchange::displayTraders() const {
    std::cout << "[" << getCurrentTimestamp() << "] REGISTERED TRADERS:\n"
              << "=============================\n";
    if (traders.empty()) {
        std::cout << "No traders registered.\n";
        return;
    }
    for (const auto& [traderId, trader] : traders) {
        std::cout << "Trader ID: " << traderId << std::endl;
    }
    std::cout << "Total traders: " << traders.size() << std::endl;
}

// Submits an order: tries to match, then adds leftover limit order to the book
std::vector<Trade> Exchange::submitOrder(std::shared_ptr<Order> order) {
    std::vector<Trade> newTrades = matchOrder(order);
    if (order->getType() == OrderType::LIMIT && order->getQuantity() > 0) {
        orderBook.addOrder(order);
    }
    trades.insert(trades.end(), newTrades.begin(), newTrades.end());
    return newTrades;
}

// Cancels an existing order by ID
bool Exchange::cancelOrder(const std::string& orderId) {
    return orderBook.removeOrder(orderId);
}

// Modifies limit order price/quantity, then resubmits it
bool Exchange::modifyOrder(const std::string& orderId, double newPrice, double newQuantity) {
    auto existingOrder = orderBook.findOrder(orderId);
    if (!existingOrder) return false;
    if (existingOrder->getType() != OrderType::LIMIT) return false;
    if (newQuantity <= 0 || newPrice <= 0) return false;
    if (!orderBook.removeOrder(orderId)) return false;

    existingOrder->setQuantity(newQuantity);
    auto limitPtr = std::dynamic_pointer_cast<LimitOrder>(existingOrder);
    if (!limitPtr) return false;
    limitPtr->setPrice(newPrice);

    submitOrder(existingOrder);
    return true;
}

// Returns a const reference to the OrderBook
const OrderBook& Exchange::getOrderBook() const {
    return orderBook;
}

// Returns the vector of all executed trades
const std::vector<Trade>& Exchange::getTrades() const {
    return trades;
}

} // namespace trading
