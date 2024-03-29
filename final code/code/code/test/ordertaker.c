
int BagTheOrders()
{
	int index = 0;	
	int cannotBeBagged = 0;
	
	/* if there are no more customers in the customer line */
	/* then Order Taker starts bagging the orders */
	for(index = 1; index <= CUST_COUNT; index++){
		cannotBeBagged = 0;
		
		Acquire(customerDataLock);
		/* check for all the orders which are not bagged */
		if(custData[index].bagged != 1)
		{
			if(custData[index].sixBurger == 1)
			{
				Release(customerDataLock);
				/*if six$burgers available */
				Acquire(foodReadyDBLock);
				if(foodReadyData.sixBurger != 0)
				{
					Release(foodReadyDBLock);
					/*reserve a burger for this customer */
					Acquire(customerDataLock);
					custData[index].sixBurger = 2;
					Release(customerDataLock);
					/* if greater than minimum amount of cooked food */
					/* then bag it, no worries */
					Acquire(foodReadyDBLock);
					if(foodReadyData.sixBurger > MINCOOKEDFOOD)
					{
						foodReadyData.sixBurger--;	
						Release(foodReadyDBLock);
						Acquire(foodToBeCookedDBLock);
						foodToBeCookedData.sixBurger--;
						Release(foodToBeCookedDBLock);
					}
					/* else if it is less than minimum then  */
					/* bag it and increment the food to be cooked */
					else{
						foodReadyData.sixBurger--;	
						Release(foodReadyDBLock);
						/* Do not decrement the foodtoBeCookedData because the */
						/* amount of cooked food is less than minimum */
					}
				}
				else if(foodReadyData.sixBurger == 0){
					cannotBeBagged = 1;
					Release(foodReadyDBLock);
				}
			}else
				Release(customerDataLock);
				/* if 3$burger is ordered */
				
				Acquire(customerDataLock);
				if(custData[index].threeBurger == 1)
				{
					Release(customerDataLock);
					/*if three$burgers available */
					Acquire(foodReadyDBLock);
					if(foodReadyData.threeBurger != 0)
					{
						Release(foodReadyDBLock);
						/*reserve a burger for this customer */
						Acquire(customerDataLock);
						custData[index].threeBurger = 2;
						Release(customerDataLock);
						/* if greater than minimum amount of cooked food */
						/* then bag it, no worries */
						Acquire(foodReadyDBLock);
						if(foodReadyData.threeBurger > MINCOOKEDFOOD)
						{
							foodReadyData.threeBurger--;	
							Release(foodReadyDBLock);
							Acquire(foodToBeCookedDBLock);
							foodToBeCookedData.threeBurger--;
							Release(foodToBeCookedDBLock);
						}
						/* else if it is less than minimum then  */
						/* bag it and do not decrement the food to be cooked */
						else{
							foodReadyData.threeBurger--;	
							Release(foodReadyDBLock);
							/* Do not decrement the foodtoBeCookedData because the */
							/* amount of cooked food is less than minimum */
						}
					}
					else if(foodReadyData.threeBurger == 0)
					{
						cannotBeBagged = 1;
						Release(foodReadyDBLock);
					}	
				}else
					Release(customerDataLock);
				
				
				/* if vegburger is ordered */
				Acquire(customerDataLock);
				if(custData[index].vegBurger == 1)
				{
					Release(customerDataLock);
					/*if vegburgers available */
					Acquire(foodReadyDBLock);
					if(foodReadyData.vegBurger != 0)
					{
						Release(foodReadyDBLock);
						/*reserve a burger for this customer */
						Acquire(customerDataLock);
						custData[index].vegBurger = 2;
						Release(customerDataLock);
						/* if greater than minimum amount of cooked food */
						/* then bag it, no worries */
						Acquire(foodReadyDBLock);
						if(foodReadyData.vegBurger > MINCOOKEDFOOD)
						{
							foodReadyData.vegBurger--;	
							Release(foodReadyDBLock);
							Acquire(foodToBeCookedDBLock);
							foodToBeCookedData.vegBurger--;
							Release(foodToBeCookedDBLock);
						}
						/* else if it is less than minimum then  */
						/* bag it and do not decrement the food to be cooked */
						else{
							foodReadyData.vegBurger--;	
							Release(foodReadyDBLock);
							/* Do not decrement the foodtoBeCookedData because the */
							/* amount of cooked food is less than minimum */
						}
					}
					else if(foodReadyData.vegBurger == 0)
					{
						cannotBeBagged = 1;
						Release(foodReadyDBLock);
					}
				}else
					Release(customerDataLock);
				
				
				/* if fries is ordered */
				Acquire(customerDataLock);
				if(custData[index].fries == 1)
				{
					Release(customerDataLock);
					/*if fries available */
					Acquire(foodReadyDBLock);
					if(foodReadyData.fries != 0)
					{
						Release(foodReadyDBLock);
						/*reserve a fries for this customer */
						Acquire(customerDataLock);
						custData[index].fries = 2;
						Release(customerDataLock);
						/* if greater than minimum amount of cooked food */
						/* then bag it, no worries */
						Acquire(foodReadyDBLock);
						if(foodReadyData.fries > MINCOOKEDFOOD)
						{
							foodReadyData.fries--;
							Release(foodReadyDBLock);
							Acquire(foodToBeCookedDBLock);	
							foodToBeCookedData.fries--;
							Release(foodToBeCookedDBLock);
						}
						/* else if it is less than minimum then */
						/* bag it and do not decrement the food to be cooked */
						else{
							foodReadyData.fries--;	
							Release(foodReadyDBLock);
							/* Do not decrement the foodtoBeCookedData because the */
							/* amount of cooked food is less than minimum */
						}
					}
					else if(foodReadyData.fries == 0){
						cannotBeBagged = 1;
						Release(foodReadyDBLock);
					}
				}else
					Release(customerDataLock);
					
					
				if(cannotBeBagged == 1)
				{
					continue;
				}else if(cannotBeBagged == 0)
				{					
					/* Order is bagged so update the food to bag count */
					Acquire(foodToBagLock);
					foodToBag-- ;
					Release(foodToBagLock);
					/* make the customer order status as bagged */
	
/*CHANGE:ADD ============*/	Acquire(BagTheOrdersLock);
					custData[index].bagged = 1;
/*CHANGE:ADD ============*/	Release(BagTheOrdersLock);
					
					return (index);
				}
		}
		else
		{
				Release(customerDataLock);
		}
	}	
	return -1;
}



