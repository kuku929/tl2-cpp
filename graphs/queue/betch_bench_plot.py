import matplotlib.pyplot as plt

def read_sections(filename):
    data = {}
    current = None

    with open(filename) as f:
        for line in f:
            line = line.strip()

            if not line:
                continue

            # section header
            if line.startswith("#"):
                current = line[2:]
                data[current] = {"x": [], "y": []}

            # skip header row
            elif line.startswith("threads"):
                continue

            # actual data
            elif current:
                t, val = line.split(",")
                data[current]["x"].append(int(t))
                data[current]["y"].append(float(val))

    return data


# read both files
cpp = read_sections("bench_cc.csv")
ocaml = read_sections("bench_ml.csv")


# plot each section
for section in cpp:
    if section not in ocaml:
        continue

    plt.figure()

    plt.plot(
        cpp[section]["x"],
        cpp[section]["y"],
        marker='o',
        label="C++ TL2"
    )

    plt.plot(
        ocaml[section]["x"],
        ocaml[section]["y"],
        marker='s',
        label="OCaml Kcas"
    )

    plt.title(section.replace("_", " ").title())
    plt.xlabel("Threads / Domains")
    plt.ylabel("Throughput (ops/sec)")
    plt.legend()
    plt.grid()

    filename = section + ".png"
    plt.savefig(filename)
    print(f"Saved {filename}")


plt.show()