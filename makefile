CC = gcc
CFLAGS = -Wall -Wextra -lm -ggdb -Isrc
DEPDIR = .d
DEPFLAGS = -MT $@ -MMD -MF $(DEPDIR)/$*.d
SOURCEDIR = src
OBJECTDIR = obj
SOURCES = $(wildcard $(SOURCEDIR)/*.c) $(wildcard $(SOURCEDIR)/**/*.c)
HEADERS = $(wildcard $(SOURCEDIR)/*.h) $(wildcard $(SOURCEDIR)/**/*.h)
OBJECTS = $(patsubst $(SOURCEDIR)/%.c, $(OBJECTDIR)/%.o, $(SOURCES))
DEPFILES = $(patsubst $(SOURCEDIR)/%.c, $(DEPDIR)/%.d, $(SOURCES))
TARGET = outfile

$(warning $(SOURCES))
.PHONY: clean all run

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS)

$(OBJECTDIR)/%.o: $(SOURCEDIR)/%.c $(DEPDIR)/%.d
	@mkdir -p $(@D)
	@mkdir -p $(patsubst $(OBJECTDIR)%, $(DEPDIR)%, $(@D))
	$(CC) $(DEPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJECTDIR) $(TARGET) $(DEPDIR)

run: $(TARGET)
	./$(TARGET)

$(DEPDIR)/%.d: ;

-include $(DEPFILES)
