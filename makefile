CC = gcc
CFLAGS = -Wall -Wextra -ggdb 
SOURCEDIR = src
OBJECTDIR = obj
SOURCES = $(wildcard $(SOURCEDIR)/*.c)
HEADERS = $(wildcard $(SOURCEDIR)/*.h)
OBJECTS = $(patsubst $(SOURCEDIR)/%.c, $(OBJECTDIR)/%.o,$(SOURCES))
TARGET = outfile

$(warning $(SOURCES))
.PHONY: clean all run

all: $(TARGET)

$(TARGET): $(OBJECTS) $(HEADERS)
	$(CC) -o $@ $^ $(CFLAGS)

$(OBJECTDIR)/%.o: $(SOURCEDIR)/%.c
	@mkdir -p $(@D)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -rf $(OBJECTDIR) $(TARGET)

run: $(TARGET)
	./$(TARGET)