/* OrderTaker thread body */  
void OrderTaker()
{
	int index = 0, indexj = 0;	/* index for looping through customer database	*/
	int custIndex = 0;			/* to store the cust index who being served 	*/
	int custHasToWait = 0;		/* if to-go and food not ready set this flag	*/
	int billAmount = 0;			/* to store the bill amount payed by customer	*/
	int myIndex = 0;
	
	/* Get the Index for this Customer before starting the order */
	Acquire(nextOrderTakerIndexLock);
	myIndex = nextOrderTakerIndex;
	nextOrderTakerIndex++;
	Release(nextOrderTakerIndexLock);
	
	while(1)
	{
		Acquire(custServedLock);
			if(custServed == CUST_COUNT){
				Release(custServedLock);
				/* @ */
				/* Print_1Arg("OrderTaker %d calling E---X--I---T\n$", myIndex); */
				Exit(0);
			}
		Release(custServedLock);
				
		/* Acquire custLineLock to access the custLineLength monitor variable	*/
		Acquire(custLineLock);
		/* Check for any customers waiting in the customer waiting line	*/
		if(custLineLength > 0){
			/* customers are available so I am releasing the food to bag lock	*/	
			/* There are customers in the customer waiting line	*/
			/* Signal a customer which indicates the customer to come out of	*/
			/* the waiting line and give his order	*/
			Signal(custLineCV, custLineLock);
			
			/* Decrement the custLineLength moniter variable value by 1	*/
			custLineLength--;
			
			/* Before releasing the custLineLock, 	*/
			/* Order Taker changes his status to WAITING	*/
			orderTakerStatus[myIndex] = OT_WAIT;
			Release(custLineLock);
		}		
		else{ 
			Release(custLineLock);
			/* Acquire the foodToBagLock to check if there are any orders to bag	*/
			Acquire(foodToBagLock);
		
			if(foodToBag > 0){
				/* If No customers in the customer waiting line and	*/
				/* if there is some food to bag	*/
			
				/* bag 1 at a time	*/
				/* index of the customer whose order was bagged is returned	*/
				Release(foodToBagLock);
				
				Acquire(BagTheOrdersLock);
				index = BagTheOrders();
				if(index > 0){
					/* OG	*/
					Print_2Arg("OrderTaker %d packed the food for Customer %d\n$", myIndex, index);
/*CHANGE:DELETE ============						Acquire(customerDataLock); */
					custData[index].bagged = 1;
					
/*CHANGE:DELETE============					Release(customerDataLock); */
					Release(BagTheOrdersLock);
					/* after bagging an order 	*/
					/* if the customer has chosen to-go then broadcast the signal	*/
					/* for the customers waiting in the toGoWaiting line	*/
					if(custData[index].dineType == 1){
						/* Acquire the Lock which is used to match with To-go	*/
						/* waiting condition variable waitingLock	*/
						Acquire(waitToGoLock);
						
						Broadcast(toGoCV, waitToGoLock);
						Release(waitToGoLock);	
						
						/* set the broadcastedTokenNo monitor variable to the	*/
						/* tokenNo of the customer whose order was bagged	*/
						Acquire(customerDataLock);
						if(custData[index].bagged == 1)
							custData[index].delivered = 1;
						
						/* Order Taker Broadcasts the signal to all to-go waiting	*/
						Release(customerDataLock);
						
						/* Release the waiting line Lock so that the customers	*/
						/* who received the signal can acquire the Lock and compare	*/
						/* their tokenNo with the broadcastedTokenNo	*/
						
					}/* else if the customer has chosen to eat-in then the tokenNo 	*/
					/* of the order which is bagged is added to the baggedList 	*/
					/* from which the waiter will remove the tokenNo and 	*/
					/* deliver it to the eat-in seated customer 	*/
					else if(custData[index].dineType == 0){
						/* Acquire the Lock to Append the tokenNo to the 	*/
						/* bagged List		*/
/*CHANGE:ADD ============*/	Acquire(BagTheOrdersLock);	
						custData[index].bagged = 1;
/*CHANGE:ADD ============*/	Release(BagTheOrdersLock);
						
						Acquire(foodBaggedListLock);
						for(indexj = 1; indexj <= CUST_COUNT; indexj++){
							if(baggedList[indexj] == 0){
								baggedList[indexj] = custData[index].tokenNo;
								break;
							}
						}
						/* OG	*/
						Print_3Arg("Ordertaker %d gives Token number %d to Waiter for Customer %d\n$", myIndex, custData[index].tokenNo, index);
						Release(foodBaggedListLock);
					}
				}
				Yield(); 
				Release(BagTheOrdersLock);
			} 
			else{
				/* no customers are available and no food to bag so	*/
				/* releasing the food to bag lock	*/
				Release(foodToBagLock);
				/* No customers to serve and no food to bag so the Order Taker 	*/
				/* has nothing to do, Set the status of that Order Taker as FREE	*/
				/* Check if all the Customers are serviced	*/
				Acquire(custServedLock);
				if(custServed == CUST_COUNT){
					Release(custServedLock);
					Print_1Arg("OrderTaker %d calling E---X---I---T\n$", myIndex);
					Exit(0);
				}
				Release(custServedLock);
				Acquire(custLineLock);
				orderTakerStatus[myIndex] = OT_FREE;	
				Release(custLineLock);
			}		
		}
	
		if((orderTakerStatus[myIndex] == OT_WAIT) || (orderTakerStatus[myIndex] == OT_FREE))
		{
			/* Acquire the orderTakerLock before releasing the custLineLock 	*/
			/* - To ensure that the Customer acquires the orderTakerLock 	*/
			/*   after Order Taker does	*/
			Acquire(orderTakerLock[myIndex]);
			/*Release the custLineLock 	*/
			/* - so that the Customer can access the updated status of the Order Taker	*/
		    /* @ */
			 /* Print_1Arg("OrderTaker %d is waiting\n$", myIndex); */
			
			/* Order Taker goes on wait to receive a signal from the Customer 	*/
			/* which indicates Customer is ready to place the order	*/
			Wait(orderTakerCV[myIndex], orderTakerLock[myIndex]);
			
		 	Release(orderTakerLock[myIndex]);
			
			Acquire(custServedLock);
			if(custServed == CUST_COUNT){
				Release(custServedLock);
				/* @ */
				/* Print_1Arg("OrderTaker %d calling E---X---I---T\n$", myIndex); */
				Exit(0);
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
			if(custServed == CUST_COUNT){
				Release(custServedLock);
				/* @ */
				/* Print_1Arg("OrderTaker %d calling E---X---I---T\n$", myIndex); */
				Exit(0);
			}
			Release(custServedLock);
			
			/* Received a signal from the customer after placing an order 	*/
			/* Find the customer index once the order is placed	*/
			Acquire(customerDataLock);
			for( index = 1; index <= CUST_COUNT; index++){
				/* find the customer index by finding the customer who has their Order	*/
				/* taker index as the current Order taker Index Thread	*/
				if(custData[index].myOT == myIndex){
					if(custData[index].ordered != 1){
						custIndex = index;
						custData[index].ordered = 1;
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
			
			if(custData[custIndex].dineType == 1){
				custHasToWait = 0;
				/* if 6$burger is ordered	*/
				Acquire(customerDataLock);
				if(custData[custIndex].sixBurger == 1){
					Release(customerDataLock);
					/* if 6$burger is ready	*/
					Acquire(foodReadyDBLock);
					if(foodReadyData.sixBurger != 0){
						Release(foodReadyDBLock);
						/*reserve a burger for this customer	*/
						Acquire(customerDataLock);
						custData[custIndex].sixBurger = 2;
						Release(customerDataLock);
						/* if greater than minimum amount of cooked food	*/
						/* then bag it, no worries	*/
						Acquire(foodReadyDBLock);
						if(foodReadyData.sixBurger > MINCOOKEDFOOD){
							foodReadyData.sixBurger--;	
							Release(foodReadyDBLock);
							Acquire(foodToBeCookedDBLock);
							foodToBeCookedData.sixBurger--;
							Release(foodToBeCookedDBLock);
						}
						else{
						/* else if it is less than minimum then 	*/
						/* bag it and increment the food to be cooked	*/
							foodReadyData.sixBurger--;	
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
				if(custData[custIndex].threeBurger == 1){
					Release(customerDataLock);
					/* if 3$burger is ready	*/
					Acquire(foodReadyDBLock);
					if(foodReadyData.threeBurger != 0){
						Release(foodReadyDBLock);
						/*reserve a burger for this customer	*/
						Acquire(customerDataLock);
						custData[custIndex].threeBurger = 2;
						Release(customerDataLock);
						/* if greater than minimum amount of cooked food	*/
						/* then bag it, no worries	*/
						Acquire(foodReadyDBLock);
						if(foodReadyData.threeBurger > MINCOOKEDFOOD){
						foodReadyData.threeBurger--;
						Release(foodReadyDBLock);
						Acquire(foodToBeCookedDBLock);
						foodToBeCookedData.threeBurger--;
						Release(foodToBeCookedDBLock);
						}
						/* else if it is less than minimum then 	*/
						/* bag it and do not decrement the food to be cooked	*/
						else{
							foodReadyData.threeBurger--;
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
				if(custData[custIndex].vegBurger == 1){
					Release(customerDataLock);
					/* if vegburger is ready	*/
					Acquire(foodReadyDBLock);
					if(foodReadyData.vegBurger != 0){
					Release(foodReadyDBLock);
						/*reserve a burger for this customer*/
						Acquire(customerDataLock);
						custData[custIndex].vegBurger = 2;
						Release(customerDataLock);
						/* if greater than minimum amount of cooked food	*/
						/* then bag it, no worries	*/
						Acquire(foodReadyDBLock);
						if(foodReadyData.vegBurger > MINCOOKEDFOOD){
							foodReadyData.vegBurger--;	
							Release(foodReadyDBLock);
							Acquire(foodToBeCookedDBLock);
							foodToBeCookedData.vegBurger--;
							Release(foodToBeCookedDBLock);
						}
						/* else if it is less than minimum then 	*/
						/* bag it and do not decrement the food to be cooked	*/
						else{
							foodReadyData.vegBurger--;	
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
				if(custData[custIndex].fries == 1){
				Release(customerDataLock);
					/* if fries is ready	*/
					Acquire(foodReadyDBLock);
					if(foodReadyData.fries != 0){
					Release(foodReadyDBLock);
						/*reserve a fries for this customer	*/
						Acquire(customerDataLock);
						custData[custIndex].fries = 2;
						Release(customerDataLock);
						/* if greater than minimum amount of cooked food	*/
						/* then bag it, no worries	*/
						Acquire(foodReadyDBLock);
						if(foodReadyData.fries > MINCOOKEDFOOD){
							foodReadyData.fries--;	
							Release(foodReadyDBLock);
							Acquire(foodToBeCookedDBLock);
							foodToBeCookedData.fries--;
							Release(foodToBeCookedDBLock);
						}
						/* else if it is less than minimum then 	*/
						/* bag it and do not decrement the food to be cooked	*/
						else{
							foodReadyData.fries--;
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
					
					nextTokenNumber++;
					custData[custIndex].tokenNo = nextTokenNumber;
					
					/* Release the lock after obtaining the new token number	*/
					Release(nextTokenNumberLock);
					/* Acquire the foodToBagLock to increment the foodToBag count	*/
					Acquire(foodToBagLock);
					foodToBag++;
					/* Release the foodToBagLock after updating the foodToBag count	*/
					Release(foodToBagLock);
					
					/* OG	*/
					Print_3Arg("OrderTaker %d gives token number %d to TOGO Customer %d\n$", myIndex, custData[custIndex].tokenNo, custIndex);
				}else{
					/* food is ready and can be bagged	*/
					/* Even if the customer has ordered only soda	*/
					/* it is bagged here	*/
					/* Order is bagged so update the food to bag count
	
/*CHANGE:ADD ============*/	Acquire(BagTheOrdersLock);					
					/* make the customer order status as bagged	*/
					custData[custIndex].bagged = 1;
/*CHANGE:ADD ============*/	Release(BagTheOrdersLock);					
/*CHANGE:ADD ============*/	Acquire(customerDataLock);
			
				custData[custIndex].delivered = 1;
					/* OG	*/
					Print_2Arg("OrderTaker %d packs and gives food to TOGOCustomer %d\n$", myIndex, custIndex);
					/* Release the foodToBagLock after updating the foodToBag count	*/
					Release(customerDataLock);
					/* Order Taker will bag only one order at a time so break	*/
				}
			}	/* dineType = to-go		*/
				
			/* if eat-in	*/
			if(custData[custIndex].dineType == 0){
				custHasToWait = 0;
				/* always customer has to wait, he will not get food right away	*/
				/* set the token number for the customer	*/
				/* Acquire the lock to get the next token number to be given 	*/
				/* to the customer, all Order Takers access this value	*/
				/* - increment this monitor variable to generate the new unique	*/
				/*   token number and give it to customer	*/
				Acquire(nextTokenNumberLock);
				nextTokenNumber++;
				custData[custIndex].tokenNo = nextTokenNumber;		
				/* Release the lock after obtaining the new token number	*/
				/* OG	*/
				Print_3Arg("OrderTaker %d gives token number %d to EATINCustomer %d\n$", myIndex, custData[custIndex].tokenNo, custIndex);
				Release(nextTokenNumberLock);

				/* eat-in customer always waits	*/			
	
				/* Acquire the foodToBagLock to increment the foodToBag count	*/
				/* Release the foodToBagLock after updating the foodToBag count	*/
				
				/* Order taker has to update the foodToBeCooked & foodReady database	*/
				/* if 6$burger is ordered	*/
				Acquire(customerDataLock);
				if(custData[custIndex].sixBurger == 1){
					Release(customerDataLock);
					/* if 6$burger is ready	*/
					Acquire(foodReadyDBLock);
					if(foodReadyData.sixBurger != 0){
					Release(foodReadyDBLock);
						/*reserve a burger for this customer	*/
						Acquire(customerDataLock);
						custData[custIndex].sixBurger = 2;
						Release(customerDataLock);
						/* if greater than minimum amount of cooked food	*/
						/* then bag it, no worries	*/
						Acquire(foodReadyDBLock);
						if(foodReadyData.sixBurger > MINCOOKEDFOOD){
							foodReadyData.sixBurger--;	
							Release(foodReadyDBLock);
							Acquire(foodToBeCookedDBLock);
							foodToBeCookedData.sixBurger--;
							Release(foodToBeCookedDBLock);
						}
						/* else if it is less than minimum then 	*/
						/* bag it and increment the food to be cooked	*/
						else{
							foodReadyData.sixBurger--;	
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
				if(custData[custIndex].threeBurger == 1){
					Release(customerDataLock);
					/* if 3$burger is ready	*/
					Acquire(foodReadyDBLock);
					if(foodReadyData.threeBurger != 0){
						Release(foodReadyDBLock);
						/*reserve a burger for this customer	*/
						Acquire(customerDataLock);
						custData[custIndex].threeBurger = 2;
						Release(customerDataLock);
						/* if greater than minimum amount of cooked food	*/
						/* then bag it, no worries	*/
						Acquire(foodReadyDBLock);
						if(foodReadyData.threeBurger > MINCOOKEDFOOD){
						foodReadyData.threeBurger--;
						Release(foodReadyDBLock);
						Acquire(foodToBeCookedDBLock);
						foodToBeCookedData.threeBurger--;
						Release(foodToBeCookedDBLock);
						}
						/* else if it is less than minimum then 	*/
						/* bag it and do not decrement the food to be cooked	*/
						else{
							foodReadyData.threeBurger--;
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
				if(custData[custIndex].vegBurger == 1){
					Release(customerDataLock);
					/* if vegburger is ready	*/
					Acquire(foodReadyDBLock);
					if(foodReadyData.vegBurger != 0){
					Release(foodReadyDBLock);
						/*reserve a burger for this customer*/
						Acquire(customerDataLock);
						custData[custIndex].vegBurger = 2;
						Release(customerDataLock);
						/* if greater than minimum amount of cooked food	*/
						/* then bag it, no worries	*/
						Acquire(foodReadyDBLock);
						if(foodReadyData.vegBurger > MINCOOKEDFOOD){
							foodReadyData.vegBurger--;	
							Release(foodReadyDBLock);
							Acquire(foodToBeCookedDBLock);
							foodToBeCookedData.vegBurger--;
							Release(foodToBeCookedDBLock);
						}
						/* else if it is less than minimum then 	*/
						/* bag it and do not decrement the food to be cooked	*/
						else{
							foodReadyData.vegBurger--;	
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
				if(custData[custIndex].fries == 1){
				Release(customerDataLock);
					/* if fries is ready	*/
					Acquire(foodReadyDBLock);
					if(foodReadyData.fries != 0){
					Release(foodReadyDBLock);
						/*reserve a fries for this customer	*/
						Acquire(customerDataLock);
						custData[custIndex].fries = 2;
						Release(customerDataLock);
						/* if greater than minimum amount of cooked food	*/
						/* then bag it, no worries	*/
						Acquire(foodReadyDBLock);
						if(foodReadyData.fries > MINCOOKEDFOOD){
							foodReadyData.fries--;	
							Release(foodReadyDBLock);
							Acquire(foodToBeCookedDBLock);
							foodToBeCookedData.fries--;
							Release(foodToBeCookedDBLock);
						}
						/* else if it is less than minimum then 	*/
						/* bag it and do not decrement the food to be cooked	*/
						else{
							foodReadyData.fries--;
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
			
/*CHANGE:ADD ============*/	Acquire(BagTheOrdersLock);
					custData[custIndex].bagged = 1;
					
/*CHANGE:ADD ============*/	Release(BagTheOrdersLock);
				
					Acquire(foodBaggedListLock);
					for(indexj = 1; indexj <= CUST_COUNT; indexj++){	
						if(baggedList[indexj] == 0){
						baggedList[indexj] = custData[custIndex].tokenNo;
						break;
						}
					}
					/* OG	*/
					Print_3Arg("Ordertaker %d gives Token number %d to Waiter for EATIN Customer %d rightnow \n$", myIndex, custData[custIndex].tokenNo, custIndex);
					Release(foodBaggedListLock);
				}else{
					/* Acquire the foodToBagLock to increment the foodToBag count	*/
					Acquire(foodToBagLock);
					foodToBag++;
					/* Release the foodToBagLock after updating the foodToBag count	*/
					Release(foodToBagLock);
				}
			}
		
			Acquire(customerDataLock);
			if(custData[custIndex].sixBurger == 2){
				Release(customerDataLock);
				Acquire(foodToBeCookedDBLock);
				foodToBeCookedData.sixBurger++;
				Release(foodToBeCookedDBLock);
			}else
				Release(customerDataLock);
			
			Acquire(customerDataLock);
			if(custData[custIndex].threeBurger == 2){
				Release(customerDataLock);
				Acquire(foodToBeCookedDBLock);
				foodToBeCookedData.threeBurger++;
				Release(foodToBeCookedDBLock);
			}else
				Release(customerDataLock);
			
			Acquire(customerDataLock);
			if(custData[custIndex].vegBurger == 2){
				Release(customerDataLock);
				Acquire(foodToBeCookedDBLock);
				foodToBeCookedData.vegBurger++;
				Release(foodToBeCookedDBLock);
			}else
				Release(foodToBeCookedDBLock);
			
			Acquire(customerDataLock);
			if(custData[custIndex].fries == 2){
				Release(customerDataLock);
				Acquire(foodToBeCookedDBLock);
				foodToBeCookedData.fries++;
				Release(foodToBeCookedDBLock);
			}else
			Release(foodToBeCookedDBLock);
 
			
			Acquire(orderTakerLock[myIndex]);
			/* Send a signal to the Customer which indicates that the 	*/
			/* Order Taker has processed the order	*/
			Signal(orderTakerCV[myIndex], orderTakerLock[myIndex]);
				
			/* wait for the customer to pay money	*/
			Wait(orderTakerCV[myIndex], orderTakerLock[myIndex]);
			Release(orderTakerLock[myIndex]);
				
			Acquire(custServedLock);
			if(custServed == CUST_COUNT){
				Release(custServedLock);
				/* @ */
				/* Print_1Arg("OrderTaker %d calling E---X---I---T\n$", myIndex); */
				Exit(0);
			}
			Release(custServedLock);
			
			/* received signal from customer which indicates that the customer	*/
			/* has paid the money	*/
			/* Time to put the money received in a place where all money is	*/
			/* stored by all the Order takers. This money will be used by the	*/ 
			/* manager to refill the inventory	*/
			Acquire(customerDataLock);
			if(custData[custIndex].sixBurger > 0)
				billAmount += 6;	/* 6$Burger	*/
			if(custData[custIndex].threeBurger > 0)
				billAmount += 3;	/* 3$Burger	*/
			if(custData[custIndex].vegBurger > 0)
				billAmount += 8;	/* vegBurger costs 8$ */
			if(custData[custIndex].fries > 0)
				billAmount += 2;	/* fries costs 2$ */
			if(custData[custIndex].soda > 0)
				billAmount += 1;	/* soda costs 1$ */
			Release(customerDataLock);
			
			Acquire(moneyAtRestaurantLock);
			moneyAtRestaurant += billAmount;
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
			if(custServed == CUST_COUNT){
				Release(custServedLock);
				/* @ */
				/* Print_1Arg("OrderTaker %d calling E---X---I---T\n$", myIndex); */
				Exit(0);
			}
			Release(custServedLock);	
		}
		
	}   /* While(1) 	*/
}	
