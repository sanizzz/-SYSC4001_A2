/**
 *
 * @file interrupts.cpp
 * @author Sasisekhar Govind
 *
 */

#include<interrupts_101307214_101306172.hpp>

std::tuple<std::string, std::string, int> simulate_trace(std::vector<std::string> trace_file, int time, std::vector<std::string> vectors, std::vector<int> delays, std::vector<external_file> external_files, PCB current, std::vector<PCB> wait_queue) {

    std::string trace;      //!< string to store single line of trace file
    std::string execution = "";  //!< string to accumulate the execution output
    std::string system_status = "";  //!< string to accumulate the system status output
    int current_time = time;

    //parse each line of the input trace file. 'for' loop to keep track of indices.
    for(size_t i = 0; i < trace_file.size(); i++) {
        auto trace = trace_file[i];

        std::string activity, program_name;
        int duration_intr;
        std::tie(activity, duration_intr, program_name) = parse_trace(trace);

        if(activity == "CPU") { //As per Assignment 1
            execution += std::to_string(current_time) + ", " + std::to_string(duration_intr) + ", CPU Burst\n";
            current_time += duration_intr;
        } else if(activity == "SYSCALL") { //As per Assignment 1
            std::string intr;
            int time;
            std::tie(intr, time) = intr_boilerplate(current_time, duration_intr, 10, vectors);
            execution += intr;
            current_time = time;

            execution += std::to_string(current_time) + ", " + std::to_string(delays[duration_intr]) + ", SYSCALL ISR body\n";
            current_time += delays[duration_intr];

            execution +=  std::to_string(current_time) + ", 1, IRET\n";
            current_time += 1;
            
            execution += std::to_string(current_time) + ", 1, switch to user mode\n";
            current_time += 1;
            
            execution += std::to_string(current_time) + ", 10, context restored\n";
            current_time += 10;
        } else if(activity == "END_IO") {
            std::string intr;
            int time;
            std::tie(intr, time) = intr_boilerplate(current_time, duration_intr, 10, vectors);
            current_time = time;
            execution += intr;

            execution += std::to_string(current_time) + ", " + std::to_string(delays[duration_intr]) + ", END_IO ISR body\n";
            current_time += delays[duration_intr];

            execution +=  std::to_string(current_time) + ", 1, IRET\n";
            current_time += 1;
            
            execution += std::to_string(current_time) + ", 1, switch to user mode\n";
            current_time += 1;
            
            execution += std::to_string(current_time) + ", 10, context restored\n";
            current_time += 10;
        } else if(activity == "FORK") {
            std::string intr;
            int time;
            std::tie(intr, time) = intr_boilerplate(current_time, 2, 10, vectors);
            execution += intr;
            current_time = time;

            ///////////////////////////////////////////////////////////////////////////////////////////
            //Add your FORK output here
            
            // FORK ISR body: clone parent PCB to child PCB
            int fork_duration = duration_intr;  // Use duration from trace
            execution += std::to_string(current_time) + ", " + std::to_string(fork_duration) + ", clone parent PCB to child PCB\n";
            current_time += fork_duration;
            
            // Create child PCB with new PID
            unsigned int child_pid = next_pid++;
            PCB child(child_pid, current.PID, current.program_name, current.size, current.partition_number, "running");
            
            // Update parent state to waiting and add to wait queue
            current.state = "waiting";
            wait_queue.push_back(current);
            
            // Log scheduler called (0 ms)
            execution += std::to_string(current_time) + ", 0, scheduler called\n";
            
            // Generate system_status snapshot
            system_status += "time: " + std::to_string(current_time) + "; current trace: " + trace + "\n";
            system_status += print_PCB(child, wait_queue);
            system_status += "\n";

            ///////////////////////////////////////////////////////////////////////////////////////////

            //The following loop helps you do 2 things:
            // * Collect the trace of the chile (and only the child, skip parent)
            // * Get the index of where the parent is supposed to start executing from
            std::vector<std::string> child_trace;
            bool skip = true;
            bool exec_flag = false;
            int parent_index = 0;

            for(size_t j = i; j < trace_file.size(); j++) {
                std::string _activity, _pn;
                int _duration;
                std::tie(_activity, _duration, _pn) = parse_trace(trace_file[j]);
                if(skip && _activity == "IF_CHILD") {
                    skip = false;
                    continue;
                } else if(_activity == "IF_PARENT"){
                    skip = true;
                    parent_index = j;
                    if(exec_flag) {
                        break;
                    }
                } else if(skip && _activity == "ENDIF") {
                    skip = false;
                    continue;
                } else if(!skip && _activity == "EXEC") {
                    skip = true;
                    child_trace.push_back(trace_file[j]);
                    exec_flag = true;
                }

                if(!skip) {
                    child_trace.push_back(trace_file[j]);
                }
            }
            i = parent_index;

            ///////////////////////////////////////////////////////////////////////////////////////////
            //With the child's trace, run the child (HINT: think recursion)
            
            // Run child process first (child-first semantics)
            std::string child_execution, child_system_status;
            int child_end_time;
            std::tie(child_execution, child_system_status, child_end_time) = simulate_trace(
                child_trace, current_time, vectors, delays, external_files, child, std::vector<PCB>()
            );
            execution += child_execution;
            system_status += child_system_status;
            current_time = child_end_time;
            
            // After child completes, restore parent from wait queue and continue
            // Remove parent from wait queue (it's the last one we added)
            if (!wait_queue.empty()) {
                current = wait_queue.back();
                wait_queue.pop_back();
                current.state = "running";
            }

            ///////////////////////////////////////////////////////////////////////////////////////////


        } else if(activity == "EXEC") {
            std::string intr;
            int time;
            std::tie(intr, time) = intr_boilerplate(current_time, 3, 10, vectors);
            current_time = time;
            execution += intr;

            ///////////////////////////////////////////////////////////////////////////////////////////
            //Add your EXEC output here
            
            // Step 1: Lookup program metadata
            int lookup_time = random_1_to_10();
            execution += std::to_string(current_time) + ", " + std::to_string(lookup_time) + ", lookup program metadata\n";
            current_time += lookup_time;
            
            unsigned int program_size = get_size(program_name, external_files);
            
            // Step 2: Find free partition that fits
            int find_partition_time = random_1_to_10();
            execution += std::to_string(current_time) + ", " + std::to_string(find_partition_time) + ", find free partition that fits\n";
            current_time += find_partition_time;
            
            // Step 3: Program is X Mb large (use duration from trace)
            execution += std::to_string(current_time) + ", " + std::to_string(duration_intr) + ", Program is " + std::to_string(program_size) + " Mb large\n";
            current_time += duration_intr;
            
            // Step 4: Load program into memory (sizeMB * 15 ms)
            int loader_time = program_size * 15;
            execution += std::to_string(current_time) + ", " + std::to_string(loader_time) + ", loading program into memory\n";
            current_time += loader_time;
            
            // Step 5: Mark partition occupied (done by allocate_memory)
            // Free old memory first if current process has one
            if (current.partition_number != -1) {
                free_memory(&current);
            }
            
            // Update PCB with new program info
            current.program_name = program_name;
            current.size = program_size;
            
            // Allocate memory for the new program
            if (!allocate_memory(&current)) {
                std::cerr << "ERROR! Memory allocation failed for " << program_name << std::endl;
            }
            
            // Step 6: Update PCB
            int update_pcb_time = random_1_to_10();
            execution += std::to_string(current_time) + ", " + std::to_string(update_pcb_time) + ", update PCB\n";
            current_time += update_pcb_time;
            
            // Step 7: Scheduler called (0 ms)
            execution += std::to_string(current_time) + ", 0, scheduler called\n";
            
            // Add epilogue (IRET, switch to user mode, context restored)
            execution += std::to_string(current_time) + ", 1, IRET\n";
            current_time += 1;
            
            execution += std::to_string(current_time) + ", 1, switch to user mode\n";
            current_time += 1;
            
            execution += std::to_string(current_time) + ", 10, context restored\n";
            current_time += 10;
            
            // Generate system_status snapshot
            system_status += "time: " + std::to_string(current_time) + "; current trace: " + trace + "\n";
            system_status += print_PCB(current, wait_queue);
            system_status += "\n";

            ///////////////////////////////////////////////////////////////////////////////////////////


            std::ifstream exec_trace_file("input_files/" + program_name + ".txt");

            std::vector<std::string> exec_traces;
            std::string exec_trace;
            while(std::getline(exec_trace_file, exec_trace)) {
                exec_traces.push_back(exec_trace);
            }

            ///////////////////////////////////////////////////////////////////////////////////////////
            //With the exec's trace (i.e. trace of external program), run the exec (HINT: think recursion)
            
            // Run the loaded program's trace
            std::string exec_execution, exec_system_status;
            int exec_end_time;
            std::tie(exec_execution, exec_system_status, exec_end_time) = simulate_trace(
                exec_traces, current_time, vectors, delays, external_files, current, wait_queue
            );
            execution += exec_execution;
            system_status += exec_system_status;
            current_time = exec_end_time;

            ///////////////////////////////////////////////////////////////////////////////////////////

            break; //Why is this important? (answer in report)
            // Answer: The break prevents continuing with the rest of the parent trace after EXEC.
            // EXEC replaces the current process's code, so execution should not continue from
            // the old trace after loading and running the new program.

        }
    }

    return {execution, system_status, current_time};
}

