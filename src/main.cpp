#include "exchange.hpp"
#include "trader.hpp"
#include "order_book.hpp"
#include <iostream>
#include <fstream>
#include <random>
#include <chrono>
#include <iomanip>
#include <cmath>
#include <stdexcept>
#include <string>
#include <memory>
#include <algorithm>
#include <vector>
#include <cstdlib>

// Sets the ask quote based on the current belief and model parameters.
double computeAsk(double p, double alpha, double vHigh, double vLow) {
    double numerator = (alpha + 0.5 * (1.0 - alpha)) * p * vHigh
                     + 0.5 * (1.0 - alpha) * (1.0 - p) * vLow;
    double denominator = (alpha + 0.5 * (1.0 - alpha)) * p
                       + 0.5 * (1.0 - alpha) * (1.0 - p);
    if (denominator <= 0.0) {
        return vHigh;
    }
    return numerator / denominator;
}

// Sets the bid quote based on the current belief and model parameters.
double computeBid(double p, double alpha, double vHigh, double vLow) {
    double numerator = 0.5 * (1.0 - alpha) * p * vHigh
                     + (alpha + 0.5 * (1.0 - alpha)) * (1.0 - p) * vLow;
    double denominator = 0.5 * (1.0 - alpha) * p
                       + (alpha + 0.5 * (1.0 - alpha)) * (1.0 - p);
    if (denominator <= 0.0) {
        return vLow;
    }
    return numerator / denominator;
}

// Updates the market maker's belief after observing a buy or sell trade.
double updateBeliefAfterTrade(bool wasBuy, double p, double alpha) {
    double numerator = 0.0;
    double denominator = 0.0;
    if (wasBuy) {
        numerator = (alpha + 0.5 * (1.0 - alpha)) * p;
        denominator = numerator + 0.5 * (1.0 - alpha) * (1.0 - p);
    } else {
        numerator = 0.5 * (1.0 - alpha) * p;
        denominator = numerator + (alpha + 0.5 * (1.0 - alpha)) * (1.0 - p);
    }
    if (denominator == 0.0) {
        return p;
    }
    return numerator / denominator;
}

