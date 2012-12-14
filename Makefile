# Copyright (c) 1992 The Regents of the University of California.
# All rights reserved.  See copyright.h for copyright notice and limitation 
# of liability and disclaimer of warranty provisions.

MAKE = make
LPR = lpr

all: 
	cd threads; $(MAKE) depend
	cd threads; $(MAKE) nachos
	cd userprog; $(MAKE) depend 
	cd userprog; $(MAKE) nachos 
	cd vm; $(MAKE) depend
	cd vm; $(MAKE) nachos 
	cd filesys; $(MAKE) depend
	cd filesys; $(MAKE) nachos 
	cd network; $(MAKE) depend
	cd network; $(MAKE) nachos 
	cd bin; make all
	cd test; make all

# don't delete executables in "test" in case there is no cross-compiler
# origin command didn't work:/bin/sh -c "rm -f *~ */{core,nachos,DISK,*.o,swtch.s,*~} test/{*.coff} bin/{coff2flat,coff2noff,disassemble,out}"
clean:
	rm -rf *~
	rm -rf */*.o
	rm -rf */nachos
	rm -rf */swtch.s
	rm -rf */*~

cleanall:
	rm -rf *~
	rm -rf */*.o
	rm -rf */nachos
	rm -rf */swtch.s
	rm -rf */*~
	rm -rf test/*.coff
	rm -rf bin/coff2flat
	rm -rf bin/coff2noff
	rm -rf bin/disassemble
	rm -rf bin/out

print:
	/bin/sh -c "$(LPR) Makefile* */Makefile"
	/bin/sh -c "$(LPR) threads/*.h threads/*.cc threads/*.s"
	/bin/sh -c "$(LPR) userprog/*.h userprog/*.cc" 
	/bin/sh -c "$(LPR) filesys/*.h filesys/*.cc
	/bin/sh -c "$(LPR) network/*.h network/*.cc 
	/bin/sh -c "$(LPR) machine/*.h machine/*.cc
	/bin/sh -c "$(LPR) bin/noff.h bin/coff.h bin/coff2noff.c"
	/bin/sh -c "$(LPR) test/*.h test/*.c test/*.s"
