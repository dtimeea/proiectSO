#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#define CMD_FILE "comenzi.txt"

pid_t monitor_pid = -1;// PID-ul procesului monitor(-1 -> nu este pornit)
int waiting_for_monitor_exit = 0;// flag pentru a verifica daca asteptam terminarea monitorului(initializat pe 0 nu se opreste)

//monitor->copilul (treasure_hub->parintele)
// handler pentru SIGCHLD, detecteaza terminarea procesului monitor
void sigchld_handler(int sig){
    int status;
    pid_t pid = waitpid(-1, &status, WNOHANG);//WHNOHANG -> nu blocheaza waitpid asteapta terminarea unui proces copil
    if (pid == monitor_pid){
        printf("Monitorul s-a oprit cu codul de iesire %d\n", WEXITSTATUS(status));
        monitor_pid = -1;
        waiting_for_monitor_exit = 0;
    }
}

// Functia care creaza un proces copil care va rula monitorul
void start_monitor() {
    if (monitor_pid > 0) {
        printf("Monitorul ruleaza deja cu PID %d\n", monitor_pid);
        return;
    }
    pid_t pid = fork();//fork->creaza un proces copil
    if (pid < 0) {
        perror("Eroare la crearea procesului monitor");
        exit(1);
    }
    if (pid == 0) {//daca pid e 0 -> suntem in procesul copil iar execl lanseaza executabilul monitor
        execl("./monitor", "./monitor", NULL);
        perror("Eroare la executarea monitorului");
        exit(1);
    } else {//daca fork e > 0 -> suntem in procesul parinte si afiseaza PID-ul monitorului
        monitor_pid = pid;
        printf("Monitorul a fost pornit cu PID %d\n", monitor_pid);
    }
}

//functie care scrie comanda in fisierul de comenzi
// si trimite semnalul SIGUSR1 catre monitor pentru a-l anunta ca o comanda e disponibila
void send_command(const char *cmd) {
    if (monitor_pid <= 0) {//monitorul nu ruleaza 
        printf("Monitorul nu ruleaza.\n");
        return;
    }

    if (waiting_for_monitor_exit) {
        printf("Monitorul este in proces de oprire. Asteptati...\n");
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
        perror("Eroare la inchiderea fisierului de comenzi");
        return;
    }

    if (kill(monitor_pid, SIGUSR1) == -1) {
        perror("Eroare la trimiterea semnalului catre monitor");
        return;
    }
}

void stop_monitor() {
    if (monitor_pid <= 0) {
        printf("Monitorul nu ruleaza.\n");
        return;
    }

    waiting_for_monitor_exit = 1;//asteptam monitorul sa se opreasca
    send_command("stop_monitor");

    while (waiting_for_monitor_exit) {
        pause(); // Asteapta semnalul SIGCHLD
    }

    printf("Monitorul a fost oprit.\n");
}

//verifica daca monitorul este sau nu pornit prin verificarea valorii monitor_pid
void status_monitor() {
    if (monitor_pid > 0) {
        printf("Monitorul ruleaza cu PID %d.\n", monitor_pid);
    } else {
        printf("Monitorul nu ruleaza.\n");
    }
}

int main(){
    char input[256];

    // Setam handler-ul pentru SIGCHLD
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


    // Bucla principala pentru a citi comenzile de la utilizator
    while(1){
        printf(">");
        if(!fgets(input, sizeof(input), stdin))//fgets citeste o linie de la tastatura si o salveaza in input
             break;
        input[strcspn(input, "\n")] = 0; // eliminam newline-ul de la sfarsitul sirului


        //procesarea comenzilor
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
