CC = gcc
CFLAGS = -Wall

TARGET = my_bfm
SRC = my_bfm.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)
