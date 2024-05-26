#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_INSTRUCTIONS 210
#define MAX_INSTRUCTION_LENGTH 100
#define MAX_PROCESS_LENGTH 100
#define MAX_PROCESSES 100
#define MAX_NAME_LENGTH 100
#define MAX_LINE_LENGTH 100 // Adjust as needed

int closest_arrival = 99999999; // Initialize closest arrival time to a high value
int last_arrival = 0; // Store the time of the last arrival
int current_time = 0; // Keep track of the current time

typedef enum {
    SILVER,
    GOLD,
    PLATINUM
} ProcessType;

typedef struct {// Structure defining a process with various attributes
    char name[MAX_NAME_LENGTH]; // Process name
    int priority; // Priority of the process
    int arrival_time; // Time at which the process arrives
    int fixed_arrival; // Fixed arrival time
    char type[10]; // Change type to a char array for strings
    char process_inst[MAX_INSTRUCTIONS][MAX_NAME_LENGTH]; // Array to store instructions
    int process_instruction_times[MAX_INSTRUCTIONS]; // Time for each instruction
    int num_instructions; // Track the number of instructions for each process
    int last_executed ; // Track the last executed instruction
    bool defined; // Added boolean field to indicate if the process is defined
    bool finished; // Indicates if the process has finished execution
    bool arrived; // Indicates if the process has arrived
    int number_quantum; // Number of quantum time units
    int turn_around ; // Turnaround time for the process
    int burst_time ; // Burst time of the process
} Process;


typedef struct {
    char name[MAX_INSTRUCTION_LENGTH];
    int duration;
} Instruction;

typedef struct {
    Process queue_time[MAX_PROCESSES]; // Queue sorted by arrival time in descending order
    int rear_time;
    int rear_priority;
} PriorityQueue;

void enqueue(PriorityQueue *pq, Process process) { // Function to enqueue a process into the priority queue
    int i;
    for (i = pq->rear_time - 1; i >= 0; --i) {    // Enqueue based on arrival time
        // Logic to find the correct position based on arrival time and priority
        if (process.arrival_time == pq->queue_time[i].arrival_time) {
            if (process.priority > pq->queue_time[i].priority) {
                pq->queue_time[i + 1] = pq->queue_time[i];
            } else {
                if (process.priority == pq->queue_time[i].priority && strcmp(process.name, pq->queue_time[i].name) < 0) {
                    pq->queue_time[i + 1] = pq->queue_time[i];
                } else {
                    break;
                }
            }
        } else if (process.arrival_time < pq->queue_time[i].arrival_time) {
            pq->queue_time[i + 1] = pq->queue_time[i];
        } else {
            break;
        }
    }
    pq->queue_time[i + 1] = process; // Insert the process at the correct position
    pq->rear_time++; // Increment the rear pointer
}




