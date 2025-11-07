# SYSC4001 Assignment 2 Part III - API Simulator

## Authors
- Student 1: 101307214
- Student 2: 101306172

## Overview
This project implements an API simulator for FORK and EXEC system calls in C++. The simulator processes trace files and generates detailed execution logs with micro-step timing and system status snapshots.

## Project Structure
```
Assignment_2/
├── interrupts_101307214_101306172.cpp  # Main implementation
├── interrupts_101307214_101306172.hpp  # Header file with data structures
├── build.sh                             # Build script
├── input_files/                         # Input directory
│   ├── trace.txt                        # Main trace file (Scenario 1)
│   ├── trace_scenario2.txt              # Scenario 2 trace
│   ├── trace_scenario3.txt              # Scenario 3 trace
│   ├── vector_table.txt                 # ISR addresses
│   ├── device_table.txt                 # Device ISR durations
│   ├── external_files.txt               # Program sizes
│   ├── program1.txt - program5.txt      # Program traces
├── output_files/                        # Output directory
│   ├── execution.txt                    # Detailed execution log
│   └── system_status.txt                # PCB snapshots
└── bin/                                 # Compiled binary
```

## Building

### Using build.sh (Linux/Git Bash):
```bash
bash build.sh
```

### Manual compilation:
```bash
g++ -std=c++17 -g -O0 -I . -o bin/interrupts interrupts_101307214_101306172.cpp
```

## Running

```bash
./bin/interrupts input_files/trace.txt input_files/vector_table.txt input_files/device_table.txt input_files/external_files.txt
```

### Test Scenarios

**Scenario 1 (Default):**
```bash
./bin/interrupts input_files/trace.txt input_files/vector_table.txt input_files/device_table.txt input_files/external_files.txt
```
Tests: FORK with child EXEC program1, parent EXEC program2

**Scenario 2:**
```bash
./bin/interrupts input_files/trace_scenario2.txt input_files/vector_table.txt input_files/device_table.txt input_files/external_files.txt
```
Tests: Nested FORK (FORK inside program loaded by EXEC)

**Scenario 3:**
```bash
./bin/interrupts input_files/trace_scenario3.txt input_files/vector_table.txt input_files/device_table.txt input_files/external_files.txt
```
Tests: SYSCALL and END_IO within EXEC'd program

## Implementation Details

### Key Features
1. **FORK System Call**
   - Uses vector 2 (memory position 0x0004)
   - Clones parent PCB to create child
   - Child-first execution semantics
   - Parent moves to waiting state

2. **EXEC System Call**
   - Uses vector 3 (memory position 0x0006)
   - 7-step ISR process:
     1. Lookup program metadata
     2. Find free partition
     3. Check program size
     4. Load program (sizeMB * 15ms)
     5. Mark partition occupied
     6. Update PCB
     7. Call scheduler
   - Replaces current process with new program

3. **Memory Management**
   - 6 partitions: 40, 25, 15, 10, 8, 2 MB
   - First-fit allocation from smallest partition
   - Init process (PID 0) uses partition 6 (2MB, 1MB size)

4. **Micro-step Timing**
   - Context save/restore: 10ms
   - Mode switches: 1ms
   - Vector lookup: 1ms
   - PC load: 1ms
   - EXEC loader: sizeMB * 15ms
   - Random substeps: 1-10ms (seed=42 for determinism)

### Output Format

**execution.txt:**
```
START_TIME_MS, DURATION_MS, DESCRIPTION
```

**system_status.txt:**
```
time: X; current trace: <trace_line>
+------------------------------------------------------+
| PID |program name |partition number | size |   state |
+------------------------------------------------------+
| ... rows for running + waiting PCBs ...             |
+------------------------------------------------------+
```

## Design Decisions

### Why the `break` after EXEC?
The `break` statement after EXEC is critical because EXEC replaces the current process's code with a new program. After loading and executing the new program, the simulator should not continue executing the old trace. This models the real behavior of the exec() system call, which replaces the calling process's memory space.

### Child-First Semantics
After a FORK, the child process executes immediately to completion (following IF_CHILD blocks) before the parent resumes. This is implemented through recursive calls to `simulate_trace()` with the child's trace.

### Random Number Generation
Uses a fixed seed (42) with std::mt19937 for deterministic 1-10ms random substeps in FORK/EXEC operations, ensuring reproducible results.

## Requirements
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- Standard C++ library with <random>, <tuple>, <vector>, <string>

## Notes
- Empty lines in input files may generate "Malformed input line" warnings but do not affect execution
- All timing is deterministic due to fixed RNG seed
- Vector positions are calculated as: vectorIndex * 2 bytes (hex format)
