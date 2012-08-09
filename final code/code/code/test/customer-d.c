#include "syscall.h"


	/* cust info*/
	int myIndex = 0;
	int tempV = -1;
	int myOT = -1;
	int inOT = 0;
	
	int orderTakerStatus = 0;
	
	int custDataSixBurger = 0;
	int custDataThreeBurger = 0;
	int custDataVegBurger = 0;
	int custDataFries = 0;
	int custDataSoda = 0;
	int custDataDineType = 0;	
	int custDataDelivered = 0;
	int custDataTokenNo = 0;
	int custDataTableNo = 0;
	int tableAvailable = 0;
	int tables = 0;
	int custLineLength = -1;
	/* cust info*/
	
	
	
	int OT_WAIT = 2;
	int OT_FREE = 0;
	int OT_BUSY = 1;
	 
	int CUST_COUNT = 4;
	int WAITER_COUNT = 2;
	int COOK_COUNT = 2;
	int OT_COUNT = 2;
	 
	int FILLED = 100;
	int MININVENTORY = 50;
	int TABLE_COUNT = 20;
	int MINCOOKEDFOOD = 2;
	int MAXDIFF = 4;
	 
	int SIXBURGER = 1;
	int THREEBURGER = 2;
	int VEGBURGER = 3;
	int FRIES = 4;
	 
	int index = 0;
	 
	 
	/* ----------------------------- */
	/* FOOD TO BE COOKED DATABASE ATTRIBUTES */
	/* FOOD READY DATABASE ATTRIBUTES */
	/* CUST DATABASE DATABASE ATTRIBUTES */
	
	int readySixBurgerMVi;
	int readyThreeBurgerMVi;
	int readyVegBurgerMVi;
	int readyFriesMVi;
	
	int toBeCookedSixburgerMVi;
	int toBeCookedThreeburgerMVi;
	int toBeCookedVegburgerMVi;
	int toBeCookedFriesMVi;	
	
	/* 12 ARRAYS FOR STORING INDEX VALUES OF 12 DIFFERENT ATTRIBUTES IN CUSTOMER DATABASE */
	int dineTypeincustDBMVi[20];
	int deliveredincustDBMVi[20];
	int baggedincustDBMVi[20];
	int baggedListMVi[20];
	int myOTMVi[20];
	int orderedincustDBMVi[20];
	int sixBurgerincustDBMVi[20];
	int threeBurgerincustDBMVi[20] ;
	int vegBurgerincustDBMVi[20] ;
	int friesincustDBMVi[20] ;
	int sodaincustDBMVi[20] ; 
	int tokenNoincustDBMVi[20];
	int tableNoincustDBMVi[20];
	/* ----------------------------- */
	
	
	
	/*------------------------------------------------------------------------ */
	/* KERNEL LOCK INDEX VARIABLES */
	/*------------------------------------------------------------------------ */

	/* Locks - for customer-ordertaker interaction */
	/* Lock to establish mutual exclusion on customer line */
	int custLineLock = 0;          

	/* Lock to establish mutual exclusion on the OT status */
	/* and for actions between customer and OT */
	int orderTakerLock[30];	

	/* Lock to establish mutual exclusion on the foodReadyDataBase */
	int foodReadyDBLock = 0;

	/* Lock to establish mutual exclusion on the foodToBeCookedDatabase */
	int foodToBeCookedDBLock = 0;

	/* Lock to get the next token number to be given to the customer */
	int nextTokenNumberLock = 0;

	/* Lock to access the location where all money collected at the restaurant */
	/* is stored by the Order Takers */
	int moneyAtRestaurantLock = 0;

	/* Lock to wait in the to-go waiting line */
	int waitToGoLock;

	/* Lock to establish mutual exclusion on the tables data */
	int tablesDataLock = 0;

	/* Lock to wait in the eat-in table waiting line */
	int eatInWaitingLock = 0;

	/* Lock for the eat-in seated customers waiting for food */
	int eatInFoodWaitingLock = 0;

	/* Lock to update the foodToBag count */
	int foodToBagLock = 0;

	/* Lock to add/delete tokenNo into the List of bagged orders */
	int foodBaggedListLock = 0;

	/* Lock to access the raw materials inventory */
	int inventoryLock = 0;

	/* Lock to update the manager line length monitor */
	int managerLock = 0;

	/* Lock to update the what to cook value */
	int whatToCookNextLock = 0;

	/* Locks to set the stop cooking flags for different food items */
	int stopSixBurgerLock = 0;
	int stopThreeBurgerLock = 0;
	int stopVegBurgerLock = 0;
	int stopFriesLock = 0;

	/* Lock for the communication between Manager and Waiter */
	int waiterSleepLock = 0;

	/* Customer data Lock */
	int customerDataLock = 0;

	/* Lock to enter a cook waiting queue / removing a cook from waiting queue */
	int wakeUpCookLock = 0;

	/* Lock to update the number of customers serviced */
	int custServedLock = 0;

	/* Lock to update the cooks on break count */
	int cooksOnBreakCountLock = 0;

	/* Lock to get the next customer index */
	int nextCustomerIndexLock = 0;

	/* Lock to get the next waiter index */
	int nextWaiterIndexLock = 0;

	/* Lock to get the next Cook Index */
	int nextCookIndexLock = 0;

	/* Lock to get the next ordertaker Index */
	int nextOrderTakerIndexLock = 0;

	/* Lock to set the item to be cooked by the cook when he is hired */
	int whatToCookFirstLock = 0;

	/*lock used when show how many working cooks for itme */
	int workingCookLock = 0;

	/* to access the bag the orders Lock */
	int BagTheOrdersLock = 0;

	/* ---------------------------------------------------------------------------- */
	
	
	
	
	
	/*------------------------------------------------------------------------ */
	/* GLOBAL MONITOR VARIABLES */
	/*------------------------------------------------------------------------ */
	int custLineLengthMVi = 0;		/* to access the line status one at a time */
	int nextTokenNumberMVi = 0;		/* to ensure that token number is unique */
	int moneyAtRestaurantMVi = 0;		/* to store the money collected  */
	int RefillcashMVi = 0;
	int broadcastedTokenNoMVi = 0;		/* to store the current broadcasted token */
	int managerLineLengthMVi = 0;		/* to store information on the number of eat-in */
								/* customers waiting for manager reply  */
	int tablesMVi[20];							
	int tableAvailableMVi = 0;			/* manager sets this if table is available  */
	int foodToBagMVi = 0;				/* number of orders still to be bagged	 */
	int inventoryMVi = 0;				/* stores the current raw materials available */
									/* in the restaurant inventory */
	int whatToCookNextMVi = 0;			/* tells the cook what to cook next	 */						
	int stopSixBurgerMVi = 0;		/* Flags for the cooks to stop cooking */
	int stopThreeBurgerMVi = 0;
	int stopVegBurgerMVi = 0;
	int stopFriesMVi = 0;		
	int custServedMVi = 0;			/* Counter to keep a count of customers served */
	int workingCooksMVi[5];		/*indicate the number of working cooks */
	int nextWaiterIndexMVi = 0;		/* Every time a new waiter is spawned , he will set his index to */
								/* this value incremented once */
	int nextCookIndexMVi = 0;		/* Every time a new Cook is spawned , he will set his index to */
								/* this value incremented once */							
	int nextCustomerIndexMVi = 0;	/* Every time a new Customer is spawned , he will set his index to */
								/* this value incremented once */
	int nextOrderTakerIndexMVi = 0;	/* Every time a new waiter is spawned , he will set his index to */
								/* this value incremented once */	

	int whatToCookFirstMVi = 0;		/* This value tells the Cook newly hired, what to start with */

	int cooksOnBreakCountMVi = 0;	/* Keeps track of the number of cooks who are on break
								   Manager checks this value to be 0 before hiring a new cook */
	int orderTakerStatusMVi[20];		
	
	int startWaitLocki = 0;
	int startWaitCVi = 0;
	int startWaitMVi = 0;
	int StartWaitLk = 0;
	int getValueStartWaitMV = 0;
	
	
	/*------------------------------------------------------------------------ */
	
	
	/*------------------------------------------------------------------------ */
	/*              KERNEL CONDITION VARIABLES INDEX VARIABLES                 */
	/*------------------------------------------------------------------------ */							
	/* CV - for customer-ordertaker interaction */
	/* CV to sequence the customers in the customer line */
	/* - customers to join the customer line */
	/* - OT to remove a waiting customer from customer line */
	int custLineCV = 0;

	/* array of CV to sequence the actions between customer & OT */
	int orderTakerCV[30];

	/* CV for waiting in line to find a free table */
	int tablesDataCV = 0;

	/* CV for sequencing the customers waiting for the free table */
	int eatInWaitingCV = 0;

	/* CV for sequencing the eat-in seated customers waiting for the order */
	int eatInFoodWaitingCV = 0;

	/* CV for sequencing the eat-in customers in the manager reply waiting queue */
	int managerCV = 0;

	/* CV for waiters to wait for the signal from the manager */
	int waiterSleepCV = 0;

	/* Common cook waiting queue  */
	int wakeUpCookCV = 0;

	/* CV for the to-go customers to join the to-go waiting line */
	int toGoCV = 0;
	
	/* ----------------------------------------------------------------------------- */
	





