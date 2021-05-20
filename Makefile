CC=gcc
TARGET=myls

all: $(TARGET)

$(TARGET): myls.o confParser.o
	$(CC) $^ $(LOADLIBES) $(LDLIBS) -o $@

clean:
	rm -f  *.o $(TARGET)