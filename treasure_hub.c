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
int waiting_for_monitor_exit = 0;


void sigchld_handler(int sig){
    int status;
    pid_t pid = waitpid(-1, &status, WNOHANG);
    if (pid == monitor_pid){
        printf("Monitorul s-a oprit cu codul de iesire %d\n", WEXITSTATUS(status));
        monitor_pid = -1;
        waiting_for_monitor_exit = 0;
    }
}
void start_monitor() {
    if (monitor_pid > 0) {
        printf("Monitorul ruleaza deja cu PID %d\n", monitor_pid);
        return;
    }
    pid_t pid = fork();
    if (pid < 0) {
        perror("Eroare la crearea procesului monitor");
        exit(1);
    }
    if (pid == 0) {
        execl("./monitor", "./monitor", NULL);
        perror("Eroare la executarea monitorului");
        exit(1);
    } else {
        monitor_pid = pid;
        printf("Monitorul a fost pornit cu PID %d\n", monitor_pid);
    }
}

void send_command(const char *cmd) {
    if (monitor_pid <= 0) {
        printf("Monitorul nu ruleaza.\n");
        return;
    }

    if (waiting_for_monitor_exit) {
        printf("Monitorul este în proces de oprire. Așteptati...\n");
        return;
    }

    FILE *f = fopen(CMD_FILE, "w");
    if (!f) {
        perror("Eroare la deschiderea fisierului de comenzi");
        return;
    }

    if(fprintf(f, "%s\n", cmd) < 0) {
        perror("Eroare la scrierea comenzii");
        fclose(f);
        return;
    }

    if(fclose(f) != 0) {
        perror("Eroare la închiderea fisierului de comenzi");
        return;
    }

    if (kill(monitor_pid, SIGUSR1) == -1) {
        perror("Eroare la trimiterea semnalului catre monitor");
        return;
    }
}

void stop_monitor(){
    if(monitor_pid <=0){
        printf("Monitor not running\n");
        return;
    }

    waiting_for_monitor_exit = 1;
    send_command("stop_monitor");

    int status;
    waitpid(monitor_pid, &status, 0);
    monitor_pid = -1;
    waiting_for_monitor_exit = 0;

    printf("Monitor stopped. Exit code: %d\n", WEXITSTATUS(status));
}

void status_monitor() {
    if (monitor_pid > 0) {
        printf("Monitorul ruleaza cu PID %d.\n", monitor_pid);
    } else {
        printf("Monitorul nu ruleaza.\n");
    }
}

int main(){
    char input[256];

    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags= SA_NOCLDSTOP;
    if(sigaction(SIGCHLD, &sa, NULL) == -1){
        perror("eroare la setarea handler-ului pentru SIGCHLD");
        exit(1);
    }

    printf("Treasure Hub- Comenzi valide:\n");
    printf("start_monitor\n");
    printf("list_hunts\n");
    printf("list_treasures <hunt_id>\n");
    printf("view_treasure <hunt_id> <treasure_id>\n");
    printf("stop_monitor\n");
    printf("exit\n");

    
    while(1){
        printf(">");
        if(!fgets(input, sizeof(input), stdin))
             break;
        input[strcspn(input, "\n")] = 0; 


        if(waiting_for_monitor_exit){
            printf("Monitor is stopping. Please wait...\n");
            continue;
        }

        if(strcmp(input, "start_monitor") == 0){
            start_monitor();
        }else if(strcmp(input, "list_hunts") == 0){
            send_command("list_hunts");
        }else if(strncmp(input, "list_treasures", 14) == 0){
            send_command(input);
        }else if(strncmp(input, "view_treasure", 13) == 0){
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
