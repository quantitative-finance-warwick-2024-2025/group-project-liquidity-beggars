This README file outlines supplementary implementation notes and instructions
for building, running, and extending our project.

## 1. Building and Running

1. **Requirements**:
   - C++20 compiler (we tested on Clang and G++).
   - CMake (version 3.10 or higher).
   - Python 3 with libraries `pandas`, `numpy`, and `matplotlib` for data analysis
     and plotting (used for `simulation_analysis.py`).

2. **Compilation**:
   - Create a build directory (e.g., `mkdir build && cd build`).
   - Run `cmake .. && make` in the build directory. 
   - This should generate an executable (for instance, `./my_program`).

3. **Running the Simulation**:
   - From the build directory, execute `./my_program`.
   - The program outputs a CSV file named `masterpiece_simulation.csv` in the
     `build` folder (or wherever the executable is run).
   - The Python script `simulation_analysis.py` is then automatically called to 
     parse the CSV file and produce the figure (`figure.png`).

4. **Tests**:
   - We included unit tests in the `tests` directory.
   - The command `make test` (or `ctest`) can be used after a successful build 
     to run all the Catch2-based tests.

## 2. Implementation Details

- **Core Classes**:
  - `Order` (parent), `LimitOrder`, `MarketOrder`
  - `OrderBook`
  - `Exchange`
  - `Trader`

- **Advanced Glosten--Milgrom Variation**:
  - We track a probabilistic belief about a latent “true value” that flips between
    `vHigh` and `vLow`.
  - The parameter `alpha` controls how strongly the market maker reacts to each
    observed trade.
  - We incorporate Poisson arrivals for new orders and a Markov chain for the
    fundamental flips.

- **Parameter Interpretation**:
  - `T = 200.0`: total simulation horizon (abstract time units).
  - `dt = 0.01`: step size for time integration.
  - `lambda = 80.0`: roughly indicates how many order arrivals are expected in a
    unit of time. (Higher means more arrivals, simulating an active market.)
  - `pInformed = 0.25`: fraction of orders from traders with potential private
    information, driving price discovery.
  - `vHigh = 120.0`, `vLow = 80.0`: possible true fundamental values, used in 
    computing the quotes and final trade outcomes. These are simplified but 
    chosen to emphasise visible price changes.
  - `transitionHighToLow = 0.02`, `transitionLowToHigh = 0.02`: the chance per 
    unit time that the underlying state flips. This randomises the fundamental 
    environment for the market maker.
  - `alpha = 0.85`: the weight the market maker places on informed signals. 
    A higher alpha implies the maker is more sensitive to the presence of
    informed traders.
  - `meanQuantity = 5.0`: average order size in the simulation (exponential
    distribution). Tied to real-world observation that typical order lots are 
    not extremely large but can vary significantly.
  - `transactionFeeRate = 0.001`: minimal transaction cost to reflect real 
    exchanges.

## 3. Possible Extensions

- **Varying \(\alpha\)**: Investigate how different intensities of informed trading
  sensitivity affect bid--ask spreads.
- **Alternate Order Type Mix**: Adjust the ratio of limit to market orders to see
  how depth and liquidity change.
- **Multiple Fundamental States**: Expand from two-state (high/low) to a more 
  granular Markov chain representing additional price levels.

## 4. Contact and Contributions

- Each group member contributed unique code modules (orders, exchange logic,
  simulation, tests, and documentation). Our commits reflect the history on 
  our GitHub repository. Please see the project report (PDF) for a high-level
  explanation of how tasks were allocated and tested.

---------------------------
