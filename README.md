*This project has been created as part of the 42 curriculum by tkern.*

# Codexion

## Description

Codexion is a concurrency simulation written in C with POSIX threads.

A number of **coders** sit around a circular table with a shared Quantum-
Compiler in the middle. Between every two coders lies one **USB dongle**, so
there are as many dongles as coders. To compile, a coder needs both the dongle
on their left and the dongle on their right. Each coder repeats the same cycle:

1. take two dongles and **compile**,
2. put the dongles back and **debug**,
3. **refactor**, then try to compile again.

If a coder does not start compiling within `time_to_burnout` milliseconds of
their last compile (or of the start of the simulation), they **burn out** and
the simulation stops. It also stops when every coder has compiled at least
`number_of_compiles_required` times.

The goal of the project is to share limited resources between threads without
deadlocks, without starvation and without data races, while respecting a
cooldown on each dongle and a fair scheduling policy (`fifo` or `edf`).

## Instructions

### Compilation

```sh
cd coders
make
```

The Makefile compiles with `cc -Wall -Wextra -Werror -pthread` and provides
the rules `all`, `clean`, `fclean` and `re`.

### Usage

```sh
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
           time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

| Argument                      | Meaning                                                        |
|-------------------------------|----------------------------------------------------------------|
| `number_of_coders`            | Number of coders and dongles (at least 1)                      |
| `time_to_burnout`             | ms a coder can go without starting a compile                   |
| `time_to_compile`             | ms spent compiling (holding two dongles)                       |
| `time_to_debug`               | ms spent debugging                                             |
| `time_to_refactor`            | ms spent refactoring                                           |
| `number_of_compiles_required` | The simulation ends when every coder compiled this many times |
| `dongle_cooldown`             | ms a dongle stays unavailable after being released             |
| `scheduler`                   | `fifo` (first come, first served) or `edf` (earliest deadline) |

All numbers must be non-negative integers without a sign. Invalid input prints
a usage message and the program exits with status 1.

### Examples

```sh
./codexion 5 800 200 200 200 5 0 edf     # nobody should burn out
./codexion 4 310 200 100 100 5 0 fifo    # impossible timing: a coder burns out
./codexion 1 800 200 200 200 3 0 fifo    # one dongle only: the coder burns out
```

Every state change is printed as `timestamp_in_ms coder_id message`:

```
0 1 has taken a dongle
0 1 has taken a dongle
0 1 is compiling
200 1 is debugging
...
801 3 burned out
```

## Blocking cases handled

### Deadlock (Coffman's conditions)

A deadlock needs all four Coffman conditions at the same time:

1. **Mutual exclusion**: a dongle can be held by only one coder. This is
   required by the subject.
2. **Hold and wait**: a coder holds one dongle while waiting for the second.
3. **No preemption**: a dongle cannot be taken away from a coder.
4. **Circular wait**: every coder holds one dongle and waits for the next.

The solution breaks **circular wait**. Coders with an odd id take their left
dongle first, coders with an even id take their right dongle first. For a cycle,
every coder would have to grab in the same direction, which this alternation
makes impossible. It also prevents long chains of coders each holding one
dongle, so about half of the coders can compile in parallel from the start.

### Starvation and fairness

Each dongle has its own **priority queue** (a binary min-heap, `heap.c`).
A coder who wants a dongle pushes a request into that dongle's queue and may
only take the dongle when their request is at the top:

- `fifo`: the key is a ticket number that increases with every request, so
  requests are served in arrival order. Tickets are unique, so there are no ties.
- `edf`: the key is `last_compile_start + time_to_burnout`, so the coder closest
  to burning out is served first.

Because a released dongle always goes to the head of the queue, a coder can
never be overtaken forever by a faster neighbour.

### Dongle cooldown

When a dongle is released, its `available_at` time is set to
`now + dongle_cooldown`. The coder at the head of the queue waits with
`pthread_cond_timedwait` exactly until `available_at`, so the cooldown is
respected without busy waiting.

### Precise burnout detection

A separate **monitor thread** checks every coder every 0.5 ms. When a coder has
not started compiling for more than `time_to_burnout` ms, the monitor prints the
burnout message and stops the simulation. In tests the message appears about
1 ms after the deadline, well within the required 10 ms.

### Single coder

With one coder there is only one dongle (left and right are the same dongle).
The coder takes it, never asks for it a second time, and waits until the
monitor detects the burnout.

### Clean shutdown

When the simulation stops, every coder blocked on a dongle is woken up
(`wake_all_dongles`), all sleeps check the stop flag, and the main thread joins
all threads and frees all memory before exiting.

### Log serialization

Every message is printed while holding `log_mutex`, so two lines can never
be mixed up. The burnout message sets the stop flag under the same mutex, so no
other message can be printed after it.

The two "has taken a dongle" lines and the "is compiling" line are printed
together in one `log_mutex` block, once both dongles are held. This way, every
"is compiling" line comes right after its own two dongle lines. A single coder
only gets one dongle, so that one line is printed as soon as it is taken.

## Thread synchronization mechanisms

| Primitive                        | Protects                                                      |
|----------------------------------|---------------------------------------------------------------|
| `dongle->mutex` (one per dongle) | `in_use`, `available_at`, `next_ticket` and the request queue |
| `dongle->cond` (one per dongle)  | Coders waiting for their turn or for the cooldown             |
| `coder->mutex` (one per coder)   | `last_compile_start` and `compiles_done`                      |
| `sim->stop_mutex`                | The `stop` flag                                               |
| `sim->log_mutex`                 | Standard output                                               |

### Coders and dongles

`acquire_dongle` locks the dongle mutex, pushes a request into the queue and
waits on the dongle's condition variable until three things are true: the coder
is at the head of the queue, the dongle is free, and the cooldown is over.
`pthread_cond_wait` releases the mutex while waiting, so other coders can
release the dongle or join the queue. `release_dongle` marks the dongle free,
starts the cooldown and calls `pthread_cond_broadcast` to wake the waiters,
who then check again whether it is their turn.

### Coders and the monitor

Coders and the monitor never talk directly. They only share data protected by
mutexes:

- A coder writes `last_compile_start` and `compiles_done` under its own
  `coder->mutex`. The monitor reads them under the same mutex, so it never sees
  a half-written value.
- The monitor sets `stop` under `stop_mutex`. Coders read it through
  `sim_stopped()`, which also takes `stop_mutex`.
- After setting `stop`, the monitor locks each dongle mutex and broadcasts its
  condition variable. Because a coder checks `stop` while holding the dongle
  mutex and only releases it inside `pthread_cond_wait`, the wake-up cannot be
  lost between the check and the wait.

### Lock order

To avoid deadlocks between the locks themselves, they are always taken in the
same order:

- `log_mutex` before `stop_mutex`
- a dongle mutex before `stop_mutex`
- a dongle mutex and `log_mutex` are never held at the same time

### Race conditions prevented (examples)

- Two coders cannot both take the same dongle: `in_use` is checked and set
  while holding the dongle mutex.
- The monitor cannot read `last_compile_start` while a coder is updating it.
- A coder cannot print "is compiling" after a burnout message: `log_state`
  checks the stop flag while holding `log_mutex`.

## Project structure

| File                 | Content                                                  |
|----------------------|----------------------------------------------------------|
| `codexion.c`         | `main`, thread creation and joining                      |
| `codexion.h`         | Structures, constants and prototypes                     |
| `parse.c`            | Argument validation                                      |
| `sim_setup.c`        | Allocation and cleanup of the simulation                 |
| `dongle_setup.c`     | Initialization and destruction of the dongles            |
| `coder_setup.c`      | Initialization of the coders and their dongle pointers   |
| `coder.c`            | The coder thread: compile, debug, refactor               |
| `dongle.c`           | Taking and releasing a dongle (queue, cooldown, scheduler) |
| `monitor.c`          | The monitor thread: burnout and end detection            |
| `heap.c`, `heap_utils.c` | The priority queue used by each dongle               |
| `log.c`              | Serialized output                                        |
| `stop.c`             | Stop flag and waking up blocked coders                   |
| `time.c`             | Time in milliseconds and precise sleeping                |

## Resources

- `man pthread_create`, `man pthread_mutex_lock`, `man pthread_cond_wait`,
  `man pthread_cond_timedwait`, `man gettimeofday`, `man usleep`
- [POSIX Threads Programming (LLNL)](https://hpc-tutorials.llnl.gov/posix/)
- E. W. Dijkstra, the Dining Philosophers problem, which this project is based on
- E. G. Coffman, M. J. Elphick, A. Shoshani, *System Deadlocks* (1971), the four
  conditions for a deadlock
- C. L. Liu, J. W. Layland, *Scheduling Algorithms for Multiprogramming in a
  Hard-Real-Time Environment* (1973), the origin of Earliest Deadline First
- T. H. Cormen et al., *Introduction to Algorithms*, chapter on heaps and
  priority queues

### Use of AI

AI (Claude, by Anthropic) was used during this project:

- to read the subject and plan which structure owns which data,
- to explain concepts (binary heaps, condition variables, Coffman's conditions,
  `usleep`),
- to test the program with AddressSanitizer, Valgrind and Helgrind and to find
  a performance problem in the first dongle ordering.
