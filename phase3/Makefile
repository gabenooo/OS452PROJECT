PREFIX = ../..

CC = gcc

CSRCS = $(wildcard *.c)
COBJS = $(CSRCS:.c=.o)

LIBS = -lusloss4.7 -lphase1 -lphase2

LIB_DIR     = ${PREFIX}/lib
INCLUDE_DIR = ${PREFIX}/include

CFLAGS = -Wall -g -I${INCLUDE_DIR} -I.
LDFLAGS = -Wl,--start-group -L${LIB_DIR} -L. ${LIBS} -Wl,--end-group



VPATH = testcases
TESTS = test00 test01 test02 test03 test04 test05 test06 test07 test08 test09 \
        test10               test13 test14 test15 test16 test17 test18 test19 \
        test20 test21 test22 test23 test24 test25 test26 test27



all: ${TESTS}

${TESTS}: phase3_common_testcase_code.o $(COBJS) libphase1.a libphase2.a

ARCH=$(shell uname | tr '[:upper:]' '[:lower:]')-$(shell uname -p | sed -e "s/aarch/arm/g")

phase3_no_debug_symbols-${ARCH}.o: phase3.c
	gcc -I${INCLUDE_DIR} -I. -c phase3.c -o phase3_no_debug_symbols-${ARCH}.o

phase3_usermode_no_debug_symbols-${ARCH}.o: phase3_usermode.c
	gcc -I${INCLUDE_DIR} -I. -c phase3_usermode.c -o phase3_usermode_no_debug_symbols-${ARCH}.o

libphase3-${ARCH}.a: phase3_no_debug_symbols-${ARCH}.o phase3_usermode_no_debug_symbols-${ARCH}.o
	-rm $@
	ar -r $@ $^

clean:
	-rm *.o ${TESTS} term[0-3].out

