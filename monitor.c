#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "treasure_manager.c"

#define CMD_FILE "cmd.txt"

volatile sig_atomic_t command_received = 0;

void handle_signal(int sig){
    command_received = 1;
}

void process_command(const char *cmd){
    char hunt_id[50];
    if(strcmp(cmd, "list_hunts")==0){
        printf("List of hunts:\n");
    } else if (sscanf(cmd, "list_treasures %s", hunt_id) == 1) {
        printf("Listing treasures for hunt: %s\n", hunt_id);
        list_treasures(hunt_id);
    } else if (strncmp(cmd, "view_treasure", 13) == 0) {
        int treasure_id;
        if (sscanf(cmd, "view_treasure %s %d", hunt_id, &treasure_id) == 2) {
            printf("Viewing treasure %d in hunt: %s\n", treasure_id, hunt_id);
            view_treasure(hunt_id, treasure_id);
        } else {
            printf("Invalid view_treasure command.\n");
        }
    } else if (strcmp(cmd, "stop_monitor") == 0) {
        printf("Monitor stopping...\n");
        exit(0); // Oprește monitorul
    } else {
        printf("Unknown command: %s\n", cmd);
    }
}


void monitor_loop() {
    char command[256];

    while (1) {
        if (command_received) {
            command_received = 0;

            FILE *f = fopen(CMD_FILE, "r");
            if (!f) {
                perror("Failed to open command file");
                continue;
            }

            if (fgets(command, sizeof(command), f)) {
                command[strcspn(command, "\n")] = 0; // Eliminăm newline-ul
                process_command(command);
            }
            fclose(f);
        }
        pause(); // Așteaptă următorul semnal
    }
}


int main() {
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    printf("Monitor started, PID: %d\n", getpid());
    monitor_loop();

    return 0;
}