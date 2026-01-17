import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

def drop_first(group): #to drop the first execution and have a more reliable average
    return group.iloc[1:]

data_seq = "./test_bench/seq.csv"
data_AOS = "./test_bench/AOS_parallel_SIMD_noPadding.csv"




df_seq = pd.read_csv(data_seq, comment='#', header=None, names=['N', 'frames', 'threads', 'time_ms'])
df_seq = df_seq[df_seq['N'] != 'N']

df_seq = df_seq.astype({'N': int, 'frames': int, 'threads': int, 'time_ms': int})

df_filtered_seq = df_seq.groupby('N', group_keys=False).apply(drop_first)

#averages for every different N value
mean_times_seq = df_filtered_seq.groupby('N')['time_ms'].mean()


print("To visualize if averages have been properly calculated (sequential) (ms): ")
print(mean_times_seq)

df_AOS = pd.read_csv(data_AOS, comment='#', header=None, names=['N', 'frames', 'threads', 'time_ms'])
df_AOS = df_AOS[df_AOS['N'] != 'N']

df_AOS = df_AOS.astype({'N': int, 'frames': int, 'threads': int, 'time_ms': int})

df_AOS['run_idx'] = df_AOS.groupby(['N', 'threads']).cumcount()

df_filtered_AOS = df_AOS[df_AOS['run_idx'] > 0]

#averages for every different N value
mean_times_AOS = df_filtered_AOS.groupby(['N', 'threads'])['time_ms'].mean()


print("To visualize if averages have been properly calculated (AOS) (ms): ")
print(mean_times_AOS)


speedup_AOS = {}

for (N, threads), t_par in mean_times_AOS.items():
    t_seq = mean_times_seq.loc[N]
    speedup_AOS[(N, threads)] = t_seq / t_par

df_speedup = (
    pd.Series(speedup_AOS)
    .rename("speedup")
    .reset_index()
    .rename(columns={"level_0": "N", "level_1": "threads"})
)

print(df_speedup)

#---------PLOTS---------

plt.figure(figsize=(10,6))

for threads in sorted(df_speedup["threads"].unique()):
    sub = df_speedup[df_speedup["threads"] == threads]
    plt.plot(sub["N"], sub["speedup"], marker="o", label=f"{threads} threads")


ticks = np.arange(1500, 12001, 1500)
plt.xticks(ticks)
plt.xlim(1400, 12100)
plt.ylim(0, 6)
plt.xlabel("N")
plt.ylabel("Speed-up")
plt.title("AOS_SIMD_noPadding vs Sequential. 300 Frames")
plt.legend()

plt.grid(True)

plt.show()

