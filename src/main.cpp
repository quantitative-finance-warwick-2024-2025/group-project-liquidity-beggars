#include "exchange.hpp"
#include <iostream>

using namespace trading;

int main() {
    try {
        // Create exchange
        Exchange exchange;
        
        std::cout << "Trading System initialised...\n";
        std::cout << "==================\n\n";
        
        // Register traders
        auto trader1 = exchange.registerTrader();
        auto trader2 = exchange.registerTrader();
        
        // Create and submit limit orders
        std::cout << "Submitting orders...\n";
        
        auto buyOrder1 = trader1 -> createLimitOrder(100.0, 10, true);  // Buy 10 @ $100
        // auto buyOrder2 = trader1 -> createLimitOrder(99.0, 20, true);   // Buy 20 @ $99
        // auto sellOrder1 = trader2 -> createLimitOrder(101.0, 15, false); // Sell 15 @ $101
        // auto sellOrder2 = trader2 -> createLimitOrder(102.0, 10, false); // Sell 10 @ $102
        
        exchange.submitOrder(buyOrder1);
        // exchange.submitOrder(buyOrder2);
        // exchange.submitOrder(sellOrder1);
        // exchange.submitOrder(sellOrder2);
        
        // Show order book
        std::cout << exchange.getOrderBook().toString() << "\n";
        
        // Modify an order
        std::cout << "Modifying orders...\n";
        trader1 -> modifyOrder(buyOrder1->getId(), 100.0, 15);  // Modify to buy 15 @ $100
        
        // Show updated order book
        std::cout << exchange.getOrderBook().toString() << "\n";
        
        // Create and submit market orders
        std::cout << "Submitting market order...\n";
        auto marketOrder = trader2 -> createMarketOrder(5, false);  // Market order: sell 5
        exchange.submitOrder(marketOrder);
        
        // Show final order book
        std::cout << "\nFinal Order Book:\n";
        std::cout << exchange.getOrderBook().toString() << "\n";
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
