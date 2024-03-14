CC=gcc
CFLAGS=-O3 -Wall
ARCH := $(shell uname -m)
ifeq ($(ARCH), x86_64)
ALL= average_avx512 average_avx2 average_sse
all: $(ALL)
average_sse: average_sse.c
	$(CC) $(CFLAGS) average_sse.c -o average_sse

average_avx2: average_avx2.c
	$(CC) -mavx2 $(CFLAGS) average_avx2.c -o average_avx2

average_avx512: average_avx512.c
	$(CC) -mavx512f $(CFLAGS) average_avx512.c -o average_avx512
else ifeq ($(ARCH), aarch64)
	CFLAGS += -march=native
	ALL=average_neon
	all: $(ALL)
	average_neon: average_neon.c
		$(CC) $(CFLAGS) -o average_neon average_neon.c
endif


.PHONY: clean
clean:
	-rm -f $(ALL)