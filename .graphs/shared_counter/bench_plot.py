import csv
import matplotlib.pyplot as plt

threads = []
throughput = []

with open("cpp.csv", "r") as f:
    reader = csv.DictReader(f)
    for row in reader:
        threads.append(int(row["threads"]))
        throughput.append(float(row["throughput"]))

plt.figure()
plt.plot(threads, throughput, marker='o')
plt.xlabel("Number of Threads")
plt.ylabel("Throughput (ops/sec)")
plt.title("STM Shared Counter Throughput")
plt.grid(True)

plt.savefig("plot.png")
plt.show()