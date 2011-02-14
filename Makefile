VER = 0.1
CC = gcc
CFLAGS = -O2 `pkg-config --cflags gtk+-2.0`
LDFLAGS = `pkg-config --libs gtk+-2.0`
OBJS = gtk_lcd.o test.o
BIN = test

all: $(BIN)

$(BIN): $(OBJS)
	@echo \\t[LD] $@
	@$(CC) $(LDFLAGS) -o $(BIN) $(OBJS)

$(OBJS): %.o: %.c
	@echo \\t[CC] $@
	@$(CC) -c $(CFLAGS) $< -o $@

.PHONY: dist
dist:
	@echo \\t[TAR] gtk_lcd-$(VER).tar.bz2
	@tar -C .. -cjf gtk_lcd-$(VER).tar.bz2 `cd ..; ls gtk_lcd/*.[ch]` gtk_lcd/Makefile

.PHONY: clean
clean:
	@echo \\t[RM] $(BIN) $(OBJS)
	@rm $(BIN) $(OBJS)

