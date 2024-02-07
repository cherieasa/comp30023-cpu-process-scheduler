// Process Scheduler Algorithm
// Code Written By Terada Asavapakuna (tasavapakuna)
// Student ID: 1012869 
// For COMP30023 - Computer Systems at The University of Melbourne
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

// Define constants
#define FILEPATH 'f'
#define PROCESSORS 'p'
#define OPTIONAL 'c'
#define MAX_PROCESSORS 1024
#define MAX_TIME_ARRIVED pow(2,32)

// Define Process Struct
typedef struct p_node P_node;
struct p_node {
    int process_id;
    int time_arrived;
    int execution_time;
    char parallelisable; // n (not), p (yes)
    int remaining_time;
    float turnaround_time;
    float subprocess;
    P_node *prev;
    P_node *next;
};

// A list containing number of processes and linked lists of processes
typedef struct p_list{
    int p_count;     // p_count is the number of parallelisable process' subprocesses and the non parallelisable processes
    P_node *head;
    P_node *tail;
} P_list;

// Function to insert a process node into our linked list of processes
void insert_p_node_end(P_list *p_list, P_node *p_node);

// Function to delete the head of our linked list of processes
void delete_node(P_list *p_list);

// Function to print our elements in the node
void print_p_list(P_list *p_list);

// Function to sort items in our linked list based on their remaining time
void sort_p_list(P_list *wait_p_list);

// Function to check if there are any arriving processes at a particular time, if so add it to waiting list and return the waiting list.
int check_arriving_process(P_list *p_list, P_list *wait_p_list, int time_elapsed);

// Function to check if the head of the list is the process with the shortest remaining time
int current_shorter(P_list *p_list);

// Function to calculate total remaining time
int calc_total_remain_time(P_list *p_list);

// Function to check if a processor has been added to the list of k smallest remaining time processor indexes
int index_added(int* processor_indexes, int processor_index, int k_processors);

// Function to add the turnaround time to the linked list of all processes
void add_turnaround_time(P_list* p_list, int p_id, int turnaround_time);

// Functions to calculate turnaround time, max and time overhead for each processor
int calc_turnaround_time(P_list* p_list);

float calc_time_overhead(P_list *p_list);

float find_max_time_overhead(P_list *p_list);

// Function which returns 1 - if all the a process' subprocesses are ALL completed and 0 otherwise
int check_all_subprocess_completed(P_list* all_processors[], int NProcessors, int* completed_processes, int processor_id, int find_process_id);

// Function returns k number of processors with the shortest remaining time (breaking ties) as an array of processor IDs
int* k_processors_total_remain_time(P_list *all_processors[], int NProcessors, int k);

// Function which adds the parallelisable process' subprocesses to k CPUs (which we have their ids)
void add_parallelisable_process(P_list* processors[], int* processor_indexes, P_node* process, int RT, int k);

// Function calculates the total number of processes (of non parallelisable processes) combined with all the subprocesses (of parallelisable processes)
int calc_total_proc_subproc(P_list *p_list, int NProcessors);

// Function to schedule and execute processes based on time t and return total executed processes so far at time t (for n processors)
int time_input_one_processor_scheduler(P_list *p_list, P_list* all_processors[], P_list *processor, int* completed_processes, int sim_time, int executed_num, int processor_id, int NProcessors, int *NRemainingProcess);

// Function to run our simulation
void one_processor_scheduler(P_list *p_list, P_list *wait_p_list, int NProcess);

void n_processor_scheduler(P_list *p_list, P_list *wait_p_list, P_list* all_processors[], int NProcessors);

