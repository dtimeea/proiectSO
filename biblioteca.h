#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>   
#include <sys/types.h>  
#include <errno.h>  
#include <time.h>

typedef struct {
    int treasure_id;    
    char username[50];  
    float latitude; 
    float longitude;   
    char clue[256];     
    int value;      
} Treasure;


void log_operation(char *operation, char *hunt_id){     
    char file_path[70];     
    snprintf(file_path, sizeof(file_path), "%s/logged_hunt.txt", hunt_id);
    

    int log_fd = open(file_path, O_RDWR | O_APPEND | O_CREAT, 0666); 
    if(log_fd <0){
        perror("eroare la deschiderea fisierului de log");
        return;
    }

    time_t now = time(NULL);  
    char time_str[26];   
    ctime_r(&now, time_str);   
    time_str[strlen(time_str) - 1] = '\0';   

    char mesaj_log[256];    
    snprintf(mesaj_log, sizeof(mesaj_log), "[%s] %s: %s\n",time_str, operation, hunt_id);

    write(log_fd, mesaj_log, strlen(mesaj_log));    
    close(log_fd);   

}


void directory_hunt(char *hunt_id){
    if(mkdir(hunt_id, 0777)== -1 && errno != EEXIST){      
        perror("eroare la crearea directorului de vanatoare");     
    }
}




void symbolic_link(char *hunt_id){
    char link_name[70];     
    snprintf(link_name, sizeof(link_name), "logged_hunt-%s", hunt_id);      
    
    char log_file[70];
    snprintf(log_file, sizeof(log_file), "%s/logged_hunt.txt", hunt_id);    
    
    struct stat st;
    if(lstat(link_name, &st) == 0){   
            return;
        }
    
    if(symlink(log_file,link_name) == -1){  
        perror("eroare la crearea linkului simbolic");    
    }
}

void add_treasure(char *hunt_id, Treasure *treasure){
    char path[70];      
    snprintf(path, sizeof(path), "%s/date", hunt_id);   
    
    int fd = open(path, O_RDWR | O_APPEND | O_CREAT, 0666);     
    if(fd < 0){      
        perror("eroare la deschiderea fisierului de comori");
        return;
    }

    ssize_t bytes_written = write(fd, treasure, sizeof(Treasure));   
    if(bytes_written != sizeof(Treasure)){
        perror("eroare la scrierea comorii in fisier");
    }
    close(fd);
    log_operation("add", hunt_id);     
}


void list_treasures(char *hunt_id){
    char path[70];      
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


void view_treasure(char *hunt_id, int treasure_id){
    char path[70];      
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
            treasures = realloc(treasures, sizeof(Treasure) * (count + 1));     
            treasures[count++] = temp;     
        }
    }
    close(fd);

    fd = open(path, O_WRONLY | O_TRUNC);     
    if(fd < 0){   
        perror("eroare la rescierea fisierului de comori");
        free(treasures);
        return;
    }

    for(int i=0; i<count; i++){     
        write(fd, &treasures[i], sizeof(Treasure));     
    }
    close(fd);
    free(treasures);     

    printf("comoara cu id-ul %d a fost stearsa\n", treasure_id);     
    log_operation("remove_treasure", hunt_id);     
}


void remove_hunt(char *hunt_id){
    char command[256];
    snprintf(command, sizeof(command), "rm -rf %s", hunt_id);     

    if(system(command) == -1){     
        perror("eroare la stergerea vanatorii");
        return;
    }
    printf("vanatoarea %s a fost stearsa\n", hunt_id);    
    log_operation("remove_hunt", hunt_id);    
}

void calculate_score( char *hunt_id){
    char path[70];      
    snprintf(path, sizeof(path), "%s/date", hunt_id);   
    
    int fd = open(path, O_RDONLY);     
    if(fd < 0){      
        perror("eroare la deschiderea fisierului de comori");
        return;
    }

    Treasure treasure;     
    int total_score =0;

    while(read(fd, &treasure, sizeof(Treasure)) == sizeof(Treasure)){    
        total_score += treasure.value; 
    }
    close(fd);

    printf("%s : scor total = %d\n", hunt_id, total_score);
}