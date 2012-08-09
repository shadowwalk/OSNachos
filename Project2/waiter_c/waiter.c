
/* Waiter thread body */  
void Waiter()
{
	int index = 0, indexj = 0;
	int tableNo = 0;
	int baggedListNotEmpty = 0;
	int custHasNoTable = 0;
	int myIndex = 0;
	
	/* Get the Index for this Cook before starting the job */
	Acquire(nextWaiterIndexLock);
	myIndex = nextWaiterIndex ;
	nextWaiterIndex++;
	Release(nextWaiterIndexLock);
	
	while(1){
		
		custHasNoTable = 0;
		index = 0;
		tableNo = 0;
		baggedListNotEmpty = 0;
		/* Acquire Lock to check if any orders are bagged*/
		Acquire(foodBaggedListLock);
		
		/* check if token numbers of bagged food are present in bagged list*/
		for(index = 1; index <= CUST_COUNT; index++){
			if(baggedList[index] != 0){
				baggedListNotEmpty = baggedList[index];
				baggedList[index] = 0;
				break;
			}
		}
		
		Release(foodBaggedListLock);
		
		if(baggedListNotEmpty != 0){
			int firstTokenNo;
			
			/* remove first token number from the bagged list */
			/* to serve it for the customer waiting for it*/
			firstTokenNo = baggedListNotEmpty;
			/* Acquire tableDataLock to obtain the table number of the */
			/* Eat-in seated customer */			
			Acquire(tablesDataLock);
	
			/* Determine the table number of the Eat-in Customer*/
			
			for(index = 1; index <= TABLE_COUNT; index++){
				/* Find the table number using token number removed from */
				/* bagged list*/
				if(tables[index] == firstTokenNo){
					tableNo = index;
				    break;
				}
			}
			
			Release(tablesDataLock);
			
			/* Search for a particular token number with all customers */
			for(index = 1; index <= CUST_COUNT; index++){
				/* If token number of customer matches set the delivered which*/
				/* indicates the food has been served.*/
				
				Acquire(customerDataLock);
				if(custData[index].tokenNo == firstTokenNo){
					/* check if the customer is seated */
					if(custData[index].tableNo != 0){ 
					
						/* OG */
						Print_2Arg("Waiter %d validates the token number for Customer %d and serves the food\n$", myIndex, index);
						custData[index].delivered = 1; 
						Release(customerDataLock);
						/* OG */
						Print_2Arg("Waiter %d serves food to Customer %d\n$", myIndex, index);
						/* Notifying the Eatin seated customer that the order */
						/* has been delivered*/
						Acquire(eatInFoodWaitingLock);
						
						/* Broadcasting the notification about the order*/
						/* delivered. This signal will be processed only by*/
						/* that customer whose order was bagged, others*/
						/* will go back to wait again.*/
						Broadcast(eatInFoodWaitingCV, eatInFoodWaitingLock);
						Release(eatInFoodWaitingLock);
					}
					/* else if the customer has not yet got his table to sit*/
					/* then put back the bagged order into the List*/
					else{ 
						Release(customerDataLock);
						Acquire(foodBaggedListLock);
						for(indexj = 1; indexj <= CUST_COUNT; indexj++){
							if(baggedList[indexj] == 0){
								baggedList[indexj] = firstTokenNo;
								custHasNoTable = 1;
								break;
							
							}
						}
						Release(foodBaggedListLock);
					}	
					if(custHasNoTable == 0)
						break;
				}
				else
					Release(customerDataLock);
			}
		}
		if((baggedListNotEmpty == 0) || (custHasNoTable == 1)){
			/* Waiter Acquires Lock and Goes on wait (break) till he receives */
			/* a wake up signal from the manager */
			Acquire(waiterSleepLock);
			/* OG */
			Print_1Arg("Waiter %d is going on break\n$", myIndex);
			Wait(waiterSleepCV,waiterSleepLock);
			/* Received a signal from the manager, waiter starts with his job */
			Release(waiterSleepLock);
			/* Check if all the Customers are serviced */
			Acquire(custServedLock);
			if(custServed == CUST_COUNT){			
				Release(custServedLock);
				Exit(0);
				break;
			}
			Release(custServedLock);
			/* OG */
			Print_1Arg("Waiter %d returned from break\n$", myIndex);
		}
	}
}