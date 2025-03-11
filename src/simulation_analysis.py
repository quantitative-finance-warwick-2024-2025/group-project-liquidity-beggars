import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Read the CSV file including actual best quotes and true value
data = pd.read_csv('../build/masterpiece_simulation.csv')

# Select the relevant columns
df = data[['time', 'best_bid', 'best_ask', 'spread', 'quantity', 'belief_p', 'true_value']]
df = df.rename(columns={'quantity': 'volume'})

# Compute exponential moving averages (EMA) for best_bid, best_ask and belief
df['best_bid_ema'] = df['best_bid'].ewm(span=100, adjust=False).mean()
df['best_ask_ema'] = df['best_ask'].ewm(span=100, adjust=False).mean()
df['belief_ema']   = df['belief_p'].ewm(span=100, adjust=False).mean()

# Plotting
fig, axs = plt.subplots(4, 1, figsize=(15, 12))

# Subplot 0: Raw Best Bid/Ask
axs[0].plot(df['time'], df['best_bid'], label='Best Bid', color='blue')
axs[0].plot(df['time'], df['best_ask'], label='Best Ask', color='red')
axs[0].set_title('Simulated Best Bid/Ask (Raw)')
axs[0].set_xlabel('Time')
axs[0].set_ylabel('Price')
axs[0].legend()

# Subplot 1: Simulated Spread
axs[1].plot(df['time'], df['spread'], label='Spread', color='green')
axs[1].set_title('Simulated Spread')
axs[1].set_xlabel('Time')
axs[1].set_ylabel('Spread')
axs[1].legend()

# Subplot 2: EMA of Best Bid/Ask and True Value
axs[2].plot(df['time'], df['best_bid_ema'], label='Bid (EMA)', color='blue')
axs[2].plot(df['time'], df['best_ask_ema'], label='Ask (EMA)', color='red')
axs[2].plot(df['time'], df['true_value'], label='True Value', color='orange')
axs[2].set_title('Exponentially Smoothed Bid/Ask and True Value')
axs[2].set_xlabel('Time')
axs[2].set_ylabel('Price')
axs[2].legend()

# Subplot 3: Market Maker's Confidence with EMA
axs[3].plot(df['time'], df['belief_p'], label='Belief (Raw)', color='black')
axs[3].plot(df['time'], df['belief_ema'], '--', label='Belief (EMA)', color='gray')
axs[3].set_title("Market Maker's Confidence")
axs[3].set_xlabel('Time')
axs[3].set_ylabel('Belief Probability')
axs[3].legend()

plt.tight_layout()
plt.savefig('../figure.png')
plt.show()