// Main function
int main(int argc, char *argv[]) {

    // Number of processors
    int processor_num = 0;
    // Stores file path to get correct file
    char *filepath;
    // If our optional parameter is present or not
    int optional = 0;

    // Get command line inputs
    for (int i = 1; i < argc; i++){
        
        // Get value of -p
        if (argv[i][1] == PROCESSORS){
            processor_num = atoi(argv[i+1]);
            i++;
        } 
        // Get value of -f
        else if (argv[i][1] == FILEPATH){
            filepath = argv[i+1];
            i++;
        } 
        // Check if optional parameter -c is present
        else if (argv[i][1] == OPTIONAL){
            optional = 1;
        }
    }

    // Open testing file
    FILE *fp;
    fp = fopen(filepath, "r");
    if (fp == NULL){
        perror("fopen()");
        return 1;
    }
    
    // Find the number of processes in file = count newline
    int process_num = 0;
    int c;
    while (!feof(fp)){
        c = fgetc(fp);
        if (c == '\n'){
            process_num++;
        }
    }

    // Resets position of our file pointer
    rewind(fp);

    P_list *all_processes, *wait_processes; // Stores information of all processes & processes waiting to be executed
    all_processes = malloc(sizeof(P_list));
    wait_processes = malloc(sizeof(P_list));

    // Initialise wait_processes
    wait_processes->head = NULL;
    wait_processes->tail = NULL;

    all_processes->p_count = 0;
    wait_processes->p_count = 0;

    int val_ta; // Stores value of time arrived for each process
    int val_id; // Stores value of process id for each process
    int val_et; // Stores value of execution time for each process
    char val_pl; // Stores value of parallelisable for each process
    int val_rt; // Stores value of remaining time left for each 
    int val_tt = 0; // Stores value of turnaround time for each process
    float val_sp = -1; // Stores value of subprocess id for each subprocess (if it is parallelisable)
    int process_index = 0; // Index of all_processes array

    while (fscanf(fp, "%d %d %d %c", &val_ta, &val_id, &val_et, &val_pl) == 4) {
        // Create a new node to store these values
        P_node *tmp = malloc(sizeof(P_node));
        tmp->time_arrived = val_ta;
        tmp->process_id = val_id;
        tmp->execution_time = val_et;
        tmp->parallelisable = val_pl;
        tmp->remaining_time = val_et;
        tmp->turnaround_time = val_tt;
        tmp->subprocess = val_sp;
        insert_p_node_end(all_processes, tmp);
        process_index++;
    }

    // Call Processor Scheduler Simulation
    
    // For one processor scheduler
    if (processor_num == 1){
        one_processor_scheduler(all_processes, wait_processes, all_processes->p_count);
        free(all_processes);
        free(wait_processes);
    }
    // For 2 or N processor scheduler
    if (processor_num > 1){
        P_list* processors[MAX_PROCESSORS];
        // Initialise list of size N of pointers to P_list
        for (int i=0; i < process_num; i++){
            P_list *processor = malloc(sizeof(P_list)*process_num);
            processors[i] = processor;
            processors[i]->p_count = 0;

        }
        n_processor_scheduler(all_processes, wait_processes, processors, processor_num);
        free(all_processes);
        free(wait_processes);
    }

    return 0;
}

// Function to insert a node at the end of a linked list
void insert_p_node_end(P_list *p_list, P_node *p_node){

    // Check if our linked list and node is NULL
    assert(p_list);
    assert(p_node);

    // Copy values into new node
    P_node *tmp = malloc(sizeof(P_node));
    tmp->time_arrived = p_node->time_arrived;
    tmp->process_id = p_node->process_id;
    tmp->execution_time = p_node->execution_time;
    tmp->parallelisable = p_node->parallelisable;
    tmp->remaining_time = p_node->remaining_time;
    tmp->turnaround_time = p_node->turnaround_time;
    tmp->subprocess = p_node->subprocess;

    // New and only data in our linked list
    if(p_list->p_count == 0){
        p_list->head = tmp;
        p_list->tail = tmp;
        p_list->p_count++;
    }   
    else{
        // If not only data -> add to tail
        P_node *tmp2 = p_list->tail;
        p_list->tail->next = tmp;
        tmp->prev = tmp2;
        p_list->tail = tmp;
        p_list->p_count++;
    }

}

