#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#define CMD_FILE "cmd.txt"

pid_t monitor_pid = -1;

void start_monitor(){
    if(monitor_pid > 0){
        printf("Monitor already running with PID %d\n", monitor_pid);
        return;
    }
    pid_t pid = fork();
    if(pid <0) {
        perror("eroare");
        exit(1);
    }
    if(pid == 0){
        execl("./monitor", "./monitor", NULL);
        perror("eroare la executarea monitorului");
        exit(1);
    }else{
        monitor_pid = pid;
        printf("Monitor started with PID %d\n", monitor_pid);
    }
}

void send_command(const char*cmd){
    if(monitor_pid <=0){
        printf("Monitor not running\n");
        return;
    }
    FILE *f = fopen(CMD_FILE, "w");
    if(!f){
        perror("eroare la deschiderea fisierului de comenzi");
        return;
    }

    fprintf(f, "%s\n", cmd);
    fclose(f);
    kill(monitor_pid, SIGUSR1);
}

void stop_monitor(){
    if(monitor_pid <=0){
        printf("Monitor not running\n");
        return;
    }
    send_command("stop");
    waitpid(monitor_pid, NULL, 0);
    monitor_pid = -1;
    printf("Monitor stopped\n");
}

int main(){
    char input[256];

    printf("Treasure Hub - Comenzi: start, stop, exit\n");


    while(1){
        printf(">");
        if(!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = 0; 

        if(strcmp(input, "start_monitor") == 0){
            start_monitor();
        }else if(strcmp(input, "list_hunts") == 0){
            send_command("list_hunts");
        }else if(strncmp(input, "list_treasures", 14) == 0){
            send_command(input);
        }else if(strncmp(input, "view_tresure", 13) == 0){
            send_command(input);
        }else if(strcmp(input, "stop_monitor") == 0){
            stop_monitor();
        }else if(strcmp(input, "exit") == 0){
            if(monitor_pid > 0){
                printf("Stopping monitor...\n");
            }else{
                printf("exiting...\n");
                break;
            }
        
    }else{
            printf("Comanda necunoscuta: %s\n", input);
        }
    }
    return 0;
}
