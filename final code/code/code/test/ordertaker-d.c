#include "syscall.h"

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
	
	
	
	/* Print_Stmt("after creating locks\n"); */
	
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
	
/* 	for(index = 1; index <= 4; index++)
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
	Print_Stmt("OT INITIALISATION END\n");

	 OrderTaker(); 

	/* ------------------------------------------------------------------------------ */

}

void OrderTaker() {

	
	/* Get the Index for this Customer before starting the order */
	Acquire(nextOrderTakerIndexLock);
	
	myIndex = GetMV(nextOrderTakerIndexMVi);
	
	Print_1Arg("\n\n\nOT my index = %d\n", myIndex);
	
	tempV = myIndex + 1;
	SetMV(nextOrderTakerIndexMVi, tempV);
	
	Release(nextOrderTakerIndexLock);
	
	
	/* -------------now -----------------------*/
	
	while(1)
	{
		Print_Stmt("OT in while\n");
		
		/* Acquire custLineLock to access the custLineLength monitor variable	*/
		Acquire(custLineLock);
		/* Check for any customers waiting in the customer waiting line	*/
		
		custLineLength = GetMV(custLineLengthMVi);
		
		if(custLineLength > 0)
		{
			
			Print_1Arg("OT custLineLength > 0 = %d\n", custLineLength);

			/* Signal a customer which indicates the customer to come out of	*/
			/* the waiting line and give his order	*/
			Signal(custLineCV, custLineLock);
			
			/* Decrement the custLineLength moniter variable value by 1	*/
			custLineLength--;
			SetMV(custLineLengthMVi, custLineLength);
			
			SetMV(orderTakerStatusMVi[myIndex], OT_WAIT);
		}
			
		
		else	
		{		
			Print_1Arg("OT custLineLength <= 0 = %d\n", custLineLength);
			
			SetMV(orderTakerStatusMVi[myIndex], OT_FREE);
		
			Print_3Arg("OT custLineLength <= 0 = %d  so making him FREE and going on wait with CV = %d and lock = %d\n", custLineLength, custLineCV, custLineLock);
			Wait(custLineCV, custLineLock);
		}
				
			/* Acquire the orderTakerLock before releasing the custLineLock 	*/
			/* - To ensure that the Customer acquires the orderTakerLock 	*/  
			/*   after Order Taker does	*/
			Acquire(orderTakerLock[myIndex]);
			
			
			/*Release the custLineLock 	*/
			/* - so that the Customer can access the updated status of the Order Taker	*/
		    Release(custLineLock);
			
			/* Order Taker goes on wait to receive a signal from the Customer 	*/
			/* which indicates Customer is ready to place the order	*/
			Print_1Arg("Ordertaker %d is making himself OT_WAIT and going on WAIT\n\n", myIndex);
			Wait(orderTakerCV[myIndex], orderTakerLock[myIndex]);
			
			Print_1Arg("Ordertaker %d is back from  WAIT\n\n\n", myIndex);
		 	Release(orderTakerLock[myIndex]);
			
			Acquire(custServedLock);
			if(GetMV(custServedMVi) == CUST_COUNT){
				Release(custServedLock);
				Print_1Arg("All served so OrderTaker %d calling E---X---I---T\n$", myIndex); 
				continue;
			}
			Release(custServedLock);

			Acquire(orderTakerLock[myIndex]);
			
			/* Send a signal to the Customer which indicates that the Order Taker	*/
			/* is ready to take the order	*/
			Signal(orderTakerCV[myIndex], orderTakerLock[myIndex]);
			
			/* Order Taker will go on wait till he receives a signal from the 	*/
			/* customer after placing the order	*/
			Wait(orderTakerCV[myIndex], orderTakerLock[myIndex]);
			
			Release(orderTakerLock[myIndex]);
			
			Acquire(custServedLock);
			if(GetMV(custServedMVi) == CUST_COUNT){
				Release(custServedLock);
				
				continue;
			}
			Release(custServedLock);		
		
			/* Received a signal from the customer after placing an order 	*/
			/* Find the customer index once the order is placed	*/
			Acquire(customerDataLock);
			for( index = 1; index <= CUST_COUNT; index++){
				/* find the customer index by finding the customer who has their Order	*/
				/* taker index as the current Order taker Index Thread	*/
				if(GetMV(myOTMVi[index]) == myIndex){
					if(GetMV(orderedincustDBMVi[index]) != 1){
						custIndex = index;
						SetMV(orderedincustDBMVi[index], 1);
						break;
					}
				}
			}
			Release(customerDataLock);

			/* OG	*/
			Print_2Arg("OrderTaker %d is taking order of Customer %d\n$", myIndex, custIndex);
			
			/* if to-go customer then  - to check if the food is ready right away	*/
			/*						   - if not ready then make customer wait	*/
			/*						   - modify the food required monitor variable	*/

			
			
			
			
			
			
			
			
			
			
			
			
			/* 			if(GetMV(dineTypeincustDBMVi[custIndex]) == 1) */

			if(1)
			{
			
				custHasToWait = 0;
				/* if 6$burger is ordered*/
				Acquire(customerDataLock);
				if(GetMV(sixBurgerincustDBMVi[custIndex]) == 1)
				{
					Release(customerDataLock);
					/* if 6$burger is ready	*/
					Acquire(foodReadyDBLock);
					
					if(GetMV(readySixBurgerMVi) != 0)
					{
					
						Release(foodReadyDBLock);
						/*reserve a burger for this customer	*/
						Acquire(customerDataLock);
						SetMV(sixBurgerincustDBMVi[custIndex],2);
						Release(customerDataLock);
						/* if greater than minimum amount of cooked food	*/
						/* then bag it, no worries	*/
						Acquire(foodReadyDBLock);
						
						if(GetMV(readySixBurgerMVi) > MINCOOKEDFOOD)
						{
							tempV = GetMV(readySixBurgerMVi) - 1;
							SetMV(readySixBurgerMVi, tempV);
							Release(foodReadyDBLock);
							Acquire(foodToBeCookedDBLock);
							tempV = GetMV(toBeCookedSixburgerMVi) - 1;
							SetMV(toBeCookedSixburgerMVi, tempV);
			
							Release(foodToBeCookedDBLock);
						}
						else{
							/* else if it is less than minimum then 	*/
							/* bag it and increment the food to be cooked	*/
							tempV = GetMV(readySixBurgerMVi) - 1;
							SetMV(readySixBurgerMVi, tempV);

							Release(foodReadyDBLock);
							/* Do not decrement the foodtoBeCookedData because the	*/
							/* amount of cooked food is less than minimum	*/
						}
					}	
					else{ /* customer has to wait	*/
						custHasToWait = 1;
						Release(foodReadyDBLock);
					}
				}
				else
					Release(customerDataLock);
	
				
				/* if 3$burger is ordered	*/
				Acquire(customerDataLock);
				if(GetMV(threeBurgerincustDBMVi[custIndex]) == 1)
				{
					Release(customerDataLock);
					/* if 3$burger is ready	*/
					Acquire(foodReadyDBLock);
					
					if(GetMV(readyThreeBurgerMVi) != 0)
					{
						Release(foodReadyDBLock);
						/*reserve a burger for this customer	*/
						Acquire(customerDataLock);
						SetMV(threeBurgerincustDBMVi[custIndex],2);
						Release(customerDataLock);
						
						/* if greater than minimum amount of cooked food*/
						/* then bag it, no worries	*/
						Acquire(foodReadyDBLock);
						
						if(GetMV(readyThreeBurgerMVi) > MINCOOKEDFOOD)
						{
							tempV = GetMV(readyThreeBurgerMVi) - 1;
							SetMV(readyThreeBurgerMVi, tempV);
							Release(foodReadyDBLock);
							Acquire(foodToBeCookedDBLock);
							
							tempV = GetMV(toBeCookedThreeburgerMVi) - 1;
							SetMV(toBeCookedThreeburgerMVi, tempV);

							Release(foodToBeCookedDBLock);
						}
						
						/* else if it is less than minimum then 	*/
						/* bag it and do not decrement the food to be cooked	*/
						else{
							tempV = GetMV(readyThreeBurgerMVi) - 1;
							SetMV(readyThreeBurgerMVi, tempV);
							Release(foodReadyDBLock);
							/* Do not decrement the foodtoBeCookedData because the	*/
							/* amount of cooked food is less than minimum	*/						
						}						
					}	
					else{ /* customer has to wait	*/
							custHasToWait = 1;
							Release(foodReadyDBLock);
					}
				}
				else
					Release(customerDataLock);
				
				/* if vegburger is ordered	*/
				Acquire(customerDataLock);
				if(GetMV(vegBurgerincustDBMVi[custIndex]) == 1){
					Release(customerDataLock);
					/* if vegburger is ready	*/
					Acquire(foodReadyDBLock);
					if(GetMV(readyVegBurgerMVi) != 0){
					Release(foodReadyDBLock);
						/*reserve a burger for this customer*/
						Acquire(customerDataLock);
						SetMV(vegBurgerincustDBMVi[custIndex], 2);
						Release(customerDataLock);
						/* if greater than minimum amount of cooked food	*/
						/* then bag it, no worries	*/
						Acquire(foodReadyDBLock);
						if(GetMV(readyVegBurgerMVi) > MINCOOKEDFOOD){
							tempV = GetMV(readyVegBurgerMVi) - 1;
							SetMV(readyVegBurgerMVi, tempV);
							
							Release(foodReadyDBLock);
							Acquire(foodToBeCookedDBLock);
							tempV = GetMV(toBeCookedVegburgerMVi) - 1;
							SetMV(toBeCookedVegburgerMVi, tempV);
							Release(foodToBeCookedDBLock);
						}
						/* else if it is less than minimum then 	*/
						/* bag it and do not decrement the food to be cooked	*/
						else{
							tempV = GetMV(readyVegBurgerMVi) - 1;
							SetMV(readyVegBurgerMVi, tempV);
							Release(foodReadyDBLock);
							/* Do not decrement the foodtoBeCookedData because the	*/
							/* amount of cooked food is less than minimum	*/
						}	
					}
					else {/* customer has to wait	*/
						custHasToWait = 1;
						Release(foodReadyDBLock);
					}
				}
				else
					Release(customerDataLock);
				
				/* if fries is ordered	*/
				Acquire(customerDataLock);
				if(GetMV(friesincustDBMVi[custIndex]) == 1){
				Release(customerDataLock);
					/* if fries is ready	*/
					Acquire(foodReadyDBLock);
					if(GetMV(readyFriesMVi) != 0){
					Release(foodReadyDBLock);
						/*reserve a fries for this customer	*/
						Acquire(customerDataLock);
						SetMV(friesincustDBMVi[custIndex], 2);
						Release(customerDataLock);
						/* if greater than minimum amount of cooked food	*/
						/* then bag it, no worries	*/
						Acquire(foodReadyDBLock);
						if(GetMV(readyFriesMVi) > MINCOOKEDFOOD){
							tempV = GetMV(readyFriesMVi) - 1;
							SetMV(readyFriesMVi, tempV);
							Release(foodReadyDBLock);
							Acquire(foodToBeCookedDBLock);
							tempV = GetMV(toBeCookedFriesMVi) - 1;
							SetMV(toBeCookedFriesMVi, tempV);
							Release(foodToBeCookedDBLock);
						}
						/* else if it is less than minimum then 	*/
						/* bag it and do not decrement the food to be cooked	*/
						else{
							tempV = GetMV(readyFriesMVi) - 1;
							SetMV(readyFriesMVi, tempV);
							Release(foodReadyDBLock);							
							/* Do not decrement the foodtoBeCookedData because the	*/
							/* amount of cooked food is less than minimum	*/
						}		
					}
					else{ /* customer has to wait	*/
						custHasToWait = 1;
						Release(foodReadyDBLock);
					}
				}
				else 
					Release(customerDataLock);
	
				/* if to-go and if customer has to wait	*/
				if(custHasToWait == 1){
					/* set the token number for the customer	*/
					/* Acquire the lock to get the next token number to be given 	*/
					/* to the customer, all Order Takers access this value	*/
					/* - increment this monitor variable to generate the new unique	*/
					/*   token number and give it to customer	*/
					Acquire(nextTokenNumberLock);
					tempV = GetMV(nextTokenNumberMVi) + 1;
					SetMV(nextTokenNumberMVi,tempV);
					SetMV(tokenNoincustDBMVi[custIndex],GetMV(nextTokenNumberMVi));
					
					/* Release the lock after obtaining the new token number	*/
					Release(nextTokenNumberLock);
					/* Acquire the foodToBagLock to increment the foodToBag count	*/
					Acquire(foodToBagLock);
					tempV = GetMV(foodToBagMVi) + 1;
					SetMV(foodToBagMVi, tempV);
					/* Release the foodToBagLock after updating the foodToBag count	*/
					Release(foodToBagLock);
					
					/* OG	*/
					Print_3Arg("OrderTaker %d gives token number %d to TOGO Customer %d\n$", myIndex, GetMV(tokenNoincustDBMVi[custIndex]), custIndex);
				}else{
					/* food is ready and can be bagged	*/
					/* Even if the customer has ordered only soda	*/
					/* it is bagged here	*/
					/* Order is bagged so update the food to bag count
	
/*CHANGE============*/	Acquire(BagTheOrdersLock);					
					/* make the customer order status as bagged	*/
					SetMV(baggedincustDBMVi[custIndex], 1);
/*CHANGE ============*/	Release(BagTheOrdersLock);					
/*CHANGE============*/	Acquire(customerDataLock);
					SetMV(deliveredincustDBMVi[custIndex], 1);

					/* OG	*/
					Print_2Arg("OrderTaker %d packs and gives food to TOGOCustomer %d\n$", myIndex, custIndex);
					/* Release the foodToBagLock after updating the foodToBag count	*/
					Release(customerDataLock);
					/* Order Taker will bag only one order at a time so break	*/
				}
			}	/* dineType = to-go		*/

			if(GetMV(dineTypeincustDBMVi[custIndex]) == 0){
				custHasToWait = 0;
				/* always customer has to wait, he will not get food right away	*/
				/* set the token number for the customer	*/
				/* Acquire the lock to get the next token number to be given 	*/
				/* to the customer, all Order Takers access this value	*/
				/* - increment this monitor variable to generate the new unique	*/
				/*   token number and give it to customer	*/
				Acquire(nextTokenNumberLock);
				tempV = GetMV(nextTokenNumberMVi) + 1;
				SetMV(nextTokenNumberMVi,tempV);
				SetMV(tokenNoincustDBMVi[custIndex],GetMV(nextTokenNumberMVi));
				/* Release the lock after obtaining the new token number	*/
				/* OG	*/
				Print_3Arg("OrderTaker %d gives token number %d to EATINCustomer %d\n$", myIndex, GetMV(tokenNoincustDBMVi[custIndex]), custIndex);
				Release(nextTokenNumberLock);

				/* eat-in customer always waits	*/			
	
				/* Acquire the foodToBagLock to increment the foodToBag count	*/
				/* Release the foodToBagLock after updating the foodToBag count	*/
				
				/* Order taker has to update the foodToBeCooked & foodReady database	*/
				/* if 6$burger is ordered	*/
				Acquire(customerDataLock);
				
				if(GetMV(sixBurgerincustDBMVi[custIndex] == 1))
				{
					Release(customerDataLock);
					/* if 6$burger is ready	*/
					Acquire(foodReadyDBLock);
					if(GetMV(readySixBurgerMVi) != 0){
					Release(foodReadyDBLock);
						/*reserve a burger for this customer	*/
						Acquire(customerDataLock);
						SetMV(sixBurgerincustDBMVi[custIndex],2);
						Release(customerDataLock);
						/* if greater than minimum amount of cooked food	*/
						/* then bag it, no worries	*/
						Acquire(foodReadyDBLock);
						if(GetMV(readySixBurgerMVi) > MINCOOKEDFOOD){
							tempV = GetMV(readySixBurgerMVi) - 1;
							SetMV(readySixBurgerMVi, tempV);
							Release(foodReadyDBLock);
							Acquire(foodToBeCookedDBLock);
							tempV = GetMV(toBeCookedSixburgerMVi) - 1;
							SetMV(toBeCookedSixburgerMVi, tempV);
							Release(foodToBeCookedDBLock);
						}
						/* else if it is less than minimum then 	*/
						/* bag it and increment the food to be cooked	*/
						else{
							tempV = GetMV(readySixBurgerMVi) - 1;
							SetMV(readySixBurgerMVi, tempV);
							Release(foodReadyDBLock);
							/* Do not decrement the foodtoBeCookedData because the	*/
							/* amount of cooked food is less than minimum	*/
						}
					}
	 				else 
						custHasToWait = 1;
						Release(foodReadyDBLock);
				}
				else
				Release(customerDataLock);
			
				/* if 3$burger is ordered	*/
				Acquire(customerDataLock);
				if(GetMV(threeBurgerincustDBMVi[custIndex]) == 1){
					Release(customerDataLock);
					/* if 3$burger is ready	*/
					Acquire(foodReadyDBLock);
					if(GetMV(readyThreeBurgerMVi) != 0){
						Release(foodReadyDBLock);
						/*reserve a burger for this customer	*/
						Acquire(customerDataLock);
						SetMV(threeBurgerincustDBMVi[custIndex],2);
						Release(customerDataLock);
						/* if greater than minimum amount of cooked food	*/
						/* then bag it, no worries	*/
						Acquire(foodReadyDBLock);
						if(GetMV(readyThreeBurgerMVi) > MINCOOKEDFOOD){
						tempV = GetMV(readyThreeBurgerMVi) - 1;
						SetMV(readyThreeBurgerMVi, tempV);
						Release(foodReadyDBLock);
						Acquire(foodToBeCookedDBLock);
						
						tempV = GetMV(toBeCookedThreeburgerMVi) - 1;
						SetMV(toBeCookedThreeburgerMVi, tempV);

						Release(foodToBeCookedDBLock);
						}
						/* else if it is less than minimum then 	*/
						/* bag it and do not decrement the food to be cooked	*/
						else{
						tempV = GetMV(readyThreeBurgerMVi) - 1;
						SetMV(readyThreeBurgerMVi, tempV);
							Release(foodReadyDBLock);
							/* Do not decrement the foodtoBeCookedData because the	*/
							/* amount of cooked food is less than minimum	*/						
						}						
					}	
					else{ /* customer has to wait	*/
							custHasToWait = 1;
							Release(foodReadyDBLock);
					}
				}
				else
					Release(customerDataLock);
					
				/* if vegburger is ordered	*/
				Acquire(customerDataLock);
				if(GetMV(vegBurgerincustDBMVi[custIndex]) == 1){
					Release(customerDataLock);
					/* if vegburger is ready	*/
					Acquire(foodReadyDBLock);
					if(GetMV(readyVegBurgerMVi) != 0){
					Release(foodReadyDBLock);
						/*reserve a burger for this customer*/
						Acquire(customerDataLock);
						SetMV(vegBurgerincustDBMVi[custIndex], 2);
						Release(customerDataLock);
						/* if greater than minimum amount of cooked food	*/
						/* then bag it, no worries	*/
						Acquire(foodReadyDBLock);
						if(GetMV(readyVegBurgerMVi) > MINCOOKEDFOOD){
							tempV = GetMV(readyVegBurgerMVi) - 1;
							SetMV(readyVegBurgerMVi, tempV);
							
							Release(foodReadyDBLock);
							Acquire(foodToBeCookedDBLock);
							tempV = GetMV(toBeCookedVegburgerMVi) - 1;
							SetMV(toBeCookedVegburgerMVi, tempV);
							Release(foodToBeCookedDBLock);
						}
						/* else if it is less than minimum then 	*/
						/* bag it and do not decrement the food to be cooked	*/
						else{
							tempV = GetMV(readyVegBurgerMVi) - 1;
							SetMV(readyVegBurgerMVi, tempV);
							Release(foodReadyDBLock);
							/* Do not decrement the foodtoBeCookedData because the	*/
							/* amount of cooked food is less than minimum	*/
						}	
					}
					else {/* customer has to wait	*/
						custHasToWait = 1;
						Release(foodReadyDBLock);
					}
				}
				else
					Release(customerDataLock);
				
				/* if fries is ordered	*/
				Acquire(customerDataLock);
				if(GetMV(friesincustDBMVi[custIndex]) == 1){
				Release(customerDataLock);
					/* if fries is ready	*/
					Acquire(foodReadyDBLock);
					if(GetMV(readyFriesMVi) != 0){
					Release(foodReadyDBLock);
						/*reserve a fries for this customer	*/
						Acquire(customerDataLock);
						SetMV(friesincustDBMVi[custIndex], 2);
						Release(customerDataLock);
						/* if greater than minimum amount of cooked food	*/
						/* then bag it, no worries	*/
						Acquire(foodReadyDBLock);
						if(GetMV(readyFriesMVi) > MINCOOKEDFOOD){
							tempV = GetMV(readyFriesMVi) - 1;
							SetMV(readyFriesMVi, tempV);
							Release(foodReadyDBLock);
							Acquire(foodToBeCookedDBLock);
							tempV = GetMV(toBeCookedFriesMVi) - 1;
							SetMV(toBeCookedFriesMVi, tempV);
							Release(foodToBeCookedDBLock);
						}
						/* else if it is less than minimum then 	*/
						/* bag it and do not decrement the food to be cooked	*/
						else{
							tempV = GetMV(readyFriesMVi) - 1;
							SetMV(readyFriesMVi, tempV);
							Release(foodReadyDBLock);							
							/* Do not decrement the foodtoBeCookedData because the	*/
							/* amount of cooked food is less than minimum	*/
						}		
					}
					else{ /* customer has to wait	*/
						custHasToWait = 1;
						Release(foodReadyDBLock);
					}
				}
				else 
					Release(customerDataLock);

				if (custHasToWait = 0){
/*CHANGE ============*/	Acquire(BagTheOrdersLock);
					SetMV(baggedincustDBMVi[custIndex],1);
/*CHANGE============*/	Release(BagTheOrdersLock);
					Acquire(foodBaggedListLock);
					for(indexj = 0; indexj < CUST_COUNT; indexj++){	
						if(GetMV(baggedListMVi[indexj]) == 0){
						SetMV(baggedListMVi[indexj], GetMV(tokenNoincustDBMVi[index]));
						break;
						}
					}
						Print_3Arg("Ordertaker %d gives Token number %d to Waiter for Customer %d\n$", myIndex, GetMV(tokenNoincustDBMVi[index]), index);
						Release(foodBaggedListLock);
				}else{
					/* Acquire the foodToBagLock to increment the foodToBag count	*/
					Acquire(foodToBagLock);
					tempV = GetMV(foodToBagMVi) + 1;
					SetMV(foodToBagMVi, tempV);
					/* Release the foodToBagLock after updating the foodToBag count	*/
					Release(foodToBagLock);
				}
			}
	
			Acquire(customerDataLock);
			if(GetMV(sixBurgerincustDBMVi[custIndex]) == 2){
				Release(customerDataLock);
				Acquire(foodToBeCookedDBLock); 
				tempV = GetMV(toBeCookedSixburgerMVi) + 1;
				SetMV(toBeCookedSixburgerMVi, tempV);
				Release(foodToBeCookedDBLock);
			}else
				Release(customerDataLock);
			
			Acquire(customerDataLock);
			if(GetMV(threeBurgerincustDBMVi[custIndex]) == 2){
				Release(customerDataLock);
				Acquire(foodToBeCookedDBLock);
				tempV = GetMV(toBeCookedThreeburgerMVi) + 1;
				SetMV(toBeCookedThreeburgerMVi, tempV);
				Release(foodToBeCookedDBLock);
			}else
				Release(customerDataLock);
			Acquire(customerDataLock);
			if(GetMV(vegBurgerincustDBMVi[custIndex]) == 2){
				Release(customerDataLock);
				Acquire(foodToBeCookedDBLock);
				tempV = GetMV(toBeCookedVegburgerMVi) + 1;
				SetMV(toBeCookedVegburgerMVi, tempV);
				Release(foodToBeCookedDBLock);
			}else
/* CHANGE======== */Release(customerDataLock);
			
			Acquire(customerDataLock);
			if(GetMV(friesincustDBMVi[custIndex]) == 2){
				Release(customerDataLock);
				Acquire(foodToBeCookedDBLock);
				tempV = GetMV(toBeCookedFriesMVi) + 1;
				SetMV(toBeCookedFriesMVi, tempV);
				Release(foodToBeCookedDBLock);
			}else
/* CHANGE======== */Release(customerDataLock);
			
			Acquire(orderTakerLock[myIndex]);
			
			/* Send a signal to the Customer which indicates that the 	*/
			/* Order Taker has processed the order	*/
			Signal(orderTakerCV[myIndex], orderTakerLock[myIndex]);		
			/* wait for the customer to pay money	*/
			Wait(orderTakerCV[myIndex], orderTakerLock[myIndex]);
			Release(orderTakerLock[myIndex]);
			Acquire(custServedLock);
			if(GetMV(custServedMVi) == CUST_COUNT){
				Release(custServedLock);
				/* @ */
				Print_1Arg("All served OrderTaker %d calling E---X---I---T\n$", myIndex); 
				continue;
			}
			
			Release(custServedLock);
			Acquire(customerDataLock);
			if(GetMV(sixBurgerincustDBMVi[custIndex]) > 0)
				billAmount += 6;	/* 6$Burger	*/
			if(GetMV(threeBurgerincustDBMVi[custIndex]) > 0)
				billAmount += 3;	/* 3$Burger	*/
			if(GetMV(vegBurgerincustDBMVi[custIndex]) > 0)
				billAmount += 8;	/* vegBurger costs 8$ */
			if(GetMV(friesincustDBMVi[custIndex]) > 0)
				billAmount += 2;	/* fries costs 2$ */
			if(GetMV(sodaincustDBMVi[custIndex]) > 0)
				billAmount += 1;	/* soda costs 1$ */
			Release(customerDataLock);
			
			Acquire(moneyAtRestaurantLock);
			tempV = GetMV(moneyAtRestaurantMVi);
			SetMV(moneyAtRestaurantMVi, tempV);
			/* release the lock after storing the money	*/
			Release(moneyAtRestaurantLock);
			/* Send a signal to the Customer which indicates that the 	*/
			/* Order Taker has processed the order	*/
			Acquire(orderTakerLock[myIndex]);
			Signal(orderTakerCV[myIndex],orderTakerLock[myIndex]);	
			/* interaction with the current customer is completed so the 	*/
			/* Order Taker releases the Lock	*/
			Release(orderTakerLock[myIndex]);
			
			Acquire(custServedLock);
			if(GetMV(custServedMVi) == CUST_COUNT)
			{
				Release(custServedLock);
				
				/* Print_1Arg("OrderTaker %d calling E---X---I---T\n$", myIndex); */
				continue;
			}
			Release(custServedLock);	
		
	}   /* While(1)
 	*/
	
	/* ----------now ---------------*/
}