int main() {
    try {
        // Simulation parameters and rates.
        const double T = 200.0;                  // Total simulation time
        const double dt = 0.01;                  // Time-step size for each iteration
        const int numSteps = static_cast<int>(T / dt); // Number of discrete simulation steps
        const double lambda = 80.0;              // Rate parameter for poisson process of order arrivals
        const double pInformed = 0.25;           // Proportion of traders who are informed
        const double pBuyNoise = 0.5;            // Probability that a noise trader submits a buy order
        const double vHigh = 120.0;             // High fundamental value of the asset
        const double vLow  = 80.0;              // Low fundamental value of the asset
        const double transitionHighToLow = 0.02; // Probability rate of switching from high to low value
        const double transitionLowToHigh = 0.02; // Probability rate of switching from low to high value
        const double alpha = 0.85;               // Weighting factor for market maker's reaction to informed trades
        const double meanQuantity = 5.0;         // Average (mean) order size
        const double maxQuantity = 50.0;         // Upper bound on order size
        const double probLimitOrder = 0.7;       // Probability a trader submits a limit order
        const double transactionFeeRate = 0.001; // Transaction fee rate charged on the traded notional
        const bool allowInformedLimitOrders = true;  // Whether informed traders can place limit orders
        const double informedOrderAggression = 0.5;  // Price offset used by informed traders placing limit orders
        const double longRunMean = 0.5;          // Long-run equilibrium belief for the market maker
        const double meanReversionRate = 0.1;    // Speed at which the market maker's belief reverts to the long-run mean
        const double beliefLowerBound = 0.05;    // Lower bound on market maker's belief
        const double beliefUpperBound = 0.95;    // Upper bound on market maker's belief

        // Random number generators.
        std::random_device rd;
        std::mt19937_64 rng(rd());
        std::uniform_real_distribution<double> uniDist(0.0, 1.0);
        std::exponential_distribution<double> expQty(1.0 / meanQuantity);
        std::normal_distribution<double> noisePriceDist(0.0, 1.0);

        // Initialisation of the fundamental value.
        bool isHighValue = (uniDist(rng) < 0.5);
        double trueValue = isHighValue ? vHigh : vLow;

        // Create an exchange and register three traders.
        trading::Exchange exchange;
        auto marketMaker    = exchange.registerTrader();
        auto informedTrader = exchange.registerTrader();
        auto noiseTrader    = exchange.registerTrader();

        // Set up the market maker with an initial belief and corresponding quotes.
        double beliefP = 0.5;
        double currentAsk = computeAsk(beliefP, alpha, vHigh, vLow);
        double currentBid = computeBid(beliefP, alpha, vHigh, vLow);
        auto mmBidOrder = marketMaker->createLimitOrder(currentBid, 1e6, true);
        auto mmAskOrder = marketMaker->createLimitOrder(currentAsk, 1e6, false);
        exchange.submitOrder(mmBidOrder);
        exchange.submitOrder(mmAskOrder);
        std::string mmBidId = mmBidOrder->getId();
        std::string mmAskId = mmAskOrder->getId();

        // Output file for logging.
        std::ofstream outFile("masterpiece_simulation.csv");
        if (!outFile.is_open()) {
            std::cerr << "Error: unable to open output file." << std::endl;
            return 1;
        }
        // CSV Header: note we now log best_bid, best_ask and spread from the order book
        outFile << "time,arrival,trader_type,order_type,is_buy,quantity,exec_price_avg,num_trades,"
                << "best_bid,best_ask,spread,belief_p,true_value,fees\n";

        // Main time-stepping loop.
        for (int step = 0; step < numSteps; ++step) {
            double currentTime = step * dt;

            // Randomly evolve the fundamental value using a simple Markov process.
            bool oldIsHighValue = isHighValue; // Track pre-flip state
            if (isHighValue) {
                if (uniDist(rng) < transitionHighToLow * dt) {
                    isHighValue = false;
                    trueValue = vLow;
                }
            } else {
                if (uniDist(rng) < transitionLowToHigh * dt) {
                    isHighValue = true;
                    trueValue = vHigh;
                }
            }

            // If the fundamental value has flipped, reset the market-maker's quotes.
            if (oldIsHighValue != isHighValue) {
                exchange.cancelOrder(mmBidId);
                exchange.cancelOrder(mmAskId);
                double newAsk = computeAsk(beliefP, alpha, vHigh, vLow);
                double newBid = computeBid(beliefP, alpha, vHigh, vLow);
                auto newBidOrder = marketMaker->createLimitOrder(newBid, 1e6, true);
                auto newAskOrder = marketMaker->createLimitOrder(newAsk, 1e6, false);
                exchange.submitOrder(newBidOrder);
                exchange.submitOrder(newAskOrder);
                mmBidId = newBidOrder->getId();
                mmAskId = newAskOrder->getId();
                currentAsk = newAsk;
                currentBid = newBid;
            }

            // Determine if an order arrives based on a Poisson process approximation.
            double arrivalProb = 1.0 - std::exp(-lambda * dt);
            bool arrivalOccurs = (uniDist(rng) < arrivalProb);

            std::vector<trading::Trade> tradesExecuted;
            std::string traderTypeStr = "none";
            std::string orderTypeStr = "none";
            bool isBuy = false;
            double quantity = 0.0;
            double execPriceAvg = 0.0;
            int numTrades = 0;
            double feesCharged = 0.0;

            // Generate an order if it arrives.
            if (arrivalOccurs) {
                bool isInformed = (uniDist(rng) < pInformed);
                traderTypeStr = isInformed ? "informed" : "noise";

                if (isInformed) {
                    // Informed trader decides direction based on the true fundamental.
                    isBuy = (trueValue == vHigh);
                    double q = expQty(rng);
                    quantity = std::min(std::max(q, 1.0), maxQuantity);
                    double marketPrice = isBuy ? currentAsk : currentBid;
                    double notional = marketPrice * quantity;
                    double feeEstimate = notional * transactionFeeRate;
                    double netBenefit = isBuy
                            ? (trueValue - (marketPrice + feeEstimate))
                            : ((marketPrice - feeEstimate) - trueValue);
                    if (netBenefit <= 0) {
                        orderTypeStr = "SKIPPED";
                    } else {
                        if (allowInformedLimitOrders) {
                            orderTypeStr = "LIMIT";
                            double limitPrice = isBuy
                                ? (trueValue - informedOrderAggression)
                                : (trueValue + informedOrderAggression);
                            auto lo = informedTrader->createLimitOrder(limitPrice, quantity, isBuy);
                            tradesExecuted = exchange.submitOrder(lo);
                        } else {
                            orderTypeStr = "MARKET";
                            auto mo = informedTrader->createMarketOrder(quantity, isBuy);
                            tradesExecuted = exchange.submitOrder(mo);
                        }
                    }
                } else {
                    // Noise trader randomly chooses direction and order type.
                    isBuy = (uniDist(rng) < pBuyNoise);
                    bool placeLimit = (uniDist(rng) < probLimitOrder);
                    double q = expQty(rng);
                    quantity = std::min(std::max(q, 1.0), maxQuantity);
                    if (placeLimit) {
                        orderTypeStr = "LIMIT";
                        auto lo = noiseTrader->createLimitOrder(isBuy ? currentBid : currentAsk, quantity, isBuy);
                        tradesExecuted = exchange.submitOrder(lo);
                    } else {
                        orderTypeStr = "MARKET";
                        auto mo = noiseTrader->createMarketOrder(quantity, isBuy);
                        tradesExecuted = exchange.submitOrder(mo);
                    }
                }

                // If trades occur, gather results and update the market maker's belief and quotes.
                if (!tradesExecuted.empty()) {
                    double sumPrices = 0.0;
                    double totalQty = 0.0;
                    for (const auto& tr : tradesExecuted) {
                        sumPrices += tr.price * tr.quantity;
                        totalQty += tr.quantity;
                        feesCharged += tr.price * tr.quantity * transactionFeeRate;
                    }
                    numTrades = static_cast<int>(tradesExecuted.size());
                    if (totalQty > 0.0) {
                        execPriceAvg = sumPrices / totalQty;
                    }
                    beliefP = updateBeliefAfterTrade(isBuy, beliefP, alpha);
                    double newAsk = computeAsk(beliefP, alpha, vHigh, vLow);
                    double newBid = computeBid(beliefP, alpha, vHigh, vLow);
                    exchange.cancelOrder(mmBidId);
                    exchange.cancelOrder(mmAskId);
                    auto newBidOrder = marketMaker->createLimitOrder(newBid, 1e6, true);
                    auto newAskOrder = marketMaker->createLimitOrder(newAsk, 1e6, false);
                    exchange.submitOrder(newBidOrder);
                    exchange.submitOrder(newAskOrder);
                    mmBidId = newBidOrder->getId();
                    mmAskId = newAskOrder->getId();
                    currentAsk = newAsk;
                    currentBid = newBid;
                }
            }

            // Move the belief toward a long-run mean after each step.
            beliefP += meanReversionRate * (longRunMean - beliefP) * dt;
            if (beliefP < beliefLowerBound) {
                beliefP = beliefLowerBound;
            } else if (beliefP > beliefUpperBound) {
                beliefP = beliefUpperBound;
            }

            // Query the actual best quotes from the order book.
            double bestBid = 0.0;
            double bestAsk = 0.0;
            auto bestBidOrder = exchange.getOrderBook().getHighestBid();
            auto bestAskOrder = exchange.getOrderBook().getLowestAsk();
            if (bestBidOrder) {
                bestBid = bestBidOrder->getPrice();
            }
            if (bestAskOrder) {
                bestAsk = bestAskOrder->getPrice();
            }
            double spread = 0.0;
            if (bestAsk > 0.0 && bestBid > 0.0) {
                spread = bestAsk - bestBid;
            }

            // Write the current step's data to the CSV.
            outFile << std::fixed << std::setprecision(4)
                    << currentTime << ","
                    << (arrivalOccurs ? "yes" : "no") << ","
                    << traderTypeStr << ","
                    << orderTypeStr << ","
                    << (isBuy ? "buy" : "sell") << ","
                    << quantity << ","
                    << execPriceAvg << ","
                    << numTrades << ","
                    << bestBid << ","
                    << bestAsk << ","
                    << spread << ","
                    << beliefP << ","
                    << trueValue << ","
                    << feesCharged << "\n";
        }

        outFile.close();
        int returncode = system("python ../src/simulation_analysis.py");
        if (returncode == 0){
            std::cout << "Simulation completed. Results written to masterpiece_simulation.csv\n";
            std::cout << "Figure generated to build/figure.png\n";
            return 0;
        }
        else{
            std::cerr << "Python execution failed\n";
            return 1;
        }
    } catch (const std::exception &e) {
        std::cerr << "Simulation error: " << e.what() << "\n";
        return 1;
    }
}