// Deletes a node from the beginning of a linked list
void delete_node(P_list *p_list){
    
    if (p_list->p_count == 1){
        p_list->head = NULL;
    }
    else if (p_list->p_count > 1){
        p_list->head = p_list->head->next;
    }

    // Decrement the number of processes
    p_list->p_count--;

}
// Function to display the elements of a linked list - for debugging
void print_p_list(P_list *p_list){
    P_node *curr_node = p_list->head;
    int counter = p_list->p_count;
    if(counter == 0){
        printf("list is empty - CANNOT DISPLAY");
    }
    else{
        while (counter > 0){
            printf("%d", curr_node->process_id);
            curr_node = curr_node->next;
            counter--;
        }
    }
    printf("\n");
}

// Function to check if there are any processes that arrive at a time t, if so add to our waiting linked list.
int check_arriving_process(P_list *p_list, P_list *wait_p_list, int time_elapsed){
    P_node *curr_node = p_list->head;
    int curr_node_count = p_list->p_count;

    int has_arrived = 0;
    if(!p_list->p_count){
        printf("P_LIST IS EMPTY\n");
    }
    else{
        while (curr_node_count > 0){
            if (curr_node->time_arrived == time_elapsed){

                // Create a new node to store these values
                insert_p_node_end(wait_p_list, curr_node);
                has_arrived = 1;
            }
            // Check next node
            curr_node = curr_node->next;
            
            curr_node_count--;
        }
    }
    return has_arrived;
}
// Function to check if the head of the list is the process with the shortest remaining time
int current_shorter(P_list *p_list){

    int curr_rt = p_list->head->remaining_time;
    int curr_id = p_list->head->process_id;
    sort_p_list(p_list);
    int new_rt = p_list->head->remaining_time;
    int new_id = p_list->head->process_id;

    // Current remaining time is shorter
    if (curr_rt < new_rt){
        return 1;
    } 
    // Remaining time is equal, current is shorter if it has a smaller id
    else if (curr_rt == new_rt){
        // If the ids are the same then current is still the process with the shortest remaining time (when its sorted)
        if (curr_id <= new_id){
            return 1;
        }
        else{
            return 0;
        }
    } 
    // Otherwise current remaining time is larger than new process
    else {
        return 0;
    }
}

