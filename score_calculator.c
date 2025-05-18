#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "biblioteca.h"

int main(int argc, char *argv[]) {
    if (argc != 2){
        fprintf(stderr, "Utilizare: %s <hunt_id>\n", argv[0]);
        exit(1);
    }

    char *hunt_id = argv[1];  
    char path[256];
    snprintf(path, sizeof(path), "%s/date", hunt_id); 

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("eroare la deschiderea fisierului de comori");
        exit(1);
    }

    Treasure t;
    int total_score =0;

    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        total_score += t.value; 
    }

    close(fd);

    printf("%s : scor total = %d\n", hunt_id, total_score);
    return 0;
}