#!/usr/bin/env python3
"""Convert a Phyphox GPS export (zip or csv) into time_s,speed_mps,heading_deg at a
fixed rate, ready for ride_cli.

Why resample? Phone GPS rows arrive at irregular times, sometimes a few milliseconds
apart. Acceleration = change in speed / change in time, so tiny time gaps would turn
small speed noise into huge fake accelerations.

Usage: python tools/convert_ride.py export.zip data/ride_01.csv
"""
import argparse
import csv
import io
import math
import sys
import zipfile


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


def read_text(path):
    if path.lower().endswith(".zip"):
        with zipfile.ZipFile(path) as z:
            names = [n for n in z.namelist()
                     if n.lower().endswith(".csv") and not n.lower().startswith("meta/")]
            if not names:
                sys.exit("No data CSV found inside the zip.")
            name = next((n for n in names if "raw" in n.lower()), names[0])
            print("Reading", name, "from the zip")
            return z.read(name).decode("utf-8-sig")
    with open(path, encoding="utf-8-sig", newline="") as f:
        return f.read()


def interp_heading(h0, h1, f):
    d = ((h1 - h0 + 540.0) % 360.0) - 180.0  # shortest way round the circle
    return (h0 + d * f) % 360.0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("input")
    ap.add_argument("output")
    ap.add_argument("--kmh", action="store_true", help="speed column is in km/h")
    ap.add_argument("--hz", type=float, default=1.0, help="output rate (default 1)")
    ap.add_argument("--max-acc", type=float, default=25.0,
                    help="drop fixes with horizontal accuracy worse than this, metres")
    ap.add_argument("--max-gap", type=float, default=5.0,
                    help="do not bridge GPS gaps longer than this, seconds")
    args = ap.parse_args()

    text = read_text(args.input)
    try:
        dialect = csv.Sniffer().sniff(text[:2048], delimiters=",;\t")
    except csv.Error:
        dialect = csv.excel
    rows = list(csv.reader(io.StringIO(text), dialect))
    headers, data = rows[0], rows[1:]
    print("Columns found:", headers)

    t_i = find(headers, "time")
    sp_i = find(headers, "speed", "velocity", exclude=("acc",))
    hd_i = find(headers, "direction", "bearing", "heading", "course", exclude=("acc",))
    acc_i = find(headers, "horizontal acc", "hacc")
    if t_i is None or sp_i is None:
        sys.exit("Need a time column and a speed/velocity column.")

    pts, no_speed, bad_acc = [], 0, 0
    for r in data:
        t = num(r, t_i)
        sp = num(r, sp_i)
        if t is None:
            continue
        if sp is None or sp < 0:
            no_speed += 1  # iOS reports no speed when it has no good fix
            continue
        if args.kmh:
            sp /= 3.6
        acc = num(r, acc_i)
        if acc is not None and acc > args.max_acc:
            bad_acc += 1
            continue
        pts.append({"t": t, "speed": sp, "heading": num(r, hd_i)})

    print(f"Rows: {len(data)}, no valid speed: {no_speed}, poor accuracy: {bad_acc}, "
          f"usable: {len(pts)}")
    if len(pts) < 2:
        sys.exit("Not enough usable GPS rows. Was there a clear sky view and were you "
                 "moving? Indoors or standing still the phone often gives no speed.")

    pts.sort(key=lambda p: p["t"])
    clean = []
    for p in pts:  # collapse bursts of rows less than 0.2 s apart, keep the later one
        if clean and p["t"] - clean[-1]["t"] < 0.2:
            clean[-1] = p
        else:
            clean.append(p)
    last_h = 0.0
    for p in clean:  # unknown heading (standing still) -> carry the last known one
        if p["heading"] is None or p["heading"] < 0:
            p["heading"] = last_h
        else:
            last_h = p["heading"]
    if len(clean) < 2:
        sys.exit("Not enough usable GPS rows after cleaning.")

    t0, t1 = clean[0]["t"], clean[-1]["t"]
    out, j = [], 0
    for k in range(int(math.floor((t1 - t0) * args.hz)) + 1):
        g = t0 + k / args.hz
        while j + 2 < len(clean) and clean[j + 1]["t"] < g:
            j += 1
        a, b = clean[j], clean[j + 1]
        span = b["t"] - a["t"]
        if span > args.max_gap:
            continue  # inside a GPS gap, do not invent data
        f = min(max((g - a["t"]) / span, 0.0), 1.0) if span > 0 else 0.0
        out.append((g - t0, a["speed"] + f * (b["speed"] - a["speed"]),
                    interp_heading(a["heading"], b["heading"], f)))

    with open(args.output, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["time_s", "speed_mps", "heading_deg"])
        for t, s, h in out:
            w.writerow([f"{t:.3f}", f"{s:.3f}", f"{h:.1f}"])

    max_kmh = max(s for _, s, _ in out) * 3.6 if out else 0.0
    print(f"Wrote {len(out)} rows covering {t1 - t0:.0f} s, max speed {max_kmh:.1f} km/h")
    if max_kmh > 200:
        print("WARNING: max speed looks too high. Is the speed column km/h? Try --kmh.")
    elif 0 < max_kmh < 5:
        print("WARNING: max speed under 5 km/h. Was the phone actually moving?")


if __name__ == "__main__":
    main()