// Function to run the simulation for shortest remaining time of processes
void one_processor_scheduler(P_list *p_list, P_list *wait_p_list, int NProcess){

    // Time elapsed
    int sim_time = 0;
    // Total number of processes that have been finished
    int executed_num = 0;
    // if a process has arrived = 1, if no processes arrived = 0
    int has_arrived = 0;

    // Turnaround time of each process to be added to list of all processes
    int each_turnaround_time = 0;
    // Average turnaround time
    int turnaround_time = 0;

    // Maximum time overhead 
    float max_time_overhead = 0;
    // Average time overhead
    float time_overhead = 0;

    // Run until all the processes have been finished
    while(executed_num != NProcess){

        // Check if any processes arrive at sim_time - if so add to wait_p_list at that time
        has_arrived = check_arriving_process(p_list, wait_p_list, sim_time);
  
        // Check if a process is currently queued to run at this 
        // If there are no processes executing at this time - and >=1 processes arrive then execute
        if (wait_p_list->p_count == 1 && has_arrived == 1){
            printf("%d,RUNNING,pid=%d,remaining_time=%d,cpu=0\n", sim_time, wait_p_list->head->process_id, wait_p_list->head->remaining_time);
            wait_p_list->head->remaining_time--;

        }
        else{
            // If a process is going to execute and there are no processes waiting to be executed -> just run
            if (wait_p_list->p_count == 1 && has_arrived == 0){
                if (wait_p_list->head->remaining_time == 1){
                    wait_p_list->head->remaining_time--;
                    printf("%d,FINISHED,pid=%d,proc_remaining=%d\n", sim_time, wait_p_list->head->process_id, wait_p_list->p_count-1);
                    
                    // Calculate turnaround time 
                    each_turnaround_time = sim_time - wait_p_list->head->time_arrived;

                    // Add turnaround time value to p_list
                    add_turnaround_time(p_list, wait_p_list->head->process_id, each_turnaround_time);

                    // delete if finished
                    delete_node(wait_p_list);
                    executed_num++;
                }
                else{
                    wait_p_list->head->remaining_time--;
                }
            }
            // Check if it still can run - shortest remaining time compared to other in waiting list
            // or same remaining time but current running one has smaller process id
            else if (current_shorter(wait_p_list)){

                // If the process finishes -> schedule the one in the head in queue next
                if (wait_p_list->head->remaining_time == 1){
                    printf("%d,FINISHED,pid=%d,proc_remaining=%d\n", sim_time, wait_p_list->head->process_id, wait_p_list->p_count-1);
                    executed_num++;

                    // Calculate turnaround time 
                    each_turnaround_time = sim_time - wait_p_list->head->time_arrived;

                    // Add turnaround time value to p_list
                    add_turnaround_time(p_list, wait_p_list->head->process_id, each_turnaround_time);

                    // delete if finished
                    delete_node(wait_p_list);

                    // Schedule next and remove from wait_p_list if there is a process to be run
                    if (wait_p_list->head != NULL){
                        sort_p_list(wait_p_list);
                        printf("%d,RUNNING,pid=%d,remaining_time=%d,cpu=0\n", sim_time, wait_p_list->head->process_id, wait_p_list->head->remaining_time);
                    }
                }
                // If process can run does not finish at time t 
                else{
                    wait_p_list->head->remaining_time--;
                }
            }
            // If the arriving process has shorter remaining time than the executing process 
            // or the same remaining time but the arriving process has smaller process id -> swap execution
            else{
                // printf("waitplist head id %d\n", wait_p_list->head->process_id);
                sort_p_list(wait_p_list);
                // Execute the new process at the head
                printf("%d,RUNNING,pid=%d,remaining_time=%d,cpu=0\n", sim_time, wait_p_list->head->process_id, wait_p_list->head->remaining_time);
                //wait_p_list->head->remaining_time--;
            }
        }
        has_arrived = 0;
        sim_time++;
    }
    // Display statistics
    turnaround_time = calc_turnaround_time(p_list);
    max_time_overhead = find_max_time_overhead(p_list);
    time_overhead = calc_time_overhead(p_list);
    printf("Turnaround time %d\n", turnaround_time);
    printf("Time overhead %g %g\n", roundf(max_time_overhead*100.0)/100.0, roundf(time_overhead*100.0)/100.0);
    printf("Makespan %d\n", sim_time-1);
}

// Function to calculate average turnaround time for a processor
int calc_turnaround_time(P_list* p_list){

    // Turnaround time combined for all processes
    float total_turnaround_time = 0;
    // Average turnaround time
    int turnaround_time = 0;

    P_node* current_node = p_list->head;
    
    // Find each turnaround time - for total to find average
    for (int i = 0; i < p_list->p_count; i++){
        total_turnaround_time+= current_node->turnaround_time;
        current_node = current_node->next;
    }

    // Calculate Turnaround time - average of turnaround times for all processes
    turnaround_time = ceil(total_turnaround_time/(p_list->p_count));
    return turnaround_time;
}

// Function to calculate average time overhead for a given processor
float calc_time_overhead(P_list *p_list){
    
    // Time overhead for each process
    float each_time_overhead = 0;
    // Combined Time overhead for all processes
    float total_time_overhead = 0;
    // Average time overhead
    float time_overhead = 0;

    P_node* current_node = p_list->head;

    // Loop through each process to find turnaround time
    for (int i = 0; i < p_list->p_count; i++){
        each_time_overhead = (current_node->turnaround_time)/(current_node->execution_time);
        total_time_overhead += each_time_overhead;
        current_node= current_node->next;
    }
    time_overhead = total_time_overhead/(p_list->p_count);
    return time_overhead;
}

