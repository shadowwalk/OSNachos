/* Customer thread body */  
void Customer()
{
    int index = 0;
	int internalOT = 0;
	int managerOT = 0;
	int myIndex = 0;
	
	/* Get the Index for this Customer before starting the order*/
	Acquire(nextCustomerIndexLock);
	myIndex = nextCustomerIndex;
	nextCustomerIndex++;
	Release(nextCustomerIndexLock);
	
    /* Acquire this LOCK to become the NEXT eligible customer */
	
	/* Check if any Order Taker is FREE */
	/* - if any then make him as the Order Taker for the current customer */
	/* - Make that Order Taker status as BUSY */
	for(index = 1; index <= OT_COUNT; index++){
		Acquire(custLineLock);
		if(orderTakerStatus[index] == OT_FREE){	
			orderTakerStatus[index] = OT_BUSY;
			Release(custLineLock);
			Acquire(customerDataLock);
			custData[myIndex].myOT = index;
			Release(customerDataLock);
			break;
		}
		Release(custLineLock);
	}
	
	/*If no Order Takers are free */
	/* - increment the linelength monitor variable */
	/* - then wait in the customer waiting line */
	Acquire(customerDataLock);
	if(custData[myIndex].myOT == 0){
		Release(customerDataLock);
		Acquire(custLineLock);
		custLineLength++;
		Wait(custLineCV, custLineLock);
		Release(custLineLock);
	}else	
		Release(customerDataLock);
		
	/* Customer is here because he received a signal from the Order Taker */
	/* - customer has also acquired the custLineLock */

	/* Check for the Order Taker who is WAITING */
	/* - make him as the Order Taker for the current customer */
	/* - Make that Order Taker status as BUSY */
	Acquire(customerDataLock);
	if(custData[myIndex].myOT == 0){
		Release(customerDataLock);
		for(index = 1; index <= OT_COUNT; index++){ 
			Acquire(custLineLock);
			if(orderTakerStatus[index] == OT_WAIT){	
				orderTakerStatus[index] = OT_BUSY;
				Release(custLineLock);
				Acquire(customerDataLock);
				custData[myIndex].myOT = index;
				Release(customerDataLock);	
				break;
			}else
				Release(custLineLock);
		
		}
	}else	
		Release(customerDataLock);	
	
	/* Acquire the orderTakerLock to wake up the Order Taker who is waiting */
	/* on the signal from the current customer */
	internalOT = custData[myIndex].myOT;
	Acquire(orderTakerLock[internalOT]);
	/* Send a signal to the Order Taker which indicates that the customer */
	/* is ready to give the order */
	Signal(orderTakerCV[internalOT], orderTakerLock[internalOT]);
    /* Customer goes on wait to receive a signal from the Order Taker */
	/* which indicates Order Taker is ready to take the order */
	Wait(orderTakerCV[internalOT], orderTakerLock[internalOT]);
	Release(orderTakerLock[internalOT]);
	/* Customer received a signal from the Order Taker to place an order */
	/* Order is randomly generated and is stored in the customer database */
	
	/* OG */
	if(managerOT != 1){
		Print_2Arg("Customer %d is giving order to OrderTaker %d\n$", myIndex, internalOT);
	}else{
		Print_1Arg("Customer %d is giving order to the manager\n$", myIndex);
	}
	
	/*OG */
	Acquire(customerDataLock);
	if(custData[myIndex].sixBurger == 1){
		Print_1Arg("Customer %d is ordering 6-dollar burger\n$", myIndex);
	}else{
		Print_1Arg("Customer %d is not ordering 6-dollar burger\n$", myIndex);
	}
	if(custData[myIndex].threeBurger == 1){
		Print_1Arg("Customer %d is ordering 3-dollar burger\n$", myIndex);
	}else{
		Print_1Arg("Customer %d is not ordering 3-dollar burger\n$", myIndex);
	}
	if(custData[myIndex].vegBurger == 1){
		Print_1Arg("Customer %d is ordering veggie burger\n$", myIndex);
	}else{
		Print_1Arg("Customer %d is not ordering veggie burger\n$", myIndex);
	}
	if(custData[myIndex].fries == 1){
		Print_1Arg("Customer %d is ordering french fries\n$", myIndex);
	}else{
		Print_1Arg("Customer %d is ordering french fries\n$", myIndex);
	}
	if(custData[myIndex].soda == 1){
		Print_1Arg("Customer %d is ordering soda\n$", myIndex);
	}else{
		Print_1Arg("Customer %d is not ordering soda\n$", myIndex);
	}
	if(custData[myIndex].dineType == 1){
		Print_1Arg("Customer %d chooses to to-go the food\n$", myIndex);
	}else{
		Print_1Arg("Customer %d chooses to eat-in the food\n$", myIndex);
	}
	Release(customerDataLock);
	/* Send a signal to the Order Taker which indicates that the customer */
	/* has placed an order */
	Acquire(orderTakerLock[internalOT]);
	Signal(orderTakerCV[internalOT],orderTakerLock[internalOT]);
	/* wait for the Order Taker to reply once he checks the food availability */
	Wait(orderTakerCV[internalOT],orderTakerLock[internalOT]);
	
	/* Received a signal from Order Taker which indicates that the order is */
	/* processed and its time to pay money */
	
	/* Send a signal to the Order Taker which indicates that the customer */
	/* has payed the bill amount */ 
	Signal(orderTakerCV[internalOT],orderTakerLock[internalOT]);
	
	/* Wait for the Order Taker to acknowledge the payment being done */ 
	/* successfully */
	Wait(orderTakerCV[internalOT], orderTakerLock[internalOT]);
	Release(orderTakerLock[internalOT]);
	/* Received a signal from the Order Taker to indicate that the payment */
	/* was completed and the customer can move to the next stage */ 

	/* If to-go then check if the order is bagged right away */
	/* else go and wait for a broadcast of the tokenNo */
	
	if(custData[myIndex].dineType == 1){
		/* if to-go and if the order is delivered then the customer takes the */
		/* bag and just leaves the restaurant */
		/* release the Order Taker Lock */
		Acquire(customerDataLock);
		if(custData[myIndex].delivered == 1){
			Release(customerDataLock);	
			/* OG */
			if(managerOT != 1){
				Print_2Arg("Customer %d receives food from the OrderTaker %d\n$",myIndex, internalOT);
			}else{
				Print_2Arg("Customer %d receives food from the Manager %d\n$",myIndex, internalOT);
			}
			
			/* Acquire the Lock to update the custer count served the food */
			Acquire(custServedLock);
			custServed++;
			/* @ */
			/* Print_2Arg("\n\nCUSTOMER %d SERVICED IS %d\n\n$", myIndex, custServed); */
			Release(custServedLock);
			/* Print_1Arg("To Go Customer %d calling E---X---I---T\n$", myIndex); */
			Exit(0);
	
			/* if to-go and if the order is not ready then customer has to wait */
			/* for a broadcast signal from the Order Taker when the order is bagged */
		
			/* Acquire the Lock which is used to match with To-go waiting */
			/* condition variable waitingLock */
		}else{
			Release(customerDataLock);	
			/* OG */
			if(managerOT != 1){
				Print_3Arg("Togo Customer %d is given token number %d by the OrderTaker %d\n$", myIndex, custData[myIndex].tokenNo, internalOT);
			}else{
				Print_2Arg("Customer %d is given token number %d by the Manager\n$",myIndex, custData[myIndex].tokenNo);
			}
			Acquire(waitToGoLock);
				
			/* Go on wait to receive the broadcast signal from the Order Taker */
			Wait(toGoCV,waitToGoLock);
			Release(waitToGoLock);
	
			while(1){
				/* Received a Broadcast signal from one of the Order Taker */
				/* - Check if the broadcasted tokenNo is customer's tokenNo */
				Acquire(customerDataLock);
				if(custData[myIndex].delivered = 1)	{
					Release(customerDataLock);
					/* Acquire the Lock to update the custer count served the food */
					Acquire(custServedLock);
					custServed++;
					/* @ */
					/* Print_2Arg("\n\nCUSTOMER %d SERVICED IS %d\n\n$", myIndex,custServed); */
					Release(custServedLock);
					/* Print_1Arg("Customer %d calling E---X---I---T\n$", myIndex); */
					Exit(0);
					break;
				}else{ 
					Release(customerDataLock);
					/* Go on wait to receive the next broadcast signal from the waiter */
					Acquire(waitToGoLock);
					Wait(toGoCV,waitToGoLock);
					Release(waitToGoLock);
				}	
			}
	    }
	}/* if dineType is eat-in */
	else if(custData[myIndex].dineType == 0){
		
		/*  OG */
		/* Print_3Arg("EatinCustomer %d is given token number %d by the OrderTaker %d\n$", myIndex, custData[myIndex].tokenNo, internalOT); */
		/* Acquire a Lock to add customer onto the queue waiting for */
		/* manager's reply */
		Acquire(managerLock);
		/* Increment the customer waiting count */
		managerLineLength++;
		/* go on wait till manager signals with table availability */
		Wait(managerCV,managerLock);

		/* Received a signal from manager */
		/* Customer has acquired the manager lock */
		/* Manager replied saying "restaurant is not full" */
		if(tableAvailable == 1){		
			/* OG */
			Print_1Arg("Customer %d is informed by the Manager-the restaurant is not full\n$", myIndex);
			tableAvailable = 0;
			Release(managerLock);
			for(index = 1; index <= TABLE_COUNT; index++){
				Acquire(tablesDataLock);
				if(tables[index] == 0){
					/* make the first table which is free as the customer */
					/* table */
					tables[index] = custData[myIndex].tokenNo;
					Release(tablesDataLock);
					Acquire(customerDataLock);
					custData[myIndex].tableNo = index;
					Release(customerDataLock);
					/* @ */
					/* Print_3Arg("table %d is taken by custom %d tokenNo %d\n$", index, myIndex, tables[index]);	 */
					break;
				}else
					Release(tablesDataLock);
			}
			
			/* if a table is found then go sit and wait for food */
			if(custData[myIndex].tableNo != 0){	
				/* OG	*/
				Print_2Arg("customer %d is seated at table number %d\n$",myIndex, custData[myIndex].tableNo);
					
				/* Before releasing the tablesDataLock Acquire the */
				/* eatInFoodWaitingLock to ensure that this customer is the */ 
				/* next one to join the queue waiting for the food to be bagged */

				/* customer goes on wait to receive a signal from the waiter */
				/* after the tokenNo is validated */
				while(1){
					Acquire(eatInFoodWaitingLock);
					
					Wait(eatInFoodWaitingCV,eatInFoodWaitingLock);
				
					Release(eatInFoodWaitingLock);
		 			/* upon receiving the signal check if order is delivered */
					Acquire(customerDataLock);
					if(custData[myIndex].delivered == 1){
						/* if delivered then the customer eats the food and */
						/* leaves the restaurant */
						Release(customerDataLock);
						Acquire(tablesDataLock);			
						tables[custData[myIndex].tableNo] = 0;
						Release(tablesDataLock);
						
						Acquire(eatInWaitingLock);
						Signal(eatInWaitingCV,eatInWaitingLock);
						Release(eatInWaitingLock);	
						/* Acquire the Lock to update the custer count served the food */
						Acquire(custServedLock);
						custServed++;
						/* @ */
						/* Print_2Arg("\n\nCUSTOMER %d SERVICED IS %d\n\n$", myIndex, custServed); */
						Release(custServedLock);
						/* Print_1Arg("Customer %d calling E---X---I---T\n$", myIndex); */
						Exit(0);	
					}
					Release(customerDataLock);
				}
			}
			
			/* Manager replied saying "restaurant is full"	*/
		}else if(tableAvailable == 0){
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
				
			for(index = 1; index <= TABLE_COUNT; index++){
				if(tables[index] == 0){
					/* make the first table which is free as the customer */
					/* table */
					tables[index] = custData[myIndex].tokenNo;
					
					Release(tablesDataLock);
					Acquire(customerDataLock);
					custData[myIndex].tableNo = index;
					Release(customerDataLock);
					Acquire(tablesDataLock);
					break;
				}	
			}
				
			/* Release the tables data lock */
			Release(tablesDataLock);
			Acquire(customerDataLock);
			
			if(custData[myIndex].tableNo != 0){
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
				while(custData[myIndex].delivered != 1){
					/* else Go Back to wait */
					Wait(eatInFoodWaitingCV,eatInFoodWaitingLock);
				}
				Release(eatInFoodWaitingLock);
				/* OG */
				Print_1Arg("Customer %d is served by waiter\n$", myIndex);
				/* if delivered then the customer eats the food and */
				/* leaves the restaurant */
						
				/* OG */
				if(managerOT != 1){	
					Print_2Arg("Customer %d is leaving the restaurant after OrderTaker %d packed the food\n$",myIndex, internalOT);
				}else{
					Print_1Arg("Customer %d is leaving the restaurant after the Manager packed the food\n$",myIndex);
				}	
						Acquire(tablesDataLock);			
						tables[custData[myIndex].tableNo] = 0;
						Release(tablesDataLock);

				/* once the order is delivered, the seated customer before */
				/* leaving the restaurant signals one of the customers */
				/* who is waiting for a table to sit */
				Acquire(eatInWaitingLock);		
				Signal(eatInWaitingCV,eatInWaitingLock);
					
				Release(eatInWaitingLock);
				/* OG */	
				Print_1Arg("EAT-IN  Customer %d is leaving the restaurant after having food\n$",myIndex);
					 
				/* Acquire the Lock to update the custer count served the food	*/	
				Acquire(custServedLock);
				custServed++;		
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
