#!/usr/bin/env python3
"""Convert a phone GPS log into time_s,speed_mps,heading_deg for ride_cli."""
import argparse
import csv
import math
import sys
from datetime import datetime


def find(headers, *keys, exclude=()):
    for i, h in enumerate(headers):
        hl = h.lower()
        if any(k in hl for k in keys) and not any(x in hl for x in exclude):
            return i
    return None


def num(row, i):
    if i is None or i >= len(row):
        return None
    try:
        v = float(row[i])
    except ValueError:
        return None
    return None if math.isnan(v) else v


def parse_time(text):
    try:
        x = float(text)
        return x / 1000.0 if x > 1e11 else x  # epoch milliseconds -> seconds
    except ValueError:
        return datetime.fromisoformat(text.strip().replace("Z", "+00:00")).timestamp()


def haversine_m(lat1, lon1, lat2, lon2):
    r = 6371000.0
    p1, p2 = math.radians(lat1), math.radians(lat2)
    dp, dl = p2 - p1, math.radians(lon2 - lon1)
    a = math.sin(dp / 2) ** 2 + math.cos(p1) * math.cos(p2) * math.sin(dl / 2) ** 2
    return 2 * r * math.asin(math.sqrt(a))


def bearing_deg(lat1, lon1, lat2, lon2):
    p1, p2 = math.radians(lat1), math.radians(lat2)
    dl = math.radians(lon2 - lon1)
    y = math.sin(dl) * math.cos(p2)
    x = math.cos(p1) * math.sin(p2) - math.sin(p1) * math.cos(p2) * math.cos(dl)
    return (math.degrees(math.atan2(y, x)) + 360.0) % 360.0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("input")
    ap.add_argument("output")
    ap.add_argument("--kmh", action="store_true", help="speed column is in km/h")
    args = ap.parse_args()

    with open(args.input, newline="", encoding="utf-8-sig") as f:
        text = f.read()
    try:
        dialect = csv.Sniffer().sniff(text[:2048], delimiters=",;\t")
    except csv.Error:
        dialect = csv.excel
    rows = list(csv.reader(text.splitlines(), dialect))
    headers, data = rows[0], rows[1:]
    print("Columns found:", headers)

    t_i = find(headers, "time")
    sp_i = find(headers, "speed", exclude=("acc",))
    hd_i = find(headers, "direction", "bearing", "heading", "course", exclude=("acc",))
    la_i = find(headers, "lat", exclude=("acc",))
    lo_i = find(headers, "lon", exclude=("acc",))

    if t_i is None:
        sys.exit("No time column found.")
    if (sp_i is None or hd_i is None) and (la_i is None or lo_i is None):
        sys.exit("Need speed+heading columns, or latitude+longitude to derive them.")

    pts = []
    for r in data:
        try:
            t = parse_time(r[t_i])
        except (ValueError, IndexError):
            continue
        sp = num(r, sp_i)
        if sp is not None and args.kmh:
            sp /= 3.6
        hd = num(r, hd_i)
        if hd is not None and hd < 0:
            hd = None  # some apps use -1 for "unknown"
        pts.append({"t": t, "lat": num(r, la_i), "lon": num(r, lo_i), "speed": sp, "heading": hd})

    if len(pts) < 2:
        sys.exit("Not enough valid rows.")

    for k in range(1, len(pts)):
        a, b = pts[k - 1], pts[k]
        if None in (a["lat"], a["lon"], b["lat"], b["lon"]):
            continue
        dt = b["t"] - a["t"]
        if dt <= 0:
            continue
        if b["speed"] is None:
            b["speed"] = haversine_m(a["lat"], a["lon"], b["lat"], b["lon"]) / dt
        if b["heading"] is None:
            b["heading"] = bearing_deg(a["lat"], a["lon"], b["lat"], b["lon"])

    t0 = pts[0]["t"]
    last_heading = 0.0
    kept = 0
    max_speed = 0.0
    last_t = t0
    with open(args.output, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["time_s", "speed_mps", "heading_deg"])
        for p in pts:
            if p["speed"] is None:
                continue
            if p["heading"] is not None:
                last_heading = p["heading"]
            w.writerow([f"{p['t'] - t0:.3f}", f"{p['speed']:.3f}", f"{last_heading:.1f}"])
            kept += 1
            max_speed = max(max_speed, p["speed"])
            last_t = p["t"]

    print(f"Wrote {kept} rows, {last_t - t0:.0f} s, max speed {max_speed * 3.6:.1f} km/h")
    if max_speed * 3.6 > 200:
        print("WARNING: max speed looks too high. Is the speed column km/h? Try --kmh.")
    if 0 < max_speed * 3.6 < 5:
        print("WARNING: max speed looks too low. Check the speed units.")


if __name__ == "__main__":
    main()