

VPATH = ccheck:src:test
CFLAGS += -fpic -MMD -MF $*.d -g -Wall -fprofile-arcs -ftest-coverage -Isrc -Iccheck
SRCS_CCHECK = $(wildcard ccheck/*.c)
SRCS_PFDS = $(wildcard src/*.c)
SRCS_TEST = $(wildcard test/*.c)


OBJS_CCHECK = $(SRCS_CCHECK:%.c=%.o)
OBJS_PFDS = $(SRCS_PFDS:%.c=%.o)
OBJS_TEST = $(SRCS_TEST:%.c=%.o)
OBJS = $(SRCS:%.c=%.o)
SRCS += $(SRCS_CCHECK) $(SRCS_PFDS) $(SRCS_TEST)
DEPFILES := $(SRCS:%.c=%.d)
HEADERS = pfds.h pfds/pfds-intl.h
HEADERS += $(wildcard pfds/*.h)



.PHONY: all
all: libccheck.so libpfds.so libpfds.a

src/pfds/pfds-intl.h : src/pfds/pfds-intl.h.sh config.inc.sh
	bash $< > $@

config.status :
	@echo "run ./configure first"
	@exit 1

config.inc.sh : config.status configure configure.inc
	bash config.status

$(OBJS_PFDS) : pfds/pfds-intl.h
$(OBJS_TEST) : pfds/pfds-intl.h

.PHONY: distclean
distclean: clean
	rm -f config.status

.PHONY: clean
clean:
	rm -f config.inc.sh
	rm -f src/pfds/pfds-intl.h
	rm -f libccheck.a libccheck.so
	rm -f libpfds.so libpfds.a libccheck.so \
		test_pfds test_splitmix \
		${OBJS_CCHECK} \
		${OBJS_PFDS} \
		${OBJS_TEST} \
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
test_pfds: $(filter-out test/test_splitmix%,$(OBJS_TEST)) libccheck.a libpfds.a
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_splitmix: test/test_splitmix.o ccheck/splitmix.o
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

libpfds.a: ${OBJS_PFDS}
	$(AR) rcs $@ $^

libpfds.so: ${OBJS_PFDS}
	$(CC) -shared -o $@ $^

libccheck.a: ${OBJS_CCHECK}
	$(AR) rcs $@ $^


libccheck.so: ${OBJS_CCHECK}
	$(CC) -shared -o $@ $^

-include ${DEPFILES}

.PHONY: docs
docs: Doxyfile ${HEADERS}
	doxygen Doxyfile

