CC = gcc
CFLAGS  = -g 
SRCS =  ../kbhit.c handlers.c com.c list.c
OBJS = $(SRCS:.c=.o)
TARGET = server
LIBS =  -lm -lpthread 
all: $(TARGET)

$(TARGET): $(TARGET).c .FORCE
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c $(SRCS) $(LIBS) 
.PHONY: .FORCE

clean:
	$(RM) $(TARGET) $(OBJS)