// Function to calculate maximum time overhead for a given processor
float find_max_time_overhead(P_list *p_list){
    float max_time_overhead = 0;
    float each_time_overhead = 0;
    P_node* current_node = p_list->head;

    for (int i = 0; i < (p_list->p_count); i++){
        each_time_overhead = (current_node->turnaround_time)/(current_node->execution_time);
        // Find maximum time overhead
        if (each_time_overhead > max_time_overhead){
            max_time_overhead = each_time_overhead;
        }
        current_node= current_node->next;
    }
    return max_time_overhead;
}

// Function to iterate through p_list to find the process id we want to add the turnaround time to
void add_turnaround_time(P_list* p_list, int p_id, int turnaround_time){

    P_node *curr_node = p_list->head;
    int counter = p_list->p_count;
    if(counter == 0){
        printf("list is empty\n");
    }
    else{
        while (counter > 0){
            if (curr_node->process_id == p_id){
                // Change value of turnaround time to new turnaround time
                curr_node->turnaround_time = turnaround_time;
            }
            curr_node = curr_node->next;
            counter--;
        }
    }
}

// Function to sort a list of processes (in a processor)
void sort_p_list(P_list *wait_p_list){

    P_node *current, *index;
    P_node *temp = malloc(sizeof(P_node));

    int curr_wait_count = wait_p_list->p_count;
    int index_wait_count = curr_wait_count-1;

    // If there are no items in our linked list
    if (wait_p_list->head == NULL){
        return;
    }
    else if (wait_p_list->p_count > 1){

        // Let current point to head first
        for(curr_wait_count = wait_p_list->p_count; curr_wait_count > 0; curr_wait_count--) {  
            current = wait_p_list->head;
            index = current->next;
            // Let index point to node that is next to current
            for(index_wait_count=curr_wait_count-1; index_wait_count > 0; index_wait_count--) {
                // If Current node has greater remaining time than index -> swap
                // If they both have same remaining time and if current has a larger process_id -> swap
                if(current->remaining_time > index->remaining_time || ((current->remaining_time == index->remaining_time) && (current->process_id > index->process_id))){
                    
                    // Store current data in temp node
                    temp->time_arrived = current->time_arrived;
                    temp->process_id = current->process_id;
                    temp->execution_time = current->execution_time;
                    temp->parallelisable = current->parallelisable;
                    temp->remaining_time = current->remaining_time;
                    temp->subprocess = current->subprocess;

                    // Make current node -> index node
                    current->time_arrived = index->time_arrived;
                    current->process_id = index->process_id;
                    current->execution_time = index->execution_time;
                    current->parallelisable = index->parallelisable;
                    current->remaining_time = index->remaining_time;
                    current->subprocess = index->subprocess;

                    // Index node now current (temp) node
                    index->time_arrived = temp->time_arrived;
                    index->process_id = temp->process_id;
                    index->execution_time = temp->execution_time;
                    index->parallelisable = temp->parallelisable;
                    index->remaining_time = temp->remaining_time;
                    index->subprocess = temp->subprocess;
                }       
                current = index;
                index = current->next;
            }   
        }
    }
}

