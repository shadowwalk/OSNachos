#include "syscall.h"



int main() {
	
	int rv1 = 0, rv2 = 0, rv3 = 0, rv4 = 0 ,rv5 = 0,index = 0, cvrv = 0;
	int mvVal = 0, result = 0, CVrv1 = 0 ,CVrv2 = 0, CVrv3 = 0,CVrv4 = 0,CVrv5 = 0;
	int flag = 0;
	Print_Stmt("Calling start simulation\n");
	StartSimulation();
	Print_Stmt("In halt1.c\n");

	
		rv1 = CreateLock("firstlock", 10, 1); 
		rv2 = CreateLock("firstlock", 10, 2);
		
	CVrv1 = CreateCV("firstCV",7,1);
	CVrv2 = CreateCV("firstCV",7,2);
  	
	result = Acquire(rv1);
	
	result = Wait(CVrv1,rv1);
	Print_Stmt("\n*******TestFiles : SETP4 Wait by validlock owned by me ********\n");	
 	Print_1Arg("\n*******TestFiles : SETP4 the return value is %d*********\n\n",result);	

	Release(rv1);
	Print_Stmt("\n*******TestFiles : SETP5 Release the lock after CVwait ********\n");	
	Print_1Arg("\n*******TestFiles : SETP5 the return value is %d********\n\n",result);	
	
	}
	
	
	