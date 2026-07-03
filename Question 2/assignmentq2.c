/**
 * @file      q2_sensor_sim.c
 * @brief     Producer-Consumer Sensor Buffer Simulation for Online Compilers
 * * @assumption 1: Online compilers like Programiz have execution time limits. 
 * To ensure the reviewer sees the full 10s and 20s cycles, 
 * time is scaled down: 1 simulated second = 200ms of real time.
 * @assumption 2: The environment supports POSIX threads (pthreads), which is 
 * standard for Linux-based online compilers like Programiz.
 * @assumption 3: "Deletes the printed bytes" means extracting the newest 50 bytes 
 * from the tail of the buffer and leaving any older unprinted overflow 
 * data intact at the head of the buffer.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>

/* --- Configuration Macros --- */
#define MAX_BUFFER_CAPACITY 1024
#define PRINT_THRESHOLD     50
#define SIM_TICK_MS         200  // 200ms real time = 1 simulated second
#define TOTAL_SIM_SECONDS   35   // Run simulation for 35 simulated seconds

/* --- Globally Accessible Data Structure --- */
typedef struct {
    uint8_t payload[MAX_BUFFER_CAPACITY];
    int current_size;
    pthread_mutex_t mutex;
} SensorDataBuffer;

// Initialize the global structure
SensorDataBuffer global_sensor = {
    .current_size = 0,
    .mutex = PTHREAD_MUTEX_INITIALIZER
};

/* -----------------------------------------------------------------------------
 * THREAD 1: Sensor Data Producer
 * Triggers every 1 simulated second, generates 0-5 random bytes.
 * ----------------------------------------------------------------------------- */
void* sensor_data_generator(void* arg) {
    for (int sim_time = 1; sim_time <= TOTAL_SIM_SECONDS; sim_time++) {
        // Sleep for 1 simulated second
        usleep(SIM_TICK_MS * 1000);

        // Generate random number of bytes (0 to 5)
        int new_bytes_count = rand() % 6;

        // Lock the shared data structure
        pthread_mutex_lock(&global_sensor.mutex);

        for (int i = 0; i < new_bytes_count && global_sensor.current_size < MAX_BUFFER_CAPACITY; i++) {
            global_sensor.payload[global_sensor.current_size] = (uint8_t)(rand() % 256);
            global_sensor.current_size++;
        }
        
        printf("[Sensor] t=%02ds | Generated %d bytes | Total in buffer: %d\n", 
               sim_time, new_bytes_count, global_sensor.current_size);

        // Unlock the shared data structure
        pthread_mutex_unlock(&global_sensor.mutex);
    }
    return NULL;
}

/* -----------------------------------------------------------------------------
 * THREAD 2: Periodic Data Consumer
 * Wakes up every 10 simulated seconds. Prints and deletes the latest 50 bytes.
 * ----------------------------------------------------------------------------- */
void* periodic_data_processor(void* arg) {
    // Wake up exactly on the 10th, 20th, 30th... simulated second
    for (int sim_time = 10; sim_time <= TOTAL_SIM_SECONDS; sim_time += 10) {
        // Sleep for 10 simulated seconds
        usleep(SIM_TICK_MS * 1000 * 10);

        // Lock the shared data structure
        pthread_mutex_lock(&global_sensor.mutex);
        
        printf("\n======================================================\n");
        printf("[Processor] Woke up at t=%02ds. Buffer size: %d bytes\n", sim_time, global_sensor.current_size);

        if (global_sensor.current_size >= PRINT_THRESHOLD) {
            printf("[Processor] Threshold met. Dumping latest 50 bytes:\n\n");

            // Calculate the starting index for the latest 50 bytes
            int start_idx = global_sensor.current_size - PRINT_THRESHOLD;
            
            // Print the latest 50 bytes in Hex format
            for (int i = start_idx; i < global_sensor.current_size; i++) {
                printf("0x%02X ", global_sensor.payload[i]);
                if ((i - start_idx + 1) % 10 == 0) {
                    printf("\n"); // Newline every 10 bytes for readability
                }
            }
            printf("\n");

            // Delete ONLY the printed bytes.
            // By subtracting the threshold, the newest 50 are mathematically "removed" 
            // while any older bytes (0 to start_idx-1) remain untouched in the buffer.
            global_sensor.current_size -= PRINT_THRESHOLD;
            
            printf("[Processor] Latest 50 bytes deleted. Remaining: %d bytes\n", global_sensor.current_size);
        } else {
            printf("[Processor] Not enough data (< 50 bytes). Going back to sleep.\n");
        }
        printf("======================================================\n\n");

        // Unlock the shared data structure
        pthread_mutex_unlock(&global_sensor.mutex);
    }
    return NULL;
}

/* -----------------------------------------------------------------------------
 * MAIN FUNCTION: Application Entry Point
 * ----------------------------------------------------------------------------- */
int main() {
    // Seed the random number generator
    srand((unsigned int)time(NULL));
    
    printf("Starting RTOS-style Sensor Simulation (Time Scaled 5x faster)...\n\n");

    pthread_t producer_thread, consumer_thread;

    // Create the independent threads
    if (pthread_create(&producer_thread, NULL, sensor_data_generator, NULL) != 0) {
        perror("Failed to create producer thread");
        return 1;
    }
    if (pthread_create(&consumer_thread, NULL, periodic_data_processor, NULL) != 0) {
        perror("Failed to create consumer thread");
        return 1;
    }

    // Wait for the simulation to finish
    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    printf("Simulation Complete.\n");
    return 0;
}