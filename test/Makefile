P_NAME		:= testClient
P_SRCS		:= testClient.c $(wildcard ../*.c)
P_INCLUDE	:= ../
CC 			:= gcc
CFLAGS		:= $(addprefix -I,$(P_INCLUDE))
LDFLAGS 	:= -lpthread

.PHONY: all clean
all: $(P_NAME)

clean:
	rm -f testClient

$(P_NAME): $(P_SRCS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) 
