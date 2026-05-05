# CSE 320 - TCP Congestion Control Programming Assignment

## Group Members

- Student 1: 20220808071 - Burak Büyükelçi
- Student 2: 20220808074 - Mert Osman Ayhan

## Algorithm Assignment

```
(20220808071 + 20220808074) mod 3 = 0  -->  TCP Tahoe
```

The primary algorithm is **TCP Tahoe**. For comparison purposes,
**TCP Reno** and **TCP NewReno** are also implemented in the same program.

## Files

| File         | Purpose |
|--------------|---------|
| `tcp_sim.c`  | Main, single-file simulator (final deliverable). |
| `step1.c`    | Step 1 - basic skeleton, single counter. |
| `step2.c`    | Step 2 - real RTT-based slow start / congestion avoidance for Tahoe. |
| `step3.c`    | Step 3 - Reno added; side-by-side comparison with Tahoe. |
| `step4.c`    | Step 4 - NewReno added; multi-loss scenario. |
| `step5.c`    | Step 5 - 6-node topology + Dijkstra + routing tables. |
| `step6.c`    | Step 6 - hop-by-hop message forwarding. |
| `README.md`  | This file. |

`tcp_sim.c` contains the same functionality as `step4.c` + `step6.c`
combined, with an interactive menu added.

## How to Compile (Windows + MinGW)

```
gcc tcp_sim.c -o tcp_sim.exe
```

Then run:

```
tcp_sim.exe
```

(On Linux / macOS the same `gcc` command works; the resulting binary
will be `tcp_sim` instead of `tcp_sim.exe`.)

## How to Use the Menu

After launching, the program shows:

```
 1. Show network topology
 2. Show routing table for one node
 3. Show routing tables for ALL nodes
 4. Send a message between two nodes
 5. Run TCP Tahoe (basic scenario)
 6. Run TCP Reno (basic scenario)
 7. Run TCP NewReno (basic scenario)
 8. Compare all three (basic scenario)
 9. Compare all three (multi-loss scenario)
10. Demo: PDF examples (A->D and F->E)
 0. Exit
```

Type a number and press Enter.

## Network Topology

Six nodes (A, B, C, D, E, F) with the link costs that match the
adjacency described in the assignment PDF. Node A's routing table is:

```
Destination | Next Hop | Cost | Path
A           | -        | 0    | A
B           | B        | 4    | A -> B
C           | C        | 7    | A -> C
D           | B        | 12   | A -> B -> D     (NOT direct A->D=13)
E           | B        | 7    | A -> B -> E
F           | F        | 5    | A -> F
```

This matches the table shown on page 3 of the assignment PDF.

## Congestion Control Behavior

Three sender state variables drive the simulation:

- `cwnd`     - congestion window size (in MSS units)
- `ssthresh` - slow-start threshold (initialized to 16)
- `state`    - one of SLOW_START / CONG_AVOIDANCE / FAST_RECOVERY

Per-RTT growth rules:

| State          | cwnd update             |
|----------------|-------------------------|
| SLOW_START     | cwnd doubles each RTT   |
| CONG_AVOIDANCE | cwnd += 1 per RTT       |
| FAST_RECOVERY  | hold (Reno/NewReno only)|

Loss reactions:

| Event         | Tahoe                          | Reno                                   | NewReno                                |
|---------------|--------------------------------|----------------------------------------|----------------------------------------|
| TIMEOUT       | cwnd=1, ssthresh=cwnd/2, SS    | same as Tahoe                          | same as Tahoe                          |
| 3 DUP ACK     | cwnd=1, ssthresh=cwnd/2, SS    | cwnd=ssthresh=cwnd/2, FAST_RECOVERY    | cwnd=ssthresh=cwnd/2, FAST_RECOVERY    |
| Partial ACK   | (panic, like timeout)          | another halving (treats as new loss)   | stay in FR, no shrink (key difference) |

## Demonstration Scenarios

The program ships with two pre-built scenarios:

**Basic scenario (15 RTTs):** at RTT 7 a timeout fires, at RTT 12 a
triple-duplicate-ACK fires. Highlights the difference between Tahoe
(panics on dup-ACK, cwnd to 1) and Reno (halves only).

**Multi-loss scenario (10 RTTs):** at RTT 6 a 3-dup-ACK fires; at RTT 7
a partial ACK indicates a second loss in the same window. Highlights
the difference between Reno (halves twice, ends at cwnd=6) and
NewReno (halves once, ends at cwnd=10).

## Notes on Simulation vs. Real TCP

This is a **single-process simulation**, not a real socket-based TCP.
The 6 nodes are modelled as entries in an adjacency matrix; "messages"
are passed by calling functions, not by sending bytes over a network.

Choosing simulation over real sockets is allowed by the assignment
text ("simulate or implement") and was selected for clarity:
all behaviour is observable in a single terminal, and the cwnd
evolution table is printed step-by-step as required.

The simulator works at **RTT granularity** (one row per RTT), which
means slow-start doubling, congestion-avoidance linear growth, and
fast recovery transitions all appear as discrete events in the trace.
This matches the level of detail typically used in textbook diagrams
of TCP behavior.
