### Network Switch Fabric Simulator (C++)

Simulates a packet-switch (input-queued) router fabric with multiple scheduling algorithms. Useful for learning, experimenting, and benchmarking arbitration policies under different traffic patterns.

## Features

Input-queued switch fabric (e.g., 8×8; configurable)

Per-input FIFO queues; head-of-line contention modeled

Traffic models: uniform, non-uniform (hotspot), bursty

## Pluggable schedulers:

Priority (static/dynamic priority)

WFQ (Weighted Fair Queuing)

Round Robin

iSLIP (iterative SLIP)

Metrics: average delay, per-flow fairness, throughput, queue lengths

Reproducible runs with fixed seeds

## How it Works (High-Level)

Packets arrive at input ports (left). Each input maintains a queue. In each time slot, a scheduler selects input–output matches subject to port conflicts:

Arrival: Traffic generator enqueues packets at inputs (destinations set to outputs).

Scheduling: The chosen algorithm builds a matching between inputs and outputs.

Switching: Matched packets move across the fabric to outputs; unmatched packets wait.

Accounting: We update delays, queue stats, and fairness metrics.

## Scheduling Algorithms

Priority:
Each packet carries a priority. Higher priority packets win contention. Good for latency-sensitive traffic, but can starve low priority if unchecked.

WFQ (Weighted Fair Queuing):
Approximates fair bandwidth sharing by maintaining virtual finish times. Flows with higher weights get proportionally more service, reducing starvation.

Round Robin:
Each output (or input) serves contenders in cyclic order, providing simple fairness and low overhead.

iSLIP:
Iterative matching using per-port round-robin pointers. Converges quickly to a high-quality matching with good throughput in practice.






