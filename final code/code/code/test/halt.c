#include "syscall.h"



int main() {
	
	int rv1 = 0, rv2 = 0, rv3 = 0, rv4 = 0 ,rv5 = 0,index = 0, cvrv = 0;
	int mvVal = 0, result = 0, CVrv1 = 0 ,CVrv2 = 0, CVrv3 = 0,CVrv4 = 0,CVrv5 = 0;
	int flag = 0;
	
	Print_Stmt("Calling start simulation\n");
	StartSimulation();
	Print_Stmt("In halt.c\n");
	Print_Stmt("\n----------------------------------------------------------------\n");
	Print_Stmt("\t\tCREATELOCK	TEST CASE \n");
	Print_Stmt("-----------------------------------------------------------------\n");
	
		rv1 = CreateLock("firstlock", 10, 1); 
	Print_1Arg("\n***********TestFiles : SETP1 Create firstlock1 return value is %d************\n\n",rv1);	
		rv2 = CreateLock("firstlock", 10, 2);
	Print_1Arg("\n***********TestFiles : SETP2 Create firstlock2  return value is %d************\n\n", rv2);	
 		rv3 = CreateLock("", 1, 0);		
	Print_1Arg("\n***********TestFiles : SETP3 Create invalidlock return value is %d************\n\n",rv3);	
		rv4 = CreateLock("secondlock", 10, 0);
	Print_1Arg("\n***********TestFiles : SETP4 Create secondlock return value is %d************\n\n",rv4);	
		rv5 = CreateLock("secondlock", 10 , 0);
	Print_1Arg("\n***********TestFiles : SETP5 Create secondlock return value is %d************\n\n",rv5);	
 	
	Print_Stmt("\n----------------------------------------------------------------\n");
	Print_Stmt("\t\tCREATE CONDITION TEST \n");
	Print_Stmt("-----------------------------------------------------------------\n");
	CVrv1 = CreateCV("firstCV",7,1);
	Print_1Arg("\n******TestFiles : SETP1 Create a CV have index 1 return value is %d*******\n\n",result);	
	CVrv2 = CreateCV("firstCV",7,2);
	Print_1Arg("\n******TestFiles : SETP2 Create a CV have index 2 return value is %d*******\n\n",result);	
	CVrv3 = CreateCV("", 1, 0);	
	Print_1Arg("\n******TestFiles : SETP3 Create a CV have no name return value is %d*******\n\n",result);	
	CVrv4 = CreateCV("secondCV", 8, 0);
	Print_1Arg("\n******TestFiles : SETP4 Create a CV name is secondCV return value is %d*******\n\n",result);	
	CVrv5 = CreateCV("secondCV", 8, 0);
	Print_1Arg("\n******TestFiles : SETP4 Create same CV name secondCV return value is %d*******\n\n",result);	

		
 	Print_Stmt("\n----------------------------------------------------------------\n");
	Print_Stmt("\t\tSIGNAL AND WAIT TEST \n");
	Print_Stmt("-----------------------------------------------------------------\n");

		CVrv1 = CreateCV("firstCV",7,1);
		CVrv2 = CreateCV("firstCV",7,2);
		
		result = Acquire(rv1);
		result = Wait(-1,rv1);
		Print_1Arg("\n******TestFiles : SETP1 Wait in invalid CVIndex return value is %d*******\n\n",result);	
		result = Wait(CVrv1,10);
		Print_1Arg("\n******TestFiles : SETP2 Wait in invalid LockIndex return value is %d********\n\n",result);	
		result = Release(rv1);
		result = Wait(CVrv1,rv1);
		Print_1Arg("\n*****TestFiles : SETP3 Wait in validlock not own by me return value is %d*****\n\n",result);	

		
	result = Acquire(rv1);

	result = Signal(-1,rv1);
	Print_1Arg("\n******TestFiles : SETP1 Signal in invalid CVIndex return value is %d******\n\n",result);	
	result = Signal(CVrv1,10);
	Print_1Arg("\n******TestFiles : SETP2 Signal in invalid LockIndex return value is %d******\n\n",result);	
	result = Release(rv1);
	result = Signal(CVrv1,rv1);
	Print_1Arg("\n******TestFiles : SETP3 Signal in validlock not own by me return value is %d*****\n\n",result);	

 	Print_Stmt("\n---------------------------------------------------------------\n");
	Print_Stmt("\t\tACQUIRE AND RELEASE TEST \n");
	Print_Stmt("-----------------------------------------------------------------\n");
	
	result = Acquire(rv1);
	Print_1Arg("\n******TestFiles : STEP1 Halt in critical region of lockIndex %d*****\n\n",result);	
 	result = Acquire(-1);
	Print_1Arg("\n ****** TestFiles1 : STEP2 Acquire lock -1  return value is %d******\n\n",result);
	result = Acquire(5);
	Print_1Arg("\n ****** TestFiles1 : STEP3 Acquire lock 5  return value is %d******\n\n",result);
 	result = Release(rv2);
	Print_1Arg("\n ****** TestFiles1 : STEP4 Release firstlock2 return value is %d******\n\n",result);
	result = Acquire(rv1);
	Print_1Arg("\n ****** TestFiles1 : STEP5 Halt1 Acquire lock1 again the return value is %d******\n\n",result);
	result = Release(rv1);
	Print_1Arg("\n ******TestFiles1 : STEP6 Halt1 Release the firstlock return value is %d******\n\n",result);
	result = Release(rv1);
	Print_1Arg("\n ****** TestFiles1 : STEP7 Halt1 Release the firstlock return value is %d******\n\n",result);
 
	Print_Stmt("\n---------------------------------------------------------------\n");
	Print_Stmt("\t\tCREATE MONITOR VARIABLE TEST \n");
	Print_Stmt("-----------------------------------------------------------------\n");

	result = CreateMV("firstMV",7,1);
	Print_1Arg("\n******TestFiles : SETP1 Create a MV have index 1 return value is %d*******\n\n",result);	
	result = CreateMV("firstMV",7,2);
	Print_1Arg("\n******TestFiles : SETP2 Create a MV have index 2 return value is %d*******\n\n",result);	
	result = CreateMV("", 1, 0);	
	Print_1Arg("\n******TestFiles : SETP3 Create a MV have no name return value is %d*******\n\n",result);	
	result = CreateMV("secondMV", 8, 0);
	Print_1Arg("\n******TestFiles : SETP4 Create a MV name is secondMV return value is %d*******\n\n",result);	
	result = CreateMV("secondMV", 8, 0);
	Print_1Arg("\n******TestFiles : SETP4 Create same MV name secondMV return value is %d*******\n\n",result);	

	
}