// Function to calculate the total remaining time of all the processes queued within a processor
int calc_total_remain_time(P_list *p_list){
    
    P_node *curr_node = p_list->head;
    int remaining_time = 0;
    int counter = p_list->p_count;
    if(counter == 0){
        remaining_time = 0;
    }
    else{
        while (counter > 0){
            remaining_time += curr_node->remaining_time;
            curr_node = curr_node->next;
            counter--;
        }
    }
    return remaining_time;
}
// Function returns k number of processors with the shortest remaining time (breaking ties) as an array of processor IDs
int* k_processors_total_remain_time(P_list *all_processors[], int NProcessors, int k){

    int smallest_RT_id = MAX_PROCESSORS;
    double smallest_RT = MAX_TIME_ARRIVED;
    int processor_num = 0;

    // Added keeps track of how many processors are added to the array (of indexes)
    int added = 0;

    // Create an int array (of size k to store processor id/index
    int *processor_indexes = malloc(sizeof(int)*k);
    for (int i=0; i<k; i++){
        // Initialise value of array
        processor_indexes[i]=-1;
    }
    
    while (added != k){

        // Reset smallest value
        smallest_RT_id = MAX_PROCESSORS;
        smallest_RT = MAX_TIME_ARRIVED;
        processor_num = 0;

        while (processor_num < NProcessors){
            // Check if this processor index is already in our array of indexes
            // Proceed if this processor has NOT been added
            
            if (index_added(processor_indexes, processor_num, k) == 0){
                
                // New smallest RT found
                if (calc_total_remain_time(all_processors[processor_num]) < smallest_RT){
                    smallest_RT_id = processor_num;
                    smallest_RT = calc_total_remain_time(all_processors[processor_num]);

                }
                // Break ties if same smallest RT
                else if (calc_total_remain_time(all_processors[processor_num]) == smallest_RT){
                    // Compare which has smaller processor index
                    if (processor_num < smallest_RT_id){
                        smallest_RT_id = processor_num;
                    }
                }
            }
            processor_num++;
        }
        // Add smallest RT id to our array (of indexes)
        processor_indexes[added] = smallest_RT_id;
        added++;

    }
    
    return processor_indexes;
}

// Function which checks if a given processor has already been added to our list of processors
int index_added(int* processor_indexes, int processor_num, int k_processors){

    // Check if its been added (if there is a processor number in the array), if so - return 1
    for (int i = 0; i< k_processors; i++){
        if (processor_indexes[i] == processor_num){
            return 1;
        }
    }
    return 0;
}

// Function which adds the parallelisable process' subprocesses to k CPUs (which we have their ids)
void add_parallelisable_process(P_list* processors[], int* processor_indexes, P_node* process, int RT, int k){

    P_node *tmp = process;
    int processor_id;
    for (int i=0; i < k; i++){
        processor_id = processor_indexes[i];
        tmp->subprocess = (tmp->process_id) + i/10.0;
        tmp->remaining_time = RT;
        // Insert subprocess with its new subprocess id to processor queue
        insert_p_node_end(processors[processor_id], tmp);
    }
}

// Function calculates the total number of processes (of non parallelisable processes) combined with all the subprocesses (of parallelisable processes)
int calc_total_proc_subproc(P_list *p_list, int NProcessors){
    P_node *curr_node = p_list->head;
    int counter = p_list->p_count;
    int total = 0;
    int x;
    int k;
    if(counter == 0){
        total = 0;
    }
    else{
        while (counter > 0){
            // if it is not parallelisable - 1 process
            if (curr_node->parallelisable == 'n'){
                total++;
            }
            else{
                // Calculate k
                x = curr_node->execution_time;
                if (x > NProcessors){
                    k = NProcessors;
                } else{
                    k = x;
                }
                total+=k;
            }
            curr_node = curr_node->next;
            counter--;
        }
    }
    return total;
}

// Function which returns 1 - if all a process' subprocesses are ALL completed and 0 otherwise
int check_all_subprocess_completed(P_list* all_processors[], int NProcessors, int* completed_processes, int processor_id, int find_process_id){

    P_node *curr_node;

    // Check all processors besides i (the one which subprocess has just finished)
    if (completed_processes[find_process_id-1] == 1){
        return 0;
    }
    for (int i = 0; i<NProcessors; i++){

        // Only need to check if its in another processor
        if (i != processor_id){

            curr_node = all_processors[i]->head;

            // Check all the processes in this processor
            for (int j=0; j<all_processors[i]->p_count; j++){

                // Process detected in a queue
                if (curr_node->process_id == find_process_id){
                    if(i < processor_id){
                        return 0;
                    }
                    else if (curr_node->remaining_time > 0){
                        return 0;
                    }
                }
                curr_node = curr_node->next;
            }
        }
    }

    // If process is not detected in a queue
    return 1;
    
}

