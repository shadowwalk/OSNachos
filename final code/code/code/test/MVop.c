#include "syscall.h"



int main() {
	
	int rv1 = 0, rv2 = 0, rv3 = 0, rv4 = 0 ,rv5 = 0,index = 0, cvrv = 0;
	int mvVal = 0, result = 0, MVrv1 = 0 ,MVrv2 = 0, CVrv3 = 0,CVrv4 = 0,CVrv5 = 0;
	int flag = 0;
	Print_Stmt("Calling start simulation\n");
	StartSimulation();

 	Print_Stmt("\n----------------------------------------------------------------\n");
	Print_Stmt("\t\tSIGNAL AND WAIT TEST \n");
	Print_Stmt("-----------------------------------------------------------------\n");
	


	MVrv1 = CreateMV("firstMV",7,1);
	MVrv2 = CreateMV("firstMV",7,2);

	result = GetMV(-1);
	Print_1Arg("\n ****** TestFiles1 : STEP1 Get MVIndex -1  result is %d******\n\n",result);
	result = GetMV(5);
	Print_1Arg("\n ****** TestFiles1 : STEP2 Get MVIndex  5  result is %d******\n\n",result);
	result = SetMV(-1,0);
	Print_1Arg("\n ****** TestFiles1 : STEP3 Set 0 to MVIndex  -1  result is %d******\n\n",result);
	result = SetMV(5,0);
	Print_1Arg("\n ****** TestFiles1 : STEP4 Set 0 to MVIndex  5  result is %d******\n\n",result);
	
	
	result = GetMV(MVrv1);
	Print_2Arg("\n ****** TestFiles1 : STEP5 Get MVIndex %d result is %d******\n\n",MVrv1,result);
	result = GetMV(MVrv2);
	Print_2Arg("\n ****** TestFiles1 : STEP6 Get MVIndex %d result is %d******\n\n",MVrv2,result);
	result = SetMV(MVrv1,100);
	Print_2Arg("\n ****** TestFiles1 : STEP7 Set 100 to MVIndex %d result is %d******\n\n",MVrv1,result);
	result = GetMV(MVrv1);
	Print_2Arg("\n ****** TestFiles1 : STEP5 Get MVIndex %d result is %d******\n\n",MVrv1,result);
	result = GetMV(MVrv2);
	Print_2Arg("\n ****** TestFiles1 : STEP6 Get MVIndex %d result is %d******\n\n",MVrv2,result);

	
	
	
}