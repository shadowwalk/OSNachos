/* Cook thread body */ 
void Cook()
{	
	int myIndex = 0;
	int whatToCook = 0;
	
	/* Get the Index for this Cook before starting the job */
	Acquire(nextCookIndexLock);
	myIndex = nextCookIndex ;
	nextCookIndex++;
	Release(nextCookIndexLock);

	/* To know what the manager wants this cook to cook */
	Acquire(whatToCookFirstLock);
	while(whatToCookFirst == 0){
		Release(whatToCookFirstLock);	
		Yield();
		Acquire(whatToCookFirstLock);
	}
	whatToCook = whatToCookFirst;
	whatToCookFirst = 0;
	Release(whatToCookFirstLock);
	
	/* Acquire a lock on inventory to check status*/
	while(1){	
		/* Check if the stop cooking flag is set by the manager*/
		/* Acquire the current cooking item lock*/
		/* Check if the stop cooking flag is set*/
		/* if set then it indicates manager wants the Cook to go on break*/
		/* Add the current cook thread to the cooks on break queue*/
		/* Cooks goes on break (sleep)*/
		
		if(whatToCook == SIXBURGER){
			Acquire(stopSixBurgerLock);
			
			if(stopSixBurger == 1){
				Release(stopSixBurgerLock);
				/* Increase the cooks on break Count to indicate the manager who checks this value
					before hiring a new cook */
				Acquire(cooksOnBreakCountLock);
				cooksOnBreakCount++;
				Release(cooksOnBreakCountLock);
				
				Acquire(workingCookLock);
				workingCooks[whatToCook]--;
				Release(workingCookLock);
				
				/* OG @ */
				Print_2Arg("Cook %d is going on break\n$", myIndex, whatToCook);
				
				Acquire(wakeUpCookLock);
				Wait(wakeUpCookCV,wakeUpCookLock);
				Release(wakeUpCookLock);
				
				Acquire(custServedLock);
				if(custServed == CUST_COUNT){
					Release(custServedLock);
					/* @ */
					/* Print_1Arg("Cook %d calling E---X---I---T\n$", myIndex); */
					Exit(0);
				}
				Release(custServedLock);
				
				/* cook is back from break*/
				/* Acquire the what to cook next lock and check */
				/* what to cook next*/
				Acquire(whatToCookNextLock);
				/* update what the cook was cooking before going to */
				/* sleep with what he has to cook after the break*/
				while(whatToCookNext == 0){
					Release(whatToCookNextLock);
					Yield();
					Acquire(whatToCookNextLock);
				}	
				whatToCook = whatToCookNext;
				whatToCookNext = 0;
				/* release the lock after setting the value*/
				Release(whatToCookNextLock);
				
				/* OG */
				Print_2Arg("Cook %d returned from break\n$", myIndex, whatToCook);	
			}else{	
				Release(stopSixBurgerLock);
			}	
		}
		else if(whatToCook == THREEBURGER){
			Acquire(stopThreeBurgerLock);
			if(stopThreeBurger == 1){
				Release(stopThreeBurgerLock);
				/* Increase the cooks on break Count to indicate the manager who checks this value
					before hiring a new cook */
				Acquire(cooksOnBreakCountLock);
				cooksOnBreakCount++;
				Release(cooksOnBreakCountLock);
				
				Acquire(workingCookLock);
				workingCooks[whatToCook]--;
				Release(workingCookLock);
				
				/* OG */
				Print_2Arg("Cook %d is going on break\n$", myIndex, whatToCook);
				
				Acquire(wakeUpCookLock);
				Wait(wakeUpCookCV, wakeUpCookLock);
				Release(wakeUpCookLock);
				
				Acquire(custServedLock);
				if(custServed == CUST_COUNT){
					Release(custServedLock);
					/* @ */
					/* Print_1Arg("Cook %d calling E---X---I---T\n$", myIndex); */
					Exit(0);
				}
				Release(custServedLock);
				/* cook is back from break*/
				/* Acquire the what to cook next lock and check */
				/* what to cook next*/
				Acquire(whatToCookNextLock);
				/* update what the cook was cooking before going to */
				/* sleep with what he has to cook after the break*/
				while(whatToCookNext == 0){
					Release(whatToCookNextLock);
					Yield();
					Acquire(whatToCookNextLock);
				}	
				whatToCook = whatToCookNext;
				whatToCookNext = 0;
				/* release the lock after setting the value*/
				Release(whatToCookNextLock);
				
				/* OG*/
				Print_2Arg("Cook %d returned from break\n$", myIndex, whatToCook);
				
			}else{	
				Release(stopThreeBurgerLock);
			}	
		}
		else if(whatToCook == VEGBURGER){
		
			Acquire(stopVegBurgerLock);
			if(stopVegBurger == 1){
			
				Release(stopVegBurgerLock);
				/* Increase the cooks on break Count to indicate the manager who checks this value
					before hiring a new cook */
				Acquire(cooksOnBreakCountLock);
				cooksOnBreakCount++;
				Release(cooksOnBreakCountLock);
				
				Acquire(workingCookLock);
				workingCooks[whatToCook]--;
				Release(workingCookLock);
				
				/* OG*/
				Print_2Arg("Cook %d is going on break\n$", myIndex, whatToCook);
				Acquire(wakeUpCookLock);
				
				Wait(wakeUpCookCV,wakeUpCookLock);
				Release(wakeUpCookLock);
				
				Acquire(custServedLock);
				if(custServed == CUST_COUNT){
					Release(custServedLock);
					/* @ */
					/* Print_1Arg("Cook %d calling E---X---I---T\n$", myIndex); */
					Exit(0);
				}
				Release(custServedLock);
				/* cook is back from break*/
				/* Acquire the what to cook next lock and check */
				/* what to cook next*/
				Acquire(whatToCookNextLock);
				/* update what the cook was cooking before going to */
				/* sleep with what he has to cook after the break*/
				while(whatToCookNext == 0){
					Release(whatToCookNextLock);
					Yield();
					Acquire(whatToCookNextLock);
				}	
				whatToCook = whatToCookNext;
				whatToCookNext = 0;
				/* release the lock after setting the value*/
				Release(whatToCookNextLock);
				
				/* OG*/
				Print_2Arg("Cook %d returned from break\n$", myIndex, whatToCook);
				
			}else{
				Release(stopVegBurgerLock);
			}	
		}
		else if(whatToCook == FRIES){
			Acquire(stopFriesLock);
			
			if(stopFries == 1){
				Release(stopFriesLock);
				
				/* Increase the cooks on break Count to indicate the manager who checks this value
					before hiring a new cook */
					
				Acquire(cooksOnBreakCountLock);
				cooksOnBreakCount++;
				Release(cooksOnBreakCountLock);
				
				Acquire(workingCookLock);
				workingCooks[whatToCook]--;
				Release(workingCookLock);
				
				/* OG*/
				Print_2Arg("Cook %d is going on break\n$", myIndex, whatToCook);
				Acquire(wakeUpCookLock);
				Wait(wakeUpCookCV, wakeUpCookLock);
				Release(wakeUpCookLock);
				
				
				Acquire(custServedLock);
				if(custServed == CUST_COUNT){
					Release(custServedLock);
					/* @ */
					/* Print_1Arg("Cook %d calling E---X---I---T\n$", myIndex); */
					Exit(0);
				}
				Release(custServedLock);
				
				/* cook is back from break*/
				/* Acquire the what to cook next lock and check */
				/* what to cook next*/
				Acquire(whatToCookNextLock);
				/* update what the cook was cooking before going to */
				/* sleep with what he has to cook after the break*/
				while(whatToCookNext == 0){
					Release(whatToCookNextLock);
					Yield();
					Acquire(whatToCookNextLock);
				}	
				whatToCook = whatToCookNext;
				whatToCookNext = 0;
				/* release the lock after setting the value*/
				Release(whatToCookNextLock);
				
				/* OG*/
				Print_2Arg("Cook %d returned from break\n$", myIndex, whatToCook);
				
			}else{
				Release(stopFriesLock);
			}	
		} 
		
		/* If all the customers are serviced then the cooks will Go Home*/
		/* Check if all the Customers are serviced*/
		Acquire(custServedLock);
		if(custServed == CUST_COUNT){
			Release(custServedLock);
			/* @ */
			/* Print_1Arg("Cook %d calling E---X---I---T\n$", myIndex); */
			Exit(0);
		}
		Release(custServedLock);
		
		/* decrement one from the inventory*/
		Acquire(inventoryLock);
		while(inventory == 0){
			Release(inventoryLock);
			Yield();
			Acquire(inventoryLock);
		}	
		inventory--;
		Release(inventoryLock);
		
		/* check what Manager has ordered the cook to prepare.*/
		/* Increase the food count in Ready DataBase*/
		if(whatToCook == SIXBURGER){
			/* OG*/
			Print_1Arg("Cook %d is going to cook 6-dollar burger\n$", myIndex);
			Acquire(foodReadyDBLock);
			foodReadyData.sixBurger++; 
			Release(foodReadyDBLock);
		}
		
		if(whatToCook == THREEBURGER){		
			/* OG*/
			Print_1Arg("Cook %d is going to cook 3-dollar burger\n$", myIndex);
			Acquire(foodReadyDBLock);
			foodReadyData.threeBurger++;		
			Release(foodReadyDBLock);
		}
		
		if(whatToCook == VEGBURGER){
			/* OG*/
			Print_1Arg("Cook %d is going to cook veggie burger\n$", myIndex);	 
			Acquire(foodReadyDBLock);
			foodReadyData.vegBurger++; 
			Release(foodReadyDBLock);
		}
		
		if(whatToCook == FRIES){
			/* OG*/
			Print_1Arg("Cook %d is going to cook french fries\n$", myIndex);	
			Acquire(foodReadyDBLock);
			foodReadyData.fries++;  
			Release(foodReadyDBLock);
		}	
		
		/* cooks takes some Time to prepare the food that */
		/* is instructed by the manager */
		Yield(); 
	}
} 
