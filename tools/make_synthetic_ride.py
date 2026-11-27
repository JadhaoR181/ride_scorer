#!/usr/bin/env python3
"""Generate a SYNTHETIC motorcycle ride (1 Hz GPS-style log) with known events.

NOT real ride data. It exists to exercise and test the C++ engine before a real
ride is recorded. Output columns match ride_cli: time_s,speed_mps,heading_deg

Usage: python tools/make_synthetic_ride.py data/synthetic_ride_01.csv
"""
import csv
import random
import sys

random.seed(350)  # fixed seed -> same file every run

# (label, duration_s, longitudinal accel m/s^2, yaw rate deg/s)
SEGMENTS = [
    ("stopped at start",        8,  0.0,   0.0),
    ("smooth pull away",        7,  1.6,   0.0),
    ("cruise",                 30,  0.0,   0.0),
    ("gentle right turn",       9, -0.1,  10.0),
    ("cruise",                 25,  0.0,   0.0),
    ("ease down for speed breaker", 4, -1.8, 0.0),
    ("crawl over breaker",      3,  0.0,   0.0),
    ("accelerate after breaker", 5,  1.5,  0.0),
    ("cruise",                 20,  0.0,   0.0),
    ("overtake: harsh accel",   4,  2.8,   0.0),
    ("cruise",                  8,  0.0,   0.0),
    ("slow for corner",         5, -2.2,   0.0),
    ("hard corner (left)",      5,  0.0, -17.0),
    ("speed up",                4,  1.4,   0.0),
    ("cruise",                  6,  0.0,   0.0),
    ("traffic: harsh brake",    5, -3.5,   0.0),
    ("stopped at red light",   12,  0.0,   0.0),
    ("smooth pull away",        8,  1.5,   0.0),
    ("cruise",                 35,  0.0,   0.0),
    ("gentle left turn",        8,  0.0,  -8.0),
    ("cruise",                 25,  0.0,   0.0),
    ("smooth stop",            10, -1.3,   0.0),
    ("stopped at end",          6,  0.0,   0.0),
]


def main():
    out_path = sys.argv[1] if len(sys.argv) > 1 else "data/synthetic_ride_01.csv"
    v, heading, t = 0.0, 300.0, 0  # start heading crosses north during the first turn
    rows = []
    for label, dur, acc, yaw in SEGMENTS:
        for _ in range(dur):
            v = max(0.0, v + acc)
            heading = (heading + yaw) % 360.0
            speed_meas = max(0.0, v + random.gauss(0, 0.08))
            # a stopped phone reports junk headings; the engine must ignore them
            h_meas = random.uniform(0, 360) if v < 0.5 else heading + random.gauss(0, 0.5)
            rows.append((t, round(speed_meas, 3), round(h_meas % 360.0, 1)))
            t += 1
    with open(out_path, "w", newline="") as f:
        w = csv.writer(f, lineterminator="\n")
        w.writerow(["time_s", "speed_mps", "heading_deg"])
        w.writerows(rows)
    print(f"Wrote {len(rows)} rows ({rows[-1][0]} s) to {out_path}")


if __name__ == "__main__":
    main()
