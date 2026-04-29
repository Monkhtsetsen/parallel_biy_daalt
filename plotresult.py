# ============================================================
# Plot benchmark results from results.csv
# Run: python plot_results.py
# Outputs:
#   execution_time.png
#   speedup.png
#   cuda_breakdown.png
#   performance_mops.png
# ============================================================
import math
from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd

CSV_FILE = Path("results.csv")

if not CSV_FILE.exists():
    raise FileNotFoundError("results.csv not found. Run the C++/CUDA benchmarks first.")

df = pd.read_csv(CSV_FILE)

# Keep latest row per Algorithm/N in case the benchmark was run multiple times.
df = df.drop_duplicates(subset=["Algorithm", "N"], keep="last")

# Ensure numeric columns are numeric.
for col in ["N", "ExecutionTimeMs", "H2DMs", "KernelMs", "D2HMs", "TotalTimeMs", "TransferredBytes", "TotalOperations", "AchievableMOPS"]:
    df[col] = pd.to_numeric(df[col], errors="coerce").fillna(0)

# Compute speedup using total time, because CUDA total includes H2D + Kernel + D2H.
seq = df[df["Algorithm"] == "Sequential"][["N", "TotalTimeMs"]].rename(columns={"TotalTimeMs": "SequentialTotalMs"})
df = df.merge(seq, on="N", how="left")
df["SpeedUp"] = df["SequentialTotalMs"] / df["TotalTimeMs"]
df.loc[df["TotalTimeMs"] <= 0, "SpeedUp"] = math.nan

# Save a richer CSV with speedup included.
df.to_csv("results_with_speedup.csv", index=False)

alg_order = ["Sequential", "std_thread", "OpenMP", "CUDA"]
df["Algorithm"] = pd.Categorical(df["Algorithm"], categories=alg_order, ordered=True)
df = df.sort_values(["N", "Algorithm"])

# 1) Execution time graph
pivot_time = df.pivot(index="N", columns="Algorithm", values="TotalTimeMs")
ax = pivot_time.plot(kind="bar", figsize=(10, 6))
ax.set_title("Merge Sort Total Execution Time")
ax.set_xlabel("Number of elements")
ax.set_ylabel("Total time (ms)")
ax.grid(axis="y", linestyle="--", alpha=0.4)
plt.xticks(rotation=0)
plt.tight_layout()
plt.savefig("execution_time.png", dpi=200)
plt.close()

# 2) Speedup graph
pivot_speed = df.pivot(index="N", columns="Algorithm", values="SpeedUp")
ax = pivot_speed.plot(kind="bar", figsize=(10, 6))
ax.set_title("SpeedUp Compared to Sequential Merge Sort")
ax.set_xlabel("Number of elements")
ax.set_ylabel("SpeedUp (Sequential total time / Algorithm total time)")
ax.grid(axis="y", linestyle="--", alpha=0.4)
plt.xticks(rotation=0)
plt.tight_layout()
plt.savefig("speedup.png", dpi=200)
plt.close()

# 3) CUDA breakdown graph
cuda = df[df["Algorithm"] == "CUDA"].sort_values("N")
if not cuda.empty:
    cuda_plot = cuda.set_index("N")[["H2DMs", "KernelMs", "D2HMs"]]
    ax = cuda_plot.plot(kind="bar", stacked=True, figsize=(10, 6))
    ax.set_title("CUDA Time Breakdown")
    ax.set_xlabel("Number of elements")
    ax.set_ylabel("Time (ms)")
    ax.grid(axis="y", linestyle="--", alpha=0.4)
    plt.xticks(rotation=0)
    plt.tight_layout()
    plt.savefig("cuda_breakdown.png", dpi=200)
    plt.close()

# 4) Achievable performance graph
pivot_perf = df.pivot(index="N", columns="Algorithm", values="AchievableMOPS")
ax = pivot_perf.plot(kind="bar", figsize=(10, 6))
ax.set_title("Achievable Performance")
ax.set_xlabel("Number of elements")
ax.set_ylabel("Million operations per second (MOPS)")
ax.grid(axis="y", linestyle="--", alpha=0.4)
plt.xticks(rotation=0)
plt.tight_layout()
plt.savefig("performance_mops.png", dpi=200)
plt.close()

print("Generated:")
print("- results_with_speedup.csv")
print("- execution_time.png")
print("- speedup.png")
print("- cuda_breakdown.png")
print("- performance_mops.png")
