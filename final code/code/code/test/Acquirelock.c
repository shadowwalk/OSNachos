int main() {
	
	int rv1 = 0, rv2 = 0, rv3 = 0, rv4 = 0 ,rv5 = 0,index = 0, cvrv = 0,junk = 0;
	int mvVal = 0, result = 0, CVrv1 = 0 ,CVrv2 = 0, CVrv3 = 0,CVrv4 = 0,CVrv5 = 0;
	int flag = 0;
	Print_Stmt("Calling start simulation\n");
	StartSimulation();
	Print_Stmt("In halt1.c\n");

	
		rv1 = CreateLock("firstlock", 10, 1); 
		rv2 = CreateLock("firstlock", 10, 2);
		
  			for(index = 0; index <= 100000; index ++){
		for(index = 0; index <= 100000; index ++){
			for(index = 0; index <= 100000; index ++)
				junk = 2;
		}
	}	

	result = Acquire(rv1);
	
	Print_1Arg("\n*******TestFiles : Acquire the lock use valid Index %d ********\n",rv1);	
 	Print_1Arg("\n*******TestFiles : The return value is %d*********\n\n",result);	

	
	}
