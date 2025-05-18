CC = gcc
CFLAGS = -Wall

# Main targets
all: monitor treasure_hub score_calculator treasure_manager

# Individual targets
monitor: monitor.c
	$(CC) $(CFLAGS) -o monitor monitor.c

treasure_hub: treasure_hub.c
	$(CC) $(CFLAGS) -o treasure_hub treasure_hub.c

score_calculator: score_calculator.c
	$(CC) $(CFLAGS) -o score_calculator score_calculator.c

treasure_manager: treasure_manager.c
	$(CC) $(CFLAGS) -o treasure_manager treasure_manager.c

# Clean target
clean:
	rm -f monitor treasure_hub score_calculator treasure_manager

# Phony targets
.PHONY: all clean