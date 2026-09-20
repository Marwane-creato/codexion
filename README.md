*This project has been created as part of the 42 curriculum by msmouni.*

# Codexion

## Description

Codexion is a POSIX-thread simulation of coders sharing a circular set of scarce USB dongles. A coder needs its left and right dongles to compile, then debugs and refactors. The program ends when every coder has completed the requested number of full compile-debug-refactor routines, or when the monitor detects a burnout deadline.

Requests are arbitrated by a custom binary heap. `fifo` orders requests by arrival sequence; `edf` orders them by the compile-start deadline, then arrival sequence as the deterministic tie-breaker.

## Instructions

Compile and run:

```sh
make
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug time_to_refactor number_of_compiles_required dongle_cooldown fifo|edf
```

Example:

```sh
./codexion 4 1000 150 100 100 3 20 edf
```

All timing values are positive integer milliseconds; cooldown may be zero. Invalid argument counts, non-integers, negative values, and unknown schedulers are rejected.

## Simple code map

The program is deliberately split into small files, each with one job:

- `main.c`: starts the coder and monitor threads, then waits for them.
- `parse.c`: validates command-line arguments and stores them in the configuration.
- `simulation.c`: creates and destroys shared data, mutexes, and condition variables.
- `worker.c`: the coder loop: request dongles, compile, debug, and refactor.
- `resource.c`: safely reserves and releases the two dongles needed by one coder.
- `heap.c`: keeps waiting requests in FIFO or EDF order.
- `monitor.c`: checks deadlines and stops the simulation on burnout.
- `time.c`: time, interruptible waits, and protected printing helpers.

The main idea is simple: a coder places one request in the queue. The highest-priority request whose pair is currently usable reserves both dongles atomically. Once compilation ends, both dongles are released together. This avoids a coder holding one dongle while waiting for another, while allowing an unavailable pair not to block unrelated ready work.

## Blocking cases handled

- **Deadlock:** acquisition is arbitrated as one atomic request for both dongles, so no coder holds one dongle while waiting for the other. This breaks circular wait and hold-and-wait Coffman conditions.
- **Starvation:** among requests whose pairs are usable, the heap selects the first FIFO request or earliest EDF deadline, with an arrival-order tie-breaker for equal EDF deadlines.
- **Cooldown:** reservation records each dongle’s next usable timestamp; the dongle remains held through compilation and cannot be acquired before that timestamp.
- **Burnout precision:** a dedicated monitor checks deadlines every 0.5 ms and immediately stops the simulation and emits the burnout line.
- **Log serialization:** one output mutex guarantees complete, non-interleaved lines and prevents ordinary state logs after termination.

## Thread synchronization mechanisms

Each `pthread_mutex_t` has a narrow ownership role. Every dongle has its own mutex guarding `busy` and cooldown state. The arbitration mutex protects the heap and makes the two-dongle check-and-reserve transaction atomic. A condition variable wakes waiting requests after releases and termination; timed waits also re-evaluate cooldown deadlines even without a signal.

The state mutex protects stop status, compile starts, completion counts, and the monitor’s deadline scan. For example, a coder updates `last_start` while holding this mutex, and the monitor reads it under the same mutex, avoiding a data race. The output mutex serializes `printf`; state is rechecked while logging so a competing coder cannot print a later state after burnout.

## Resources

- [POSIX Threads](https://pubs.opengroup.org/onlinepubs/9799919799.2024edition/functions/pthread_create.html)
- [POSIX condition variables](https://pubs.opengroup.org/onlinepubs/9799919799/functions/pthread_cond_wait.html)
- [Dining philosophers problem](https://en.wikipedia.org/wiki/Dining_philosophers_problem)

AI was used as a drafting and review aid for project scaffolding, edge-case analysis, and README wording. The implementation was reviewed and tested locally; no external code was copied.
