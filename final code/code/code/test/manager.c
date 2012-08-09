#include"syscall.h"




/* ordertaker info*/

	int indexj = 0;
	int custIndex = 0;			/* to store the cust index who being served 	*/
	int custHasToWait = 0;		/* if to-go and food not ready set this flag	*/
	int billAmount = 0;			/* to store the bill amount payed by customer	*/
	int myIndex = 0;
	/* bag the order function variables*/
	int cannotBeBagged = 0;
	int tempV = -1;
	int temp = 0;
	/* to store the get value */
	int foodToBag = 0;
	int custServed = 0;			
	int custLineLength = 0;	
	int tempDineType = 0;	
	int baggedList = 0;
	/* to stote the get value */
/* ordertaker info*/	


	
	
	int OT_WAIT = 2;
	int OT_FREE = 0;
	int OT_BUSY = 1;
	 
	int CUST_COUNT = 2;
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
	
	Print_Stmt("Calling start simulation\n");
	StartSimulation();
	
	
	startWaitLocki = CreateLock("StartWaitLk", 11, 0);
	startWaitCVi = CreateCV("startWaitCV", 12, 0);
	startWaitMVi = CreateMV("startWaitMV", 11, 0, 0);
	
	Print_3Arg("START INDEX VAL -> lock %d cv %d mv %d\n\n\n\n\n\n\n", startWaitLocki, startWaitCVi, startWaitMVi);
	
	Acquire(startWaitLocki);
	getValueStartWaitMV = GetMV(startWaitMVi);
	
	if(getValueStartWaitMV == 0){
	
		
		getValueStartWaitMV++;
		
		SetMV(startWaitMVi, getValueStartWaitMV);
		
		
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
	readySixBurgerMVi = CreateMV("ready6B", 7, MINCOOKEDFOOD, 0);
	
	/* foodReadyData.threeeBurger */
	readyThreeBurgerMVi = CreateMV("ready3B", 7, MINCOOKEDFOOD, 0);
	
	/* foodReadyData.vegBurger */
	readyVegBurgerMVi = CreateMV("ready6VB", 8, MINCOOKEDFOOD, 0);
	
	/* foodReadyData.fries */
	readyFriesMVi = CreateMV("readyF", 6, MINCOOKEDFOOD, 0);

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
	stopSixBurgerMVi = CreateMV("stop6B", 6, 0, 0);
	stopThreeBurgerMVi = CreateMV("stop3B", 6, 0, 0);
	stopVegBurgerMVi = CreateMV("stopVB", 6, 0, 0);
	stopFriesMVi = CreateMV("stopF", 5, 0, 0);
	
	/* tables[index] */
	for(index = 0; index < TABLE_COUNT; index++)
	{ 	
		tablesMVi[index] = CreateMV("tablesMV", 8, 0, index);
	}
	
	for(index = 1; index <= OT_COUNT; index++)
	{
		/* orderTakerStatus[index] */
		orderTakerStatusMVi[index] = CreateMV("OTstatusMV", 10, OT_BUSY, index);
	
	}
	
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
		
		
		Broadcast(startWaitCVi, startWaitLocki);
		
		getValueStartWaitMV++;
		
		SetMV(startWaitMVi, getValueStartWaitMV);
	
		Release(startWaitLocki);
		
		Manager();
		
	}
	else if(getValueStartWaitMV == 1)
	{
		Wait(startWaitCVi, startWaitLocki);
		
		Release(startWaitLocki);
		
		Manager();
	}
	else
	{
		Release(startWaitLocki);
		
		Manager();
	}
	

	/* ------------------------------------------------------------------------------ */

}



