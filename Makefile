
CFLAGS += -fpic -MMD -MF $*.d -g -Wall -fprofile-arcs -ftest-coverage
SRCS = $(wildcard *.c)
OBJS = $(filter-out test_%,$(SRCS:%.c=%.o))
DEPFILES := $(SRCS:%.c=%.d)

.PHONY: all
all: libpfds.so libpfds.a

pfds/pfds-intl.h : pfds/pfds-intl.h.sh config.inc.sh
	bash $< > $@

config.status :
	@echo "run ./configure first"
	@exit 1

config.inc.sh : config.status
	bash config.status

$(OBJS) : pfds/pfds-intl.h

.PHONY: distclean
distclean: clean
	rm -f config.status

.PHONY: clean
clean:
	rm -f config.inc.sh
	rm -f libpfds.so libpfds.a \
		test_pfds test_pfds.o test_pfds.d \
		test_splitmix test_splitmix.o test_splitmix.d \
		${OBJS} \
		${DEPFILES} \
		${SRCS:%.c=%.c.gcov} \
		${SRCS:%.c=%.gcda} \
		${SRCS:%.c=%.gcno} \
		$(wildcard *.h.gcov)
	rm -rf ./builddocs

.PHONY: install
install: libpfds.so libpfds.o
	mkdir -p $(DESTDIR)/lib
	mkdir -p $(DESTDIR)/include

test_pfds: LDFLAGS += -lcunit -lffi -lm
test_pfds: test_pfds.o libpfds.a
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_splitmix: test_splitmix.o splitmix.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: check
check: test_pfds test_splitmix
	./test_splitmix
	./test_pfds -v
	gcov -r ${SRCS} || echo $$?
	grep '#####' *.gcov | wc

.PHONY: bench
bench: test_pfds
	./test_pfds -v -p '^bench/' --benchmarks=bench.csv -i 3 -n 100

.PHONY: coverage
coverage: test_pfds test_splitmix
	gcov -r ${SRCS} || echo $$?
	grep '#####' *.gcov | wc

libpfds.a: ${OBJS}
	$(AR) rcs $@ $^

libpfds.so: ${OBJS}
	$(CC) -shared -o $@ $^

-include ${DEPFILES}

.PHONY: docs
docs: Doxyfile pfds.h
	doxygen Doxyfile