void main()
{
	
	/* Print_Stmt("Calling start simulation\n"); */
	StartSimulation();
	/* Print_Stmt("Back from start simulation\n"); */
		
	/* -----------------INITIALISATION -----------------------*/
		
	/* ------------------FOOD TO BE COOKED AND FOOD READY STRUCTURE ATTRIBUTES MONITOR VARIABLES CREATION------------------- */
	
	/* --------------------------------------------------------------- */
	/* --------------------------------------------------------------- */
	/* --------------------------------------------------------------- */
	/* --------------------------------------------------------------- */
	/*						INITIALISATION BLOCK BEGINS				   */
	/* --------------------------------------------------------------- */
	/* --------------------------------------------------------------- */
	/* --------EVERY ENTITY DOES EVERYTHING BELOW--------------------- */
	/* --------------------------------------------------------------- */

	
	
	/*------------------------------------------------------------	*/
	/* 				CREATE all the Locks using Syscalls				*/
	/*------------------------------------------------------------	*/
	custLineLock = CreateLock("custLineLk", 10, 0);
	foodReadyDBLock = CreateLock("foodReadyDBLk", 13, 0);
	foodToBeCookedDBLock = CreateLock("fd2BCokdDBLk", 12, 0);
	nextTokenNumberLock = CreateLock("nextTokenNoLk", 13, 0);
	moneyAtRestaurantLock = CreateLock("moneyAtRestLk", 13, 0);
	waitToGoLock = CreateLock("waitToGoLk", 10, 0);
	tablesDataLock = CreateLock("tablesDataLk", 12, 0);
	eatInWaitingLock = CreateLock("eatInWaitingLk", 14, 0);
	eatInFoodWaitingLock = CreateLock("eatInFWLk", 9, 0);
	foodToBagLock = CreateLock("foodToBagLk", 11, 0);
	foodBaggedListLock = CreateLock("foodBgdListLk", 13, 0);
	inventoryLock = CreateLock("inventoryLk", 11, 0);
	managerLock = CreateLock("managerLk", 9, 0);
	whatToCookNextLock = CreateLock("whatToCookNextLk", 16, 0);
	stopSixBurgerLock = CreateLock("stop6BLk", 8, 0);
	stopThreeBurgerLock = CreateLock("stop3BLk", 8, 0);
	stopVegBurgerLock = CreateLock("stopVBLk", 8, 0);
	stopFriesLock = CreateLock("stopFLk", 7, 0);
	waiterSleepLock = CreateLock("waiterSleepLk", 13, 0);
	customerDataLock = CreateLock("custDataLk", 10, 0);
	wakeUpCookLock = CreateLock("wakeUpCookLk", 12, 0);
	custServedLock = CreateLock("custServedLk", 12, 0);
	nextWaiterIndexLock = CreateLock("nxtWtrIdxLk", 11, 0);
	nextCookIndexLock = CreateLock("nxtCkIdxLk", 10, 0);
	nextCustomerIndexLock = CreateLock("nxtCustIdxLk", 12, 0);
	nextOrderTakerIndexLock = CreateLock("nxtOTIdxLk", 10, 0);
	whatToCookFirstLock = CreateLock("what2CkFrstLk", 13, 0); 
	cooksOnBreakCountLock = CreateLock("cksOnBrkCntLk", 13, 0);
	workingCookLock = CreateLock("workingCooklk", 13, 0);
	BagTheOrdersLock	= CreateLock("BagTheOrdersLk", 14, 0);	
	
	/* Create OT_COUNT number of locks, one Lock for each Ordertaker	*/
	for(index = 1; index <= OT_COUNT; index++)
		orderTakerLock[index] = CreateLock("orderTakerLk", 12, index);
	
	/* --------------------------------------------------------------- */
	
	
	
/* 	Print_Stmt("after creating locks\n"); */
	
	/*------------------------------------------------------------	*/
	/* Create all the Condition variables using Syscalls			*/
	/*------------------------------------------------------------	*/
	custLineCV = CreateCV("custLineCV", 10, 0);
	tablesDataCV = CreateCV("tablesDataCV", 12, 0); 
	eatInWaitingCV = CreateCV("eatInWaitingCV", 14, 0);
	eatInFoodWaitingCV = CreateCV("eatInFWtngCV", 12, 0);
	managerCV = CreateCV("managerCV", 9, 0);
	waiterSleepCV = CreateCV("waiterSleepCV", 13, 0);
	wakeUpCookCV = CreateCV("wakeUpCookCV", 12, 0);
	toGoCV = CreateCV("toGoCV", 6, 0); 
	/* Create OT_COUNT number of Cvs, one CV for each Ordertaker	*/
	for(index = 1; index <= OT_COUNT; index++)
		orderTakerCV[index] = CreateCV("orderTakerCV", 12, index);
		
	/* --------------------------------------------------------------- */
	
	
	/* foodReadyData.sixBurger */
	readySixBurgerMVi = CreateMV("ready6B", 7, 100, 0);
	
	/* foodReadyData.threeeBurger */
	readyThreeBurgerMVi = CreateMV("ready3B", 7, 100, 0);
	
	/* foodReadyData.vegBurger */
	readyVegBurgerMVi = CreateMV("ready6VB", 8, 100, 0);
	
	/* foodReadyData.fries */
	readyFriesMVi = CreateMV("readyF", 6, 100, 0);

	/* foodToBeCookedData.sixBurger */
	toBeCookedSixburgerMVi = CreateMV("toBecooked6B", 12, 0, 0);
	
	/* foodToBeCookedData.threeBurger */
	toBeCookedThreeburgerMVi = CreateMV("toBecooked3B", 12, 0, 0);
	
	/* foodToBeCookedData.vegBurger */
	toBeCookedVegburgerMVi = CreateMV("toBecookedVB", 12, 0, 0);
	
	/* foodToBeCookedData.fries */
	toBeCookedFriesMVi = CreateMV("toBecookedF", 11, 0, 0); 
	
	/*--------------------------------------------------------------------------------------------- */
	
	
	
	
	
	
	/* -------- FILL IN THE CUST DATABASE WITH DEFAULT VALUES--------------------------------------- */
	
	for(index = 1; index <= CUST_COUNT; index++)
	{
		/* custData[index].bagged */
		baggedincustDBMVi[index] = CreateMV("baggedMV", 8, 0, index);
		
		/* custData[index].dineType */
		dineTypeincustDBMVi[index] = CreateMV("dineTypeMV", 10, 0, index);
		
		/* custData[index].delivered */
		deliveredincustDBMVi[index] = CreateMV("deliveredMV", 11, 0, index);
		
		/* baggedlist[index] */
		baggedListMVi[index] = CreateMV("baggedMV", 8, 0, index);
		
		/* custData[index].myOT - initially myOT is -1 */
		myOTMVi[index] = CreateMV("myOTMV", 6, -1, index);
		
		/* custData[index].ordered */
		orderedincustDBMVi[index] = CreateMV("orderedMV", 9, 0, index);
		
		/* custData[index].sixBurger/threeBurger/etc. */
		sixBurgerincustDBMVi[index] = CreateMV("sixBurgerMV", 11, 0, index);
		
		threeBurgerincustDBMVi[index] = CreateMV("threeBurgerMV", 13, 0, index);
		
		vegBurgerincustDBMVi[index] = CreateMV("vegBurgerMV", 11, 0, index);
		
		friesincustDBMVi[index] = CreateMV("friesMV", 7, 0, index);
		
		sodaincustDBMVi[index] = CreateMV("sodaMV", 6, 0, index);
		
		/* custData[Index].tokenNo/tableNo */
		tokenNoincustDBMVi[index] = CreateMV("tokenNoMV", 9, 0, index);
		
		tableNoincustDBMVi[index] = CreateMV("tableNoMV", 9, -1, index);
	}
	/* -------------------------------------------------------------------------------- */
	
	
	
	
	
	/* ---------------------  CREATE ALL THE GLOBAL MONITOR VARIABLES ----------------- */
	
	nextOrderTakerIndexMVi = CreateMV("nextOTIndex", 11, 1, 0);
	custServedMVi = CreateMV("custServed", 10, 0, 0);
	foodToBagMVi = CreateMV("foodToBag", 9, 0, 0);
	nextCustomerIndexMVi = CreateMV("nextCustIndex", 13, 1, 0);
	custLineLengthMVi = CreateMV("custLineLength", 14, 0, 0);
	managerLineLengthMVi = CreateMV("mgrLineLength", 13, 0, 0);
	tableAvailableMVi = CreateMV("tableAvailable", 14, 0, 0);
	nextTokenNumberMVi = CreateMV("nextTokenNo", 11, 0, 0);
	moneyAtRestaurantMVi = CreateMV("moneyAtRest", 11, 0, 0);
	RefillcashMVi = CreateMV("Refillcash", 10, 0, 0);
	broadcastedTokenNoMVi = CreateMV("bcastTokenNo", 12, 0, 0);
	inventoryMVi = CreateMV("inventory", 9, 100, 0);
	whatToCookNextMVi = CreateMV("whatToCookNext", 14, 0, 0);									
	stopSixBurgerMVi = CreateMV("stop6B", 6, 1, 0);
	stopThreeBurgerMVi = CreateMV("stop3B", 6, 1, 0);
	stopVegBurgerMVi = CreateMV("stopVB", 6, 1, 0);
	stopFriesMVi = CreateMV("stopF", 5, 1, 0);
	
	/* tables[index] */
	for(index = 0; index < TABLE_COUNT; index++)
	{ 	
		tablesMVi[index] = CreateMV("tablesMV", 8, 0, index);
	}
	
	for(index = 1; index <= OT_COUNT; index++)
	{
		
		orderTakerStatusMVi[index] = CreateMV("OTstatusMV", 10, OT_BUSY, index);
		
	}
/* 	
	for(index = 1; index <= 4; index++)
	{
		switch(index) 
		{
			case 1:
					workingCooksMVi[index] = CreateMV("wkingCks6$", 9, 0, index);
					break;
			case 2:
					workingCooksMVi[index] = CreateMV("wkingCks3$", 9, 0, index);
					break;		
			case 3:
					workingCooksMVi[index] = CreateMV("wkingCksV$", 9, 0, index);
					break;
			case 4:
					workingCooksMVi[index] = CreateMV("wkingCksF$", 9, 0, index);
					break;
		}
	} */
	
/* 	for(index = 1; index <= CUST_COUNT; index++)
	{

		SetMV(baggedincustDBMVi[index], 0);
		SetMV(deliveredincustDBMVi[index], 0);
		SetMV(baggedListMVi[index],0);
		SetMV(myOTMVi[index],0);
		SetMV(orderedincustDBMVi[index],0);
		SetMV(tokenNoincustDBMVi[index],-1);
		SetMV(tableNoincustDBMVi[index],-1); 
		if (index%2 == 0)
			tempV = 1;
		else
			tempV = 0;
			SetMV(dineTypeincustDBMVi[index],tempV);	
			SetMV(friesincustDBMVi[index],tempV);		
			SetMV(threeBurgerincustDBMVi[index],tempV);
				if (index%2 == 0)
			tempV = 0;
		else
			tempV = 1;
			SetMV(vegBurgerincustDBMVi[index],tempV);
			SetMV(sixBurgerincustDBMVi[index],tempV);		
			SetMV(sodaincustDBMVi[index],tempV);

	} */
	/* --------------------------------------------------------------- */
	/* --------------------------------------------------------------- */
	/* --------------------------------------------------------------- */
	/* --------------------------------------------------------------- */
	/*						INITIALISATION BLOCK END				   */
	/* --------------------------------------------------------------- */
	/* --------------------------------------------------------------- */
	/* --------EVERY ENTITY DOES EVERYTHING ABOVE--------------------- */
	/* --------------------------------------------------------------- */
	/* -----------------INITIALISATION END-----------------------*/
	Print_Stmt("Cust INITIALISATION END\n");

	Customer(); 

	/* ------------------------------------------------------------------------------ */

}




