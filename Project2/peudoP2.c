                                         //Create Condition variable
//in user prog
	int my_Condition = CreateConditionVariable(name, size);

//in Exception Handler
	switch(type)
		case CS_CreateCondition
		//return value is the index position of a kernel structure	array of actual Condition objects
		rv = Create_Condition_Syscall(machine -> ReadRegister(4),machine -> ReadRegister(5));
		break;
		
//in specific Systemcall for Create Condition variable		
int Create_Condition_Syscall(unsigned int vaddr, int len){
	
	//I think we shoule use the variable vaddr and len already passed, so that no need to read it again in next line
	//read the arguement to get the name and the size of this ConditionVariable
	
	//   int name = machine -> ReadRegister(4);
	//   int size = machine -> ReadRegister(5);
	//------------choose one method---------
	//   int name = vaddr;
	//   int size = len;
	
	if ((size < 1) || (size > MAX_CHAR)) //size too big or negtive value
		//return wrong msg
	if (name < 0) || (name + size >= currentThread -> spave.addressspacesize)//not enough space for this CV or negtive value
		//return wrong msg
	
	//work correct then create actual name and size for it 
	char *ConditionName = new char[size + 1];
	ConditionName[size] = '\&'	;
	copyin(ConditionName, name, size);	
	KernelConditionTableLock -> Acquire();
		//exceeded the max number of condition
		if (nextconditionIndex >= Max_Condition)
			//reurn wrong msg
	    else {
		//Create specific condition object for this index
		//include the entity of this conditionvariable and its lock	
		OSCondition[nextConditionIndex].ConditionVariable = new Condition(ConditionVariableName);
		OSCondition[nextConditionIndex].ConditionLock = new lock(ConditionVariableName);
		//give some other information needed like space and destroy FLAG
		OSCondition[nextConditionIndex].AddressSpace = currentThread -> space;
		OSCondition[nextConditionIndex].usageCounter = 0;
		OSCondition[nextConditionIndex].ToBeDestroyed = FALSE;
		//finish and return this index and increase the next index
		rv = nextConditionIndex;
		nextConditionIndex ++;
		}
	KernelConditionTableLock -> Release();	
}

//the kernel data for Create Condition Systemcall
	extern int nextConditionIndex = 0;
	extern int MAX_Conditions = 50;
	extern lock* kernelConditionTableLock;

	struct KernelCondition{
		Condition *ConditionVariable;
		Lock *ConditionLock;
		Addrspace *AddressSpace;
		int usageCounter;
		bool ToBeDestroyed;
	}
	KernelCondition OSCondition[MAX_Conditions];


	
	                            //ConditionVariable Wait() syscall
//in userprog
	ConditionWait(myCondition, myLock);//choose a lock you want to use in CV

//in Exception Handler
	switch(type)
		case CS_ConditionWait
		//no return value, just pass the the index of correspond CV and LOCK
		ConditionWait_Syscall(machine -> ReadRegister(4),machine -> ReadRegister(5));
		break;

//in specific Systemcall for ConditionWait	
	//Condition value and lock value show the index of the CV and Lock	
	void ConditionWait_Syscall(int Condition_value, int Lock_value){
		//check wether CV is the right range 
		KernelConditionTableLock -> Acquire();
		if ((Condition_value < 0)|| (Condition_Value >= nextConditionIndex))
			//condition index value not created or negtive &return
		if ((Lock_value < 0)|| (Lock_Value >= nextLockIndex))
			//Lock index value not created or negtive &return
		//index value is OK then change current Lock and Condition
		KernelLock currentCondition = OSCondition[Condition_value]; 
		KernelLock currentLock = OSLock[Lock_value] ;
		if (currentCondition.ConditionVariable == NULL)
			//Condition been destroyed print & return
		if (currentLock == NULL)
			//Lock been destroyed print & return
		//check wether they are belong to another thread
		if (currentCondition.AddressSpace != CurrentThread -> space)
			//Condition belong to different thread
		if (currentLock.AddressSpace != CurrentThread -> space)
			//Lock belong to different thread
		//everything is OK
		KernelConditionTableLock -> Release();
		CurrentCondition.usageCounter ++
		CurrentCondition.ConditionVariable -> Wait(&CurrentLock);
	}



