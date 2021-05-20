PROGS = listfiles
all:	$(PROGS)

%:	%.c $(LIBAPUE)
	gcc $(CFLAGS) $@.c -o $@ $(LDFLAGS) $(LDLIBS)

clean:
	rm -f $(PROGS) $(TEMPFILES) *.o