void Customer() {

	/* Get the Index for this Customer before starting the order*/
	Acquire(nextCustomerIndexLock);	
	myIndex = GetMV(nextCustomerIndexMVi);
	tempV = myIndex + 1;
	SetMV(nextCustomerIndexMVi, tempV);
	
	Release(nextCustomerIndexLock);
	
	
	for(index = 1; index <= OT_COUNT; index++)
	{
		Acquire(custLineLock);
		
		orderTakerStatus = GetMV(orderTakerStatusMVi[index]);
		
		Print_2Arg("OT status of OT %d is %d\n", index, orderTakerStatus);
		
		if(orderTakerStatus == OT_FREE)
		{	
			Print_1Arg("Found OT %d FREE\n", index);
			orderTakerStatus = OT_BUSY;
			SetMV(orderTakerStatusMVi[index], orderTakerStatus);
			
			Print_3Arg("Cust Signalling FREE OT %d with CV = %d and Lock = %d\n", index, custLineCV, custLineLock);
			Signal(custLineCV, custLineLock);
	
			Acquire(customerDataLock);
			
			myOT = index;
			Print_1Arg("Found OT %d FREE I made him BUSY\n\n", index);
			SetMV(myOTMVi[myIndex], index);
			Release(customerDataLock);
			break;
		}
		myOT = -1;
		
	}
	
	/*If no Order Takers are free */
	/* - increment the linelength monitor variable */
	/* - then wait in the customer waiting line */
	Acquire(customerDataLock);
			
	if(myOT == -1){
		
		Print_1Arg("Since no OT available, Cust %d has myOT = myOT\n", myIndex);
		Release(customerDataLock);
	
		tempV = GetMV(custLineLengthMVi);	
		tempV++;
		SetMV(custLineLengthMVi, tempV);
		
		Print_1Arg("Increased the linelength to = %d", tempV);
		
		Print_3Arg("Customer %d going on wait on CV %d and Lock %d\n\n", myIndex, custLineCV, custLineLock);
		Wait(custLineCV, custLineLock);
		Print_1Arg("Customer %d is out of wait \n\n", myIndex);

	}else	
		Release(customerDataLock);
		
	/* Customer is here because he received a signal from the Order Taker */
	/* - customer has also acquired the custLineLock */

	/* Check for the Order Taker who is WAITING */
	/* - make him as the Order Taker for the current customer */
	/* - Make that Order Taker status as BUSY */
	Acquire(customerDataLock);
	if(GetMV(myOTMVi[myIndex]) == -1){
	
		Print_1Arg("Cust %d searching for the waiting OT\n", myIndex);
		Release(customerDataLock);
		for(index = 1; index <= OT_COUNT; index++){ 
	
			orderTakerStatus = GetMV(orderTakerStatusMVi[index]);
			
			Print_2Arg("OT status (in waiting ot loop) of OT %d is %d\n", index, orderTakerStatus);
			
			if(orderTakerStatus == OT_WAIT){
				
				Print_1Arg("Cust found waiting OT %d\n", index);
				orderTakerStatus = OT_BUSY;
				SetMV(orderTakerStatusMVi[index], orderTakerStatus);
			
				Acquire(customerDataLock);
				
				SetMV(myOTMVi[myIndex], index); 			
				Release(customerDataLock);	
				
				myOT = index;
				break;
				
			}else
				Release(custLineLock);
		}
	}else	
		Release(customerDataLock);	

	inOT = myOT;
	
	Release(custLineLock);
	
	Acquire(custLineLock);
	Acquire(orderTakerLock[inOT]);
	
	/* Send a signal to the Order Taker which indicates that the customer */
	/* is ready to give the order */
	/* $ */
	Print_2Arg("Customer %d signalling the OT %d\n", myIndex, inOT);
	
	Release(custLineLock);
	Signal(orderTakerCV[inOT], orderTakerLock[inOT]);
	
    /* Customer goes on wait to receive a signal from the Order Taker */
	/* which indicates Order Taker is ready to take the order */
	Wait(orderTakerCV[inOT], orderTakerLock[inOT]);
	
	Release(orderTakerLock[inOT]);
	
	/* Customer received a signal from the Order Taker to place an order */
	/* Order is randomly generated and is stored in the customer database */
	Print_2Arg("Customer %d is giving order to OrderTaker %d\n$", myIndex, inOT);
	
	Acquire(customerDataLock);
		
	custDataSixBurger = GetMV(sixBurgerincustDBMVi[myIndex]);
	custDataThreeBurger = GetMV(threeBurgerincustDBMVi[myIndex]);
	custDataVegBurger = GetMV(vegBurgerincustDBMVi[myIndex]);
	custDataFries = GetMV(friesincustDBMVi[myIndex]);
	custDataSoda = GetMV(sodaincustDBMVi[myIndex]);
	custDataDineType = GetMV(dineTypeincustDBMVi[myIndex]);	
	
	custDataDineType = SetMV(dineTypeincustDBMVi[myIndex], 1); 
	custDataDineType = GetMV(dineTypeincustDBMVi[myIndex]);	
	
	
	Print_1Arg("\n\nCUST %d ORDER \n\n\n\n", myIndex);
	if(custDataSixBurger == 1){
		Print_1Arg("Customer %d is ordering 6-dollar burger\n$", myIndex);
	}else{
		Print_1Arg("Customer %d is not ordering 6-dollar burger\n$", myIndex);
	}
	if(custDataThreeBurger == 1){
		Print_1Arg("Customer %d is ordering 3-dollar burger\n$", myIndex);
	}else{
		Print_1Arg("Customer %d is not ordering 3-dollar burger\n$", myIndex);
	}
	if(custDataVegBurger == 1){
		Print_1Arg("Customer %d is ordering veggie burger\n$", myIndex);
	}else{
		Print_1Arg("Customer %d is not ordering veggie burger\n$", myIndex);
	}
	if(custDataFries == 1){
		Print_1Arg("Customer %d is ordering french fries\n$", myIndex);
	}else{
		Print_1Arg("Customer %d is ordering french fries\n$", myIndex);
	}
	if(custDataSoda == 1){
		Print_1Arg("Customer %d is ordering soda\n$", myIndex);
	}else{
		Print_1Arg("Customer %d is not ordering soda\n$", myIndex);
	}
	if(custDataDineType == 1){
		Print_1Arg("Customer %d chooses to to-go the food\n$", myIndex);
	}else{
		Print_1Arg("Customer %d chooses to eat-in the food\n$", myIndex);
	}
	Release(customerDataLock);
	Acquire(orderTakerLock[inOT]);
	
	
	Print_2Arg("Customer %d signalling the OT %d after placing order\n", myIndex, inOT);
	Signal(orderTakerCV[inOT],orderTakerLock[inOT]);
	
	/* wait for the Order Taker to reply once he checks the food availability */
	Wait(orderTakerCV[inOT],orderTakerLock[inOT]);
	
	/* Received a signal from Order Taker which indicates that the order is */
	/* processed and its time to pay money */
	
	/* Send a signal to the Order Taker which indicates that the customer */
	/* has payed the bill amount */ 
	Signal(orderTakerCV[inOT],orderTakerLock[inOT]);
	
	/* Wait for the Order Taker to acknowledge the payment being done */ 
	/* successfully */
	Wait(orderTakerCV[inOT], orderTakerLock[inOT]);
	Release(orderTakerLock[inOT]);

	/* Received a signal from the Order Taker to indicate that the payment */
	/* was completed and the customer can move to the next stage */ 

	/* If to-go then check if the order is bagged right away */
	/* else go and wait for a broadcast of the tokenNo */
	Print_1Arg("custDataDineType = %d\n", custDataDineType);
	
	if(custDataDineType == 1)
	{               
		
		
		/* if to-go and if the order is delivered then the customer takes the */
		/* bag and just leaves the restaurant */
		/* release the Order Taker Lock */
		Acquire(customerDataLock);
		
		
		custDataDelivered = GetMV(deliveredincustDBMVi[myIndex]);
		if(custDataDelivered == 1)
		{
			Release(customerDataLock);	
			/* OG */
			Print_2Arg("Customer %d receives food from the OrderTaker %d\n$",myIndex, inOT);
			/* Acquire the Lock to update the custer count served the food */
			Acquire(custServedLock);
			
			tempV = GetMV(custServedMVi);
			tempV ++;
			SetMV(custServedMVi, tempV);
			
			
			Print_2Arg("\n\nCUSTOMER %d SERVICED IS %d\n\n$", myIndex, tempV); 
			
			Release(custServedLock);
	
			/* if to-go and if the order is not ready then customer has to wait */
			/* for a broadcast signal from the Order Taker when the order is bagged */
		
			/* Acquire the Lock which is used to match with To-go waiting */
			/* condition variable waitingLock */
		}
		else
		{
			Release(customerDataLock);	
			/* OG */
			Print_3Arg("Togo Customer %d is given token number %d by the OrderTaker %d\n$", myIndex, custDataTokenNo, inOT);
			Acquire(waitToGoLock);
				
			/* Go on wait to receive the broadcast signal from the Order Taker */
			Wait(toGoCV,waitToGoLock);
			Release(waitToGoLock);

			while(1)
			{
				/* Received a Broadcast signal from one of the Order Taker */
				/* - Check if the broadcasted tokenNo is customer's tokenNo */
				Acquire(customerDataLock);
				if(custDataDelivered = 1)
				{
					Release(customerDataLock);
					/* Acquire the Lock to update the custer count served the food */
					Acquire(custServedLock);
					
					tempV = GetMV(custServedMVi);
					tempV ++;
					SetMV(custServedMVi, tempV);
			
					Print_2Arg("\n\nCUSTOMER %d SERVICED IS %d\n\n$", myIndex,tempV); 
					Release(custServedLock);
					
					break;
				}else
				{ 
					Release(customerDataLock);
					/* Go on wait to receive the next broadcast signal from the waiter */
					Acquire(waitToGoLock);
					Wait(toGoCV,waitToGoLock);
					Release(waitToGoLock);
				}	
			}
	    }
	}/* if dineType is eat-in */
	else if(custDataDineType == 0)
	{
				/*  OG */
		Print_3Arg("EatinCustomer %d is given token number %d by the OrderTaker %d\n$", myIndex, custDataTokenNo, inOT); 
		/* Acquire a Lock to add customer onto the queue waiting for */
		/* manager's reply */
		Acquire(managerLock);
		/* Increment the customer waiting count */
					
					
		tempV = GetMV(managerLineLengthMVi);
		tempV ++;
		SetMV(managerLineLengthMVi, tempV);
		
		Print_1Arg("Customer setting manager line length to %d\n", tempV);
		
		
		Print_1Arg("Customer %d going on wait till the manager replies\n", myIndex);
		
		/* go on wait till manager signals with table availability */
		Wait(managerCV,managerLock);

		/* Received a signal from manager */
		/* Customer has acquired the manager lock */
		/* Manager replied saying "restaurant is not full" */
		tableAvailable = GetMV(tableAvailableMVi);
		
		if(tableAvailable == 1)
		{		
			/* OG */
			Print_1Arg("Customer %d is informed by the Manager-the restaurant is not full\n$", myIndex);
			
			tableAvailable = 0;
			SetMV(tableAvailableMVi, tableAvailable);
			custDataTableNo = GetMV(tableNoincustDBMVi[myIndex]);

			Release(managerLock);			
			for(index = 0; index < TABLE_COUNT; index++)
			{
				Acquire(tablesDataLock);
				
				tables = GetMV(tablesMVi[index]);
				
				if(tables == 0)
				{
					/* make the first table which is free as the customer */
					/* table */
					
					custDataTokenNo = GetMV(tokenNoincustDBMVi[myIndex]);
					tables = custDataTokenNo;
					SetMV(tablesMVi[index],tables);
					
					Release(tablesDataLock);
					Acquire(customerDataLock);
					
					custDataTableNo = index;
					SetMV(tableNoincustDBMVi[myIndex], custDataTableNo);
								
					Release(customerDataLock);
					/* @ */
					/* Print_3Arg("table %d is taken by custom %d tokenNo %d\n$", index, myIndex, tables);	 */
					break;
				}else
					Release(tablesDataLock);
			}

			/* if a table is found then go sit and wait for food */
			if(custDataTableNo != 0)
			{	
				/* OG	*/
				Print_2Arg("customer %d is seated at table number %d\n$",myIndex, custDataTableNo);
					
				/* Before releasing the tablesDataLock Acquire the */
				/* eatInFoodWaitingLock to ensure that this customer is the */ 
				/* next one to join the queue waiting for the food to be bagged */

				/* customer goes on wait to receive a signal from the waiter */
				/* after the tokenNo is validated */
				while(1)
				{
					Acquire(eatInFoodWaitingLock);
					Wait(eatInFoodWaitingCV,eatInFoodWaitingLock);	
					Release(eatInFoodWaitingLock);
		 			/* upon receiving the signal check if order is delivered */
					Acquire(customerDataLock);
					
					custDataDelivered = GetMV(deliveredincustDBMVi[myIndex]);
					
					if(custDataDelivered == 1)
					{
						/* if delivered then the customer eats the food and */
						/* leaves the restaurant */
						Release(customerDataLock);
						Acquire(tablesDataLock);			
						tables = 0;
						
						SetMV(tablesMVi[custDataTableNo], tables);
						
						Release(tablesDataLock);
						Acquire(eatInWaitingLock);
						Signal(eatInWaitingCV,eatInWaitingLock);
						Release(eatInWaitingLock);	
						/* Acquire the Lock to update the custer count served the food */
						Acquire(custServedLock);
						
						tempV = GetMV(custServedMVi);
						tempV ++;
						SetMV(custServedMVi, tempV);
						/* @ */
						/* Print_2Arg("\n\nCUSTOMER %d SERVICED IS %d\n\n$", myIndex, custServed); */
						Release(custServedLock);
						/* Print_1Arg("Customer %d calling E---X---I---T\n$", myIndex); */
						Exit(0);	
					}
					Release(customerDataLock);
				}
			}

		}
		else if(tableAvailable == 0)
		{
			Release(managerLock);
			/* OG */
			Print_1Arg("Customer %d is informed by the Manager-the restaurant is full\n$", myIndex);
			/* if restaurant is full and no table is available where the */
			/* customer can sit */
				
			/* Acquire the eat-in table waiting lock before releasing the */
			/* tablesDataLock so that customer wont get context switched */
			/* and he can be the next person to join the waiting queue */
				
			/* Release the tables Data lock	 */
			
			Acquire(eatInWaitingLock);	
			/* OG */
			Print_1Arg("customer %d is waiting to sit on the table\n$", myIndex);
				
			/* Customer goes on wait till he receives a signal from */
			/* a seated customer who received his bag and is leaving */
			/* the restaurant */
			Wait(eatInWaitingCV,eatInWaitingLock);
			Release(eatInWaitingLock);
			/* Received a signal from one of the customer leaving */
			/* Acquire the Lock on the tables data to make the */
			/* freed table number as the current customer table */
			Acquire(tablesDataLock);	
		
			for(index = 0; index < TABLE_COUNT; index++)
			{
				
				tables = GetMV(tablesMVi[index]);
				
				if(tables == 0)
				{
					/* make the first table which is free as the customer */
					/* table */
					
					custDataTokenNo = GetMV(tokenNoincustDBMVi[myIndex]);
					tables = custDataTokenNo;
					SetMV(tablesMVi[index],tables);
					
					Release(tablesDataLock);
					Acquire(customerDataLock);
					
					custDataTableNo = index;
					SetMV(tableNoincustDBMVi[myIndex], custDataTableNo);
					
					Release(customerDataLock);
					Acquire(tablesDataLock);
					break;
				}	
			}
			/* Release the tables data lock */
			Release(tablesDataLock);
			Acquire(customerDataLock);
	
			if(custDataTableNo != 0)
			{
				Release(customerDataLock);
		
				/* Before releasing the tablesDataLock Acquire the */
				/* eatInFoodWaitingLock to ensure that this customer is the */
				/* next one to join the queue waiting for the food to be bagged */
				Acquire(eatInFoodWaitingLock);
				/* customer goes on wait to receive a signal from the waiter */
				/* after the tokenNo is validated */	
				/* OG */
				Print_1Arg("Customer %d is waiting for the waiter to serve the food\n$",myIndex);
		
				Wait(eatInFoodWaitingCV,eatInFoodWaitingLock);
					/*potential problem	*/
				/* upon receiving the signal check if order is delivered */
				custDataDelivered = GetMV(deliveredincustDBMVi[myIndex]);
				while(custDataDelivered != 1)
				{				
					/* else Go Back to wait */
					Wait(eatInFoodWaitingCV,eatInFoodWaitingLock);
				}
				Release(eatInFoodWaitingLock);
				/* OG */
				Print_1Arg("Customer %d is served by waiter\n$", myIndex);
				/* if delivered then the customer eats the food and */
				/* leaves the restaurant */
				Print_2Arg("Customer %d is leaving the restaurant after OrderTaker %d packed the food\n$",myIndex, inOT);						
				Acquire(tablesDataLock);			
				
				tables = 0;
				SetMV(tablesMVi[index], tables);
				
				Release(tablesDataLock);
				Acquire(eatInWaitingLock);		
				Signal(eatInWaitingCV,eatInWaitingLock);		
				Release(eatInWaitingLock);
				/* OG */	
				Print_1Arg("EAT-IN  Customer %d is leaving the restaurant after having food\n$",myIndex);				 
				/* Acquire the Lock to update the custer count served the food	*/	
				Acquire(custServedLock);

				tempV = GetMV(custServedMVi);
				tempV ++; 
				SetMV(custServedMVi, tempV);
		
				/* @ */
				/* Print_2Arg("\n\nCUSTOMER %d SERVICED IS %d\n\n$", myIndex, custServed);		 */
				Release(custServedLock);	
				/* Print_1Arg("Customer %d calling E---X---I---T\n$", myIndex);	 */
				Exit(0);
			}
			else
				Release(customerDataLock);
		}
	}	


}


