#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include "biblioteca.h"

#define CMD_FILE "comenzi.txt"

pid_t monitor_pid = -1;
int waiting_for_monitor_exit = 0;

volatile sig_atomic_t command_received = 0; 

void list_hunts(){
    DIR *dir = opendir(".");
    if(!dir){
        perror("eroare la deschiderea directorului");
        return;
    }

    struct dirent *entry;
    while((entry = readdir(dir)) != NULL){
        if(entry->d_type == DT_DIR &&
           strcmp(entry->d_name, ".") != 0 &&
           strcmp(entry->d_name, "..") != 0){
           char path[256];
           snprintf(path, sizeof(path), "%s/date", entry->d_name);
            int fd = open(path, O_RDONLY);
            if(fd < 0){
                printf("%s (0 comori)\n", entry->d_name);
                continue;
            }
            struct stat st;
            fstat(fd, &st);
            int count = st.st_size / sizeof(Treasure);
            printf("%s (%d comori)\n", entry->d_name, count);
            close(fd);
        }
    }
    closedir(dir);
}

void process_command(const char *cmd) {
    char hunt_id[50];
    if (strcmp(cmd, "list_hunts") == 0) {
        printf("Lista de vanatori:\n");
        list_hunts();
    } else if (sscanf(cmd, "list_treasures %s", hunt_id) == 1) {
        printf("Listare comori pentru vanatoarea: %s\n", hunt_id);
        list_treasures(hunt_id);
    } else if (strncmp(cmd, "view_treasure", 13) == 0) {
        int treasure_id;
        if (sscanf(cmd, "view_treasure %s %d", hunt_id, &treasure_id) == 2) {
            printf("Vizualizare comoara %d în vanatoarea: %s\n", treasure_id, hunt_id);
            view_treasure(hunt_id, treasure_id);
        } else {
            printf("Comanda view_treasure invalida.\n");
        }
    } else if (strcmp(cmd, "stop_monitor") == 0) {
        printf("Monitorul se opreste...\n");
        kill(getpid(), SIGTERM); // Folosim SIGTERM pentru oprire
    } else {
        printf("Comanda necunoscuta: %s\n", cmd);
    }
}

void handle_signal(int sig) {
    if (sig == SIGUSR1) {
        command_received = 1; // Semnalizeaza ca o comanda a fost primita
    } else if (sig == SIGTERM) {
        printf("Monitorul se opreste...\n");
        waiting_for_monitor_exit = 1;
        usleep(3000000); // asteapta 3 secunde
        exit(0);
    }
}

void monitor_loop() {
    char command[256];

    while (1) {
        if (command_received) {
            command_received = 0;

            FILE *f = fopen(CMD_FILE, "r");
            if (!f) {
                perror("Eroare la deschiderea fisierului de comenzi");
                continue;
            }

            if (fgets(command, sizeof(command), f)) {
                command[strcspn(command, "\n")] = 0; // Eliminam newline-ul
                process_command(command);
            }
            fclose(f);

            // Golește fisierul dupa procesare
            f = fopen(CMD_FILE, "w");
            if (f) fclose(f);
        }
        pause(); // Asteapta un semnal
    }
}

int main() {
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, NULL); // SIGUSR1 pentru procesarea comenzilor
    sigaction(SIGTERM, &sa, NULL); // SIGTERM pentru oprirea monitorului

    printf("Monitorul a fost pornit. PID: %d\n", getpid());
    monitor_loop();
    return 0;
}