bin_PROGRAMS = igloo

AM_CFLAGS = -Wall -Wextra -O3

igloo_SOURCES = src/igloo.c \
				src/igloo_internal.c \
				src/igloo_parser.c \
				src/igloo_befuddle.c \
				src/igloo_random.c
igloo_LDADD = -L/usr/local/lib -lm -lwolfssl
