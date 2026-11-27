#!/usr/bin/env python3
"""Plot speed and accelerations with detected events. Usage:
python tools/plot_ride.py motion.csv ride.json [out.png]"""
import csv
import json
import sys

import matplotlib.pyplot as plt

COLORS = {"harsh_braking": "tab:red", "harsh_acceleration": "tab:green",
          "hard_cornering": "tab:orange"}
LABELS = {"harsh_braking": "Harsh braking", "harsh_acceleration": "Harsh acceleration",
          "hard_cornering": "Hard cornering"}


def main():
    if len(sys.argv) < 3:
        print("Usage: python tools/plot_ride.py motion.csv ride.json [out.png]")
        sys.exit(1)
    out = sys.argv[3] if len(sys.argv) > 3 else "ride_plot.png"

    with open(sys.argv[1], newline="") as f:
        rows = list(csv.DictReader(f))
    t = [float(r["time_s"]) for r in rows]
    speed = [float(r["speed_mps"]) * 3.6 for r in rows]
    lng = [float(r["long_acc"]) for r in rows]
    lat = [float(r["lat_acc"]) for r in rows]

    with open(sys.argv[2]) as f:
        data = json.load(f)
    cfg = data["config"]

    fig, ax = plt.subplots(3, 1, sharex=True, figsize=(11, 8))
    ax[0].plot(t, speed)
    ax[0].set_ylabel("Speed (km/h)")
    ax[1].plot(t, lng)
    ax[1].axhline(cfg["brake_threshold"], ls="--", c="tab:red")
    ax[1].axhline(cfg["accel_threshold"], ls="--", c="tab:green")
    ax[1].set_ylabel("Longitudinal acc (m/s²)")
    ax[2].plot(t, lat)
    ax[2].axhline(cfg["corner_threshold"], ls="--", c="tab:orange")
    ax[2].axhline(-cfg["corner_threshold"], ls="--", c="tab:orange")
    ax[2].set_ylabel("Lateral acc (m/s²)")
    ax[2].set_xlabel("Time (s)")

    seen = set()
    for e in data["events"]:
        target = ax[2] if e["type"] == "hard_cornering" else ax[1]
        label = None if e["type"] in seen else LABELS[e["type"]]
        seen.add(e["type"])
        target.axvspan(e["start_s"], e["end_s"], color=COLORS[e["type"]], alpha=0.3, label=label)
    for a in ax:
        if a.get_legend_handles_labels()[0]:
            a.legend(loc="upper right")

    fig.suptitle(f"Smoothness score: {data['summary']['score']}/100")
    fig.tight_layout()
    fig.savefig(out, dpi=150)
    print("Saved", out)


if __name__ == "__main__":
    main()