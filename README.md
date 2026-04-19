# Transactional Locking II in C++
a STM algorithm implementation of algorithm described in this [work](https://people.csail.mit.edu/shanir/publications/Transactional_Locking.pdf) by Dave Dice, Ori Shalev and Nir Shavit.

## Requirements
- [Bazel](https://github.com/bazelbuild/bazel)

## How to Build
```bash
bazel build //src:<name-of-target>
```
Documentation for ![MODULE.bazel](https://bazel.build/external/overview)

## TODO
[x] Split into different files
[x] Use CAS operations - DP
[] See how to specify own hash function(easy)
[] Any way to detect invalid get/set at compile time?
[] Add support for arbitrary data types
[] Testbench - KP