int main(int argc, char** argv) {

    //vectors is a C++ std::vector of strings that contain the address of the ISR
    //delays  is a C++ std::vector of ints that contain the delays of each device
    //the index of these elemens is the device number, starting from 0
    //external_files is a C++ std::vector of the struct 'external_file'. Check the struct in 
    //interrupt.hpp to know more.
    std::vector<std::string> vectors;
    std::vector<int> delays;
    std::vector<external_file> external_files;
    std::tie(vectors, delays, external_files) = parse_args(argc, argv);
    std::ifstream input_file(argv[1]);

    //Just a sanity check to know what files you have
    print_external_files(external_files);

    //Make initial PCB (notice how partition is not assigned yet)
    PCB current(0, -1, "init", 1, -1);
    //Update memory (partition is assigned here, you must implement this function)
    if(!allocate_memory(&current)) {
        std::cerr << "ERROR! Memory allocation failed!" << std::endl;
    }

    std::vector<PCB> wait_queue;

    /******************ADD YOUR VARIABLES HERE*************************/


    /******************************************************************/

    //Converting the trace file into a vector of strings.
    std::vector<std::string> trace_file;
    std::string trace;
    while(std::getline(input_file, trace)) {
        trace_file.push_back(trace);
    }

    std::string execution, system_status;
    int final_time;
    std::tie(execution, system_status, final_time) = simulate_trace(   trace_file, 
                                            0, 
                                            vectors, 
                                            delays,
                                            external_files, 
                                            current, 
                                            wait_queue);

    input_file.close();

    write_output(execution, "output_files/execution.txt");
    write_output(system_status, "output_files/system_status.txt");

    return 0;
}
