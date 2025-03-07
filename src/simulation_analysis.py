import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

data = pd.read_csv('../build/masterpiece_simulation.csv')
cols = ['time', 'trader_type', 'order_type', 'is_buy', 'quantity', 'exec_price_avg', 'belief_p']
df_collapsed = data[cols]

df = pd.DataFrame(np.zeros(shape=(len(df_collapsed), 6)), columns=['time', 'bid', 'ask', 'spread', 'volume', 'belief_p'])
df['time'] = df_collapsed['time']
df['volume'] = df_collapsed['quantity']
df['belief_p'] = df_collapsed['belief_p']

for i in df_collapsed.index:
    if df_collapsed.loc[i, 'order_type'] == 'MARKET':
        if df_collapsed.loc[i, 'is_buy'] == 'buy':
            # the trader must have bought an ask from a market maker
            df.loc[i, 'ask'] = df_collapsed.loc[i, 'exec_price_avg']
        else:
            # the trader must have sold to a bid from a market maker 
            df.loc[i, 'bid'] = df_collapsed.loc[i, 'exec_price_avg']

# Forward fill the bid and ask columns
df['bid'] = df['bid'].replace(0, np.nan).ffill()
df['ask'] = df['ask'].replace(0, np.nan).ffill()

# If you want to fill NaN values with a specific value (e.g., 0), you can use fillna
df['bid'] = df['bid'].fillna(0)
df['ask'] = df['ask'].fillna(0)

df['spread'] = df['ask'] - df['bid']

i = 0
while df.loc[i, 'bid'] == 0 or df.loc[i, 'ask'] == 0:
    df = df.drop(i)
    i += 1

fig, axs = plt.subplots(4, 1, figsize=(15, 12))

axs[0].plot(df['time'], df['bid'], label='bid', color='blue')
axs[0].plot(df['time'], df['ask'], label='ask', color='red')
axs[1].plot(df['time'], df['spread'], label='spread', color='green')
axs[2].bar(df['time'], df['volume'], label='volume')
axs[3].plot(df['time'], df['belief_p'], label='belief', color='black')


axs[0].set_title('Simulated Bid/Ask')
axs[1].set_title('Simulated Spread')
axs[2].set_title('Trading volume')
axs[3].set_title("Market Maker's confidence") #### NEED TO CHANGE THIS NOT SURE
axs[0].set_xlabel('Time')
axs[1].set_xlabel('Time')
axs[2].set_xlabel('Time')
axs[3].set_xlabel('Time')
axs[0].set_ylabel('Price')
axs[1].set_ylabel('Spread')
axs[2].set_ylabel('Quantity')
axs[3].set_ylabel('Probability - belief')
axs[0].legend()

plt.tight_layout()
plt.savefig('figure.png')
