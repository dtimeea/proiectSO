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

//logheaza operatiile efectuate (add, remove, view etc)
void log_operation(char *operation, char *hunt_id){     //operation-string ,tipul de operatie(add,remove,list) 
    char file_path[70];     //calea catre fisierul de logare
    snprintf(file_path, sizeof(file_path), "%s/logged_hunt.txt", hunt_id);
    //prin functia snprintf construim calea fisierului de logare

    int log_fd = open(file_path, O_RDWR | O_APPEND | O_CREAT, 0666); //deschidem fisierul in modul de citire si scriere si adaugare la sfarsit
    if(log_fd <0){
        perror("eroare la deschiderea fisierului de log");
        return;
    }

    time_t now = time(NULL);  
    char time_str[26];   //buffer pentru timp in forma de string
    ctime_r(&now, time_str);   //converteste timpul intr-un string
    time_str[strlen(time_str) - 1] = '\0';   //eliminam '\n' de la sfarsitul sirului

    char mesaj_log[256];    //mesajul de logare
    snprintf(mesaj_log, sizeof(mesaj_log), "[%s] %s: %s\n",time_str, operation, hunt_id);

    write(log_fd, mesaj_log, strlen(mesaj_log));    //scriem in fisier mesajul de logare
    close(log_fd);   

}

//functie care creeaza un director pentru vanatoare
void directory_hunt(char *hunt_id){
    if(mkdir(hunt_id, 0777)== -1 && errno != EEXIST){      //creaza un director cu permisiuni complexe//'0777'-ofera drepturi de executie citire si scriere pentru user,grup si "grup extern"
        perror("eroare la crearea directorului de vanatoare");     
    }
}


//aceasta functie e pentru crearea unui link simbolic care ajuta la accesarea mai rapida a fisierului de logare printr-un nume mai usor de utilizat

void symbolic_link(char *hunt_id){
    char link_name[70];     //declaram un array
    snprintf(link_name, sizeof(link_name), "logged_hunt-%s", hunt_id);      //cu snprintf construim numele linkului simbolic
    
    char log_file[70];
    snprintf(log_file, sizeof(log_file), "%s/logged_hunt.txt", hunt_id);    
    
    struct stat st;
    if(lstat(link_name, &st) == 0){   
            return;
        }
    
    if(symlink(log_file,link_name) == -1){  //symlink functie care creaza un link simbolic(primul argument este calea catre fisierul original iar al doilea numele linkului simbolic)
        perror("eroare la crearea linkului simbolic");    
    }
}

//functie care adauga o comoara in fisier
void add_treasure(char *hunt_id, Treasure *treasure){
    char path[70];      //declaram un array 
    snprintf(path, sizeof(path), "%s/date", hunt_id);   //cu snprintf construim calea catre fisierul de comori
    
    int fd = open(path, O_RDWR | O_APPEND | O_CREAT, 0666);     
    if(fd < 0){      
        perror("eroare la deschiderea fisierului de comori");
        return;
    }

    ssize_t bytes_written = write(fd, treasure, sizeof(Treasure));   //scriem in fisier datele comorii
    if(bytes_written != sizeof(Treasure)){
        perror("eroare la scrierea comorii in fisier");
    }
    close(fd);
    log_operation("add", hunt_id);     //inregistram operatiunea de adaugare a comorii in fisierul de log
}

//listeaza toate comorile
void list_treasures(char *hunt_id){
    char path[70];      //declaram un array 
    snprintf(path, sizeof(path), "%s/date", hunt_id);   //
    
    int fd = open(path, O_RDONLY);     
    if(fd < 0){      
        perror("eroare la deschiderea fisierului de comori");
        return;
    }

    Treasure treasure;     
    while(read(fd, &treasure, sizeof(Treasure)) > 0){     
        printf("ID: %d, User: %s, Latitude: %.6f, Longitude: %.6f, Clue: %s, Value: %d\n", 
            treasure.treasure_id, treasure.username, treasure.latitude, 
            treasure.longitude, treasure.clue, treasure.value);
    }
    close(fd);
}

//afiseaza detalii despre o comoara dupa ID
void view_treasure(char *hunt_id, int treasure_id){
    char path[70];      //declaram un array
    snprintf(path, sizeof(path), "%s/date", hunt_id);   
    
    int fd = open(path, O_RDONLY);     
    if(fd < 0){      
        perror("eroare la deschiderea fisierului de comori");
        return;
    }

    Treasure treasure;     
    int found=0;

    while(read(fd, &treasure, sizeof(Treasure)) == sizeof(Treasure)){    
        if(treasure.treasure_id == treasure_id){     
            printf("ID: %d\nUser: %s\nLatitude: %.6f\nLongitude: %.6f\nClue: %s\nValue: %d\n", 
                treasure.treasure_id, treasure.username, treasure.latitude, 
                treasure.longitude, treasure.clue, treasure.value);
            found=1;
            break;
        }
    }
    close(fd);

    if(!found){    
        printf("comoara nu a fost gasita\n");
    }
    log_operation("view", hunt_id);  
}


//sterge o comoara dupa ID
void remove_treasure(char *hunt_id, int treasure_id){
    char path[70];      
    snprintf(path, sizeof(path), "%s/date", hunt_id);   
    
    int fd = open(path, O_RDONLY);     
    if(fd < 0){      
        perror("eroare la deschiderea fisierului de comori");
        return;
    }

    Treasure *treasures = NULL;    
    int count=0;
    Treasure temp;

    while(read(fd, &temp, sizeof(Treasure)) == sizeof(Treasure)){    
        if(temp.treasure_id != treasure_id){     
            treasures = realloc(treasures, sizeof(Treasure) * (count + 1));     //realloc pentru a redimensiona memoria alocata
            treasures[count++] = temp;     //adaugam comoara in array
        }
    }
    close(fd);

    fd = open(path, O_WRONLY | O_TRUNC);     //scriere si golire
    if(fd < 0){   
        perror("eroare la rescierea fisierului de comori");
        free(treasures);
        return;
    }

    for(int i=0; i<count; i++){     //parcurgem array
        write(fd, &treasures[i], sizeof(Treasure));     //scriem in fisier datele comorii
    }
    close(fd);
    free(treasures);     //eliberam memoria alocata

    printf("comoara cu id-ul %d a fost stearsa\n", treasure_id);     
    log_operation("remove_treasure", hunt_id);     
}

//sterge intreaga vanatoare
void remove_hunt(char *hunt_id){
    char command[256];
    snprintf(command, sizeof(command), "rm -rf %s", hunt_id);     //comanda pentru stergerea directorului de vanatoare

    if(system(command) == -1){     //apelam comanda de sistem pentru a sterge directorul
        perror("eroare la stergerea vanatorii");
        return;
    }
    printf("vanatoarea %s a fost stearsa\n", hunt_id);    
    log_operation("remove_hunt", hunt_id);    
}
