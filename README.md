# Transactional Locking II in C++
a STM algorithm implementation of algorithm described in this [work](https://people.csail.mit.edu/shanir/publications/Transactional_Locking.pdf) by Dave Dice, Ori Shalev and Nir Shavit.

## Requirements
- [Bazel](https://github.com/bazelbuild/bazel)

## How to Build
```bash
bazel build //src:<name-of-target>
```
Documentation for [MODULE.bazel](https://bazel.build/external/overview)

## On Tests

### Running
to run all tests:
```bash
bazel test //...
```
you can run specific tests using:
```bash
bazel test //test:<name-of-target>
```
the target name is defined in test/BUILD.

### Writing
Refer to googletest [documentation](https://github.com/google/googletest/blob/main/docs/primer.md) for further guidance

## TODO
- [ ] Add more tests
	- [ ] Lecture 02
		- [ ] Bakery
		- [ ] Peterson
		- [x] Counter
	- [ ] Lecture 03
		- [ ] Bounded Queue
		- [ ] Lockfree Queue
	- [ ] Lecture 05
		- [ ] Array Lock
		- [ ] Backoff Lock
	- [ ] Lecture 06
		- [ ] FIFO rw lock
		- [ ] Queue with rw lock
	- [ ] Lecture 07
		- [ ] Lazy Linked List
		- [ ] Lockfree LL
		- [ ] Optimistic LL
		- [ ] Fine LL
		- [ ] Coarse LL
	- [ ] Lecture 08
		- [ ] Bounded Queue
		- [ ] Lockfree Stack
		- [ ] Elimination Backoff Stack
- [ ] Use CAS operations
- [ ] See how to specify own hash function(easy)
- [ ] Add support for arbitrary data types
- [x] Testbench
- [x] Split into different files
- [x] Any way to detect invalid get/set at compile time?