void Manager() {

while(1)
	{
		/* Check if all the Customers are serviced*/
		/* @ */
		/* Print_Stmt("Manager checking inventory\n$"); */
		Acquire(custServedLock);
		Print_1Arg("\n\n\n cust served mv value %d\n\n\n", GetMV(custServedMV));
		if((GetMV(custServedMV)) == CUST_COUNT)
		{
			Release(custServedLock);
			
			/* Waking up all the waiters and telling them to go Home*/
			Acquire(waiterSleepLock);
			Broadcast(waiterSleepCV,waiterSleepLock);
			Release(waiterSleepLock);
			
			/* Waking up all the Cooks and telling them to go Home*/
			Acquire(wakeUpCookLock);
			Broadcast(wakeUpCookCV, wakeUpCookLock);
			Release(wakeUpCookLock);
			
			/* Signal all the ordertakers and tell them to go home*/
			for(index = 0; index < OT_COUNT; index++)
			{
				Acquire(orderTakerLock[index]);
				Signal(orderTakerCV[index],orderTakerLock[index]);
				Release(orderTakerLock[index]);
			}
			/* @ */
			Print_Stmt("\n\n\t-------------------------------------------------------------\n");
			Print_Stmt("\n\tCARL'S JR RESTAURANT SIMULATION COMPLETED SUCCESSFULLY\n$");
			Print_Stmt("\n\t-------------------------------------------------------------\n\n");
			Exit(0);
		}
		Release(custServedLock);
	
		/* Acquire the Inventory Lock */
		Acquire(inventoryLock);
		/* Ckeck the inventory Level*/
		Print_1Arg("\n\n\n inventory mv value %d\n\n\n", GetMV(inventoryMV));
		if((GetMV(inventoryMV)) <= MININVENTORY)
		{
			/* take a Lock to access the common location where all the money*/
			/* is stored by the OrderTakers*/
			Release(inventoryLock);
			Acquire(moneyAtRestaurantLock);
			moneyAtRestaurantMV = GetMV(moneyAtRestaurantMV);
			
			if(moneyAtRestaurantMV < inventoryRefillCost)
			{
				/* Take all the money in the restaurant*/
				SetMV(moneyAtRestaurantMV, 0);
				
				/* Manager goes to bank to withdraw the remaining money*/
				/* This process of going to bank and withdrawing will take*/
				/* a minimum of five times the time */
				/* OG*/
				Print_Stmt("Manager goes to bank to withdraw the cash\n$");
				Release(moneyAtRestaurantLock);
				for(index = 1; index <= timeToGoToBank; index++)
				{
					Yield();
				}					
				Acquire(moneyAtRestaurantLock);
				/* OG*/
				Print_Stmt("moneyAtRestautant is loaded in the restaurant\n$");
			}		
			tempV = GetMV(moneyAtRestaurantMV) - inventoryRefillCost;
			Print_1Arg("\n\n\n\n After purchase amount %d\n\n\n", tempV);
			SetMV(moneyAtRestaurantMV ,tempV);
			/*moneyAtRestaurant = moneyAtRestaurant - inventoryRefillCost;									*/
			Release(moneyAtRestaurantLock);
			/* Once He comes Back from the Bank The Inventory will be filled*/
			Acquire(inventoryLock);
				/*inventory = FILLED;*/
				SetMV(inventoryMV, FILLED);
			/* OG */
			Print_Stmt("Manager refills the inventory\n$");
			Release(inventoryLock);
		}
		else
			Release(inventoryLock);
	
				tablesFree = 0;
		/* Manager Interaction With The Customer*/
		Acquire(managerLock);
		managerLineLengthMV = GetMV(managerLineLengthMV);
		if(managerLineLengthMV > 0)
		{
			tempV = GetMV(managerLineLengthMV) - 1;
			SetMV(managerLineLengthMV, tempV);
			Release(managerLock);
			/* Acquire the tables data lock to check the tables free*/
			Acquire(tablesDataLock);
			/* Find the Number of Tables available free*/
			for(index = 0; index < TABLE_COUNT; index++) 
			{
				if(GetMV(tablesMV[index]) == 0)
				{
					tablesFree = 1;
					break;
				}
			}
			Release(tablesDataLock);
			Acquire(managerLock);
			if(tablesFree == 1)
			{
				tableAvailableMV = GetMV(tableAvailableMV);
				while(tableAvailableMV != 0)
				{
					Release(managerLock);
					Yield();
					Acquire(managerLock);
				}
				SetMV(tableAvailableMV, 1);	
				Signal(managerCV,managerLock);
			}
			else
			{
				tableAvailableMV = GetMV(tableAvailableMV);
				while(tableAvailableMV != 0)
				{
					Release(managerLock);
					Yield();
					Acquire(managerLock);
				}
				/*tableAvailable = 0;*/
				SetMV(tableAvailableMV, 0);	
				Signal(managerCV,managerLock);
			}				
		} 
		Release(managerLock);
	
	
				/* Manager interaction with the cook*/
		
		/* Acquire the ready food and food to be cooked locks so that the manager*/
		/* can decide whether to make a cook go on break or bring back a cook on*/
		/* break or to hire a new cook*/
		Acquire(foodToBeCookedDBLock);
		
		foodRequired[SIXBURGER] = GetMV(toBeCookedSixburgerMV);
		foodRequired[THREEBURGER] = GetMV(toBeCookedThreeburgerMV);
		foodRequired[VEGBURGER] = GetMV(toBeCookedVegburgerMV);
		foodRequired[FRIES] = GetMV(toBeCookedFriesMV);
		Release(foodToBeCookedDBLock);
		
		Acquire(foodReadyDBLock);
		
		foodRequired[SIXBURGER] = (foodRequired[SIXBURGER] - (GetMV(readySixBurgerMV) - MINCOOKEDFOOD));
		foodRequired[THREEBURGER] = (foodRequired[THREEBURGER] - (GetMV(readyThreeBurgerMV)  - MINCOOKEDFOOD));
		foodRequired[VEGBURGER] = (foodRequired[VEGBURGER] - (GetMV(readyVegBurgerMV) - MINCOOKEDFOOD));
		foodRequired[FRIES] = (foodRequired[FRIES] - (GetMV(readyFriesMV) - MINCOOKEDFOOD));
		Release(foodReadyDBLock);

		/* For all the items in the restaurant*/
		for(index = 1; index < 5; index ++)
		{
			/* if foodToBeCooked is  a little greater than the foodready*/
			if((foodRequired[index] <= MAXDIFF) && (foodRequired[index] > 0))
			{
				/* Reset the stop cooking flag for this item if it is set*/
				if(index == SIXBURGER)
				{ 
					Acquire(stopSixBurgerLock);
					SetMV(stopSixBurgerMV, 0);
					Release(stopSixBurgerLock);
				}
				else if(index == THREEBURGER)
				{   
					Acquire(stopThreeBurgerLock);
					SetMV(stopThreeBurgerMV, 0);
					Release(stopThreeBurgerLock);
				}
				else if(index == VEGBURGER)
				{
					Acquire(stopVegBurgerLock);
					SetMV(stopVegBurgerMV, 0);
					Release(stopVegBurgerLock);
				}
				else if(index == FRIES)
				{
					Acquire(stopFriesLock);
					SetMV(stopFriesMV, 0);
					Release(stopFriesLock);
				}
				/* if there are no working cooks for this item*/
				Acquire(workingCookLock);
				if(GetMV(workingCooksMV[index]) == 0)
				{
					Release(workingCookLock);
					/* if there are no cooks on break*/
					Acquire(cooksOnBreakCountLock);
					/* if cooks on break count is 0 then hire a new cook */
					if(GetMV(cooksOnBreakCountMV) == 0)
					{
						Release(cooksOnBreakCountLock);
						/* Then Create a new Cook*/
						Acquire(whatToCookFirstLock);   
															
						while(GetMV(whatToCookFirstMV) != 0)
						{
							Release(whatToCookFirstLock);
							Yield();
							Acquire(whatToCookFirstLock);
						}
						SetMV(whatToCookFirstMV, index);
						Release(whatToCookFirstLock);
						if(cookCount < COOK_COUNT)
						{
							Exec("../test/cook-d", 14);
							cookCount++;
							Acquire(workingCookLock);
							tempV = GetMV(workingCooksMV[index]) + 1;
							SetMV(workingCooksMV[index], tempV);
							Release(workingCookLock);
						}
					}
					else{
						tempV = GetMV(cooksOnBreakCountMV) - 1;
						SetMV(cooksOnBreakCountMV, tempV);
						Release(cooksOnBreakCountLock);
						/* bring back the cook on break*/
						/* Acquire the what to cook next lock and tell the cook*/
						/* who is back from the break "what to cook next"*/
						Acquire(whatToCookNextLock);
						/* make what to cook next as the current food item*/
						while(GetMV(whatToCookNextMV) != 0)
						{
							Release(whatToCookNextLock);
							Yield();
							Acquire(whatToCookNextLock);
						}
						SetMV(whatToCookNextMV, index);
						/* release the lock after setting the value*/
						Release(whatToCookNextLock);
				
						Acquire(wakeUpCookLock);
						Signal(wakeUpCookCV, wakeUpCookLock);
						Release(wakeUpCookLock);
						/* increment working cooks for this item by 1*/
						Acquire(workingCookLock);
						tempV = GetMV(workingCooksMV[index]) + 1;
						SetMV(workingCooksMV[index], tempV);
						Release(workingCookLock);
					}		
				}
				else 
					Release(workingCookLock);
			}	
			/* else if the food required is greater than the maximum difference*/ 
			/* that can be handled by the current cooks*/
			else if((foodRequired[index] > MAXDIFF))
			{
				/* Reset the stop cooking flag for this item if it is set */
				if(index == SIXBURGER)
				{
					Acquire(stopSixBurgerLock);
					SetMV(stopSixBurgerMV, 0);
					Release(stopSixBurgerLock);
				}
				else if(index == THREEBURGER)
				{
					Acquire(stopThreeBurgerLock);
					SetMV(stopThreeBurgerMV, 0); 
					Release(stopThreeBurgerLock);
				}
				else if(index == VEGBURGER)
				{
					Acquire(stopVegBurgerLock);
					SetMV(stopVegBurgerMV, 0);
					Release(stopVegBurgerLock);
				}
				else if(index == FRIES)
				{
					Acquire(stopFriesLock);
					SetMV(stopFriesMV, 0);					
					Release(stopFriesLock);
				}
				
				/* if no cooks are on break */
				Acquire(cooksOnBreakCountLock);
				/* if cooks on break count is 0 then hire a new cook */
				if(GetMV(cooksOnBreakCountMV)  == 0)
				{
					Release(cooksOnBreakCountLock);
					/* if a new cook can be created*/
					if(cookCount < COOK_COUNT)
					{
						/* Create a new Cook*/
						Acquire(whatToCookFirstLock);
						while(GetMV(whatToCookFirstMV) != 0)
						{
							Release(whatToCookFirstLock);
							Yield();
							Acquire(whatToCookFirstLock);
						}
						SetMV(whatToCookFirstMV, index);
						Release(whatToCookFirstLock);
						Exec("cook-d", 14);
						
						cookCount++;
						Acquire(workingCookLock);
						tempV = GetMV(workingCooksMV[index]) + 1;
						SetMV(workingCooksMV[index], tempV);
						Release(workingCookLock);
					}
				}
				else{
					tempV = GetMV(cooksOnBreakCountMV);
					SetMV(cooksOnBreakCountMV, tempV);
					
					Release(cooksOnBreakCountLock);
					/* bring back the cook on break*/
					/* Acquire the what to cook next lock and tell the cook*/
					/* who is back from the break "what to cook next"*/
					Acquire(whatToCookNextLock);
					/* make what to cook next as the current food item*/
					while(GetMV(whatToCookNextMV) != 0)
					{
						Release(whatToCookNextLock);
						Yield();
						Acquire(whatToCookNextLock);
					}
					SetMV(whatToCookNextMV, index);
					/* release the lock after setting the value*/
					Release(whatToCookNextLock);
					/* call the cook on break back to work*/ 
					Acquire(wakeUpCookLock);
					Signal(wakeUpCookCV,wakeUpCookLock);
					Release(wakeUpCookLock);

					/* increment working cooks for this item by 1*/
					Acquire(workingCookLock);
					tempV = GetMV(workingCooksMV[index]) + 1;
					SetMV(workingCooksMV[index], tempV);
					
					Release(workingCookLock);
				}
			}/* if the food ready is greater than the required then */
			/* set the stop cooking monitor for that item */
			else if((foodRequired[index] < 0))
			{
				if(index == SIXBURGER)
				{
					Acquire(stopSixBurgerLock);
					SetMV(stopSixBurgerMV, 1);
					Release(stopSixBurgerLock);
				}
				else if(index == THREEBURGER)
				{
					Acquire(stopThreeBurgerLock);
					SetMV(stopThreeBurgerMV, 1);
					Release(stopThreeBurgerLock);
				}
				else if(index == VEGBURGER)
				{
					Acquire(stopVegBurgerLock);
					SetMV(stopVegBurgerMV, 1);
					Release(stopVegBurgerLock);
				}
				else if(index == FRIES)
				{
					Acquire(stopFriesLock);
					SetMV(stopFriesMV, 1);
					Release(stopFriesLock);
					
				}
			}	
	

		}/* for all the items*/
		
		
		Acquire(foodBaggedListLock);
		baggedListNotEmpty = 0;
		for(indexj = 0; indexj < CUST_COUNT; indexj++)
		{
			if(GetMV(baggedListMV[indexj]) != 0)
			{
				baggedListNotEmpty = 1;
				break;
			}
		}
		Release(foodBaggedListLock);
		
		if(baggedListNotEmpty == 1 )
		{
			/* Manager Acquires Lock and signals the waiters waiting */
			Acquire(waiterSleepLock);
			Broadcast(waiterSleepCV,waiterSleepLock);
			/* OG */
			Print_Stmt("Manager calls back all Waiters from break\n");
			Release(waiterSleepLock);
		}	
		
		Yield();
	
	}	


}

