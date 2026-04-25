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
- [ ] Benchmark [STAMP](https://github.com/kozyraki/stamp/tree/master)
	- [ ] Is the read-set effecient?
		- Try : bloom filter, unordered_set, google dense hash map
		- Hashmap and vector implementation :- 
			- Hashmap to keep address and their values.
			- vector for fast insert(NEED TO SORT AT THE END TO PREVENT DEADLOCK).
- [ ] Add more tests
	- [ ] Lecture 02
		- [ ] Bakery
		- [x] Peterson
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
