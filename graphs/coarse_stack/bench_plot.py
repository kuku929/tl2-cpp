import matplotlib.pyplot as plt
import csv

def load_file(filename):
    threads = []
    throughput = []

    with open(filename) as f:
        reader = csv.DictReader(f)
        for row in reader:
            threads.append(int(row["threads"]))
            throughput.append(float(row["throughput"]))

    return threads, throughput


plt.figure()

# ---- OCaml ----
threads_ocaml, ocaml = load_file("ocaml.csv")
plt.plot(threads_ocaml, ocaml, marker='o', label="OCaml Kcas")

# ---- C++ ----
threads_cpp, cpp = load_file("cpp.csv")
plt.plot(threads_cpp, cpp, marker='o', label="C++ Stack")

# ---- Labels ----
plt.xlabel("Number of Threads")
plt.ylabel("Throughput (ops/sec)")
plt.title("Stack Throughput vs Threads")

plt.grid()
plt.legend()

plt.savefig("plot.png")
plt.show()