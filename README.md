# Transactional Locking II in C++
a STM algorithm implementation of algorithm described in this [work](https://people.csail.mit.edu/shanir/publications/Transactional_Locking.pdf) by Dave Dice, Ori Shalev and Nir Shavit.

## Requirements
- [Bazel](https://github.com/bazelbuild/bazel)

## How to Build
```bash
bazel build //src:<name-of-target>
```
Documentation for [MODULE.bazel](https://bazel.build/external/overview)

## On making changes
make sure to run the following two commands:
```bash
bazel build //... --config=clang-format-fix
bazel build //test/... --config=clang-format-fix
```
BEFORE pushing any code.

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
- [ ] Benchmark [STAMP](https://github.com/kozyraki/stamp/tree/master)
	- [ ] Is the read-set effecient?
		- Try : bloom filter, unordered_set, google dense hash map
		- Hashmap and vector implementation :- 
			- Hashmap to keep address and their values.
			- vector for fast insert(NEED TO SORT AT THE END TO PREVENT DEADLOCK).
- [ ] Add more tests
	- [x] Lecture 02
		- [x] Bakery
		- [x] Peterson
		- [x] Counter
	- [x] Lecture 03
		- [x] Bounded Queue
		- [x] Lockfree Queue
	- [x] Lecture 05
		- [x] Array Lock
		- [x] Backoff Lock
	- [ ] Lecture 06
		- [x] FIFO rw lock
		- [ ] Queue with rw lock
	- [x] Lecture 07
		- [x] Lazy Linked List
		- [x] Lockfree LL
		- [x] Optimistic LL
		- [x] Fine LL
		- [x] Coarse LL
	- [x] Lecture 08
		- [x] Bounded Queue
		- [x] Lockfree Stack
		- [x] Elimination Backoff Stack
- [ ] Better exception handling
	- [ ] A state machine
- [ ] Log memory allocation and cleanup in case of abort
- [ ] Add per-transaction buffers and synchronized pool resource
	- [ ] Test which is better.
- [ ] Graphs
    - [ ] Shared Counter(KC already has a plot)
    - [ ] Linked List(Varying reads/writes. See lecture 07)
    - [ ] Queues
    - [ ] KCAS
- [x] Add a guard to VersionLock(to remove unsafe_get/set)
- [x] Add support for arbitrary data types
- [x] Testbench
- [x] Split into different files
- [x] Any way to detect invalid get/set at compile time?
- [x] Use CAS operations
- [x] See how to specify own hash function(easy)

from [tl2](https://github.com/robert-schmidtke/stm/tree/master)
```
/*
 * Remarks on deadlock:
 * Indefinite spinning in the lock acquisition phase admits deadlock.
 * We can avoid deadlock by any of the following means:
 *
 * 1. Bounded spin with back-off and retry.
 *    If the we fail to acquire the lock within the interval we drop all
 *    held locks, delay (back-off - either random or exponential), and retry
 *    the entire txn.
 *
 * 2. Deadlock detection - detect and recover.
 *    Use a simple waits-for graph to detect deadlock.  We can recovery
 *    either by aborting *all* the participant threads, or we can arbitrate.
 *    One thread becomes the winner and is allowed to proceed or continue
 *    spinning.  All other threads are losers and must abort and retry.
 *
 * 3. Prevent or avoid deadlock by acquiring locks in some order.
 *    Address order using LockFor as the key is the most natural.
 *    Unfortunately this requires sorting -- See the LockRecord structure.
 */
```