// Function to schedule and execute processes based on time t and return total executed processes so far at time t
int time_input_one_processor_scheduler(P_list *p_list, P_list* all_processors[], P_list *processor, int* completed_processes, int sim_time, int executed_num, int processor_id, int NProcessors, int *NRemainingProcess){

    // Stores the value of the turnaround time for each process
    int each_turnaround_time = 0;

    // Value to check if all the subprocesses of a process is completed
    int subprocess_complete = 0;
    
    // Execute the process that is at the head of the list
    // If there is process that is being queued to execute -> execute it
    if (processor->p_count > 0){
        
        // Execute head
        // If the process finishes executing at time t - queue next process (if it exists)
        if (processor->head->remaining_time == 0){
            
            if (processor->head->parallelisable == 'n'){
                (*NRemainingProcess)--;
                printf("%d,FINISHED,pid=%d,proc_remaining=%d\n", sim_time, processor->head->process_id, *NRemainingProcess);
                
                // Calculate turnaround time 
                each_turnaround_time = sim_time - processor->head->time_arrived;

                // Add turnaround time value to p_list
                add_turnaround_time(p_list, processor->head->process_id, each_turnaround_time);

            }
            // If parallelisable - check if all other subprocesses are completed
            else{
                // If all subprocesses are completed - return 1
                subprocess_complete = check_all_subprocess_completed(all_processors, NProcessors, completed_processes, processor_id, processor->head->process_id);

                if (subprocess_complete){
                    (*NRemainingProcess)--;
                    printf("%d,FINISHED,pid=%d,proc_remaining=%d\n", sim_time, processor->head->process_id, *NRemainingProcess);
                    //printf("subprocess id: %f\n", processor->head->subprocess);

                    // Add subprocess' process id into completed processes 
                    completed_processes[(processor->head->process_id-1)] = 1;
                    // Calculate turnaround time 
                    each_turnaround_time = sim_time - processor->head->time_arrived;

                    // Add turnaround time value to p_list
                    add_turnaround_time(p_list, processor->head->process_id, each_turnaround_time);
                }
            }
            // delete if finished
            delete_node(processor);
            executed_num++;
            // Check if we need to queue the next process (if it exists)
            if (processor->p_count > 0){

                // Queue next process
                if (processor->head->parallelisable == 'n'){
                    printf("%d,RUNNING,pid=%d,remaining_time=%d,cpu=%d\n", sim_time, processor->head->process_id, processor->head->remaining_time, processor_id);
                }
                else{
                    printf("%d,RUNNING,pid=%.1f,remaining_time=%d,cpu=%d\n", sim_time, processor->head->subprocess, processor->head->remaining_time, processor_id);
                }
                processor->head->remaining_time--;

            }
        }

        // No finishing process, just the one that has just been queued
        else if (processor->head->time_arrived == sim_time){
            if (processor->head->parallelisable == 'n'){
                printf("%d,RUNNING,pid=%d,remaining_time=%d,cpu=%d\n", sim_time, processor->head->process_id, processor->head->remaining_time, processor_id);
            }
            else{
                printf("%d,RUNNING,pid=%.1f,remaining_time=%d,cpu=%d\n", sim_time, processor->head->subprocess, processor->head->remaining_time, processor_id);
            }
            processor->head->remaining_time--;

        }
        // If process does not finish -> just decrement remaining time
        else{
            processor->head->remaining_time--;
        }
    }
    return executed_num;

}

