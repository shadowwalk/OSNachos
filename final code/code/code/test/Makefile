# use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
#    Nachos assumes that the location of the program startup routine (the
# 	location the kernel jumps to when the program initially starts up)
#       is at location 0.  This means: start.o must be the first .o passed 
# 	to ld, in order for the routine "Start" to be loaded at location 0
#

# If you are running on SunOS 5.x machines, uncomment the next line
GCCDIR = /auto/home-scf-22/csci402/testgnu/decstation-ultrix/bin/
LDFLAGS = -T newscript -N
ASFLAGS = -mips1
CPPFLAGS = $(INCDIR)

# if you aren't cross-compiling:
# GCCDIR =
# LDFLAGS = -N -T 0
# ASFLAGS =
# CPPFLAGS = -P $(INCDIR)


CC = $(GCCDIR)gcc
AS = $(GCCDIR)as
LD = $(GCCDIR)ld
CPP = $(GCCDIR)cpp

INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c $(INCDIR)

all: matmult sort testfiles CarlsJr manager-d forkthread forkthreadSort rpctest testfiles2 testot ordertaker-d customer-d testcust broadcast CVwait CVsignal Acquirelock Releaselock MVop 
 
start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt

#CarlsJr.o: CarlsJr.c
#	$(CC) $(CFLAGS) -c CarlsJr.c
#CarlsJr: CarlsJr.o start.o
#	$(LD) $(LDFLAGS) start.o CarlsJr.o -o CarlsJr.coff
#	../bin/coff2noff CarlsJr.coff CarlsJr

sort.o: sort.c
	$(CC) $(CFLAGS) -c sort.c
sort: sort.o start.o
	$(LD) $(LDFLAGS) start.o sort.o -o sort.coff
	../bin/coff2noff sort.coff sort

matmult.o: matmult.c
	$(CC) $(CFLAGS) -c matmult.c
matmult: matmult.o start.o
	$(LD) $(LDFLAGS) start.o matmult.o -o matmult.coff
	../bin/coff2noff matmult.coff matmult

testfiles.o: testfiles.c
	$(CC) $(CFLAGS) -c testfiles.c
testfiles: testfiles.o start.o
	$(LD) $(LDFLAGS) start.o testfiles.o -o testfiles.coff
	../bin/coff2noff testfiles.coff testfiles
	
proj3testcases.o: proj3testcases.c
	$(CC) $(CFLAGS) -c proj3testcases.c
proj3testcases: proj3testcases.o start.o
	$(LD) $(LDFLAGS) start.o proj3testcases.o -o proj3testcases.coff
	../bin/coff2noff proj3testcases.coff proj3testcases
	
forkthread.o: forkthread.c
	$(CC) $(CFLAGS) -c forkthread.c
forkthread: forkthread.o start.o
	$(LD) $(LDFLAGS) start.o forkthread.o -o forkthread.coff
	../bin/coff2noff forkthread.coff forkthread
	
forkthreadSort.o: forkthreadSort.c
	$(CC) $(CFLAGS) -c forkthreadSort.c
forkthreadSort: forkthreadSort.o start.o
	$(LD) $(LDFLAGS) start.o forkthreadSort.o -o forkthreadSort.coff
	../bin/coff2noff forkthreadSort.coff forkthreadSort
	
rpctest.o: rpctest.c
	$(CC) $(CFLAGS) -c rpctest.c
rpctest: rpctest.o start.o
	$(LD) $(LDFLAGS) start.o rpctest.o -o rpctest.coff
	../bin/coff2noff rpctest.coff rpctest
	
testmanager.o: testmanager.c
	$(CC) $(CFLAGS) -c testmanager.c
testmanager: testmanager.o start.o
	$(LD) $(LDFLAGS) start.o testmanager.o -o testmanager.coff
	../bin/coff2noff testmanager.coff testmanager
	
manager-d.o: manager-d.c
	$(CC) $(CFLAGS) -c manager-d.c
manager-d: manager-d.o start.o
	$(LD) $(LDFLAGS) start.o manager-d.o -o manager-d.coff
	../bin/coff2noff manager-d.coff manager-d

testfiles2.o: testfiles2.c
	$(CC) $(CFLAGS) -c testfiles2.c
testfiles2: testfiles2.o start.o
	$(LD) $(LDFLAGS) start.o testfiles2.o -o testfiles2.coff
	../bin/coff2noff testfiles2.coff testfiles2

halt2.o: halt2.c
	$(CC) $(CFLAGS) -c halt2.c
halt2: halt2.o start.o
	$(LD) $(LDFLAGS) start.o halt2.o -o halt2.coff
	../bin/coff2noff halt2.coff halt2

testot.o: testot.c
	$(CC) $(CFLAGS) -c testot.c
testot: testot.o start.o
	$(LD) $(LDFLAGS) start.o testot.o -o testot.coff
	../bin/coff2noff testot.coff testot

ordertaker-d.o: ordertaker-d.c
	$(CC) $(CFLAGS) -c ordertaker-d.c
ordertaker-d: ordertaker-d.o start.o
	$(LD) $(LDFLAGS) start.o ordertaker-d.o -o ordertaker-d.coff
	../bin/coff2noff ordertaker-d.coff ordertaker-d

customer-d.o: customer-d.c
	$(CC) $(CFLAGS) -c customer-d.c
customer-d: customer-d.o start.o
	$(LD) $(LDFLAGS) start.o customer-d.o -o customer-d.coff
	../bin/coff2noff customer-d.coff customer-d
	
testcust.o: testcust.c
	$(CC) $(CFLAGS) -c testcust.c
testcust: testcust.o start.o
	$(LD) $(LDFLAGS) start.o testcust.o -o testcust.coff
	../bin/coff2noff testcust.coff testcust

broadcast.o: broadcast.c
	$(CC) $(CFLAGS) -c broadcast.c
broadcast: broadcast.o start.o
	$(LD) $(LDFLAGS) start.o broadcast.o -o broadcast.coff
	../bin/coff2noff broadcast.coff broadcast
		
CVwait.o: CVwait.c
	$(CC) $(CFLAGS) -c CVwait.c
CVwait: CVwait.o start.o
	$(LD) $(LDFLAGS) start.o CVwait.o -o CVwait.coff
	../bin/coff2noff CVwait.coff CVwait
	
CVsignal.o: CVsignal.c
	$(CC) $(CFLAGS) -c CVsignal.c
CVsignal: CVsignal.o start.o
	$(LD) $(LDFLAGS) start.o CVsignal.o -o CVsignal.coff
	../bin/coff2noff CVsignal.coff CVsignal
	
Acquirelock.o: Acquirelock.c
	$(CC) $(CFLAGS) -c Acquirelock.c
Acquirelock: Acquirelock.o start.o
	$(LD) $(LDFLAGS) start.o Acquirelock.o -o Acquirelock.coff
	../bin/coff2noff Acquirelock.coff Acquirelock

Releaselock.o: Releaselock.c
	$(CC) $(CFLAGS) -c Releaselock.c
Releaselock: Releaselock.o start.o
	$(LD) $(LDFLAGS) start.o Releaselock.o -o Releaselock.coff
	../bin/coff2noff Releaselock.coff Releaselock

MVop.o: MVop.c
	$(CC) $(CFLAGS) -c MVop.c
MVop: MVop.o start.o
	$(LD) $(LDFLAGS) start.o MVop.o -o MVop.coff
	../bin/coff2noff MVop.coff MVop
	
clean:
	rm -f *.o *.coff
