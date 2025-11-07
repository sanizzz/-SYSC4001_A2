# SYSC4001 Assignment 2 Part III - API Simulator Report

**Course:** SYSC 4001 - Operating Systems  
**Assignment:** Assignment 2 - Part III (FORK/EXEC API Simulator)  
**Authors:** Student 101307214, Student 101306172  
**Date:** November 06, 2025  
**Repository:** [GitHub](https://github.com/sanizzz/-SYSC4001_A2)

---

## 1. Objective

This report analyzes the execution of an API simulator for FORK and EXEC system calls. The simulator models:
- Vector-based interrupt handling (2 bytes per vector)
- FORK using vector 2 (memory position 0x0004)
- EXEC using vector 3 (memory position 0x0006)
- Child-first execution semantics (no preemption)
- EXEC loader timing of 15 ms per MB

---

## 2. Data Sources

The analysis is based on the following input files:

- `output_files/execution.txt` - Detailed execution log with micro-step timing
- `output_files/system_status.txt` - PCB snapshots after FORK/EXEC operations
- `input_files/trace.txt` - Input trace file
- `input_files/vector_table.txt` - ISR address mappings
- `input_files/device_table.txt` - Device ISR durations
- `input_files/external_files.txt` - Program sizes in MB

---

## 3. Methods

### Parsing
- **execution.txt**: Parsed as CSV (START_TIME_MS, DURATION_MS, DESCRIPTION) and categorized into:
  - CPU bursts, ISR bodies, context operations, mode switches, vector lookups, PC loads, IRET
- **system_status.txt**: Extracted PCB snapshots with time, PID, program, partition, size, and state

### Categorization
Events were classified using regex patterns to identify:
- System calls (SYSCALL, END_IO)
- Process operations (FORK, EXEC)
- Micro-steps (context save/restore, mode switches, vector lookups)
- EXEC loader operations

### Aggregations
Computed total runtime, time per category, event counts, vector verification, and child-first confirmation.

### Assumptions
- Single CPU, no preemption
- Fixed partition sizes: 40, 25, 15, 10, 8, 2 MB
- Init process occupies partition 6 (index 5)
- EXEC loader time = program_size_MB × 15 ms

---

## 4. Results

### 4.1 Execution Timeline

![Timeline Gantt Chart](figures\timeline_gantt.png)

The Gantt chart shows the complete execution timeline with all events color-coded by category.

### 4.2 Event Breakdown

![Event Breakdown](figures\event_breakdown.png)

**Total Runtime:** 1095 ms

**Time Distribution:**
- CPU Time: 75 ms (6.8%)
- SYSCALL ISR: 265 ms
- END_IO ISR: 265 ms
- FORK ISR: 20 ms
- EXEC Loader: 300 ms
- EXEC Substeps: 82 ms
- Context Operations: 70 ms
- Mode Switches: 7 ms
- Vector Lookups: 4 ms
- PC Loads: 4 ms
- IRET: 3 ms

**Overhead:** 93.2%

### 4.3 Vector Position Verification

![Vector Check](figures\vector_check.png)

**Vector Verification:**
- FORK operations use vector 2 (memory position 0x0004): ✓
- EXEC operations use vector 3 (memory position 0x0006): ✓

### 4.4 Partition Occupancy

![Partition Occupancy](figures\partition_occupancy.png)

Shows which partitions are occupied at each system snapshot.

### 4.5 PCB States

![PCB States](figures\pcb_states.png)

Tracks running vs waiting processes across snapshots, demonstrating child-first execution.

### 4.6 EXEC Loader Scaling

![EXEC Loader Scaling](figures\exec_loader_scaling.png)

Verification that EXEC loader duration follows the expected 15 ms/MB scaling.

---

## 5. Key Findings

- **Total Runtime:** 1095 ms
- **CPU vs Overhead:** 6.8% CPU, 93.2% overhead
- **Event Counts:**
  - FORK operations: 1
  - EXEC operations: 1
  - SYSCALL operations: 1
  - END_IO operations: 1
- **Child-First Execution:** ✓ Confirmed
- **Vector Verification:**
  - FORK vector 2 → 0x0004: ✓ (1 occurrences)
  - EXEC vector 3 → 0x0006: ✓ (1 occurrences)
- **EXEC Loader Timing:** Verified at 15 ms/MB

---

## 6. Reproducing This Report

To regenerate this report:

python build_report.pyOptional flags:
- `--dry-run`: Validate inputs without generating output
- `--outdir <path>`: Specify custom output directory (default: report)

---

## 7. Limitations

- **Snapshot Granularity:** System status snapshots are only captured after FORK and EXEC operations, not continuously
- **IF Block Assumptions:** The simulator assumes non-nested IF_CHILD/IF_PARENT blocks in the initial trace
- **Allocation Simplification:** First-fit allocation from smallest partition; no fragmentation handling
- **Single CPU:** No multi-core or parallel execution modeling

---

*Report generated automatically by build_report.py*