// wait_p_list is the list of processes waiting to be added to a queue of a particular processor
// all_processors contains a list of Processors which each points to P_list (their queue of processes)  
void n_processor_scheduler(P_list *p_list, P_list *wait_p_list, P_list* all_processors[], int NProcessors){

    // Time elapsed
    int sim_time = 0;

    // Number of total completed processes (for n) and subprocess (for p)
    int executed_num = 0;

    // Is 1 if a process arrives at time t
    int has_arrived = 0;

    // Counts how many processes arrives at time t and use to iterate
    int num_arriving_process = 0;

    // Stores the processor (id) with the smallest remaining time and its value
    int smallest_RT_id = MAX_PROCESSORS;
    double smallest_RT = MAX_TIME_ARRIVED;

    // For finding k, the number of processors needed for a parallelisable process and its updated remaining time
    double x = 0;
    int k = 0;
    int reduced_RT = 0;

    // Calculate total number of processes and subprocesses
    int total_proc_subproc = calc_total_proc_subproc(p_list, NProcessors);

    // Total remaining processes
    int total_remain_process = 0;
    int *remain;
    remain = &total_remain_process;

    // Average turnaround time for all CPUs
    int turnaround_time = 0;
    // Max time overhead for all CPUS
    float max_time_overhead = 0;
    // Average Time overhead for every CPU
    float time_overhead = 0;

    int *completed_processes = malloc((p_list->p_count)*sizeof(int));
    for(int i = 0; i<p_list->p_count; i++){
        completed_processes[i] = 0;
    }

    // Run until all the processes (and subprocesses are complete)
    while(executed_num != total_proc_subproc){
        
        // Adds arriving process to wait_p_list
        has_arrived = check_arriving_process(p_list, wait_p_list, sim_time);

        if (has_arrived){

            // Sort the list of arriving processes for shortest remaining/execution time
            sort_p_list(wait_p_list);

            // Check if all arrived processes are parallelisable or not
            num_arriving_process = wait_p_list->p_count;

            while(num_arriving_process > 0){
                
                // If it is not parallelisable
                if (wait_p_list->head->parallelisable == 'n'){
                    
                    // Find shortest total remaining time processor (k = 1)
                    int *shortest_processor = malloc(sizeof(int));
                    shortest_processor = (k_processors_total_remain_time(all_processors, NProcessors, 1));
                    smallest_RT_id = shortest_processor[0];

                    // Insert this process at the queue of the processor
                    insert_p_node_end(all_processors[smallest_RT_id], wait_p_list->head);

                    // Increment remaining process
                    total_remain_process++;

                    // Delete the process at head of wait_p_list
                    delete_node(wait_p_list);
                    
                    free(shortest_processor);
                
                }
                // If the process is parallelisable
                else {

                    // Find k - number of processors to allocate this parallelisable process to
                    x = wait_p_list->head->execution_time;
                    if (x > NProcessors){
                        k = NProcessors;
                    } else{
                        k = x;
                    }

                    // Find k number of processors with the shortest remaining time
                    // return as an array of processor indexes (int)
                    int * k_processors = malloc(sizeof(int)*k);
                    k_processors = k_processors_total_remain_time(all_processors, NProcessors, k);

                    // Update new remaining time
                    reduced_RT = ceil(x/k) + 1;
                    // Add this parallelisable process to the k processors with the updated reduced remaining time
                    add_parallelisable_process(all_processors, k_processors, wait_p_list->head, reduced_RT, k);

                    // Delete the process at head of wait_p_list
                    delete_node(wait_p_list);

                    total_remain_process++;

                }
                num_arriving_process--;
            }
            // First sort the newly arrived processes in each processor
            for (int i = 0; i < NProcessors; i++){
                sort_p_list(all_processors[i]);
            }
        }
        // Calculate the total remaining processes over all processors
        // (taking into account if a process were to finish at this simulation time)

        // Execute the processes that are queued in all_processors
        for (int i = 0; i < NProcessors; i++){
            executed_num = time_input_one_processor_scheduler(p_list, all_processors, all_processors[i], completed_processes, sim_time, executed_num, i, NProcessors, remain);
        }
        has_arrived = 0;
        sim_time++;
    }
    // Calculate and Display Statistics
    turnaround_time = calc_turnaround_time(p_list);
    time_overhead = calc_time_overhead(p_list);
    max_time_overhead = find_max_time_overhead(p_list);

    printf("Turnaround time %d\n", turnaround_time);
    printf("Time overhead %g %g\n", roundf(max_time_overhead*100.0)/100.0, roundf(time_overhead*100.0)/100.0);
    printf("Makespan %d\n", sim_time-1);
}



