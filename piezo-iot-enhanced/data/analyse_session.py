"""
analyse_session.py
──────────────────
Reads a CSV file exported from the Arduino Serial Monitor and produces
three plots:

  1. Voltage per step
  2. Cumulative energy harvested (µJ)
  3. Voltage distribution histogram

Usage:
    python analyse_session.py                       # uses sample_session_log.csv
    python analyse_session.py my_live_session.csv   # custom file

Requirements:
    pip install pandas matplotlib
"""

import sys
import pathlib
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec

# ── Config ────────────────────────────────────────────────────────────────────

DEFAULT_CSV = pathlib.Path(__file__).parent / "data" / "sample_session_log.csv"

# ── Load data ─────────────────────────────────────────────────────────────────

csv_path = pathlib.Path(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_CSV

if not csv_path.exists():
    raise FileNotFoundError(f"CSV not found: {csv_path}")

# Skip comment lines starting with '#'
df = pd.read_csv(csv_path, comment="#")
df.columns = df.columns.str.strip()

print(f"Loaded {len(df)} rows from {csv_path.name}")
print(df.describe())

# ── Compute derived columns ───────────────────────────────────────────────────

df["Cumulative_Energy_uJ"] = df["Energy_uJ"].cumsum()

# Keep only rows where a new step was registered for per-step analysis
steps_df = df.drop_duplicates(subset="Steps", keep="first").copy()
steps_df = steps_df[steps_df["Steps"] > 0]

# ── Plotting ─────────────────────────────────────────────────────────────────

fig = plt.figure(figsize=(14, 10))
fig.suptitle("Piezoelectric Energy Harvesting — Session Analysis",
             fontsize=15, fontweight="bold", y=0.98)

gs = gridspec.GridSpec(2, 2, figure=fig, hspace=0.45, wspace=0.35)

# ── Plot 1: Voltage vs time (all samples) ────────────────────────────────────
ax1 = fig.add_subplot(gs[0, :])
ax1.plot(df.index, df["Voltage_V"], color="#2196F3", linewidth=0.8,
         label="Measured Voltage")
ax1.fill_between(df.index, df["Voltage_V"], alpha=0.15, color="#2196F3")
ax1.plot(df.index, df["PeakV_Session"], color="#FF5722", linewidth=1.0,
         linestyle="--", label="Session Peak")
ax1.set_xlabel("Sample index")
ax1.set_ylabel("Voltage (V)")
ax1.set_title("Rectified Output Voltage Over Session")
ax1.legend(loc="upper right")
ax1.grid(alpha=0.3)

# ── Plot 2: Cumulative energy ─────────────────────────────────────────────────
ax2 = fig.add_subplot(gs[1, 0])
ax2.plot(df.index, df["Cumulative_Energy_uJ"], color="#4CAF50", linewidth=1.5)
ax2.fill_between(df.index, df["Cumulative_Energy_uJ"], alpha=0.15,
                 color="#4CAF50")
ax2.set_xlabel("Sample index")
ax2.set_ylabel("Cumulative Energy (µJ)")
ax2.set_title("Energy Harvested (Cumulative)")
ax2.grid(alpha=0.3)

# ── Plot 3: Voltage histogram ─────────────────────────────────────────────────
ax3 = fig.add_subplot(gs[1, 1])
ax3.hist(df["Voltage_V"], bins=20, color="#9C27B0", edgecolor="white",
         alpha=0.85)
ax3.axvline(df["Voltage_V"].mean(), color="#FF5722", linestyle="--",
            linewidth=1.5, label=f"Mean: {df['Voltage_V'].mean():.2f} V")
ax3.set_xlabel("Voltage (V)")
ax3.set_ylabel("Count")
ax3.set_title("Voltage Distribution")
ax3.legend()
ax3.grid(alpha=0.3, axis="y")

# ── Summary stats ────────────────────────────────────────────────────────────
total_steps  = int(df["Steps"].max())
total_energy = df["Cumulative_Energy_uJ"].iloc[-1]
peak_v       = df["PeakV_Session"].iloc[-1]
mean_v       = df[df["Voltage_V"] > 0.1]["Voltage_V"].mean()

print("\n── Session Summary ──────────────────────")
print(f"  Total steps    : {total_steps}")
print(f"  Peak voltage   : {peak_v:.3f} V")
print(f"  Mean voltage   : {mean_v:.3f} V (active steps)")
print(f"  Total energy   : {total_energy:.2f} µJ  ({total_energy/1000:.4f} mJ)")
print(f"  Energy/step    : {total_energy/max(total_steps,1):.2f} µJ")
print("─────────────────────────────────────────")

# ── Save figure ───────────────────────────────────────────────────────────────
out_path = csv_path.parent / "results.png"
plt.savefig(out_path, dpi=150, bbox_inches="tight")
print(f"\nPlot saved to: {out_path}")
plt.show()
