import matplotlib.pyplot as plt

def read(file):
    x, y = [], []
    with open(file) as f:
        next(f)  # skip header
        for line in f:
            t, thr = line.strip().split(',')
            x.append(int(t))
            y.append(float(thr))
    return x, y

cpp_x, cpp_y = read("cpp.csv")
ocaml_x, ocaml_y = read("ocaml.csv")

plt.plot(cpp_x, cpp_y, marker='o', label="C++ TL2")
plt.plot(ocaml_x, ocaml_y, marker='s', label="OCaml Kcas")

plt.xlabel("Threads / Domains")
plt.ylabel("Throughput (ops/sec)")
plt.title("STM Queue Benchmark")
plt.legend()
plt.grid()

plt.savefig("benchmark.png")
plt.show()