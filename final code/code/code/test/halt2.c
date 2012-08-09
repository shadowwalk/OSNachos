#include "syscall.h"



int main() {
	
	int rv = 0, index = 0, cvrv = 0;
	int mvVal = 0;
	
	Print_Stmt("Calling start simulation\n");
	StartSimulation();
	Print_Stmt("In halt.c\n");
	
	
	rv = CreateLock("firstLock", 9, 1);
	
	
    rv = CreateLock("secondLock", 10, 1); 
	rv = CreateLock("firstLock", 9, 2); 
 	rv = CreateLock("secondLock", 10, 2);
	rv = CreateLock("firstLock", 9, 3); 
 	rv = CreateLock("secondLock", 10, 3);
	rv = CreateLock("firstLock", 9, 4); 
 	rv = CreateLock("secondLock", 10, 4);
	rv = CreateLock("firstLock", 9, 5); 
 	rv = CreateLock("secondLock", 10, 5);
	rv = CreateLock("firstLock", 9, 6); 
 	rv = CreateLock("secondLock", 10, 6);
	rv = CreateLock("firstLock", 9, 7); 
 	rv = CreateLock("secondLock", 10, 7);
	rv = CreateLock("firstLock", 9, 8); 
 	rv = CreateLock("secondLock", 10, 8);
	rv = CreateLock("firstLock", 9, 9); 
 	rv = CreateLock("secondLock", 10, 9);
	rv = CreateLock("firstLock", 9, 10); 
 	rv = CreateLock("secondLock", 10, 10);
	
	
	Acquire(rv);
 	Release(rv); 
	
	cvrv = CreateCV("firstCV", 7, 0);
 	cvrv = CreateCV("firstCV", 7, 0);
	cvrv = CreateCV("firstCV2", 8, 0);
	cvrv = CreateCV("firstCV3", 8, 0); 
	
 	Wait(cvrv, rv); 
	Signal(cvrv, rv); 
	
	DestroyLock(rv); 
	
	rv = CreateCV("firstCV", 7, 4);  
	CreateMV("firstMV", 7, 0); 
	
 	rv = CreateMV("firstMV", 7, 0);
	SetMV(rv, 3);
	mvVal = GetMV(rv);
	
	Print_1Arg("MV value obtained in halt is %d\n", mvVal); 
	
	Print_Stmt("Exiting in halt.c\n");
	
}