int main() {
    Instruction instructions[MAX_INSTRUCTIONS];
    int inst_time[MAX_INSTRUCTIONS]; // Array to store instruction durations
    int num_instructions = 0; // Counter to keep track of the number of instructions

    FILE *instructions_file = fopen("instructions.txt", "r"); // Open the file containing instructions
    if (instructions_file == NULL) {
        perror("Error opening instructions file");
        return EXIT_FAILURE;
    }

    // Read instructions and their durations from the file
    while (fscanf(instructions_file, " %9s %d",
                instructions[num_instructions].name,
                &instructions[num_instructions].duration) == 2 ) {
        inst_time[num_instructions] = instructions[num_instructions].duration; // Store duration in a separate array
        num_instructions++; // Increment instruction count
        if (num_instructions >= MAX_INSTRUCTIONS) {
            break; // Stop if maximum instructions reached
        }
    }
    fclose(instructions_file); // Close the instructions file

    FILE *definition_file = fopen("definition.txt", "r"); // Open the file containing process definitions
    if (definition_file == NULL) {
        perror("Error opening definition file");
        return EXIT_FAILURE;
    }

    Process processes[MAX_PROCESSES];// Array to store processes and their attributes
    int num_processes = 0; // Counter to track the number of processes

    PriorityQueue priority_queue = { .rear_time = 0 }; // Initialize the priority queue with rear_time as 0

    char line[MAX_LINE_LENGTH]; // Buffer to read lines from the file

    while (fgets(line, sizeof(line), definition_file) != NULL) {    // Loop to read lines from the definition file and process each line
        char name[MAX_NAME_LENGTH]; // Variables to store process attributes
        int priority, arrival_time;
        char type[MAX_NAME_LENGTH];

        if (sscanf(line, "%s %d %d %s", name, &priority, &arrival_time, type) == 4) {        // Extracting process name, priority, arrival time, and type from the line
            Process new_process; // Creating a new process structure
            // Assigning extracted values to the new process
            strcpy(new_process.name, name);
            new_process.priority = priority;
            new_process.arrival_time = arrival_time;
            new_process.fixed_arrival = arrival_time;
            strcpy(new_process.type, type);
            new_process.defined = true; // Indicate the process is defined but not yet arrived
            new_process.arrived = false;
            new_process.burst_time = 0; // Initialize burst time to zero

            processes[num_processes] = new_process; // Add the new process to the processes array

            enqueue(&priority_queue, new_process); // Enqueue the new process into the priority queue

            num_processes++; // Increment the count of processed added
            // Update closest and last arrival times if necessary
            if (closest_arrival > arrival_time) {
                closest_arrival = arrival_time;
            }
            if (last_arrival < arrival_time) {
                last_arrival = arrival_time;
            }
            // Optional: Break if the maximum processes limit is reached
            if (num_processes >= MAX_PROCESSES) {
                printf("Exceeded maximum processes limit\n");
                break;
            }
        }
    }
    fclose(definition_file); // Close the definition file

    for (int i = 0; i < num_processes; i++) {    // Loop through all defined processes
        // Skip processes that are not defined
        if (!processes[i].defined) {
            continue;
        }

        char filename[MAX_NAME_LENGTH + 5]; // Variable to store the filename (process name + ".txt")

        // Create filename based on process name
        sprintf(filename, "%s.txt", processes[i].name);

        FILE *process_file = fopen(filename, "r");        // Read content from the process file
        if (process_file == NULL) {
            printf("Error opening %s file\n", filename);
            continue;
        }

        char instruction[MAX_NAME_LENGTH];
        int idx = 0; // Index to track the instruction count

        while (fscanf(process_file, "%s", instruction) == 1) {        // Loop to read instructions from the process file
            strcpy(processes[i].process_inst[idx], instruction);            // Store the instruction in the process structure

            // Update corresponding instructions for the process in the priority queue
            for (int k = 0; k < priority_queue.rear_time; k++) {
                if (strcmp(priority_queue.queue_time[k].name, processes[i].name) == 0) {
                    strcpy(priority_queue.queue_time[k].process_inst[idx], instruction);
                }
            }

            // Find the duration of the instruction from the instructions array
            for (int j = 0; j < MAX_INSTRUCTIONS; j++) {
                if (strcmp(instructions[j].name, instruction) == 0) {
                    processes[i].process_instruction_times[idx] = instructions[j].duration;

                    // Update corresponding instructions for the process in the priority queue
                    for (int k = 0; k < priority_queue.rear_time; k++) {
                        if (strcmp(priority_queue.queue_time[k].name, processes[i].name) == 0) {
                            priority_queue.queue_time[k].process_instruction_times[idx] = instructions[j].duration;
                            priority_queue.queue_time[k].last_executed = -1; // Reset execution status
                            priority_queue.queue_time[k].number_quantum = 0; // Reset quantum count
                            priority_queue.queue_time[k].turn_around = 0; // Reset turnaround time
                            priority_queue.queue_time[k].burst_time += instructions[j].duration; // Update burst time
                        }
                    }
                    idx++; // Move to the next instruction
                    continue;
                }
            }

            // Check if the number of instructions exceeds the limit
            if (idx > MAX_INSTRUCTIONS) {
                printf("Exceeded maximum instructions limit for %s\n", processes[i].name);
                break;
            }
        }

        // Update the number of instructions for the process in the priority queue
        for (int k = 0; k < priority_queue.rear_time; k++) {
            if (strcmp(priority_queue.queue_time[k].name, processes[i].name) == 0) {
                priority_queue.queue_time[k].num_instructions = idx;
            }
        }

        processes[i].num_instructions = idx; // Record the number of instructions for the process
        fclose(process_file); // Close the process file
    }

   // An array to track processes that are not finished
    int notfinished[MAX_PROCESSES];

    // Index to keep track of the current position in the notfinished array
    int index_not_finished = 0;

    // Variable to store the count of processes that are not finished
    int number_not_finished = 0;

    // Initialize the notfinished array, marking all processes as not finished
    for (int i = 0; i < MAX_PROCESSES; i++) {
        notfinished[i] = -1;
    }

    // Variable to store the arrival time of the next process
    int nextarrival = -1;

    // A character array to store the name of the process before context switch
    char pre_switch[MAX_NAME_LENGTH];

    // Copy the name of the first process in the priority queue to pre_switch
    strcpy(pre_switch, priority_queue.queue_time[0].name);

    // Increment the current time by 10 (assuming time units)
    current_time += 10;

    // If the arrival time of the first process in the priority queue is not zero,
    // increment the current time by the arrival time of that process
    if (priority_queue.queue_time[0].arrival_time != 0) {
        current_time += priority_queue.queue_time[0].arrival_time;
    }



    for(int i = 0; i< priority_queue.rear_time;i ++){
        int largestPriorityCount = 1;
        priority_queue.queue_time[i].arrived = true;
        if(priority_queue.queue_time[i].finished){
            continue;
        }
        int temp = -9999;
        char temp_name[MAX_NAME_LENGTH]; 
        int temp_arrival = 9999;

        int platcheck = -1;
        int plat_prio = -1;
        char plat_name[MAX_NAME_LENGTH]; 

        index_not_finished = i;
        for(int a= 0; a<priority_queue.rear_time;a ++){
            if(notfinished[i] !=  -1 && notfinished[a] != -1  ){
                // Compare priorities and update temporary variables accordingly
                if(priority_queue.queue_time[a].priority > temp){
                    temp = priority_queue.queue_time[a].priority;
                    strcpy(temp_name,priority_queue.queue_time[a].name);
                    index_not_finished = a;
                    temp_arrival = priority_queue.queue_time[a].arrival_time;
                }
                else if(priority_queue.queue_time[a].priority == temp ){
                    // Handle ties in priorities by considering arrival time and lexicographical order of names
                    if(priority_queue.queue_time[a].arrival_time < temp_arrival ){ 
                        temp = priority_queue.queue_time[a].priority;
                        strcpy(temp_name,priority_queue.queue_time[a].name);
                        index_not_finished = a;
                        temp_arrival = priority_queue.queue_time[a].arrival_time;
                    }
                    else if(priority_queue.queue_time[a].arrival_time == temp_arrival && strcmp(temp_name,  priority_queue.queue_time[a].name) > 0 ){
                        temp_arrival = priority_queue.queue_time[a].arrival_time;
                        temp = priority_queue.queue_time[a].priority;
                        strcpy(temp_name,priority_queue.queue_time[a].name);
                        index_not_finished = a;
                    }
                }
                // Identify the platinum process with the highest priority
                if(strcmp(priority_queue.queue_time[a].type, "PLATINUM") == 0){
                    if(plat_prio < priority_queue.queue_time[a].priority ){
                        platcheck = a;
                        plat_prio = priority_queue.queue_time[a].priority;
                        strcpy(plat_name,priority_queue.queue_time[a].name);
                    }
                    else if(plat_prio == priority_queue.queue_time[a].priority && (strcmp(plat_name,  priority_queue.queue_time[a].name) > 0)){
                        platcheck = a;
                        plat_prio = priority_queue.queue_time[a].priority;
                        strcpy(plat_name,priority_queue.queue_time[a].name);
                    }
                }
            }
        }
       // Determine the final index for the current iteration based on platinum process check
        if (platcheck == -1) {
            i = index_not_finished;
        } else {
            i = platcheck;
        }

        // Increment current_time by 10 if the process being switched is different from the previous one
        if (strcmp(priority_queue.queue_time[i].name, pre_switch) != 0) {
            current_time += 10;
        }

        // Update notfinished array and count if the current process is not marked as finished
        if (notfinished[i] == -1) {
            notfinished[i] = i;
            number_not_finished += 1;
        }

        // Initialize a count_quantum variable
        int count_quantum = 0;

        for(int j = priority_queue.queue_time[i].last_executed +1; j< priority_queue.queue_time[i].num_instructions;j++){
            current_time += priority_queue.queue_time[i].process_instruction_times[j];
            priority_queue.queue_time[i].last_executed = j;
            count_quantum += priority_queue.queue_time[i].process_instruction_times[j];    // Update count_quantum with the execution time of the current instruction
            
            // Temporary variables for priority comparison
            int temp_priority =  priority_queue.queue_time[i].priority;
            int temp_index = -1;
            int temp_plat = -1;

            // Iterate through all processes to check for arrivals and priorities
            for(int a = 0; a<priority_queue.rear_time;a++){
                if(a != i && current_time >=  priority_queue.queue_time[a].fixed_arrival && !priority_queue.queue_time[a].finished ){ 
                    
                    notfinished[a] = a; // Update notfinished array and count if the process just arrived
                    if(!priority_queue.queue_time[a].arrived){
                        number_not_finished += 1;
                    }
                    priority_queue.queue_time[a].arrived = true;
                    if(temp_priority == priority_queue.queue_time[a].priority){ // Update largestPriorityCount if the current process has the same priority as another
                        largestPriorityCount += 1;
                    }
                    
                    // Update temp_priority, temp_index, and largestPriorityCount based on priorities
                    if(temp_priority < priority_queue.queue_time[a].priority){
                        temp_priority = priority_queue.queue_time[a].priority;
                        temp_index = a;
                        largestPriorityCount = 1; // Reset the count for a new max priority
                    }
                    
                    if(strcmp(priority_queue.queue_time[a].type, "PLATINUM") == 0){// Update temp_plat if the process is of type "PLATINUM"
                        temp_plat = a;
                    }
                }
            }
            if(strcmp(priority_queue.queue_time[i].type, "PLATINUM") == 0){
                largestPriorityCount += 1;
            }
            // Check conditions to update process type and quantum count based on execution time
            // if there is another process with higher priority or platinum,we should end the round robin
            if(strcmp(priority_queue.queue_time[i].type, "PLATINUM") != 0 && (temp_plat != -1 || (temp_index != -1 && priority_queue.queue_time[i].priority < priority_queue.queue_time[temp_index].priority ))){ //j = priority_queue.queue_time[i].num_instructions; //continue
                j = priority_queue.queue_time[i].num_instructions;
                if(strcmp(priority_queue.queue_time[i].type, "SILVER") == 0){
                    priority_queue.queue_time[i].number_quantum += 1;
                    if(priority_queue.queue_time[i].number_quantum == 3){
                        strcpy(priority_queue.queue_time[i].type,"GOLD");
                        priority_queue.queue_time[i].number_quantum = 0;
                    }
                    
                    count_quantum = 0;     

                }
                else if (strcmp(priority_queue.queue_time[i].type, "GOLD") == 0){
                    priority_queue.queue_time[i].number_quantum += 1;
                    if(priority_queue.queue_time[i].number_quantum == 5){
                        strcpy(priority_queue.queue_time[i].type,"PLATINUM");
                        priority_queue.queue_time[i].number_quantum = 0;
                    }
                    count_quantum = 0;  
                }
            }
            else if(strcmp(priority_queue.queue_time[i].type, "PLATINUM") != 0){
                if(count_quantum >= 80 && strcmp(priority_queue.queue_time[i].type, "SILVER") == 0){
                    priority_queue.queue_time[i].number_quantum += 1;
                    if(priority_queue.queue_time[i].number_quantum == 3){
                        strcpy(priority_queue.queue_time[i].type,"GOLD");
                        priority_queue.queue_time[i].number_quantum = 0;
                        
                    }
                    count_quantum = 0;  
                    if(largestPriorityCount > 1){
                        j = priority_queue.queue_time[i].num_instructions; //continue
                    }
                }
                else if (count_quantum >= 120 && strcmp(priority_queue.queue_time[i].type, "GOLD") == 0){
                    priority_queue.queue_time[i].number_quantum += 1;
                    if(priority_queue.queue_time[i].number_quantum == 5){
                        strcpy(priority_queue.queue_time[i].type,"PLATINUM");
                        priority_queue.queue_time[i].number_quantum = 0;
                    }
                    count_quantum = 0;  
                    if(largestPriorityCount > 1){
                        j = priority_queue.queue_time[i].num_instructions; //continue
                    }
                }
            }
        }
        priority_queue.queue_time[i].arrival_time = current_time;
        strcpy(pre_switch, priority_queue.queue_time[i].name);
        if(nextarrival <= priority_queue.queue_time[i+1].arrival_time){nextarrival = priority_queue.queue_time[i+1].arrival_time;}

        if(number_not_finished != 0){
            if(priority_queue.queue_time[i].last_executed == (priority_queue.queue_time[i].num_instructions -1) && notfinished[i] == i){
                notfinished[i] = -1;
                priority_queue.queue_time[i].finished = true;
                priority_queue.queue_time[i].turn_around = current_time -priority_queue.queue_time[i].fixed_arrival;
                number_not_finished -= 1;
                largestPriorityCount -= 1;

            }           
        }
        if(number_not_finished == 0){
            notfinished[i] = -1;
            priority_queue.queue_time[i].finished = true;
            priority_queue.queue_time[i].turn_around = current_time -priority_queue.queue_time[i].fixed_arrival;
            if( current_time < nextarrival){    // If there are more processes to admit and the current time is before nextarrival, update current_time
                current_time = nextarrival ;// exiting and admitting;
                largestPriorityCount = 1;
            } 
        }        
        if(priority_queue.queue_time[i].finished || largestPriorityCount > 1){// Check conditions to reset the loop index for the next iteration
            i = -1;
        }
        
    }
    float avg_turn = 0;
    float avg_wait = 0;
    for(int i = 0; i< num_processes;i++){
        avg_turn += priority_queue.queue_time[i].turn_around; 
        avg_wait += priority_queue.queue_time[i].turn_around-priority_queue.queue_time[i].burst_time;
    }
    avg_turn = avg_turn/num_processes;
    avg_wait = avg_wait/num_processes;
    if ((int)avg_wait == avg_wait) {
        printf("%.0f\n", avg_wait); // Print without decimal and trailing zero
    } else {
        printf("%.1f\n", avg_wait); // Print with one decimal place
    }

    if ((int)avg_turn == avg_turn) {
        printf("%.0f\n", avg_turn); // Print without decimal and trailing zero
    } else {
        printf("%.1f\n", avg_turn); // Print with one decimal place
    }



}
