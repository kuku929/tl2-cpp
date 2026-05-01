import csv
import sys
from collections import defaultdict
from pathlib import Path

import matplotlib.pyplot as plt


def load_rows(csv_path: Path):
    rows = []
    with csv_path.open(newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            row["threads"] = int(row["threads"])
            row["throughput"] = float(row["throughput"])
            rows.append(row)
    return rows


def plot(csv_path: Path, out_path: Path):
    rows = load_rows(csv_path)
    by_workload = defaultdict(list)
    for row in rows:
        by_workload[row["workload"]].append(row)

    workloads = ["read_heavy", "balanced", "write_heavy"]
    lists = ["stm", "coarse", "fine", "lockfree"]

    fig, axes = plt.subplots(1, 3, figsize=(15, 4), sharey=True)

    for ax, workload in zip(axes, workloads):
        data = by_workload.get(workload, [])
        for list_name in lists:
            points = sorted(
                [r for r in data if r["list"] == list_name],
                key=lambda r: r["threads"],
            )
            xs = [p["threads"] for p in points]
            ys = [p["throughput"] for p in points]
            ax.plot(xs, ys, marker="o", label=list_name)

        ax.set_title(workload.replace("_", " "))
        ax.set_xlabel("threads")
        ax.grid(True, linestyle="--", alpha=0.4)

    axes[0].set_ylabel("throughput (ops/sec)")
    axes[-1].legend(loc="best")
    fig.tight_layout()
    fig.savefig(out_path, dpi=200)


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: plot.py <results.csv> <output.png>")
        sys.exit(1)

    csv_file = Path(sys.argv[1])
    out_file = Path(sys.argv[2])
    plot(csv_file, out_file)
