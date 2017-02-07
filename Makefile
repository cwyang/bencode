#
# Copyright 2017 Chul-Woong Yang (cwyang@gmail.com)
# Licensed under the Apache License, Version 2.0;
# See LICENSE for details.
#
# Makefile               Chul-Woong Yang (cwyang at gmail.com)
#

CC = gcc

GCOV_CCFLAGS = -fprofile-arcs -ftest-coverage
GCOV_OUTPUT = *.gcda *.gcno *.gcov 

CCFLAGS = -Wall -g $(GCOV_CCFLAGS)
LIBNAME = libbencode.a
TARGET = $(LIBNAME)
LIB_CFILES = bencode.c

$(TARGET): bencode.o
	ar r $(LIBNAME) bencode.o
	ranlib $(LIBNAME)

bencode.o: bencode.c bencode.h list.h
	$(CC) $(CCFLAGS) -c bencode.c -o $@

test: bencode_test.c $(TARGET)
	$(CC) $(CCFLAGS)  bencode_test.c -o $@ $(LIBNAME)
	valgrind --leak-check=full --error-exitcode=1 ./test

clean:
	rm -f $(TARGET) *.o test *~
