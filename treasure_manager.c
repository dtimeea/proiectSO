#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>   //structuri de date pentru fisiere
#include <sys/types.h>  //tipuri de date utilizate in sist de fisiere
#include <errno.h>  //pentru codurile de eroare
#include <time.h>

typedef struct {
    int treasure_id;    //id-ul comorii
    char username[50];  //numele utilizatorului
    float latitude;  //latitudine
    float longitude;    //longitudine
    char clue[256];     //indiciu
    int value;      //valoarea comorii
} Treasure;

//functia de logare a operatiunilor care trebuie efectuate
void log_operation(char *operation, char *hunt_id){     //operation-string ,tipul de operatie(add,remove,list) iar hunt_id e id-ul vanatorii
    char file_path[70];     //calea catre fisierul de logare
    snprintf(file_path, sizeof(file_path), "%s/logged_hunt.txt", hunt_id);
    //prin functia snprintf construim calea fisierului de logare

    int log_fd = open(file_path, O_RDWR | O_APPEND | O_CREAT, 0666); //deschidem fisierul in modul de citire si scriere si adaugare la sfarsit
    if(log_fd <0){
        perror("eroare la deschiderea fisierului de log");
        return;
    }
    char mesaj_log[256];    //mesajul de logare
    snprintf(mesaj_log, sizeof(mesaj_log), "%s: %s\n", operation, hunt_id);

    write(log_fd, mesaj_log, strlen(mesaj_log));    //scriem in fisier mesajul de logare
    close(log_fd);   

}

//functie care creaza un director pentru o vanatoare specifica
void directory_hunt(char *hunt_id){
    if(mkdir(hunt_id, 0777)== -1 && errno != EEXIST){      //creaza un director cu permisiuni complexe//'0777'-ofera drepturi de executie citire si scriere pentru user,grup si "grup extern"
        perror("eroare la crearea directorului de vanatoare");     
    }
}

//facem o functie care va stoca datele comorilor pentru o vanatoare
void create_treasure_file(char *hunt_id){
    char path[70];      //declaram un array de caractere numit path care va stoca calea catre fisierul de comori
    snprintf(path, sizeof(path), "%s/date.txt", hunt_id);   
    int fd = open(path, O_RDWR | O_CREAT, 0666);    //0666-permisiunile pentru fisier (citire si scriere pentru utilizator,grup si altii)
    if(fd <0){      //fd-file descriptor
        perror("eroare la crearea fisierlui de comori");
    }
        close(fd);
    
}


//aceasta functie e pentru crearea unui link simbolic care ajuta la accesarea mai rapida a fisierului de logare printr-un nume mai usor de utilizat

void symbolic_link(char *hunt_id){
    char link_name[70];     //declaram un array string care va stoca numele linkului simbolic ce va fi creat
    snprintf(link_name, sizeof(link_name), "logged_hunt-%s", hunt_id);      //cu snprintf construim numele linkului simbolic
    char log_file[70];
    snprintf(log_file, sizeof(log_file), "%s/logged_hunt.txt", hunt_id);    //am incercat sa pun sprintf dar nu mergea pentru ca e prea lung(sizeof)
    if(symlink(log_file,link_name) == -1){  //symlink functie care creaza un link simbolic(primul argument este calea catre fisierul original iar al doilea numele linkului simbolic)
        perror("eroare la crearea linkului simbolic");     //daca crearea linkului simbolic esueaza, afisam eroarea
    }
}

//functie care adauga o comoara in fisierul de comori
void add_treasure(char *hunt_id, Treasure *treasure){
    char path[70];      //declaram un array de caractere numit path care va stoca calea catre fisierul de comori
    snprintf(path, sizeof(path), "%s/date.txt", hunt_id);   //cu snprintf construim calea catre fisierul de comori
    
    int fd = open(path, O_RDWR | O_APPEND | O_CREAT, 0666);     //deschidem fisierul in modul de citire si scriere si adaugare la sfarsit
    if(fd < 0){      //verificam daca fisierul a fost deschis cu succes
        perror("eroare la deschiderea fisierului de comori");
        return;
    }

    char buffer[512];     //declaram un buffer pentru a stoca datele comorii
    int length = snprintf(buffer, sizeof(buffer), "%d %s %.6f %.6f %s %d\n", 
                            treasure->treasure_id, 
                            treasure->username, 
                            treasure->latitude, 
                            treasure->longitude, 
                            treasure->clue, 
                            treasure->value);
    if(write(fd, buffer, length) <0){
        perror("eroare la scrierea in fisierul de comori");    
    }     //scriem in fisier datele comorii
    close(fd);
    log_operation("add", hunt_id);     //apelam functia de logare pentru a inregistra operatiunea de adaugare a comorii
}




//functia principala care gestioneaza argumentele din linia de comanda

int main(int argc, char *argv[]){
    if(argc < 3){  //verificam daca numarul de argumente este mai mic ca 3
        printf("prea putine argumente\n");  //afisam mesajul de eroare
        
        return 1;  //returnam eroare
    }
    char *operation = argv[1];  //primim operatia dorita
    char *hunt_id = argv[2];  //primim id-ul vanatorii
    directory_hunt(hunt_id);  //apelam functia de creare a directorului de vanatoare
    create_treasure_file(hunt_id);  //apelam functia de creare a fisierului de comori

    if(strcmp(operation, "add") == 0){  //verificam daca operatia este "add"
        Treasure treasure;  //declaram o variabila de tip Treasure pentru a stoca datele comorii
        sscanf(argv[3], "%d", &treasure.treasure_id);  //citeste id-ul comorii din argumente
        sscanf(argv[4], "%s", treasure.username);   //citeste numele utilizatorului din argumente
        sscanf(argv[5], "%f", &treasure.latitude);   //citeste latitudinea din argumente
        sscanf(argv[6], "%f", &treasure.longitude);   //citeste longitudinea din argumente
        sscanf(argv[7], "%s", treasure.clue);   //citeste indiciul din argumente
        sscanf(argv[8], "%d", &treasure.value);   //citeste valoarea comorii din argumente
        add_treasure(hunt_id, &treasure);   //apelam functia de adaugare a comorii
    }else{
        printf("Operatie invalida: %s\n", operation); 
        return 1;
    }
    return 0;  
}

