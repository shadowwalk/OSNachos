 // exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "noff.h"

#ifdef CHANGED
#include "post.h"
#endif

#include <stdio.h>
#include <iostream>
int evictAPage(int pageToEvict);
using namespace std;

//******************************************************************************
//							NETWORKING THREAD
//******************************************************************************
#define LOCK_BUSY 0
#define LOCK_FREE 1
#define EVENTORDERARRAY_SIZE 1500

/*   unsigned long int getTimeStamp()
{
	struct timeval tval;  
	struct timezone tzone;  
	struct tm *tm;  
	gettimeofday(&tval, &tzone);  
	tm=localtime(&tval.tv_sec);  
	return((unsigned long int)(tval.tv_usec + tval.tv_sec));   
}
   */
  unsigned long int getTimeStamp()
{

	struct timeval tval;  
	struct timezone tzone;  
	struct tm *tm;  
	gettimeofday(&tval, NULL);
	return((unsigned long int)((tval.tv_sec - 1291005818) * 16384 +(tval.tv_usec/100)));
}  

void NetworkingThread() {
	
	// $
	//printf("NetworkingThread : in net thread with mail box = %d\n", currentThread->mailBox);

	//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
	//						VARIABLES FOR PROCESSING VARIOUS ACTION REQUEST MESSAGES
	//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$	
	
	int type = 0;		// stores the integer format of the rpcType
	bool success = false;
	bool failure = false;
	int netThreadCount = 0;		// number of Networking threads in the group
	int index = 0;
	long int prevTS = 0;
	
	// Other communication
	PacketHeader inPktHdr, outPktHdr;
	MailHeader inMailHdr, outMailHdr;
	
	// Communication with REGISTRATION SERVER
	PacketHeader regOutPktHdr, regInPktHdr;
	MailHeader regOutMailHdr, regInMailHdr;
	
	int regMsgCount = 0;		// number of msges regServer going to send
	int next = 0;
	int nextMsg = 0;

	// set this when the net thread should stop processing the event ordered messages
	int stopProcessing = 0;
	int msgFormat = 0;		// update/action message format	
	// REGISTRATION SERVER END
	
	//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
	//					GLOBAL VARIABLES FOR THIS NETWORKING THREAD
	//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$	

	//********************************
	//		SERVER LOCKS
	//********************************
	struct serverSecretLocks
	{
		char *lockName;
		int machineID;
		int mailBoxNumber;
		int status;
		int toBeDestroyedCount;
		int usageCounter; 
	};
	serverSecretLocks locksDB[MAX_LOCKS];
	int locksDBIndex = -1;

	//********************************
	//		SERVER CVS
	//********************************
	struct serverSecretCVs
	{
		char *cvName;
		int machineID;
		int mailBoxNumber;
		int cvLockIndex;
		int toBeDestroyedCount;
		int usageCounter; 
	};
	serverSecretCVs cvsDB[MAX_CVS];
	int cvsDBIndex = -1;

	//********************************
	//		SERVER MVS
	//********************************
	struct serverSecretMVs
	{
		char *mvName;
		int toBeDestroyedCount;
		int value;
	};
	serverSecretMVs mvsDB[MAX_MVS];
	int mvsDBIndex = -1;

	//********************************
	//		LOCK WAIT ENTRY
	//********************************
	struct lockWaitEntry
	{
		char *replymessage;
		PacketHeader replyOutPktHdr;
		MailHeader replyOutMailHdr;
	};
	List *lockqueue[MAX_LOCKS];
	lockWaitEntry *lockEntry[MAX_LOCKS];
	lockWaitEntry *removedLockEntry[MAX_LOCKS];
	int lockEntryCounter = -1;
	int removedLockEntryCounter = -1;
	
	//********************************
	//		CV WAIT ENTRY
	//********************************
	struct cvWaitEntry
	{
		char *replymessage;
		PacketHeader replyOutPktHdr;
		MailHeader replyOutMailHdr;
	};
	List *cvqueue[MAX_CVS];
	cvWaitEntry *CVEntry[MAX_CVS];
	cvWaitEntry* removedCVEntry[MAX_CVS];
	int cvEntryCounter = -1;
	int removedCVEntryCounter = -1;
	
	//********************************
	//		GROUP TABLE
	//********************************
	struct groupEntry
	{
		int machineId;
		int mailBox;
		long int timeStamp;
	};
	groupEntry *groupTable[MAX_NETTHREADS];
	
	//********************************
	//		EVENTORDER ENTRY
	//********************************
	struct eventOrderEntry
	{
		char replymessage[MaxMailSize];
		PacketHeader replyOutPktHdr;
		MailHeader replyOutMailHdr;
		long int timeStamp;
	};
	int eventOrderArrayCounter = -1;
	eventOrderEntry *eventOrderArray[EVENTORDERARRAY_SIZE];
	eventOrderEntry *removedEventOrderEntry = NULL;
	
	// EVENT ORDER QUEUE
	List *eventOrderQueue;
	
	//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
	//					ALLOCATE MEMORY FOR THE ABOVE ARRAY OF STRUCTURES
	// $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
	
	// Structures used to store and fetch the packets from the lock/cv wait queues	
	for(index = 0; index < MAX_LOCKS; index++){
	
		lockqueue[index] = new List;
		locksDB[index].toBeDestroyedCount = 0;
		locksDB[index].lockName = new char[MAX_CHARLEN];
		lockEntry[index] = new lockWaitEntry;
		lockEntry[index]->replymessage = new char[MaxMailSize];
		removedLockEntry[index] = new lockWaitEntry;
		removedLockEntry[index]->replymessage = new char[MaxMailSize];
		
	}
	
	for(index = 0; index < MAX_CVS; index++){
	
		cvqueue[index] = new List;
		cvsDB[index].toBeDestroyedCount = 0;
		cvsDB[index].cvName = new char[MAX_CHARLEN];
		removedCVEntry[index] = new cvWaitEntry;
		removedCVEntry[index]->replymessage = new char[MaxMailSize];	
		CVEntry[index] = new cvWaitEntry;
		CVEntry[index]->replymessage = new char[MaxMailSize];
		
	}
	
	for(index = 0; index < MAX_MVS; index++){
	
		mvsDB[index].toBeDestroyedCount = 0;
		mvsDB[index].mvName = new char[MAX_CHARLEN];
		
	}
	
	for(index = 0; index < MAX_NETTHREADS; index++){
	
		groupTable[index] = new groupEntry;
		
	}
	
	eventOrderQueue = new List;
	
	for(index = 0; index < EVENTORDERARRAY_SIZE; index++){
	
		eventOrderArray[index] = new eventOrderEntry;
		
	}	
	
//*********************************************************************************************
//*********************************************************************************************
//							COMMUNICATE WITH THE REGISTRATION SERVER
//*********************************************************************************************				
	// receive a msg from the userprog asking "Are you ready?"
	// Do not reply to this client until the group is formed
	
	// new this when ever required and delete after use
	char *sendMessageServer; 
	char *receiveMessageServer; 
	
	receiveMessageServer = new char[MaxMailSize];
	strcpy(receiveMessageServer, "\0");
	
	// $
	//printf("Networking Thread : start waiting for first msg from user program on mailbox %d\n", currentThread->mailBox);
	postOffice->Receive(currentThread->mailBox, &inPktHdr, &inMailHdr, receiveMessageServer);
//	//printf("Networking Thread : Got \"%s\" from %d, box %d\n", receiveMessageServer, inPktHdr.from, inMailHdr.from);
	fflush(stdout);	
	
	// $
	//printf("NetworkingThread : received first message from userprog\n");
	
	// delete after use
	delete receiveMessageServer;
	
	// construct packet, mail header for message to the Registration server
	// To: Registration server thread machine, mailbox 0
	// From: Networking Thread
	regOutPktHdr.to = serverMachineID;		
	regOutMailHdr.to = serverMailBoxNumber;
	regOutMailHdr.from = currentThread->mailBox;
	regOutPktHdr.from = clientMachineID;	
	
	sendMessageServer = new char[MaxMailSize]; 
	
	strcpy(sendMessageServer, "\0");
	strcpy(sendMessageServer, "Server, get me registered");
	
	regOutMailHdr.length = strlen(sendMessageServer) + 1;
	// Send the message to Registration server
	success = postOffice->Send(regOutPktHdr, regOutMailHdr, sendMessageServer); 
	
	// $
	//printf("NetworkingThread : Server, get me registered msg sent to server\n");
	
	if ( !success ) {
		//printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
		interrupt->Halt();
	}	
	
	// delete after use
	delete sendMessageServer; 
	
	
	receiveMessageServer = new char[MaxMailSize]; 
	
	// Receive a reply for it - contains the number of msges that will be sent in future
	strcpy(receiveMessageServer, "\0");
	postOffice->Receive(currentThread->mailBox, &inPktHdr, &inMailHdr, receiveMessageServer);
		
	//printf("Networking Thread : First Server reply received -  \"%s\" from %d, box %d\n", receiveMessageServer, inPktHdr.from, inMailHdr.from);
	fflush(stdout);	
	
	// Find the number of msges regServer going to send
	// MSGFORMAT : regMsgCount@netThreadCount@
	
	char *str1;
	str1 = strtok(receiveMessageServer, "@");
	
	regMsgCount = atoi(str1);
	
	// NOTE - check if this works for net threads morethan 10
	// Find the number of group members registered with the server
	str1 = strtok(NULL, "@");
	netThreadCount = atoi(str1);
	
	// delete after use
	//delete str1;
	delete receiveMessageServer;
	
	// for loop of count received from regServer- receive msg 
	nextMsg = 0;
	for(int memberIndex = 0; memberIndex < regMsgCount; memberIndex++){
		
		receiveMessageServer = new char[MaxMailSize]; 
		strcpy(receiveMessageServer, "\0");
		
		postOffice->Receive(currentThread->mailBox, &inPktHdr, &inMailHdr, receiveMessageServer);
		//printf("Networking Thread : Got \"%s\" from %d, box %d\n", receiveMessageServer, inPktHdr.from, inMailHdr.from);
		fflush(stdout);	
		
		next = 0;
		// Msg format will be [(machineid)(mailBoxTensPlace)(mailBoxUnitsPlace)].....@
		for(int i = nextMsg; i < 13; i++){
			// $
			//printf("Networking Thread : looping and receiving the data messages from reg server\n");
			// Fetch the information of the other networking threads sent from the server in the msg
			next = i*3;
			
			if(receiveMessageServer[next] == '@')
				break;

			(*groupTable[i]).machineId = (int(receiveMessageServer[next])) - '0';		
			(*groupTable[i]).mailBox = ((int(receiveMessageServer[next+1])) - '0') * 10 + (int(receiveMessageServer[next+2])) - '0';	
			(*groupTable[i]).timeStamp = -1;
			// $
			//printf("Networking Thread : storing the net thread details machine id = %d and mail box number = %d\n", (*groupTable[i]).machineId, (*groupTable[i]).mailBox);
		}
		nextMsg += 13; 
		
		// delete after use
		delete receiveMessageServer;
	}
	
/* 	// Printing INITIAL GROUP TABLE
	//printf("\n__________________________________________________________\n");
	//printf("		INITIAL GROUP  TABLE\n");
	//printf("__________________________________________________________\n");
	for(int i = 0; i < netThreadCount; i++){
		//printf(" Machine 	| 	Mailbox 	| 	Timestamp\n");
		//printf(" %d 		| 	%d 		|	%ld\n", (*groupTable[i]).machineId, (*groupTable[i]).mailBox, (*groupTable[i]).timeStamp);
	}	
	//printf("__________________________________________________________\n\n"); */
	 
//*****************COMMUNICATE WITH THE REGISTRATION SERVER END*********************************
	
	// Reply to the user program to start simulation
	// construct packet, mail header for message to the user program
	// To: User program thread in current machine
	// From: Networking Thread
	outPktHdr.to = clientMachineID;		
	outMailHdr.to = currentThread->mailBox-1;
	outMailHdr.from = currentThread->mailBox;
	outPktHdr.from = clientMachineID;	
	// $
//	//printf("Networking Thread : sending HEY USERPROG, IM READY msg to userprogram at mailbox %d\n", outMailHdr.to);
	
	sendMessageServer = new char[MaxMailSize]; 
	strcpy(sendMessageServer, "\0");
	
	strcpy(sendMessageServer, " Hey UserProg, Im ready\n");
	
	outMailHdr.length = strlen(sendMessageServer) + 1;
	
	// DELAY THE READY REPLY MSG TILL THE SERVER REPLIES TO EVERY NETWORKING THREAD
	// $
	//printf("\nWAIT for all NET threads to get ready for group communiation\n");
	Delay(1);
	
	// Send the message to user program
	success = postOffice->Send(outPktHdr, outMailHdr, sendMessageServer); 
	if ( !success ) {
		//printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
		interrupt->Halt();
	}	
	
	// delete after use 
	delete sendMessageServer;
	
	// WHILE LOOP BEGINS
	
	int leastTimeStampMachine = 0;
	int leastTimeStampMailbox = 0;
	int toBeProcessedMachine = 0;
	int toBeProcessedMailbox = 0;
	
	long int toBeProcessedLong = 0;
	long int msgTimeStamp = 0;
	long int timeStamp = 0;
	long int leastTimeStampLong = 0;
	
	int64_t *toBeProcessed = NULL;			// int64_t = long
	int64_t leastTimeStamp64 = 0;
	
	// new the pointers when ever required and delete them after use
	char *sendMessage; 
	char *receiveMessage; 
	char *msgType;
	char *msgToQueue;
	
	while(true) {
		
		// Refresh START
		msgTimeStamp = 0;
		leastTimeStamp64 = 0;
		leastTimeStampLong = 0;
		leastTimeStampMachine = 0;
		leastTimeStampMailbox = 0;
		toBeProcessed = NULL;			// int64_t = long int / long long int
		toBeProcessedLong = 0;
		toBeProcessedMachine = 0;
		toBeProcessedMailbox = 0;
		
		bool broadcastBit = false;	
		// Refresh END		
		
		// Wait for the message from anyone,
		// 1. Update message from group member
		// 2. Action message from group member
		// 3. User program message
		
		// $
//		//printf("\n\nNetworking Thread : WAITING to receive message in WHILE loop\n\n");
		
		receiveMessage = new char[MaxMailSize]; 
		strcpy(receiveMessage, "\0");
		
		postOffice->Receive(currentThread->mailBox, &inPktHdr, &inMailHdr, receiveMessage);
//		//printf("Networking Thread : Got \"%s\" from %d, box %d\n", receiveMessage, inPktHdr.from, inMailHdr.from);
		fflush(stdout);	

//*********************************************************************************************
//*********************************************************************************************
//									TOTAL ORDERING OF EVENTS
//*********************************************************************************************				
		// CASE 1 : IF FROM USER PROGRAM
		if(inPktHdr.from == clientMachineID && inMailHdr.from == (currentThread->mailBox - 1)) {
			
			// $
//			//printf("Networking Thread : Received msg from USER PROG\n");
					
			// STEP 1 : Appending the timestamp and broadcasting the message to EVERYONE in the group
			timeStamp = 0;
			index = 0;
			success = false;
			
			// Find the new timestamp to append with the message received
			timeStamp = getTimeStamp();
			
			// Send the Action message to ALL in the group	
			// MESSAGE FORMAT : #@ACTION MESSAGE@TIMESTAMP@
			
			for(index = 0; index < netThreadCount; index++) {
			
				sendMessage = new char[MaxMailSize]; 
				strcpy(sendMessage, "\0");
				
				// construct packet, mail header for message to group members
				// To: Group member thread machine & mailbox
				// From: Current Networking Thread
				outPktHdr.to = (*groupTable[index]).machineId;		
				outMailHdr.to = (*groupTable[index]).mailBox;
				outMailHdr.from = currentThread->mailBox;
				outPktHdr.from = clientMachineID;	
				
				// Concat received msg from user program with the timeStamp obtained
				sprintf(sendMessage, "#@%s@%ld@", receiveMessage, timeStamp);
				
				outMailHdr.length = strlen(sendMessage) + 1;
				
				// Send the message to "index"ed member in the group
				// $
//				//printf("Networking Thread : (BCASTING) Sending ACTION message -> %s <- to other net thread on machine %d with mailbox %d\n", sendMessage, (*groupTable[index]).machineId, (*groupTable[index]).mailBox);
				success = postOffice->Send(outPktHdr, outMailHdr, sendMessage); 
				if ( !success ) {
					//printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
				//	interrupt->Halt();
				}	
				
				// delete after use
				delete sendMessage;
			}	
		
		} // else its from networking thread
		else{
			
			msgType = new char[MaxMailSize];
				
			// Find the type of network messge (update/action)
			strcpy(msgType, receiveMessage);
			strtok(msgType, "@");
			
			
			// IF msgType = # -> action
			// IF msgType = $ -> update
			
			
			// CASE 2 : IF FROM NETWORKING THREAD - ACTION MESSAGE
			// MESSAGE FORMAT : #@ACTION MESSAGE@TIMESTAMP@
			if(!strcmp(msgType, "#")) {
			
				// $
//				//printf("Networking Thread : Received action message from NET THREAD\n");
				
				//	msgToQueue = ACTION MESSAGE
				msgToQueue = strtok(NULL, "@");
				
				//	STEP 2 : Fetch the timestamp present in the reply sent from the group member 
				//	str = TIMESTAMP
				
				char *str2;
				
				str2 = strtok(NULL, "@");
				msgTimeStamp = atol(str2);
				
	
				// STEP 3 : Update the group table entry for the group member with the NEW TIME STAMP
				for(int memberIndex = 0; memberIndex < netThreadCount; memberIndex++) {
					if((*groupTable[memberIndex]).machineId == inPktHdr.from && (*groupTable[memberIndex]).mailBox == inMailHdr.from){	
						(*groupTable[memberIndex]).timeStamp = msgTimeStamp;
						break;
					}	
				}
	 			
			/* 	// Printing GROUP TABLE	
				//printf("\n__________________________________________________________\n");
				//printf("		GROUP  TABLE OF M %d MB %d\n", clientMachineID, currentThread->mailBox);
				//printf("__________________________________________________________\n");
				for(int i = 0; i < netThreadCount; i++){
					//printf(" Machine 	| 	Mailbox 	| 	Timestamp\n");
					//printf(" %d 		| 	%d 		|	%ld\n", (*groupTable[i]).machineId, (*groupTable[i]).mailBox, (*groupTable[i]).timeStamp);
				}	
				//printf("__________________________________________________________\n\n"); */
			 
	
				// STEP 4 : Store the message in the sorted Queue in increasing order of the timestamps
				eventOrderArrayCounter++;
				
				if(eventOrderArrayCounter == 1499)
					eventOrderArrayCounter = 1;
				
			//	//printf("eventOrderArrayCounter = %d\n" ,eventOrderArrayCounter);
				//	TO : Action Request Sender Networking thread
				//	FROM : Current Networking Thread
				
				if(prevTS == msgTimeStamp)
					msgTimeStamp = msgTimeStamp + inPktHdr.from + inMailHdr.from;
					
				prevTS = msgTimeStamp;
				
			//	int64_t breakTheSameTS = msgTimeStamp + inPktHdr.from + inMailHdr.from;
			
			//	int64_t breakTheSameTS = (((int64_t)msgTimeStamp * 4096) + (inPktHdr.from * 256) + (inMailHdr.from)); 
				
				(*eventOrderArray[eventOrderArrayCounter]).replyOutPktHdr.to = inPktHdr.from;
				(*eventOrderArray[eventOrderArrayCounter]).replyOutPktHdr.from = clientMachineID;	
				(*eventOrderArray[eventOrderArrayCounter]).replyOutMailHdr.to = inMailHdr.from; 
				(*eventOrderArray[eventOrderArrayCounter]).replyOutMailHdr.from = currentThread->mailBox;
				
				// Store ACTION MESSAGE in the event order queue
				strcpy((*eventOrderArray[eventOrderArrayCounter]).replymessage, msgToQueue);
//				//printf("Networking Thread : Storing the message in event order queue = %s\n", (*eventOrderArray[eventOrderArrayCounter]).replymessage);
				
				// Store TIMESTAMP in the event order queue
				(*eventOrderArray[eventOrderArrayCounter]).timeStamp = msgTimeStamp;
					
				// Append the ACTION MESSAGE in the event order queue 
				eventOrderQueue->SortedInsert(eventOrderArray[eventOrderArrayCounter], (int64_t)msgTimeStamp);
				
				// STEP 5 : Find NEW TIMESTAMP and send it to EVERYONE in the group including yourself	
				
				timeStamp = 0;
				index = 0;
				success = false;
				
				//	Find NEW TIMESTAMP
				timeStamp = getTimeStamp();
				
				for(index = 0; index < netThreadCount; index++) {
				
					sendMessage = new char[MaxMailSize]; 
					
					// construct packet, mail header for message to "index"ed group member
					// To: "index"ed group member thread machine & mailbox
					// From: Current Networking Thread
					outPktHdr.to = (*groupTable[index]).machineId;		
					outMailHdr.to = (*groupTable[index]).mailBox;
					outMailHdr.from = currentThread->mailBox;
					outPktHdr.from = clientMachineID;	
					
					// Concat received msg from user program with the timeStamp obtained
					// MESSAGE FORMAT : $@TIMESTAMP@
					sprintf(sendMessage, "$@%ld@", timeStamp);
					
					// $
//					//printf("Networking Thread : Sending the UPDATE message -> %s <- to machine %d with mailbox %d\n", sendMessage, outPktHdr.to, outMailHdr.to);
					
					outMailHdr.length = strlen(sendMessage) + 1;
					// Send the message to indexed member in the group
					success = postOffice->Send(outPktHdr, outMailHdr, sendMessage); 
					
					if ( !success ) {
						//printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
						interrupt->Halt();
					}	
				}	
				
				// delete after use
				delete msgType;
			
			}
			
			// CASE 3 : IF FROM NETWORKING THREAD - UPDATE MESSAGE
			// MESSAGE FORMAT : $@TIMESTAMP@
			if(!strcmp(msgType, "$")) {
				
				// $
//				//printf("Networking Thread : Received UPDATE message from net thread\n");
				
				char *str3;
				
				//	 str3 = TIMESTAMP in update message
				str3 = strtok(NULL, "@");
				msgTimeStamp = atol(str3);
			
				// STEP 6 : Update the group table entry for the SENDER group member with the new time stamp
				for(int memberIndex = 0; memberIndex < netThreadCount; memberIndex++) {
					if((*groupTable[memberIndex]).machineId == inPktHdr.from && (*groupTable[memberIndex]).mailBox == inMailHdr.from){	
						(*groupTable[memberIndex]).timeStamp = msgTimeStamp;
						break;
					}	
				}
				
			/* 	// Printing GROUP TABLE	
				//printf("\n__________________________________________________________\n");
				//printf("			GROUP  TABLE OF M %d MB %d\n", clientMachineID, currentThread->mailBox);
				//printf("__________________________________________________________\n");
				for(int i = 0; i < netThreadCount; i++){
					//printf(" Machine 	| 	Mailbox 	| 	Timestamp\n");
					//printf(" %d 		| 	%d 		|	%ld\n", (*groupTable[i]).machineId, (*groupTable[i]).mailBox, (*groupTable[i]).timeStamp);
				}	
				//printf("__________________________________________________________\n\n"); */
				 
				
				// delete after use
				delete msgType;
			}
			
			//------------------------>>>>>>>>>>>>>>	DO THE FOLLOWING IF	ACTION / UPDATE MESSAGE 	<<<<<<<<<<<<<-----------------------------------
			
			// PROCESS MESSAGES IN THE EVENT ORDER QUEUE UNTIL THE TIMESTAMP IS 
			// GREATER THAN THE LEAST TIMESTAMP IN THE GROUP TABLE
		
			leastTimeStampMachine = 0;
			leastTimeStampMailbox = 0;
			leastTimeStampLong = 0;
			
			// STEP 7 : Find the least timestamp value present in the group table
			// store the first timestamp and keep updating if a smaller one is found
			leastTimeStamp64 = (*groupTable[0]).timeStamp;		
			
			for(int memberIndex = 0; memberIndex < netThreadCount; memberIndex++) {
				if((*groupTable[memberIndex]).timeStamp <= leastTimeStamp64){	
					leastTimeStamp64 = (*groupTable[memberIndex]).timeStamp;
					leastTimeStampMachine = (*groupTable[memberIndex]).machineId;
					leastTimeStampMailbox = (*groupTable[memberIndex]).mailBox;
				}	
			}
			leastTimeStampLong = (long int)leastTimeStamp64;
			
			// $
//			//printf("Least time stamp in the group table = %ld\n", leastTimeStampLong);
			
			// STEP 8 : Remove the message entry in the event ordering table, process the message
			//			if its timestamp is less than the least timestamp value in the group table 
			stopProcessing = 0;		// set when processing has to stop
			
			while(stopProcessing == 0) {
			
				// Refresh the variables in every iteration of the server
				type = 0;
				success = false;
				failure = false;
				
				index = 0;
				
				// Check if there are any messages in the queue to process
				if(eventOrderQueue->IsEmpty()){
				
//					//printf("\n THERE ARE NO MESSAGES IN EVENT ORDER QUEUE\n\n");
					break;
				}
			
				toBeProcessedLong = 0;
				toBeProcessedMachine = 0;
				toBeProcessedMailbox = 0;
				
				removedEventOrderEntry = new eventOrderEntry;
				
				toBeProcessed = new int64_t;
				
				// remove the message at the front of the queue
				removedEventOrderEntry = (eventOrderEntry*)eventOrderQueue->SortedRemove(toBeProcessed);
					
				toBeProcessedLong = (long int)(*toBeProcessed);
				toBeProcessedMachine =  (*removedEventOrderEntry).replyOutPktHdr.to;
				toBeProcessedMailbox = (*removedEventOrderEntry).replyOutMailHdr.to;
			
			/* 
				// PRINTING THE COMPARISON___________________
				//printf("__________________________________________\n");
				//printf("	    COMPARING IN M %d MB %d\n", clientMachineID, currentThread->mailBox);
				//printf("  LEAST		| 	QUEUE\n");
				//printf(" %ld 	| 	%ld\n", leastTimeStampLong, toBeProcessedLong);
				//printf(" Machine 	| 	Machine\n");
				//printf(" %d 		| 	%d\n", leastTimeStampMachine, toBeProcessedMachine);
				//printf(" Mailbox 	| 	Mailbox\n");
				//printf(" %d 		| 	%d\n", leastTimeStampMailbox, toBeProcessedMailbox);	
				//printf("__________________________________________\n");
				//___________________________________________
				 */
			
				
				// check if message is processable
				if(*toBeProcessed <= leastTimeStamp64){
		//**************************************************************************************************
		//**************************************************************************************************			
		// 								PROCESS SAME AS PROJECT 3 PART 3 FROM HERE
		//**************************************************************************************************
		//**************************************************************************************************
					// $
			//		printf("\n %s Msg CAN BE PROCESSED\n\n", removedEventOrderEntry->replymessage);
					
					int shouldBreak = 0;
					
					int senderMachineID =0;				
					int senderMailBoxNumber = 0;
							
					char *removedMessage = new char[MaxMailSize];	
					char *rpcType;
					
					// Other communication
					PacketHeader tempInPktHdr, tempOutPktHdr;
					MailHeader tempInMailHdr, tempOutMailHdr;
									
					strcpy(removedMessage, removedEventOrderEntry->replymessage);	
					
					// removedMessage -> ACTIONTYPE MESSAGEBODY
					
					// rpcType = ACTIONTYPE
					rpcType = strtok(removedMessage, " ");
					
					// type = Integer format of rpcType
					type = atoi(rpcType); 
					
					sendMessage = new char[MaxMailSize]; 
					receiveMessage = new char[MaxMailSize]; 
					
					// ******************************************************************************************************************
					// IF AT ALL the networking thread is replying after processing the queued message then it will be to ITS USERPROGRAM
					//	TO : Current Net Thread User Program
					//	FROM : Current Net Thread
					outPktHdr.to = clientMachineID;
					outMailHdr.to = currentThread->mailBox-1;	
					outMailHdr.from = currentThread->mailBox;
					outPktHdr.from = clientMachineID;
					// ******************************************************************************************************************
					
					// delete all unused dynamic variables
					delete rpcType;
					delete removedMessage;
					
					int lockIndex = -1;
//					//printf("ACTION TYPE = %d\n", type);
					switch(type)
					{
						case 1:	//Create Lock						
							
							// new only when needed
							char *sendMessageCL;
							char *nameCL;
							lockIndex = -1;
							
							// nameCL = name of the lock to be created
							
							nameCL = strtok(NULL, " ");
							shouldBreak = 0;
							
							if(nameCL == NULL){
								//printf("Networking Thread : Lock Name not sent to the Networking Thread\n");
								
								// SEND only if the message belongs to the current networking thread (check the "TO" fields of the QUEUE message)
								if(removedEventOrderEntry->replyOutPktHdr.to == clientMachineID && removedEventOrderEntry->replyOutMailHdr.to == currentThread->mailBox) {
								
									// new only if the message has to be sent
									sendMessageCL = new char[MaxMailSize];
											
									sprintf(sendMessageCL, "FAILURE : Lock Name is NULL");
									// Format the reply message to the client waiting for the Lock Index from the server
									outMailHdr.length = strlen(sendMessageCL) + 1;
									
									//	TO : Current Net Thread User Program
									//	FROM : Current Net Thread
									success = postOffice->Send(outPktHdr, outMailHdr, sendMessageCL); 

									if ( !success ) {
										//printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
										interrupt->Halt();
									}	
									
									// delete after using it
									delete sendMessageCL;
								}
								
								break;		
							}
							
							
							//*********************************************************************************************
							//									CREATE LOCK
							//	name - Name of the Lock to be created on the server
							//	senderMachineID - Owner machine of the Lock that is going to be created
							//	senderMailBoxNumber - Owner machine mail box number of the Lock that is going to be created			
							//*********************************************************************************************						
							success = 0;
				
							//------------------------------------------------------------------------------------------
							// 							Validate the Lock parameters received
							//------------------------------------------------------------------------------------------
								
							for(index = 0; index <= locksDBIndex; index++){
								if(locksDB[index].toBeDestroyedCount != 0){
									if(strcmp(locksDB[index].lockName, nameCL) == 0){
										
										// Increment the number of creators of the Lock
										locksDB[index].toBeDestroyedCount++;
//										//printf("Networking Thread : Lock already exists with the same name %s\n", nameCL);
										
										// SEND only if the message belongs to the current networking thread (check the "TO" fields of the QUEUE message)
										if(removedEventOrderEntry->replyOutPktHdr.to == clientMachineID && removedEventOrderEntry->replyOutMailHdr.to == currentThread->mailBox) {
										
											// new only if the message has to be sent
											sendMessageCL = new char[MaxMailSize];
										
											sprintf(sendMessageCL, "%d : Lock already exists", index);
											
											// Format the reply message to the user prog waiting for the Lock Index from the Net thread
											// Sending the lock index of the lock which is already created by someother user program
											outMailHdr.length = strlen(sendMessageCL) + 1;
											// $
											//printf("Networking Thread : reply message is -> %s\n", sendMessageCL);
											
											//	TO : Current Net Thread User Program
											//	FROM : Current Net Thread
											success = postOffice->Send(outPktHdr, outMailHdr, sendMessageCL); 

											if ( !success ) {
												//printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
												interrupt->Halt();
											}	
											
											// delete after send
											delete sendMessageCL;
											
										}		
										shouldBreak = 1;
										break;
									}
								}	
							}
							if(shouldBreak == 1) {
								shouldBreak = 0;
								break;
							}
							
							if(locksDBIndex == (MAX_LOCKS-1)){
								//printf("Networking Thread : Reached max Lock Count, No more locks can be created\n");	
								
								// SEND only if the message belongs to the current networking thread (check the "TO" fields of the QUEUE message)
								if(removedEventOrderEntry->replyOutPktHdr.to == clientMachineID && removedEventOrderEntry->replyOutMailHdr.to == currentThread->mailBox) {
									
									sendMessageCL = new char[MaxMailSize];
									strcpy(sendMessageCL, "\0");
									
									sendMessageCL = "FAILURE : Reached max Lock Count";
									
									// Format the FAILURE reply message to the client waiting for the Lock Index from the server
									//sprintf(message, "%s", sendMessageCL);
									outMailHdr.length = strlen(sendMessageCL) + 1;
									// $
									//printf("Networking Thread : reply message is -> %s\n", sendMessageCL);
									
									//	TO : Current Net Thread User Program
									//	FROM : Current Net Thread
									success = postOffice->Send(outPktHdr, outMailHdr, sendMessageCL); 

									if ( !success ) {
										//printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
										interrupt->Halt();
									}	
									
									delete sendMessageCL;
								}
					
								break;
							}
							
							
							//------------------------------------------------------------------------------------------
							//							Update Server secret storage	
							//------------------------------------------------------------------------------------------

							// Move the index by one to store the information of the new lock created by the server
							locksDBIndex++;
							
							// Store the Lock information
							strcpy(locksDB[locksDBIndex].lockName, nameCL);
							// toBeDestroyed flag will be set when the destroy lock is called
							locksDB[locksDBIndex].toBeDestroyedCount = 1;
							
							// used to keep track of number of waiters waiting to acquire the Lock
							// when this value becomes zero when the last thread calls release then the 
							// toBeDestroyd flag is checked for 1. If 1 then lock is destroyed 
							locksDB[locksDBIndex].usageCounter = 0;
							locksDB[locksDBIndex].machineID = -1;
							locksDB[locksDBIndex].mailBoxNumber = -1;
							locksDB[locksDBIndex].status = LOCK_FREE;
							
							//------------------------------------------------------------------------------------------
							//							Form a reply message and send to user program
							//------------------------------------------------------------------------------------------
							
							// SEND only if the message belongs to the current networking thread (check the "TO" fields of the QUEUE message)
							if(removedEventOrderEntry->replyOutPktHdr.to == clientMachineID && removedEventOrderEntry->replyOutMailHdr.to == currentThread->mailBox) {
								
								sendMessageCL = new char[MaxMailSize];
								
								// Format the reply message to the client waiting for the Lock Index from the server
								sprintf(sendMessageCL, "%d ", locksDBIndex);
								outMailHdr.length = strlen(sendMessageCL) + 1;
								// $
//								//printf("Networking Thread : reply message is -> %s\n", sendMessageCL);
								
								//	TO : Current Net Thread User Program
								//	FROM : Current Net Thread
								success = postOffice->Send(outPktHdr, outMailHdr, sendMessageCL); 

								if ( !success ) {
									//printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
									interrupt->Halt();
								}
								
								delete sendMessageCL;
							}
							
							
							break;
							// *********************************************************************************************
							//									CREATE LOCK END
							// *********************************************************************************************
							
							
						case 2:	//Acquire Lock - LOCK OWNER -> ACTUAL USER PROGRAM MACHINE ID AND MAILBOX
						
							lockIndex = -1;
							index = 0;
							
							success = false;
							failure = false;
							
							char *sendMessageAL;
							char *acquireLockIndex;
							
							// message format -> ACTIONTYPE LOCKINDEX
							acquireLockIndex = strtok(NULL, " ");
							
							lockIndex = atoi(acquireLockIndex);
							
							/* if(lockIndex == -1){
								//printf("Networking Thread : LockIndex to acquire lock not passed to the Server\n");
								break;
							}
					 */
							// *********************************************************************************************
							//									ACQUIRE LOCK
							//	lockIndex - Index of the Lock to be acquired
							//	senderMachineID - machine id of the client that is requesting for acquire
							//	senderMailBoxNumber - machine mail box number of the client that is requesting for acquire
							// *********************************************************************************************
								
							// Actual user program machine and mailbox number	
							senderMachineID = removedEventOrderEntry->replyOutPktHdr.to;
							senderMailBoxNumber = (removedEventOrderEntry->replyOutMailHdr.to)-1;
							
							
							//------------------------------------------------------------------------------------------
							// 							Validate the parameters received
							//------------------------------------------------------------------------------------------	
							
							sendMessageAL = new char[MaxMailSize];
							
							//if Invalid lock index
							if(locksDB[lockIndex].toBeDestroyedCount == 0){
								sprintf(sendMessageAL, "FAILURE : Lock %d is Destroyed" ,lockIndex);
								failure = true;
							}
							else if((lockIndex < 0)||(lockIndex > locksDBIndex)){
								sprintf(sendMessageAL, "FAILURE : Invalid Lock index %d",lockIndex);
								failure = true;
							}	
							else if(locksDB[lockIndex].lockName == NULL){
								sprintf(sendMessageAL, "FAILURE : lock %d does not exist",lockIndex);
								failure = true;
							}
							else if((senderMailBoxNumber == locksDB[lockIndex].mailBoxNumber) && (senderMachineID == locksDB[lockIndex].machineID)){
								//	This lock is held by the current thread
								sprintf(sendMessageAL, "%d : Is owner of Lock %d",lockIndex, lockIndex);
								failure = true;
							}// If the Lock is destroyed then its not possible to acquire it
							if(failure){
							
								// SEND only if the message belongs to the current networking thread (check the "TO" fields of the QUEUE message)
								if(removedEventOrderEntry->replyOutPktHdr.to == clientMachineID && removedEventOrderEntry->replyOutMailHdr.to == currentThread->mailBox) {
								
									outMailHdr.length = strlen(sendMessageAL) + 1;
									// Send the reply message
									success = postOffice->Send(outPktHdr, outMailHdr, sendMessageAL); 

									if ( !success ) {
										//printf("AcquireLockRPC : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
										interrupt->Halt();
									}
								}
								delete sendMessageAL;
								break;
				
							}
							delete sendMessageAL;
							
							if(locksDB[lockIndex].status == LOCK_FREE){
							
								// Lock is free and can be aquired
								// Make the Lock status BUSY and make the actual USER PROGRAM as the owner
								locksDB[lockIndex].status = LOCK_BUSY;	
								locksDB[lockIndex].machineID = senderMachineID;
								locksDB[lockIndex].mailBoxNumber = senderMailBoxNumber;
								//printf("AcquireLockRPC : Lock %d acquired by machine %d from mail Box Number %d\n", lockIndex, senderMachineID, senderMailBoxNumber);
								// Update the usage counter to keep track of number of users for the lock
								locksDB[lockIndex].usageCounter ++;
								
								// SEND only if the message belongs to the current networking thread (check the "TO" fields of the QUEUE message)
								if(removedEventOrderEntry->replyOutPktHdr.to == clientMachineID && removedEventOrderEntry->replyOutMailHdr.to == currentThread->mailBox) {
									
									sendMessageAL = new char[MaxMailSize];
									
									sprintf(sendMessageAL, "%d : Lock %d is Acquired",lockIndex, lockIndex);
									outMailHdr.length = strlen(sendMessageAL) + 1;
									// Send the reply message
									success = postOffice->Send(outPktHdr, outMailHdr, sendMessageAL); 

									if ( !success ) {
										//printf("AcquireLockRPC : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
										interrupt->Halt();
									}
									
									delete sendMessageAL;
								}	
							}
							else{
							
								sendMessageAL = new char[MaxMailSize];
								
								// Lock is busy and have to wait 
								// Make space in the queue for the reply packed to be stored
								sprintf(sendMessageAL, "%d : Lock %d is Acquired",lockIndex, lockIndex);
								
								// Update the usage counter to keep track of number of users for the lock 
								locksDB[lockIndex].usageCounter ++;
								// Hav a count of the number of packets being stored into the queue to use the new free structure in the array of structures
								lockEntryCounter++;
								strcpy((*lockEntry[lockEntryCounter]).replymessage, sendMessageAL);
								
								// Current networking thread - TO - sender networking thread
								(*lockEntry[lockEntryCounter]).replyOutPktHdr = removedEventOrderEntry->replyOutPktHdr;
								(*lockEntry[lockEntryCounter]).replyOutMailHdr = removedEventOrderEntry->replyOutMailHdr;
							
								//printf("AcquireLockRPC : Thread on machine %d with mailbox number %d is going on wait for the Lock %d\n", outPktHdr.to, outMailHdr.to, lockIndex);
								// Append the send packet to the queue
								lockqueue[lockIndex]->Append((void *)lockEntry[lockEntryCounter]);
								
								delete sendMessageAL;
							}
						
							break;
							// *********************************************************************************************
							//									ACQUIRE LOCK END
							// *********************************************************************************************
							
						
						case 3:	//Release Lock	
						
							char *releaseLockIndex;
							char *sendMessageRL;
							success = false;
							failure = false;
							
							lockIndex = -1;
							
							releaseLockIndex = strtok(NULL, " ");
							
							lockIndex = atoi(releaseLockIndex);
							shouldBreak = 0;
							
							/* if(lockIndex == -1){
								//printf("Networking Thread : LockIndex to release lock not passed to the Server\n");
								break;
							} */
						
							// *********************************************************************************************
							//									RELEASE LOCK
							//	lockIndex - Index of the Lock to be released
							//	senderMachineID - machine id of the client that is requesting for release
							//	senderMailBoxNumber - machine mail box number of the client that is requesting for release
							// *********************************************************************************************
							
							// Actual user program machine and mailbox number	
							senderMachineID = removedEventOrderEntry->replyOutPktHdr.to;
							senderMailBoxNumber = (removedEventOrderEntry->replyOutMailHdr.to)-1;
						
							//------------------------------------------------------------------------------------------
							// 							Validate the parameters received
							//------------------------------------------------------------------------------------------	
							
							sendMessageRL = new char[MaxMailSize];
							
							//if Invalid lock index
							if((lockIndex < 0)||(lockIndex > locksDBIndex)){
								sprintf(sendMessageRL, "FAILURE : Invalid Lock index %d",lockIndex);
								failure = true;
							}	
							else if(locksDB[lockIndex].lockName == NULL){
								sprintf(sendMessageRL, "FAILURE : Lock %d does not exist",lockIndex);
								failure = true;
							}
							else if(senderMailBoxNumber != locksDB[lockIndex].mailBoxNumber){
								//	This lock is not held by the current thread
								sprintf(sendMessageRL, "FAILURE : Not Lock %d owner",lockIndex);
								failure = true;
							}
							else if(senderMachineID != locksDB[lockIndex].machineID){
								//	This lock is not held by the current thread
								sprintf(sendMessageRL, "FAILURE : Not Lock %d owner",lockIndex);
								failure = true;
							}
							if(failure){
							
								// SEND only if the message belongs to the current networking thread (check the "TO" fields of the QUEUE message)
								if(removedEventOrderEntry->replyOutPktHdr.to == clientMachineID && removedEventOrderEntry->replyOutMailHdr.to == currentThread->mailBox) {
								
									outMailHdr.length = strlen(sendMessageRL) + 1;
									// Send the reply message
									success = postOffice->Send(outPktHdr, outMailHdr, sendMessageRL); 

									if ( !success ) {
										//printf("ReleaseLockRPC : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
										interrupt->Halt();
									}
								}
								
								delete sendMessageRL;
								break;
							}
							delete sendMessageRL;
							
							if(!lockqueue[lockIndex]->IsEmpty()){
							
								// SEND only if the message belongs to the current networking thread (check the "TO" fields of the QUEUE message)
								if(removedEventOrderEntry->replyOutPktHdr.to == clientMachineID && removedEventOrderEntry->replyOutMailHdr.to == currentThread->mailBox) {
									
									sendMessageRL = new char[MaxMailSize];
									
									sprintf(sendMessageRL, "%d : Lock %d is Released",lockIndex, lockIndex);
									outMailHdr.length = strlen(sendMessageRL) + 1;
									success = postOffice->Send(outPktHdr, outMailHdr, sendMessageRL); 
								
									if (!success ) {
										//printf("ReleaseLockRPC : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
										interrupt->Halt();
									}
									
									delete sendMessageRL;
								}	
								
								// Remove a send packet from the lock queue
								removedLockEntryCounter++;
								removedLockEntry[removedLockEntryCounter] = (lockWaitEntry*)lockqueue[lockIndex]->Remove();
								
								if(removedLockEntry[removedLockEntryCounter] != NULL){	// Reply to the removed thread
									
									// $
									//printf("Found a waiter for the lock %d\n", lockIndex);
									//printf("ReleaseLockRPC : Lock %d released by machine %d from mail Box Number %d\n", lockIndex, senderMachineID, senderMailBoxNumber);
	
									// Current networking thread - TO - sender networking thread
									tempOutPktHdr = (*removedLockEntry[removedLockEntryCounter]).replyOutPktHdr;
									tempOutMailHdr = (*removedLockEntry[removedLockEntryCounter]).replyOutMailHdr;	
									
									sendMessageRL = new char[MaxMailSize];
									
									strcpy(sendMessageRL, (*removedLockEntry[removedLockEntryCounter]).replymessage);
									
									// Check if the removed message was put into queue with "TO" being the current networking thread
									if(tempOutPktHdr.to == clientMachineID &&  tempOutMailHdr.to == currentThread->mailBox) {
										
										tempOutMailHdr.length = strlen(sendMessageRL) + 1;
										sprintf(sendMessageRL, "%d : Lock %d is Acquired",lockIndex, lockIndex);
										
										tempOutMailHdr.to = tempOutMailHdr.to - 1;
										
										// Send the reply message to user program if removed queue entry was for me
										success = postOffice->Send(tempOutPktHdr, tempOutMailHdr, sendMessageRL); 

										if (!success ) {
											//printf("ReleaseLockRPC : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
											interrupt->Halt();
										}
									}

									delete sendMessageRL;
									
								}
								// Make the Lock status BUSY and make the actual USER PROGRAM as the owner
								locksDB[lockIndex].status = LOCK_BUSY;	
								locksDB[lockIndex].machineID = (*removedLockEntry[removedLockEntryCounter]).replyOutPktHdr.to;
								locksDB[lockIndex].mailBoxNumber = (*removedLockEntry[removedLockEntryCounter]).replyOutMailHdr.to - 1;
								// Update the usage counter to keep track of number of users for the lock 
								locksDB[lockIndex].usageCounter --;
								//printf("ReleaseLockRPC : Lock %d acquired by machine %d from mail Box Number %d\n", lockIndex, (*removedLockEntry[removedLockEntryCounter]).replyOutPktHdr.to, (*removedLockEntry[removedLockEntryCounter]).replyOutMailHdr.to-1);	
								
							}	
							else{
								
								// Queue is empty so Lock is made free
								locksDB[lockIndex].status = LOCK_FREE;
								locksDB[lockIndex].machineID = -1;
								locksDB[lockIndex].mailBoxNumber = -1;
								// Update the usage counter to keep track of number of users for the lock 
								locksDB[lockIndex].usageCounter --;
								
								if((locksDB[lockIndex].usageCounter == 0) && (locksDB[lockIndex].toBeDestroyedCount == 0))
								{
									delete locksDB[lockIndex].lockName;
									locksDB[lockIndex].lockName = NULL;
									locksDB[lockIndex].machineID = -1;
									locksDB[lockIndex].mailBoxNumber = -1;
									locksDB[lockIndex].status = LOCK_FREE;
									locksDB[lockIndex].toBeDestroyedCount = 0;
									//printf("ReleaseLockRPC : Lock %d is Destroyed in release syscall\n",lockIndex);
								}
								
								// SEND only if the message belongs to the current networking thread (check the "TO" fields of the QUEUE message)
								if(removedEventOrderEntry->replyOutPktHdr.to == clientMachineID && removedEventOrderEntry->replyOutMailHdr.to == currentThread->mailBox) {
									
									sendMessageRL = new char[MaxMailSize];
									
									sprintf(sendMessageRL, "%d : Lock %d is Released",lockIndex, lockIndex);
									outMailHdr.length = strlen(sendMessageRL) + 1;
									success = postOffice->Send(outPktHdr, outMailHdr, sendMessageRL); 

									if (!success ) {
										//printf("ReleaseLockRPC : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
										interrupt->Halt();
									}
									
									delete sendMessageRL;
								}
							}
							
							break;	
							// *********************************************************************************************
							//									RELEASE LOCK END		
							// *********************************************************************************************
							
							
						case 4:	//Destroy Lock
						
							// Destroy variables
							char *strDestroyLockIndex;
							char *sendMessageDL;
							int destroyLockIndex;
							
							success = false;
							failure = false;
							
							destroyLockIndex = -1;
							strDestroyLockIndex = strtok(NULL, " ");
							destroyLockIndex = atoi(strDestroyLockIndex);
							
							/* if(destroyLockIndex == -1){
								//printf("Networking Thread : LockIndex to Destroy lock not passed to the Server\n");
								break;
							}
							 */
							
							// *********************************************************************************************
							//									DESTROY LOCK
							//	lockIndex - Index of the Lock to be destroy
							//	senderMachineID - machine id of the client that is requesting for destroy
							//	senderMailBoxNumber - machine mail box number of the client that is requesting for destroy
							// *********************************************************************************************
								
							// Actual user program machine and mailbox number	
							senderMachineID = removedEventOrderEntry->replyOutPktHdr.to;
							senderMailBoxNumber = (removedEventOrderEntry->replyOutMailHdr.to)-1;
					
							//------------------------------------------------------------------------------------------
							// 							Validate the parameters received
							//------------------------------------------------------------------------------------------	
							
							sendMessageDL = new char[MaxMailSize];
							
							//if Invalid lock index
							if(locksDB[lockIndex].toBeDestroyedCount == 0){
								//	This lock is not held by the current thread
								sprintf(sendMessageDL, "FAILURE : Lock %d already destroyed",lockIndex);
								failure = true;
							}
							else if((lockIndex < 0)||(lockIndex > locksDBIndex)){
								sprintf(sendMessageDL, "FAILURE : Invalid Lock index %d\n",lockIndex);
								failure = true;
							}	
							else if(locksDB[lockIndex].lockName == NULL){
								sprintf(sendMessageDL, "FAILURE : Lock %d does not exist",lockIndex);
								failure = true;
							}
							if(failure){
							
								outMailHdr.length = strlen(sendMessageDL) + 1;
								// Send the reply message
								success = postOffice->Send(outPktHdr, outMailHdr, sendMessageDL); 

								if ( !success ) {
									//printf("DestroyLockRPC : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
									interrupt->Halt();
								}
								
								delete sendMessageDL;
								break;
							}
							
							delete sendMessageDL;
							
							//------------------------------------------------------------------------------------------
							//							Update Server secret storage	
							//------------------------------------------------------------------------------------------
							if(locksDB[lockIndex].toBeDestroyedCount != 0){
								locksDB[lockIndex].toBeDestroyedCount--; 
							}
							
							sendMessageDL = new char[MaxMailSize];
							
							if((locksDB[lockIndex].usageCounter == 0) && (locksDB[lockIndex].toBeDestroyedCount == 0)){ 
							
								// Destroy the Lock right now
								delete locksDB[lockIndex].lockName;
								locksDB[lockIndex].lockName = NULL;
								locksDB[lockIndex].machineID = -1;
								locksDB[lockIndex].mailBoxNumber = -1;
								locksDB[lockIndex].status = LOCK_FREE;
								locksDB[lockIndex].toBeDestroyedCount = 0;
								sprintf(sendMessageDL, "%d : Lock %d is Destroyed",lockIndex, lockIndex);
								//printf("DestroyLockRPC : Lock %d Destroyed by machine %d from mail Box Number %d\n", lockIndex, senderMachineID, senderMailBoxNumber);
								
							}
							
							// Following if statements can be replaced with a single else statement but to make it more sensible 3 if statements are placed below
							if((locksDB[lockIndex].usageCounter != 0) && (locksDB[lockIndex].toBeDestroyedCount == 0)){
							
								sprintf(sendMessageDL, "%d : Lock %d is going to be Destroyed",lockIndex, lockIndex);
							}	
							
							if((locksDB[lockIndex].usageCounter == 0) && (locksDB[lockIndex].toBeDestroyedCount != 0)){
							
								sprintf(sendMessageDL, "%d : Lock %d is going to be Destroyed",lockIndex, lockIndex);
							}	
							
							if((locksDB[lockIndex].usageCounter != 0) && (locksDB[lockIndex].toBeDestroyedCount != 0)){
							
								sprintf(sendMessageDL, "%d : Lock %d is going to be Destroyed",lockIndex, lockIndex);
							}	
							
							// SEND only if the message belongs to the current networking thread (check the "TO" fields of the QUEUE message)
							if(removedEventOrderEntry->replyOutPktHdr.to == clientMachineID && removedEventOrderEntry->replyOutMailHdr.to == currentThread->mailBox) {
							
								outMailHdr.length = strlen(sendMessageDL) + 1;
								success = postOffice->Send(outPktHdr, outMailHdr, sendMessageDL); 
								if (!success ) {
									//printf("DestroyLockRPC : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
									interrupt->Halt();
								}
								
							}
							
							delete sendMessageDL;
							break;
							// *********************************************************************************************
							//									DESTROY LOCK END
							// *********************************************************************************************
						
							
						case 5:	//Create CV
						
							char *sendMessageCCV;
							char *nameCCV;
							success = 0;
							
							nameCCV = strtok(NULL, " ");
							shouldBreak = 0;
							
							if(nameCCV == NULL){
								
								// SEND only if the message belongs to the current networking thread (check the "TO" fields of the QUEUE message)
								if(removedEventOrderEntry->replyOutPktHdr.to == clientMachineID && removedEventOrderEntry->replyOutMailHdr.to == currentThread->mailBox) {
									
									sendMessageCCV = new char[MaxMailSize];
									
									//printf("Networking Thread : CV Name not sent to the Server\n");
									outPktHdr.to = inPktHdr.from;
									outMailHdr.to = currentThread->mailBox-1;	
									outMailHdr.from = currentThread->mailBox;
									outPktHdr.from = clientMachineID;
									sprintf(sendMessageCCV, "FAILURE : CV Name is NULL");
									// Format the reply message to the client waiting for the Lock Index from the server
									outMailHdr.length = strlen(sendMessageCCV) + 1;
									// Send the message
									success = postOffice->Send(outPktHdr, outMailHdr, sendMessageCCV); 

									if ( !success ) {
										//printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
										interrupt->Halt();
									}	
									
									delete sendMessageCCV;
								}			
								
								break;		
							}
							
							// *********************************************************************************************
							//									CREATE CONDITION VARIABLE
							//	name - Name of the CV to be created on the server
							//	senderMachineID - Owner machine of the CV that is going to be created
							//	senderMailBoxNumber - Owner machine mail box number of the CV that is going to be created			
							// *********************************************************************************************
								
							// Actual user program machine and mailbox number	
							senderMachineID = removedEventOrderEntry->replyOutPktHdr.to;
							senderMailBoxNumber = (removedEventOrderEntry->replyOutMailHdr.to)-1;
					
						//------------------------------------------------------------------------------------------
						// 							Validate the CV parameters received
						//------------------------------------------------------------------------------------------
							
							for(index = 0; index <= cvsDBIndex; index++){
								if(cvsDB[index].toBeDestroyedCount != 0){
									if(strcmp(cvsDB[index].cvName, nameCCV) == 0){
										//printf("CreateCVRPC : CV already exists with the same name %s\n", nameCCV);
										
										// SEND only if the message belongs to the current networking thread (check the "TO" fields of the QUEUE message)
										if(removedEventOrderEntry->replyOutPktHdr.to == clientMachineID && removedEventOrderEntry->replyOutMailHdr.to == currentThread->mailBox) {
											
											sendMessageCCV = new char[MaxMailSize];
											
											// increment the number of creators of the CV with same name before being destroyed
											cvsDB[index].toBeDestroyedCount++;
											
											sprintf(sendMessageCCV, "%d : CV already exists", index);
											// Format the reply message to the client waiting for the CV Index from the server
											// Sending the CV index of the CV which is already created by someother client
											outMailHdr.length = strlen(sendMessageCCV) + 1;
											// $
											//printf("CreateCVRPC : reply message is -> %s\n", sendMessageCCV);
											// Send the message
											success = postOffice->Send(outPktHdr, outMailHdr, sendMessageCCV); 

											if ( !success ) {
												//printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
												interrupt->Halt();
											}	
											
											
											delete sendMessageCCV;
											
										}	
										shouldBreak = 1;
										break;
									}
								}	
							}
							if(shouldBreak == 1){	
								shouldBreak = 0;
								break;
							}
							
							if(cvsDBIndex == (MAX_CVS-1)){
								//printf("CreateCVRPC : Reached max CV Count, No more CVs can be created\n");	
								
								// SEND only if the message belongs to the current networking thread (check the "TO" fields of the QUEUE message)
								if(removedEventOrderEntry->replyOutPktHdr.to == clientMachineID && removedEventOrderEntry->replyOutMailHdr.to == currentThread->mailBox) {
								
									sendMessageCCV = new char[MaxMailSize];
									
									sendMessageCCV = "FAILURE : Reached max CV Count";
									// Format the FAILURE reply message to the client waiting for the CV Index from the server
									//	sprintf(message, "%s", failureMessage);
									outMailHdr.length = strlen(sendMessageCCV) + 1;
									// $
									//printf("CreateCVRPC : reply message is -> %s\n", sendMessageCCV);
									// Send the message
									success = postOffice->Send(outPktHdr, outMailHdr, sendMessageCCV); 

									if ( !success ) {
										//printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
										interrupt->Halt();
									}	
									
									delete sendMessageCCV;
									
								}		
								break;
							}
							
						//------------------------------------------------------------------------------------------
						//							Update Server secret storage	
						//------------------------------------------------------------------------------------------

							// Move the index by one to store the information of the new CV created by the server
							cvsDBIndex++;
							// Store the Lock information
							strcpy(cvsDB[cvsDBIndex].cvName, nameCCV);
							// toBeDestroyed flag will be set when the destroy lock is called
							cvsDB[cvsDBIndex].toBeDestroyedCount = 1;
							// used to keep track of number of waiters waiting to acquire the Lock
							// when this value becomes zero when the last thread calls release then the 
							// toBeDestroyd flag is checked for 1. If 1 then lock is destroyed 
							cvsDB[cvsDBIndex].usageCounter = 0;
							cvsDB[cvsDBIndex].machineID = senderMachineID;
							cvsDB[cvsDBIndex].mailBoxNumber = senderMailBoxNumber;
							cvsDB[cvsDBIndex].cvLockIndex = -1;
							
						//------------------------------------------------------------------------------------------
						//							Form a reply message and send to client
						//------------------------------------------------------------------------------------------
							
							// SEND only if the message belongs to the current networking thread (check the "TO" fields of the QUEUE message)
							if(removedEventOrderEntry->replyOutPktHdr.to == clientMachineID && removedEventOrderEntry->replyOutMailHdr.to == currentThread->mailBox) {
								
								sendMessageCCV = new char[MaxMailSize];
								
								// Format the reply message to the client waiting for the CV Index from the server
								sprintf(sendMessageCCV, "%d ", cvsDBIndex);
								outMailHdr.length = strlen(sendMessageCCV) + 1;
								// $
								//printf("CreateCVRPC : reply message is -> %s\n", sendMessageCCV);
								// Send the first message
								success = postOffice->Send(outPktHdr, outMailHdr, sendMessageCCV); 
								if ( !success ) {
									//printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
									interrupt->Halt();
								}
								
								delete sendMessageCCV;
							
							}			

							break;
							// *********************************************************************************************
							//									CREATE CONDITION VARIABLE END
							// *********************************************************************************************
						
						
						case 6:	//CV Wait
						
							// Wait variables
							char *strWaitLockIndex;
							char *strWaitCVIndex;
							char *sendMessageWCV;
							
							int waitLockIndex;
							int waitCVIndex;
	
							success = false;
							failure = false;
							
							waitCVIndex = -1;
							waitLockIndex = -1;
							
							strWaitCVIndex = strtok(NULL, " ");
							strWaitLockIndex = strtok(NULL, " ");
							
							waitCVIndex = atoi(strWaitCVIndex);
							waitLockIndex = atoi(strWaitLockIndex);
							
							/* if((waitLockIndex == -1)||(waitCVIndex == -1)){
								//printf("Networking Thread : waitLockIndex or CVIndex for waitCV not passed to the Server\n");
								break;
							}
							 */
							// *********************************************************************************************
							//									CONDITION WAIT 
							//	cvIndex - Index of the CV for this wait
							//	waitLockIndex - Index of the Lock for this Condition
							//	senderMachineID - machine id of the client that is requesting for CVwait
							//	senderMailBoxNumber - machine mail box number of the client that is requesting for CVwait
							// *********************************************************************************************
							
							// Actual user program machine and mailbox number	
							senderMachineID = removedEventOrderEntry->replyOutPktHdr.to;
							senderMailBoxNumber = (removedEventOrderEntry->replyOutMailHdr.to)-1;
							
							//------------------------------------------------------------------------------------------
							// 							Validate the parameters received
							//------------------------------------------------------------------------------------------	
						
							sendMessageWCV = new char[MaxMailSize];
							
							if((waitLockIndex < 0)||(waitLockIndex > locksDBIndex)){
								sprintf(sendMessageWCV, "FAILURE : Invalid Lock index %d\n",waitLockIndex);
								failure = true;
							}	
							else if((waitCVIndex < 0)||(waitCVIndex > cvsDBIndex)){
								sprintf(sendMessageWCV, "FAILURE : Invalid CV index %d\n",waitCVIndex);
								failure = true;
							}	
							else if(locksDB[waitLockIndex].lockName == NULL){
								sprintf(sendMessageWCV, "FAILURE : Lockname %d does not exist",waitLockIndex);
								failure = true;
							}
							else if (cvsDB[waitCVIndex].cvName == NULL){
								sprintf(sendMessageWCV, "FAILURE : CVname %d does not exist",waitCVIndex);
								failure = true;
							}
							else if (cvsDB[waitCVIndex].toBeDestroyedCount == 0){
								sprintf(sendMessageWCV, "FAILURE : CV %d is Destroyed" ,waitCVIndex);
								failure = true;
							}
							else if(senderMailBoxNumber != locksDB[waitLockIndex].mailBoxNumber){
								sprintf(sendMessageWCV, "FAILURE : Not Lock %d owner",waitLockIndex);
								failure = true;
							}
							else if(senderMachineID != locksDB[waitLockIndex].machineID){
								sprintf(sendMessageWCV, "FAILURE : Not Lock %d owner",waitLockIndex);
								failure = true;
							}
							else if (cvsDB[waitCVIndex].cvLockIndex != waitLockIndex){
									if(cvsDB[waitCVIndex].cvLockIndex != -1) {
										sprintf(sendMessageWCV, "FAILURE : CVlock %d mismatch with Lock %d",cvsDB[waitCVIndex].cvLockIndex, waitLockIndex);
										failure = true;
									}
							}	
							
							if(failure){
								outMailHdr.length = strlen(sendMessageWCV) + 1;
								// Send the reply message
								
								// SEND only if the message belongs to the current networking thread (check the "TO" fields of the QUEUE message)
								if(removedEventOrderEntry->replyOutPktHdr.to == clientMachineID && removedEventOrderEntry->replyOutMailHdr.to == currentThread->mailBox) {
							
									success = postOffice->Send(outPktHdr, outMailHdr, sendMessageWCV); 

									if ( !success ) {
										//printf("CVWaitRPC : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
										interrupt->Halt();
									}
							
								}
								delete sendMessageWCV;	
								break;
							}
							
							delete sendMessageWCV;
							
							//------------------------------------------------------------------------------------------
							//							Update Server secret storage	
							//------------------------------------------------------------------------------------------
							// If CV has not waiters yet then make the current Lock as the CV waiting lock
							if (cvsDB[waitCVIndex].cvLockIndex == -1){
								cvsDB[waitCVIndex].cvLockIndex = waitLockIndex;
							}
							
							//Release the lock before going to wait on CV
							if(!lockqueue[waitLockIndex]->IsEmpty()){
							
								removedLockEntryCounter++;
								removedLockEntry[removedLockEntryCounter] = (lockWaitEntry*)lockqueue[waitLockIndex]->Remove();
								
								if(removedLockEntry[removedLockEntryCounter] != NULL){
								
									//printf("CVWaitRPC : Lock %d released by user prog which is on machine %d from mail Box Number %d\n", waitLockIndex, senderMachineID, senderMailBoxNumber);
									
									tempOutPktHdr = (*removedLockEntry[removedLockEntryCounter]).replyOutPktHdr;
									tempOutMailHdr = (*removedLockEntry[removedLockEntryCounter]).replyOutMailHdr;	
									
									// SEND only if the removed message belongs to the current networking thread (check the "TO" fields of the lock queue message)
									if(tempOutPktHdr.to == clientMachineID && tempOutMailHdr.to == currentThread->mailBox) {
									
										sendMessageWCV = new char[MaxMailSize];
										
										strcpy(sendMessageWCV, (*removedLockEntry[removedLockEntryCounter]).replymessage);
										outMailHdr.length = strlen(sendMessageWCV) + 1;
										// Send the reply message
										success = postOffice->Send(outPktHdr, outMailHdr, sendMessageWCV); 
										if (!success ) {
											//printf("CVWaitRPC : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
											interrupt->Halt();
										}
										
										delete sendMessageWCV;
									}			
								}
								
								// Make the Uprogram of the net thread as the owner of the lock which is released
								locksDB[waitLockIndex].status = LOCK_BUSY;	
								locksDB[waitLockIndex].machineID = (*removedLockEntry[removedLockEntryCounter]).replyOutPktHdr.to;
								locksDB[waitLockIndex].mailBoxNumber = (*removedLockEntry[removedLockEntryCounter]).replyOutMailHdr.to - 1;
								//printf("CVWaitRPC : Lock %d acquired by machine %d from mail Box Number %d\n", waitLockIndex, (*removedLockEntry[removedLockEntryCounter]).replyOutPktHdr.to, (*removedLockEntry[removedLockEntryCounter]).replyOutMailHdr.to-1);	
								
							}
							else{
							
								// If no waiters for the Lock being released then make the Lock free
								locksDB[waitLockIndex].status = LOCK_FREE;
								locksDB[waitLockIndex].machineID = -1;
								locksDB[waitLockIndex].mailBoxNumber = -1;
								//printf("CVWaitRPC : Lock %d is made FREE by machine %d mail Box No.%d since no waiters for lock\n", waitLockIndex, senderMachineID, senderMailBoxNumber);	
								
							}
							
							//printf("CVWaitRPC : Machine %d from mail box number %d going on WAIT in CV %d and Lock %d\n", senderMachineID, senderMailBoxNumber, waitCVIndex, waitLockIndex);	
							//Append this one to cvqueue - Client replied when a signal is received for this CV on this Lock
							//Store the message to be printed after receiving the signal from a thread
							
							// Update the waiter count of the CV
							cvsDB[waitCVIndex].usageCounter ++;
							
							sendMessageWCV = new char[MaxMailSize];
							
							// Make the net thread which requested to go on wait , go on wait
							sprintf(sendMessageWCV,"%d : client out of CV Q %d", waitCVIndex, waitCVIndex);
							cvEntryCounter++;
							strcpy((*CVEntry[cvEntryCounter]).replymessage, sendMessageWCV);
							(*CVEntry[cvEntryCounter]).replyOutPktHdr = removedEventOrderEntry->replyOutPktHdr;
							(*CVEntry[cvEntryCounter]).replyOutMailHdr = removedEventOrderEntry->replyOutMailHdr;	
							cvqueue[waitCVIndex]->Append((void *)CVEntry[cvEntryCounter]);
								
							delete sendMessageWCV;
							
							break;
							// *********************************************************************************************
							//									CONDITION WAIT END
							// *********************************************************************************************
							
						
						case 7:	//CV Signal
							//printf("in signal rpc\n");
							// Signal variables
							char *strSignalLockIndex;
							char *strSignalCVIndex;
							char *sendMessageSCV;
							
							int signalLockIndex;
							int signalCVIndex;
							
							signalLockIndex = -1;
							signalCVIndex = -1;
							
							success = false;
							failure = false;
								
							strSignalCVIndex = strtok(NULL, " ");
							strSignalLockIndex = strtok(NULL, " ");
							
							signalCVIndex = atoi(strSignalCVIndex);
							signalLockIndex = atoi(strSignalLockIndex);
							
							/* if((signalLockIndex == -1)||(signalCVIndex == -1)){
								//printf("Networking Thread : LockIndex or CVIndex for SignalCV not passed to the Server\n");
								break;
							}
					 */
							// *********************************************************************************************
							//									CONDITION SIGNAL 
							//	cvIndex - Index of the CV for this signal
							//	lockIndex - Index of the Lock for this Condition
							//	senderMachineID - machine id of the client that is requesting for CVsignal
							//	senderMailBoxNumber - machine mail box number of the client that is requesting for CVsignal
							// *********************************************************************************************
							
							// Actual user program machine and mailbox number	
							senderMachineID = removedEventOrderEntry->replyOutPktHdr.to;
							senderMailBoxNumber = (removedEventOrderEntry->replyOutMailHdr.to)-1;
							
							//------------------------------------------------------------------------------------------
							// 							Validate the parameters received
							//------------------------------------------------------------------------------------------	
							
							sendMessageSCV = new char[MaxMailSize];
							
							if((signalLockIndex < 0)||(signalLockIndex > locksDBIndex)){
								sprintf(sendMessageSCV, "FAILURE : Invalid Lock index %d",signalLockIndex);
								failure = true;
							}	
							else if((signalCVIndex < 0)||(signalCVIndex > cvsDBIndex)){
								sprintf(sendMessageSCV, "FAILURE : Invalid CV index %d",signalLockIndex);
								failure = true;
							}	
							else if(locksDB[signalLockIndex].lockName == NULL){
								sprintf(sendMessageSCV, "FAILURE : Lockname %d does not exist",signalLockIndex);
								failure = true;
							}
							else if (cvsDB[signalCVIndex].cvName == NULL){
								sprintf(sendMessageSCV, "FAILURE : CVname %d does not exist",signalLockIndex);
								failure = true;
							}
							else if(senderMailBoxNumber != locksDB[signalLockIndex].mailBoxNumber){
								sprintf(sendMessageSCV, "FAILURE : Not Lock %d owner",signalLockIndex);
								failure = true;
							}
							else if(senderMachineID != locksDB[signalLockIndex].machineID){
								sprintf(sendMessageSCV, "FAILURE : Not Lock %d owner",signalLockIndex);
								failure = true;
							}
							else if ((cvsDB[signalCVIndex].cvLockIndex != signalLockIndex) && (!failure) && (cvsDB[signalCVIndex].cvLockIndex != -1)) {
								sprintf(sendMessageSCV, "FAILURE : %d Lock mismatch %d",cvsDB[signalCVIndex].cvLockIndex, signalLockIndex);
								failure = true;
							}	
							
							if(failure){
								//printf("in here 1\n");
								// SEND only if the message belongs to the current networking thread (check the "TO" fields of the QUEUE message)
								if(removedEventOrderEntry->replyOutPktHdr.to == clientMachineID && removedEventOrderEntry->replyOutMailHdr.to == currentThread->mailBox) {
									
									outMailHdr.length = strlen(sendMessageSCV) + 1;
									// Send the reply message
									success = postOffice->Send(outPktHdr, outMailHdr, sendMessageSCV); 
									if ( !success ) {
										//printf("CVsignalRPC : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
										interrupt->Halt();
									}
								
								}
								delete sendMessageSCV;
								break;
							}
							
							delete sendMessageSCV;
							
							//------------------------------------------------------------------------------------------
							//							Update Server secret storage	
							//------------------------------------------------------------------------------------------
							if(cvqueue[signalCVIndex]->IsEmpty()){
								//printf("in here 2\n");
								sendMessageSCV = new char[MaxMailSize];
								
								// SEND only if the message belongs to the current networking thread (check the "TO" fields of the QUEUE message)
								if(removedEventOrderEntry->replyOutPktHdr.to == clientMachineID && removedEventOrderEntry->replyOutMailHdr.to == currentThread->mailBox) {
							
									sprintf(sendMessageSCV, "%d :No waiters in CV %d", signalCVIndex, signalCVIndex);
									outMailHdr.length = strlen(sendMessageSCV) + 1;
									// Send the reply message
									success = postOffice->Send(outPktHdr, outMailHdr, sendMessageSCV); 
									if ( !success ) {
										//printf("CVsignalRPC : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
										interrupt->Halt();
									}
								}
								
								delete sendMessageSCV;	
								break;
							}
							else {
								//printf("in else case\n");
								removedCVEntryCounter++;
								removedCVEntry[removedCVEntryCounter] = (cvWaitEntry*)cvqueue[signalCVIndex]->Remove();
								
								cvsDB[signalCVIndex].usageCounter --;
								
								// if the removed entry from CV wait queue is valid
								if(removedCVEntry[removedCVEntryCounter] != NULL){
								
									//Acquire the lock before exiting wait rpc
									if(locksDB[signalLockIndex].status == LOCK_FREE){
										//printf("in here 4\n");
										locksDB[signalLockIndex].status = LOCK_BUSY;	
										
										// if lock is free then make the user program of the "networking thread which was in the cv queue" as the owner
										locksDB[signalLockIndex].machineID = (*removedCVEntry[removedCVEntryCounter]).replyOutPktHdr.to;
										locksDB[signalLockIndex].mailBoxNumber = (*removedCVEntry[removedCVEntryCounter]).replyOutMailHdr.to-1;
										
										//printf("CVsignalRPC : Lock %d acquired by machine %d from mail Box Number %d\n", signalLockIndex, (*removedCVEntry[removedCVEntryCounter]).replyOutPktHdr.to, (*removedCVEntry[removedCVEntryCounter]).replyOutMailHdr.to);
										
										tempOutPktHdr = (*removedCVEntry[removedCVEntryCounter]).replyOutPktHdr;
										tempOutMailHdr = (*removedCVEntry[removedCVEntryCounter]).replyOutMailHdr;
										
										// SEND only if the message belongs to the current networking thread (check the "TO" fields of the CV QUEUE message)
										if(tempOutPktHdr.to == clientMachineID && tempOutMailHdr.to == currentThread->mailBox) {
											//printf("in here 3\n");
											sendMessageSCV = new char[MaxMailSize];
											
											strcpy(sendMessageSCV,(*removedCVEntry[removedCVEntryCounter]).replymessage);
											outMailHdr.length = strlen(sendMessageSCV) + 1;
											
											// Send the reply message
											success = postOffice->Send(outPktHdr, outMailHdr, sendMessageSCV); 
											if ( !success ) {
												//printf("CVsignalRPC : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
												interrupt->Halt();
											}
											
											delete sendMessageSCV;
										}
										
									}
									else{
										//printf("in here 5\n");
										// If the lock is busy then the networking thread is made to go on wait in the lock queue
										// put the entry removed from cv wait queue into lock wait queue
										lockEntryCounter++;
										strcpy((*lockEntry[lockEntryCounter]).replymessage,(*removedCVEntry[removedCVEntryCounter]).replymessage);
										(*lockEntry[lockEntryCounter]).replyOutPktHdr = (*removedCVEntry[removedCVEntryCounter]).replyOutPktHdr;
										(*lockEntry[lockEntryCounter]).replyOutMailHdr = (*removedCVEntry[removedCVEntryCounter]).replyOutMailHdr;
										lockqueue[signalLockIndex]->Append((void *)lockEntry[lockEntryCounter]);
										
									}	
									
								}
								
							}
							
							if(cvqueue[signalCVIndex]->IsEmpty()){
								cvsDB[signalCVIndex].cvLockIndex = -1;
							}
							
							if((cvsDB[signalCVIndex].usageCounter == 0) && (cvsDB[signalCVIndex].toBeDestroyedCount == 0)){ 
								
								//destroy the CV if no waiters on this CV and the destroy flag is set
								delete cvsDB[signalCVIndex].cvName;
								cvsDB[signalCVIndex].cvName = NULL;
								cvsDB[signalCVIndex].mailBoxNumber = -1;
								cvsDB[signalCVIndex].machineID = -1;
								cvsDB[signalCVIndex].toBeDestroyedCount = 0;
								//printf("%d : CV %d is Destroyed by signal",signalCVIndex, signalCVIndex);
								
							}
	
// DOUBT	
							if(broadcastBit == false){
								//printf("in here last\n");
								// SEND only if the message belongs to the current networking thread (check the "TO" fields of the QUEUE message)
								if(removedEventOrderEntry->replyOutPktHdr.to == clientMachineID && removedEventOrderEntry->replyOutMailHdr.to == currentThread->mailBox) {
											
									sendMessageSCV = new char[MaxMailSize];
									
									sprintf(sendMessageSCV, "%d : CV %d Signalled lock %d", signalCVIndex, signalCVIndex, signalLockIndex);
									outMailHdr.length = strlen(sendMessageSCV) + 1;
									success = postOffice->Send(outPktHdr, outMailHdr, sendMessageSCV); 
									if (!success ) {
										//printf("CVsignalRPC : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
										interrupt->Halt();
									}
									
									delete sendMessageSCV;
								}	
							}
						
							break;
							// *********************************************************************************************
							//									CONDITION SIGNAL END
							// *********************************************************************************************
							
							
						case 8:	// Broadcast CV 
						
							// Broadcast variables
							char *strBroadcastLockIndex;
							char *strBroadcastCVIndex;
							char *sendMessageBCV;
							
							int broadcastLockIndex;
							int broadcastCVIndex;
							
							broadcastLockIndex = -1;
							broadcastCVIndex = -1;
							
							success = false;
							failure = false;
							
							strBroadcastCVIndex = strtok(NULL, " ");
							strBroadcastLockIndex = strtok(NULL, " ");
							
							broadcastCVIndex = atoi(strBroadcastCVIndex);
							broadcastLockIndex = atoi(strBroadcastLockIndex);
							
							/* if((broadcastCVIndex == -1)||(broadcastLockIndex == -1)){
								//printf("Networking Thread : LockIndex or CVIndex for BroadcastCV not passed to the Server\n");
								break;
							}
							 */
							// *********************************************************************************************
							//									CONDITION BROADCAST 
							//	broadcastCVIndex - Index of the CV for this broadcast
							//	lockIndex - Index of the Lock for this Condition
							//	senderMachineID - machine id of the client that is requesting for CVbroadcast
							//	senderMailBoxNumber - machine mail box number of the client that is requesting for CVbroadcast
							// *********************************************************************************************
						
							// Actual user program machine and mailbox number	
							senderMachineID = removedEventOrderEntry->replyOutPktHdr.to;
							senderMailBoxNumber = (removedEventOrderEntry->replyOutMailHdr.to)-1;
					
							//------------------------------------------------------------------------------------------
							// 							Validate the parameters received
							//------------------------------------------------------------------------------------------	
							
							sendMessageBCV = new char[MaxMailSize];
							
							if((broadcastLockIndex < 0)||(broadcastLockIndex > locksDBIndex)){
								sprintf(sendMessageBCV, "FAILURE : Invalid Lock index %d\n",broadcastLockIndex);
								failure = true;
							}	
							else if((broadcastCVIndex < 0)||(broadcastCVIndex > cvsDBIndex)){
								sprintf(sendMessageBCV, "FAILURE : Invalid CV index %d\n",broadcastLockIndex);
								failure = true;
							}	
							else if(locksDB[broadcastLockIndex].lockName == NULL){
								sprintf(sendMessageBCV, "FAILURE : Lockname %d does not exist",broadcastLockIndex);
								failure = true;
							}
							else if (cvsDB[broadcastCVIndex].cvName == NULL){
								sprintf(sendMessageBCV, "FAILURE : CVname %d does not exist",broadcastLockIndex);
								failure = true;
							}
							else if((senderMailBoxNumber != locksDB[broadcastLockIndex].mailBoxNumber)&(senderMachineID != locksDB[broadcastLockIndex].machineID)){
								sprintf(sendMessageBCV, "FAILURE : Not Lock %d owner",broadcastLockIndex);
								failure = true;
							}
							else if (cvsDB[broadcastCVIndex].cvLockIndex != broadcastLockIndex) {
								sprintf(sendMessageBCV, "FAILURE : CVlock %d mismatch with %d",cvsDB[broadcastCVIndex].cvLockIndex, broadcastLockIndex);
								failure = true;
							}	
							
							if(failure){
								
								// SEND only if the message belongs to the current networking thread (check the "TO" fields of the QUEUE message)
								if(removedEventOrderEntry->replyOutPktHdr.to == clientMachineID && removedEventOrderEntry->replyOutMailHdr.to == currentThread->mailBox) {
									
									outMailHdr.length = strlen(sendMessageBCV) + 1;
									// Send the reply message
									success = postOffice->Send(outPktHdr, outMailHdr, sendMessageBCV); 
									if ( !success ) {
										//printf("CVbroadcastRPC : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
										interrupt->Halt();
									}
								}
								
								delete sendMessageBCV;
								break;	
							}
							
							delete sendMessageBCV;
							
							//------------------------------------------------------------------------------------------
							//							Update Server secret storage	
							//------------------------------------------------------------------------------------------

							while ((cvqueue[broadcastCVIndex]->IsEmpty()) == FALSE){
							
								broadcastBit = true;    
								success = false;
								failure = false;
								
								// *********************************************************************************************
								//									CONDITION SIGNAL IN BROADCAST
								//	senderMachineID - machine id of the client that is requesting for CVsignal
								//	senderMailBoxNumber - machine mail box number of the client that is requesting for CVsignal
								// *********************************************************************************************
						
								int inSenderMachineID = removedEventOrderEntry->replyOutPktHdr.to;
								int inSenderMailBoxNumber = (removedEventOrderEntry->replyOutMailHdr.to)-1;
							
								//------------------------------------------------------------------------------------------
								// 							Validate the parameters received
								//------------------------------------------------------------------------------------------	
								sendMessageBCV = new char[MaxMailSize];
								
								if((broadcastLockIndex < 0)||(broadcastLockIndex > locksDBIndex)){
									sprintf(sendMessageBCV, "FAILURE : Invalid Lock index %d",broadcastLockIndex);
									failure = true;
								}	
								else if((broadcastCVIndex < 0)||(broadcastCVIndex > cvsDBIndex)){
									sprintf(sendMessageBCV, "FAILURE : Invalid CV index %d",broadcastLockIndex);
									failure = true;
								}	
								else if(locksDB[broadcastLockIndex].lockName == NULL){
									sprintf(sendMessageBCV, "FAILURE : Lockname %d does not exist",broadcastLockIndex);
									failure = true;
								}
								else if (cvsDB[broadcastCVIndex].cvName == NULL){
									sprintf(sendMessageBCV, "FAILURE : CVname %d does not exist",broadcastLockIndex);
									failure = true;
								}
								else if(inSenderMailBoxNumber != locksDB[broadcastLockIndex].mailBoxNumber){
									sprintf(sendMessageBCV, "FAILURE : Not Lock %d owner",broadcastLockIndex);
									failure = true;
								}
								else if(inSenderMachineID != locksDB[broadcastLockIndex].machineID){
									sprintf(sendMessageBCV, "FAILURE : Not Lock %d owner",broadcastLockIndex);
									failure = true;
								}
								else if ((cvsDB[broadcastCVIndex].cvLockIndex != broadcastLockIndex) && (!failure)) {
									sprintf(sendMessageBCV, "FAILURE : CVlock %d mismatch with Lock %d",cvsDB[broadcastCVIndex].cvLockIndex, broadcastLockIndex);
									failure = true;
								}	
								
								if(failure){
								
									// SEND only if the message belongs to the current networking thread (check the "TO" fields of the QUEUE message)
									if(removedEventOrderEntry->replyOutPktHdr.to == clientMachineID && removedEventOrderEntry->replyOutMailHdr.to == currentThread->mailBox) {
								
										outMailHdr.length = strlen(sendMessageBCV) + 1;
										// Send the reply message
										success = postOffice->Send(outPktHdr, outMailHdr, sendMessageBCV); 
										if ( !success ) {
											//printf("CVsignalRPC : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
											interrupt->Halt();
										}
									
									}
									delete sendMessageBCV;
									break;	// break from while loop
								}
								
								delete sendMessageBCV;
								
								//------------------------------------------------------------------------------------------
								//							Update Server secret storage	
								//------------------------------------------------------------------------------------------
								// if no waiters to signal?
								if(cvqueue[broadcastCVIndex]->IsEmpty()){
									
									// yes, there are no one waiting
									sendMessageBCV = new char[MaxMailSize];
									
									// SEND only if the message belongs to the current networking thread (check the "TO" fields of the QUEUE message)
									if(removedEventOrderEntry->replyOutPktHdr.to == clientMachineID && removedEventOrderEntry->replyOutMailHdr.to == currentThread->mailBox) {
										
										// $
										//printf("No waiters in the CV %d wait queue\n", broadcastCVIndex);
										
										sprintf(sendMessageBCV, "%d : No waiters in CV %d wait queue", broadcastCVIndex, broadcastCVIndex);
										outMailHdr.length = strlen(sendMessageBCV) + 1;
										// Send the reply message
										success = postOffice->Send(outPktHdr, outMailHdr, sendMessageBCV); 
										if ( !success ) {
											//printf("CVsignalRPC : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
											interrupt->Halt();
										}
									}
									delete sendMessageBCV;
									shouldBreak = 1;
									break;	// break from while loop
								}
								else {
									// there are some waiters in the CV wait queue
									
									removedCVEntryCounter++;
									removedCVEntry[removedCVEntryCounter] = (cvWaitEntry*)cvqueue[broadcastCVIndex]->Remove();
									
									cvsDB[broadcastCVIndex].usageCounter --;
									
									// If the removed wait queue entry is valid
									if(removedCVEntry[removedCVEntryCounter] != NULL){
									
										//Acquire the lock before exiting from wait
										if(locksDB[broadcastLockIndex].status == LOCK_FREE){
										
											// make the removed entry as the owner of the lock whic is free right now
											locksDB[broadcastLockIndex].status = LOCK_BUSY;	
											locksDB[broadcastLockIndex].machineID = (*removedCVEntry[removedCVEntryCounter]).replyOutPktHdr.to;
											// make the userprogram of the "net thread which was in the CV wait queue" as the owner
											locksDB[broadcastLockIndex].mailBoxNumber = (*removedCVEntry[removedCVEntryCounter]).replyOutMailHdr.to-1;
											//printf("CVsignalRPC : Lock %d acquired by machine %d from mail Box Number %d\n", broadcastLockIndex, (*removedCVEntry[removedCVEntryCounter]).replyOutPktHdr.to, (*removedCVEntry[removedCVEntryCounter]).replyOutMailHdr.to);
											
											tempOutPktHdr = (*removedCVEntry[removedCVEntryCounter]).replyOutPktHdr;
											tempOutMailHdr = (*removedCVEntry[removedCVEntryCounter]).replyOutMailHdr;
											
											// SEND only if the message belongs to the current networking thread (check the "TO" fields of the QUEUE message)
											if(tempOutPktHdr.to == clientMachineID && tempOutMailHdr.to == currentThread->mailBox) {
												
												sendMessageBCV = new char[MaxMailSize];
												
												strcpy(sendMessageBCV,(*removedCVEntry[removedCVEntryCounter]).replymessage);
												outMailHdr.length = strlen(sendMessageBCV) + 1;
												// Send the reply message
												success = postOffice->Send(outPktHdr, outMailHdr, sendMessageBCV); 
												if ( !success ) {
													//printf("CVsignalRPC : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
													interrupt->Halt();
												}
												
												delete sendMessageBCV;
											}	
											
										}	// if the lock is busy then put the CV removed entry into Lock wait entry
										else{
										
											lockEntryCounter++;
											strcpy((*lockEntry[lockEntryCounter]).replymessage,(*removedCVEntry[removedCVEntryCounter]).replymessage);
											(*lockEntry[lockEntryCounter]).replyOutPktHdr = (*removedCVEntry[removedCVEntryCounter]).replyOutPktHdr;
											(*lockEntry[lockEntryCounter]).replyOutMailHdr = (*removedCVEntry[removedCVEntryCounter]).replyOutMailHdr;
											lockqueue[broadcastLockIndex]->Append((void *)lockEntry[lockEntryCounter]);
											
										}	
									} 
								}
								
								// *********************************************************************************************
								//									CONDITION SIGNAL IN BROADCAST END
								// *********************************************************************************************
							}	// END of While loop for signalling
							
							// shouldbreak will be set when there are no waiters in the cv queue 
							if(shouldBreak == 1){
							
								shouldBreak = 0;
								break; // break from broadcast rpc switch case
								
							}
							
							// If the complete CV wait queue is emptied then make the 
							if(cvqueue[broadcastCVIndex]->IsEmpty()){
								
								// since no more waiters, the waiting lock of the CV is reset to zero
								cvsDB[broadcastCVIndex].cvLockIndex = -1;
								
								// Update the waiter count of the CV
								cvsDB[broadcastCVIndex].usageCounter = 0;
								
							}
							broadcastBit == false;
							
							if((cvsDB[broadcastCVIndex].usageCounter == 0) && (cvsDB[broadcastCVIndex].toBeDestroyedCount == 0)){ 
							
								//destroy the CV if no waiters on this CV and the destroy flag is set
								delete cvsDB[broadcastCVIndex].cvName;
								cvsDB[broadcastCVIndex].cvName = NULL;
								cvsDB[broadcastCVIndex].mailBoxNumber = -1;
								cvsDB[broadcastCVIndex].machineID = -1;
								cvsDB[broadcastCVIndex].toBeDestroyedCount = 0;
								//printf("%d : CV %d is Destroyed by Broadcast",broadcastCVIndex, broadcastCVIndex);
								
							}
							
							// SEND only if the message belongs to the current networking thread (check the "TO" fields of the QUEUE message)
							if(tempOutPktHdr.to == clientMachineID && tempOutMailHdr.to == currentThread->mailBox) {
												
								sendMessageBCV = new char[MaxMailSize];
								
								// Finished broadcasting, reply to the user program if it was requested by my user program
								sprintf(sendMessageBCV, "%d : CV %d finish Broadcast with lock %d",broadcastCVIndex, broadcastCVIndex, broadcastLockIndex);
								outMailHdr.length = strlen(sendMessageBCV) + 1;
								success = postOffice->Send(outPktHdr, outMailHdr, sendMessageBCV); 
								if (!success ) {
									//printf("CVbroadcastRPC : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
									interrupt->Halt();
								}	
								
								delete sendMessageBCV;
							}		
					
							break;
							// *********************************************************************************************
							//									CONDITION BROADCAST END
							// *********************************************************************************************
							
							
						case 9:	//Destroy CV
						
							char *strDestroyCVIndex;
							char *sendMessageDCV;
							
							int destroyCVIndex;
							
							destroyCVIndex = -1;
							
							success = false;
							failure = false;
							
							strDestroyCVIndex = strtok(NULL, " ");
							destroyCVIndex = atoi(strDestroyCVIndex);
							
							/* if(destroyCVIndex == -1){
								//printf("Networking Thread : CV Index not sent to the Server\n");
								break;
							}
							 */
							// *********************************************************************************************
							//									CONDITION DESTROY 
							//	cvIndex - Index of the CV for this destroy
							//	senderMachineID - machine id of the client that is requesting for destroy
							//	senderMailBoxNumber - machine mail box number of the client that is requesting for destroy
							// *********************************************************************************************
							
							// Actual user program machine and mailbox number	
							senderMachineID = removedEventOrderEntry->replyOutPktHdr.to;
							senderMailBoxNumber = (removedEventOrderEntry->replyOutMailHdr.to)-1;
					
							//------------------------------------------------------------------------------------------
							// 							Validate the parameters received
							//------------------------------------------------------------------------------------------	
							sendMessageDCV = new char[MaxMailSize];
							
							if(cvsDB[destroyCVIndex].toBeDestroyedCount == 0){
								//	This lock is not held by the current thread
								sprintf(sendMessageDCV, "FAILURE : CV %d already destroyed",destroyCVIndex);
								failure = true;
							}
							else if((destroyCVIndex < 0)||(destroyCVIndex > cvsDBIndex)){
								//if Invalid lock index
								sprintf(sendMessageDCV, "FAILURE : Invalid CV index %d\n",destroyCVIndex);
								failure = true;
							}	
							else if(cvsDB[destroyCVIndex].cvName == NULL){
								sprintf(sendMessageDCV, "FAILURE : CV %d does not exist",destroyCVIndex);
								failure = true;
							}
							
							if(failure){
							
								// SEND only if the message belongs to the current networking thread (check the "TO" fields of the QUEUE message)
								if(removedEventOrderEntry->replyOutPktHdr.to == clientMachineID && removedEventOrderEntry->replyOutMailHdr.to == currentThread->mailBox) {
									
									outMailHdr.length = strlen(sendMessageDCV) + 1;
									// Send the reply message
									success = postOffice->Send(outPktHdr, outMailHdr, sendMessageDCV); 

									if ( !success ) {
										//printf("DestroyCVRPC : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
										interrupt->Halt();
									}
								}
								
								delete sendMessageDCV;
								break;
							}
							
							delete sendMessageDCV;
							
							//------------------------------------------------------------------------------------------
							//							Update Server secret storage	
							//------------------------------------------------------------------------------------------
							
							if(cvsDB[destroyCVIndex].toBeDestroyedCount != 0){
								
								cvsDB[destroyCVIndex].toBeDestroyedCount--;
								
							}
							
							sendMessageDCV = new char[MaxMailSize];
							
							if((cvsDB[destroyCVIndex].usageCounter == 0) && (cvsDB[destroyCVIndex].toBeDestroyedCount == 0)){ 
								//destroy
								delete cvsDB[destroyCVIndex].cvName;
								cvsDB[destroyCVIndex].cvName = NULL;
								cvsDB[destroyCVIndex].cvLockIndex = -1;
								cvsDB[destroyCVIndex].mailBoxNumber = -1;
								cvsDB[destroyCVIndex].machineID = -1;
								cvsDB[destroyCVIndex].toBeDestroyedCount = 0;
								sprintf(sendMessageDCV, "%d : CV %d is Destroyed",destroyCVIndex, destroyCVIndex);
								//printf("DestroyCVRPC : CV %d Destroyed by machine %d from mail Box Number %d\n", destroyCVIndex, senderMachineID, senderMailBoxNumber);
							}
							
							// Following if statements can be replaced with a single else statement but to make it more sensible 3 if statements are placed below
							if((cvsDB[destroyCVIndex].usageCounter != 0) && (cvsDB[destroyCVIndex].toBeDestroyedCount == 0)){
								sprintf(sendMessageDCV, "%d : CV %d is going to be Destroyed",destroyCVIndex, destroyCVIndex);
							}	
							
							if((cvsDB[destroyCVIndex].usageCounter == 0) && (cvsDB[destroyCVIndex].toBeDestroyedCount != 0)){
								sprintf(sendMessageDCV, "%d : CV %d is going to be Destroyed",destroyCVIndex, destroyCVIndex);
							}	
							
							if((cvsDB[destroyCVIndex].usageCounter != 0) && (cvsDB[destroyCVIndex].toBeDestroyedCount != 0)){
								sprintf(sendMessageDCV, "%d : CV %d is going to be Destroyed",destroyCVIndex, destroyCVIndex);
							}	
							
							// SEND only if the message belongs to the current networking thread (check the "TO" fields of the QUEUE message)
							if(removedEventOrderEntry->replyOutPktHdr.to == clientMachineID && removedEventOrderEntry->replyOutMailHdr.to == currentThread->mailBox) {
							
								outMailHdr.length = strlen(sendMessageDCV) + 1;
								success = postOffice->Send(outPktHdr, outMailHdr, sendMessageDCV); 
								if (!success ) {
									//printf("DestroyCVRPC : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
									interrupt->Halt();
								}
								
							}
							
							delete sendMessageDCV;
							
							break;
							// *********************************************************************************************
							//									CONDITION DESTROY END
							// *********************************************************************************************
							
						
						case 10: // Create MV
							
							char *nameMV;
							char *sendMessageCMV;
							char *strMVInitValue;
							
							int MVIndex;
							int MVInitValue;
							
							MVInitValue = 0;
							
							nameMV = strtok(NULL, " ");
							strMVInitValue = strtok(NULL, " ");
							
							MVInitValue = atoi(strMVInitValue);
							
							success = 0;
							
		
							if(nameMV == NULL){
							
								//printf("Networking Thread : MV Name not sent to the Server\n");
								
								// SEND only if the message belongs to the current networking thread (check the "TO" fields of the QUEUE message)
								if(removedEventOrderEntry->replyOutPktHdr.to == clientMachineID && removedEventOrderEntry->replyOutMailHdr.to == currentThread->mailBox) {
									
									sendMessageCMV = new char[MaxMailSize];
									
									sprintf(sendMessageCMV, "FAILURE : MV Name is NULL");
									// Format the reply message to the client waiting for the MV Index from the server
									outMailHdr.length = strlen(sendMessageCMV) + 1;
									// Send the message
									success = postOffice->Send(outPktHdr, outMailHdr, sendMessageCMV); 

									if ( !success ) {
										//printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
										interrupt->Halt();
									}	
									
									delete sendMessageCMV;
								}	
								break;		
							}
							
							// *********************************************************************************************
							//									CREATE MONITOR VARIABLE
							//	name - Name of the MV to be created on the server
							//	senderMachineID - Owner machine of the MV that is going to be created
							//	senderMailBoxNumber - Owner machine mail box number of the MV that is going to be created			
							// *********************************************************************************************
							
							// Actual user program machine and mailbox number	
							senderMachineID = removedEventOrderEntry->replyOutPktHdr.to;
							senderMailBoxNumber = (removedEventOrderEntry->replyOutMailHdr.to)-1;
							
							//------------------------------------------------------------------------------------------
							// 							Validate the MV parameters received
							//------------------------------------------------------------------------------------------
							
							for(index = 0; index <= mvsDBIndex; index++){
								if(mvsDB[index].toBeDestroyedCount != 0) {
									if(strcmp(mvsDB[index].mvName, nameMV) == 0){
										//printf("CreateMVRPC : MV already exists with the same name %s\n", nameMV);
										
										// increment the number of creators of the MV with same name before being destroyed
										mvsDB[index].toBeDestroyedCount++;
										
										// SEND only if the message belongs to the current networking thread (check the "TO" fields of the QUEUE message)
										if(removedEventOrderEntry->replyOutPktHdr.to == clientMachineID && removedEventOrderEntry->replyOutMailHdr.to == currentThread->mailBox) {	
										
											sendMessageCMV = new char[MaxMailSize];
											
											sprintf(sendMessageCMV, "%d : MV already exists", index);
											// Format the reply message to the client waiting for the MV Index from the server
											// Sending the MV index of the MV which is already created by someother client
											outMailHdr.length = strlen(sendMessageCMV) + 1;
											// $
											//printf("CreateMVRPC : reply message is -> %s\n", sendMessageCMV);
											// Send the message
											success = postOffice->Send(outPktHdr, outMailHdr, sendMessageCMV); 

											if ( !success ) {
												//printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
												interrupt->Halt();
											}
										
											//delete after send
											delete sendMessageCMV;
											
										}		
										shouldBreak = 1;
										break;
									}
								}	
							}
							if(shouldBreak == 1) {
								shouldBreak = 0;
								break;
							}
							
							if(mvsDBIndex == (MAX_MVS-1)){
							
								//printf("CreateMVRPC : Reached max MV Count, No more MVs can be created\n");	
								
								// SEND only if the message belongs to the current networking thread (check the "TO" fields of the QUEUE message)
								if(removedEventOrderEntry->replyOutPktHdr.to == clientMachineID && removedEventOrderEntry->replyOutMailHdr.to == currentThread->mailBox) {
									
									sendMessageCMV = new char[MaxMailSize];
									
									// Format the FAILURE reply message to the client waiting for the MV Index from the server
									sendMessageCMV = "FAILURE : Reached max MV Count";
									
									outMailHdr.length = strlen(sendMessageCMV) + 1;
									// $
									//printf("CreateMVRPC : reply message is -> %s\n", sendMessageCMV);
									// Send the message
									success = postOffice->Send(outPktHdr, outMailHdr, sendMessageCMV); 

									if ( !success ) {
										//printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
										interrupt->Halt();
									}	
									
									delete sendMessageCMV;
								}	
								break;
								
							}
							
							//------------------------------------------------------------------------------------------
							//							Update Server secret storage	
							//------------------------------------------------------------------------------------------

							// Move the index by one to store the information of the new MV created by the server
							mvsDBIndex++;
							
							// Store the Lock information
							strcpy(mvsDB[mvsDBIndex].mvName, nameMV);
							
							// toBeDestroyed counter will be decremented when the destroy MV is called
							mvsDB[mvsDBIndex].toBeDestroyedCount = 1;
							
							mvsDB[mvsDBIndex].value = MVInitValue;
							
							//------------------------------------------------------------------------------------------
							//							Form a reply message and send to client
							//------------------------------------------------------------------------------------------
							
							// SEND only if the message belongs to the current networking thread (check the "TO" fields of the QUEUE message)
							if(removedEventOrderEntry->replyOutPktHdr.to == clientMachineID && removedEventOrderEntry->replyOutMailHdr.to == currentThread->mailBox) {
									
								sendMessageCMV = new char[MaxMailSize];
							
								// Format the reply message to the client waiting for the MV Index from the server
								sprintf(sendMessageCMV, "%d ", mvsDBIndex);
								outMailHdr.length = strlen(sendMessageCMV) + 1;
								// $
								//printf("CreateMVRPC : reply message is -> %s\n", sendMessageCMV);
								// Send the first message
								success = postOffice->Send(outPktHdr, outMailHdr, sendMessageCMV); 

								if ( !success ) {
									//printf("CreateMVRPC : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
									interrupt->Halt();
								}
								
								delete sendMessageCMV;
							}
							
							break;
							// *********************************************************************************************
							//									CREATE MONITOR VARIABLE END
							// *********************************************************************************************
							
						case 11: // Get MV
							
							char *strGetMVIndex;
							char *sendMessageGCV;
						
							int getMVIndex;
							
							getMVIndex = -1;
							
							success = false;
							failure = false;
							
							strGetMVIndex = strtok(NULL, " ");
							getMVIndex = atoi(strGetMVIndex);
							
							/* if(getMVIndex == -1){
								//printf("Networking Thread : MV Index not passed to the Server\n");
								break;
							}
							 */
							// *********************************************************************************************
							//									GET MONITOR VARIABLE
							//	getMVIndex - Index of the MV whose value is requested
							//	senderMachineID - Owner machine of the MV that is going to be created
							//	senderMailBoxNumber - Owner machine mail box number of the MV that is going to be created			
							// *********************************************************************************************
							
							// Actual user program machine and mailbox number	
							senderMachineID = removedEventOrderEntry->replyOutPktHdr.to;
							senderMailBoxNumber = (removedEventOrderEntry->replyOutMailHdr.to)-1;
						
							//------------------------------------------------------------------------------------------
							// 							Validate the parameters received
							//------------------------------------------------------------------------------------------	
							
							sendMessageGCV = new char[MaxMailSize];
							
							//MV is already destroyed
							if(mvsDB[getMVIndex].toBeDestroyedCount == 0){
								sprintf(sendMessageGCV, "FAILURE : MV %d is Destroyed" ,getMVIndex);
								failure = true;
							}
							else if((getMVIndex < 0)||(getMVIndex > mvsDBIndex)){
								sprintf(sendMessageGCV, "FAILURE : Invalid MV index %d",getMVIndex);
								failure = true;
							}	
							else if(mvsDB[getMVIndex].mvName == NULL){
								sprintf(sendMessageGCV, "FAILURE : MV %d name is NULL",getMVIndex);
								failure = true;
							}
							if(failure){
								
								// SEND only if the message belongs to the current networking thread (check the "TO" fields of the QUEUE message)
								if(removedEventOrderEntry->replyOutPktHdr.to == clientMachineID && removedEventOrderEntry->replyOutMailHdr.to == currentThread->mailBox) {
							
									outMailHdr.length = strlen(sendMessageGCV) + 1;
									// Send the reply message
									success = postOffice->Send(outPktHdr, outMailHdr, sendMessageGCV); 

									if ( !success ) {
										//printf("GetMVRPC : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
										interrupt->Halt();
									}
								}	
								
								delete sendMessageGCV;	
								break;
							}
							
							delete sendMessageGCV;	
							
							// SEND only if the message belongs to the current networking thread (check the "TO" fields of the QUEUE message)
							if(removedEventOrderEntry->replyOutPktHdr.to == clientMachineID && removedEventOrderEntry->replyOutMailHdr.to == currentThread->mailBox) {
								
								sendMessageGCV = new char[MaxMailSize];
								
								// Send back the value of the MV requested by the userprog
								sprintf(sendMessageGCV, "%d : MV %d value is %d", mvsDB[getMVIndex].value, getMVIndex, mvsDB[getMVIndex].value);
								outMailHdr.length = strlen(sendMessageGCV) + 1;
								// Send the reply message
								success = postOffice->Send(outPktHdr, outMailHdr, sendMessageGCV); 

								if ( !success ) {
									//printf("GetMVRPC : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
									interrupt->Halt();
								}
								
								delete sendMessageGCV;	
							}
							
							break;
							// *********************************************************************************************
							//									GET MONITOR VARIABLE END
							// *********************************************************************************************
							
						case 12: // Set MV
							
							char *strSetMVIndex;
							char *strSetMVValue;
							char *sendMessageSMV;
							
							int setMVIndex;
							int setMVValue;
							
							setMVIndex = -1;
							setMVValue = -1;
							
							success = false;
							failure = false;
							
							strSetMVIndex = strtok(NULL, " ");
							setMVIndex = atoi(strSetMVIndex);
							
							strSetMVValue = strtok(NULL, " ");
							setMVValue = atoi(strSetMVValue);
							
				//			if(setMVIndex == -1){
				//				//printf("Networking Thread : MV Index not passed to the Server\n");
				//				break;
				//			}
							
							// *********************************************************************************************
							//									SET MONITOR VARIABLE
							//	setMVIndex - Index of the MV whose value is to be set
							//	senderMachineID - Owner machine of the MV that is going to be created
							//	senderMailBoxNumber - Owner machine mail box number of the MV that is going to be created			
							// *********************************************************************************************
							
							// Actual user program machine and mailbox number	
							senderMachineID = removedEventOrderEntry->replyOutPktHdr.to;
							senderMailBoxNumber = (removedEventOrderEntry->replyOutMailHdr.to)-1;
						
							//------------------------------------------------------------------------------------------
							// 							Validate the parameters received
							//------------------------------------------------------------------------------------------	
							
							sendMessageSMV = new char[MaxMailSize];
							
							//MV is already destroyed
							if(mvsDB[setMVIndex].toBeDestroyedCount == 0){
								sprintf(sendMessageSMV, "FAILURE : MV %d is Destroyed" ,setMVIndex);
								failure = true;
							}
							else if((setMVIndex < 0)||(setMVIndex > mvsDBIndex)){
								sprintf(sendMessageSMV, "FAILURE : Invalid MV index %d",setMVIndex);
								failure = true;
							}	
							else if(mvsDB[setMVIndex].mvName == NULL){
								sprintf(sendMessageSMV, "FAILURE : MV %d name is NULL",setMVIndex);
								failure = true;
							}
							
							if(failure){
								
								// SEND only if the message belongs to the current networking thread (check the "TO" fields of the QUEUE message)
								if(removedEventOrderEntry->replyOutPktHdr.to == clientMachineID && removedEventOrderEntry->replyOutMailHdr.to == currentThread->mailBox) {
							
									outMailHdr.length = strlen(sendMessageSMV) + 1;
									// Send the reply message
									success = postOffice->Send(outPktHdr, outMailHdr, sendMessageSMV); 

									if ( !success ) {
										//printf("SetMVRPC : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
										interrupt->Halt();
									}
								}	
								delete sendMessageSMV;
								break;
							}
							
							delete sendMessageSMV;
							
							
							// Set the value of the MV requested by the userprog
							mvsDB[setMVIndex].value = setMVValue;
							
							
							// SEND only if the message belongs to the current networking thread (check the "TO" fields of the QUEUE message)
							if(removedEventOrderEntry->replyOutPktHdr.to == clientMachineID && removedEventOrderEntry->replyOutMailHdr.to == currentThread->mailBox) {
								
								sendMessageSMV = new char[MaxMailSize];
								
								sprintf(sendMessageSMV, "%d : MV %d value is set to %d",setMVValue, setMVIndex, setMVValue);
								outMailHdr.length = strlen(sendMessageSMV) + 1;
								// Send the reply message
								success = postOffice->Send(outPktHdr, outMailHdr, sendMessageSMV); 
								if ( !success ) {
									//printf("SetMVRPC : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
									interrupt->Halt();
								}
								
								delete sendMessageSMV;
							}
							
							break;	
							// *********************************************************************************************
							//									SET MONITOR VARIABLE END
							// *********************************************************************************************
							
							
						case 13://Destroy MV
						
							char *strDestroyMVIndex;
							char *sendMessageDMV;
							
							int destroyMVIndex;
							
							destroyMVIndex = -1;
							
							success = false;
							failure = false;
							
							strDestroyMVIndex = strtok(NULL, " ");
							
							destroyMVIndex = atoi(strDestroyMVIndex);
							
				//			if(destroyMVIndex == -1){
				//				//printf("Networking Thread : CV Index not sent to the Server\n");
				//				//return;
				//				break;
				//			}
							
							// *********************************************************************************************
							//									DESTROY MONITOR VARIABLE
							//	destroyMVIndex - Index of the MV to be destroyed on the server
							//	senderMachineID - Owner machine of the MV that is going to be created
							//	senderMailBoxNumber - Owner machine mail box number of the MV that is going to be created			
							// *********************************************************************************************

							// Actual user program machine and mailbox number	
							senderMachineID = removedEventOrderEntry->replyOutPktHdr.to;
							senderMailBoxNumber = (removedEventOrderEntry->replyOutMailHdr.to)-1;
					
							//------------------------------------------------------------------------------------------
							// 							Validate the MV parameters received
							//------------------------------------------------------------------------------------------
							
							sendMessageDMV = new char[MaxMailSize];
							
							//MV is already destroyed
							if(mvsDB[destroyMVIndex].toBeDestroyedCount == 0){
								sprintf(sendMessageDMV, "FAILURE : MV %d is already destroyed" ,destroyMVIndex);
								failure = true;
							}
							else if((destroyMVIndex < 0)||(destroyMVIndex > mvsDBIndex)){
								sprintf(sendMessageDMV, "FAILURE : Invalid MV index %d",destroyMVIndex);
								failure = true;
							}	
							else if(mvsDB[destroyMVIndex].mvName == NULL){
								sprintf(sendMessageDMV, "FAILURE : MV %d name is NULL",destroyMVIndex);
								failure = true;
							}
							if(failure){
							
								// SEND only if the message belongs to the current networking thread (check the "TO" fields of the QUEUE message)
								if(removedEventOrderEntry->replyOutPktHdr.to == clientMachineID && removedEventOrderEntry->replyOutMailHdr.to == currentThread->mailBox) {
								
									outMailHdr.length = strlen(sendMessageDMV) + 1;
									// Send the reply message
									success = postOffice->Send(outPktHdr, outMailHdr, sendMessageDMV); 

									if ( !success ) {
										//printf("DestroyMVRPC : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
										interrupt->Halt();
									}
								}
								
								delete sendMessageDMV;
								break;
							}
							
							delete sendMessageDMV;
							
							//------------------------------------------------------------------------------------------
							//							Update Server secret storage	
							//------------------------------------------------------------------------------------------
							
							// toBeDestroyed flag will is set so that no one else can GET or SET the value of the MV
							if(mvsDB[destroyMVIndex].toBeDestroyedCount != 0){
								mvsDB[destroyMVIndex].toBeDestroyedCount--;
							}
							mvsDB[destroyMVIndex].value = 0;
							
							//------------------------------------------------------------------------------------------
							//							Form a reply message and send to client
							//------------------------------------------------------------------------------------------
							
							sendMessageDMV = new char[MaxMailSize];
							
							// Format the reply message to the client waiting for the MV Index from the server
							if(mvsDB[destroyMVIndex].toBeDestroyedCount == 0){
								sprintf(sendMessageDMV, "%d : MV %d is Destroyed successfully", destroyMVIndex, destroyMVIndex);
							}
							else{
								sprintf(sendMessageDMV, "%d : MV %d is going to be Destroyed", destroyMVIndex, destroyMVIndex);
							}
							
							outMailHdr.length = strlen(sendMessageDMV) + 1;
							// $
							//printf("DestroyMVRPC : reply message is -> %s\n", sendMessageDMV);
							
							// SEND only if the message belongs to the current networking thread (check the "TO" fields of the QUEUE message)
							if(removedEventOrderEntry->replyOutPktHdr.to == clientMachineID && removedEventOrderEntry->replyOutMailHdr.to == currentThread->mailBox) {
							
								// Send the first message
								success = postOffice->Send(outPktHdr, outMailHdr, sendMessageDMV); 

								if ( !success ) {
									//printf("DestroyMVRPC : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
									interrupt->Halt();
								}
							}
							
							delete sendMessageDMV;
							break;	
							// *********************************************************************************************
							//									DESTROY MONITOR VARIABLE END
							// *********************************************************************************************
				
							
					}	// end of switch
				}else{
		// **************************************************************************************************
		//			IF THE MESSAGE IS NOT TO BE PROCESSED THEN APPEND BACK THE MESSAGE TO QUEUE		
		// **************************************************************************************************
					stopProcessing = 1;
					// Append back the message in the event order queue 
					//printf("\n Cannot process the message %s-> PUT it BACK into Q\n", removedEventOrderEntry->replymessage);
					eventOrderQueue->SortedInsert(removedEventOrderEntry, *toBeProcessed);
					
				}		
			}	
		
		} // else case (if from networking thread)
				
		// delete after every while looping
		delete receiveMessage;
	}
}
// ***********************NETWORKING THREAD END**********************************



int copyin(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes from the current thread's virtual address vaddr.
    // Return the number of bytes so read, or -1 if an error occors.
    // Errors can generally mean a bad virtual address was passed in.
    bool result;
    int n=0;			// The number of bytes copied in
    int *paddr = new int;

    while ( n >= 0 && n < len) {
      result = machine->ReadMem( vaddr, 1, paddr );
      while(!result) // FALL 09 CHANGES
	  {
   			result = machine->ReadMem( vaddr, 1, paddr ); // FALL 09 CHANGES: TO HANDLE PAGE FAULT IN THE ReadMem SYS CALL
	  }	
      
      buf[n++] = *paddr;
     
      if ( !result ) {
	//translation failed
	return -1;
      }

      vaddr++;
    }

    delete paddr;
    return len;
}

int copyout(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes to the current thread's virtual address vaddr.
    // Return the number of bytes so written, or -1 if an error
    // occors.  Errors can generally mean a bad virtual address was
    // passed in.
    bool result;
    int n=0;			// The number of bytes copied in

    while ( n >= 0 && n < len) {
      // Note that we check every byte's address
      result = machine->WriteMem( vaddr, 1, (int)(buf[n++]) );

      if ( !result ) {
	//translation failed
	return -1;
      }

      vaddr++;
    }

    return n;
}


void Create_Syscall(unsigned int vaddr, int len) {
    // Create the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  No
    // way to return errors, though...
    char *buf = new char[len+1];	// Kernel buffer to put the name in

    if (!buf) return;

    if( copyin(vaddr,len,buf) == -1 ) {
	//printf("%s","Bad pointer passed to Create\n");
	delete buf;
	return;
    }

    buf[len]='\0';

    fileSystem->Create(buf,0);
    delete[] buf;
    return;
}

int Open_Syscall(unsigned int vaddr, int len) {
    // Open the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  If
    // the file is opened successfully, it is put in the address
    // space's file table and an id returned that can find the file
    // later.  If there are any errors, -1 is returned.
    char *buf = new char[len+1];	// Kernel buffer to put the name in
    OpenFile *f;			// The new open file
    int id;				// The openfile id

    if (!buf) {
	//printf("%s","Can't allocate kernel buffer in Open\n");
	return -1;
    }

    if( copyin(vaddr,len,buf) == -1 ) {
	//printf("%s","Bad pointer passed to Open\n");
	delete[] buf;
	return -1;
    }

    buf[len]='\0';

    f = fileSystem->Open(buf);
    delete[] buf;

    if ( f ) {
	if ((id = currentThread->space->fileTable.Put(f)) == -1 )
	    delete f;
	return id;
    }
    else
	return -1;
}

void Write_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one. For disk files, the file is looked
    // up in the current address space's open file table and used as
    // the target of the write.
    
    char *buf;		// Kernel buffer for output
    OpenFile *f;	// Open file for output

    if ( id == ConsoleInput) return;
    
    if ( !(buf = new char[len]) ) {
	//printf("%s","Error allocating kernel buffer for write!\n");
	return;
    } else {
        if ( copyin(vaddr,len,buf) == -1 ) {
	   ////printf("%s","Bad pointer passed to to write: data not written\n");
	    delete[] buf;
	    return;
	}
    }

    if ( id == ConsoleOutput) {
      for (int ii=0; ii<len; ii++) {
	//printf("%c",buf[ii]);
      }

    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    f->Write(buf, len);
	} else {
	   ////printf("%s","Bad OpenFileId passed to Write\n");
	    len = -1;
	}
    }

    delete[] buf;
}

int Read_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one.    We reuse len as the number of bytes
    // read, which is an unnessecary savings of space.
    char *buf;		// Kernel buffer for input
    OpenFile *f;	// Open file for output

    if ( id == ConsoleOutput) return -1;
    
    if ( !(buf = new char[len]) ) {
	//printf("%s","Error allocating kernel buffer in Read\n");
	return -1;
    }

    if ( id == ConsoleInput) {
      //Reading from the keyboard
      scanf("%s", buf);

      if ( copyout(vaddr, len, buf) == -1 ) {
	//printf("%s","Bad pointer passed to Read: data not copied\n");
      }
    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    len = f->Read(buf, len);
	    if ( len > 0 ) {
	        //Read something from the file. Put into user's address space
  	        if ( copyout(vaddr, len, buf) == -1 ) {
		   ////printf("%s","Bad pointer passed to Read: data not copied\n");
		}
	    }
	} else {
	   ////printf("%s","Bad OpenFileId passed to Read\n");
	    len = -1;
	}
    }

    delete[] buf;
    return len;
}

void Close_Syscall(int fd) {
    // Close the file associated with id fd.  No error reporting.
    OpenFile *f = (OpenFile *) currentThread->space->fileTable.Remove(fd);

    if ( f ) {
      delete f;
    } else {
     ////printf("%s","Tried to close an unopen file\n");
    }
}

//**************************************************************************************
//								CREATE LOCK SYSCALL
//**************************************************************************************
int CreateLock_Syscall(unsigned int vaddr, int len, int uniqueId) {
	// Allocates a LOCK from the kernel lock list to the user prog
	
	int rv = -1;	// index value of the Lock created
	char *lockName = new char[len+5];
	char *buffer = new char[len+1];
	if((vaddr+len) <= vaddr){
		//printf("%s", "CreateLock_Syscall : Invalid Virtual address length\n");
		delete[] lockName;
		return -1;
	}	
	
	if((vaddr < 0) || (vaddr > currentThread->space->spaceSize)){
		//printf("%s", "CreateLock_Syscall : Invalid virtual address\n");
		delete[] lockName;
		return -1;
	}	
	
	if((vaddr + len) > currentThread->space->spaceSize){
		//printf("%s", "CreateLock_Syscall : Trying to access Invalid Virtual address.\n");
		delete[] lockName;
		return -1;
	}	
	
	if(len > MAX_CHARLEN){
		//printf("%s", "CreateLock_Syscall : Exceeding Max Lock Name length\n");
		delete[] lockName;
		return -1;
	}	
	
	if (!lockName) {
		//printf("%s","CreateLock_Syscall : Can't allocate kernel buffer in CreateLock\n");
		delete[] lockName;
		return -1;
    }
	
	if (!buffer) {
		//printf("%s","CreateLock_Syscall : Can't allocate kernel buffer in CreateLock\n");
		delete[] lockName;
		delete[] buffer;
		return -1;
    }
	
    if( copyin(vaddr,len,buffer) == -1 ) {
		//printf("%s", "CreateLock_Syscall : Could not copy from the virtual address\n");
		delete[] lockName;
		return -1;
    }
	
	// If any unique id is passed from user program then append it to the lock name before passing it to the networking thread
	if(uniqueId == 0) {
		sprintf(lockName, "%s%c", buffer, '\0');
	}else{
		sprintf(lockName, "%s%d%c", buffer, uniqueId, '\0');
	}	

//----------------------------------------------------------------------------------------
//									START	NETWORKING
//----------------------------------------------------------------------------------------	
#ifdef NETWORK
	
	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
	char *lockIndexStr = NULL;
	char *sendMessage = new char[MaxMailSize]; 
	char *receiveMessage = new char[MaxMailSize];
	char *receiveMessageCopy = new char[MaxMailSize];	
	int lockIndex = -1;
	bool success = 0;
	
	// construct packet, mail header for message
    // To: Server Nachos machine, mailbox 0
    // From: Client machine, reply to: mailbox 1
    outPktHdr.to = clientMachineID;		
    outMailHdr.to = currentThread->mailBox+1;
    outMailHdr.from = currentThread->mailBox;
	outPktHdr.from = clientMachineID;	
	
	// Check if maximum number of Ports on the machine are under use
	if(currentThread->mailBox > 60){
		//printf("CreateLock_Syscall : Requesting invalid port number %d on machine %d\n", currentThread->threadId, clientMachineID);
		return -1;
	}
	
	sprintf(sendMessage, "%d %s",createLockType, lockName);
	outMailHdr.length = strlen(sendMessage) + 1;	

	//printf("CreateLock_Syscall : Client sending the Create Lock request to the Networking Thread\n");
	// Send the message to the server nachos machine with machine ID '0'
	success = postOffice->Send(outPktHdr, outMailHdr, sendMessage); 

	if ( !success ) {
	 ////printf("CreateLock_Syscall : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
	  interrupt->Halt();
	}
	
	// Wait for the reply message from the server nachos machine
	////printf("CreateLock_Syscall : Client waiting to receive Create Lock request reply\n");
	postOffice->Receive(currentThread->mailBox, &inPktHdr, &inMailHdr, receiveMessage);
	fflush(stdout);
	strcpy(receiveMessageCopy, receiveMessage);
	
	lockIndexStr = strtok(receiveMessage, " ");
	
	if(strcmp(lockIndexStr, "FAILURE") == 0){
		// FAILURE message received from the server
		////printf("CreateLock_Syscall : %s\n", receiveMessageCopy);
		rv = -1;	
	}else{
		lockIndex = atoi(lockIndexStr); 
		////printf("CreateLock_Syscall : Lock Index received from server is %d\n", lockIndex);
		////printf("CreateLock_Syscall : Msg fron Server : %s\n", receiveMessageCopy);
		rv = lockIndex;
	}	
	return rv;
	
	
#endif // NETWORK
//----------------------------------------------------------------------------------------
//									END	NETWORKING
//----------------------------------------------------------------------------------------	
	
	if(kernelLockListLock == NULL){
		////printf("%s","CreateLock_Syscall:kernelLockListLock is NULL\n");
		delete[] lockName;
		return -1;
    }
	
	if(nextLockIndex > MAX_LOCKS){
		////printf("%s","CreateLock_Syscall:No more Locks can be created, count reached MAX\n");
		delete[] lockName;
		return -1;
    }	
	
	// Acquire the Lock over the Lock list to access 
	if(kernelLockListLock == NULL){
		////printf("%s","CreateLock_Syscall:Kernel Lock List Lock is NULL\n");
		return -1;
    }	
	kernelLockListLock->Acquire();
	
	// Create the Lock for the user prog
	lockList[nextLockIndex].lock = new Lock(lockName);
	
	// Make the address space of the Lock same as the currentThread
	lockList[nextLockIndex].lockAddrSpace = currentThread->space;
	
	// Make the usage counter to zero for the new lock created
	lockList[nextLockIndex].usageCounter = 0;
	
	lockList[nextLockIndex].toBeDestroyed = true;
	
	rv = nextLockIndex;
	nextLockIndex++;
	kernelLockListLock->Release();
	delete[] lockName;
	// return the index of the Lock created to the user program
	return rv;
}	

//**************************************************************************************
//								ACQUIRE LOCK SYSCALL
//**************************************************************************************
int Acquire_Syscall(unsigned int lockIndex) {

//----------------------------------------------------------------------------------------
//									START	NETWORKING
//----------------------------------------------------------------------------------------	
#ifdef NETWORK
	
	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
	char *lockIndexStr = NULL;
	char sendMessage[MaxMailSize]; 
	char receiveMessage[MaxMailSize]; 
	char receiveMessageCopy[MaxMailSize]; 
	bool success = 0;
	int rv = -1;	// index value of the Lock acquired
	
	// construct packet, mail header for message
    // To: Server Nachos machine, mailbox 0
    // From: Client machine, reply to: mailbox 1
    outPktHdr.to = clientMachineID;		
    outMailHdr.to = currentThread->mailBox+1;
    outMailHdr.from = currentThread->mailBox;
	outPktHdr.from = clientMachineID;	
	
	// Check for the maximum number of Ports on the machine are under use
	if(currentThread->mailBox > 60){
		////printf("Acquire_Syscall : Requesting invalid port number %d on machine %d\n", currentThread->mailBox, clientMachineID);
		return -1;
	}
	
	sprintf(sendMessage, "%d %d",acquireLockType, lockIndex);
	
	outMailHdr.length = strlen(sendMessage) + 1;	
	
	// Send the message to the server nachos machine with machine ID '0'
	success = postOffice->Send(outPktHdr, outMailHdr, sendMessage); 
	if ( !success ) {
		////printf("Acquire_Syscall : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
		interrupt->Halt();
	}
	////printf("Acquire_Syscall : Client sending the AcquireLock %d request to the server\n", lockIndex);
	
	// Wait for the reply message from the server nachos machine
	postOffice->Receive(currentThread->mailBox, &inPktHdr, &inMailHdr, receiveMessage);
	
	fflush(stdout);
	strcpy(receiveMessageCopy, receiveMessage);
	
	lockIndexStr = strtok(receiveMessage, " ");
	if(strcmp(lockIndexStr, "FAILURE") == 0){
		// FAILURE message received from the server
		////printf("Acquire_Syscall : %s\n", receiveMessageCopy);
		rv = -1;	
	}else{
		lockIndex = atoi(lockIndexStr); 
		////printf("Acquire_Syscall : Acquired Lock Index received from server is %d\n", lockIndex);
		////printf("Acquire_Syscall : Msg fron Server : %s\n", receiveMessageCopy);
		rv = lockIndex;
	}	
	return rv;
#endif // NETWORK
//----------------------------------------------------------------------------------------
//									END	NETWORKING
//----------------------------------------------------------------------------------------		
	if(kernelLockListLock == NULL){
		////printf("%s","Acquire_Syscall:Kernel Lock List Lock is NULL\n");
		return -1;
    }	
	kernelLockListLock->Acquire();
	
	if((lockIndex < 0) || lockIndex >= nextLockIndex){
		////printf("%s","Acquire_Syscall:Invalid Lock index\n");
		kernelLockListLock->Release();
		return -1;
    }	
	
	if(lockList[lockIndex].lock == NULL){
		////printf("%s","Acquire_Syscall:Lock does not exist\n");
		kernelLockListLock->Release();
		return -1;
    }

	if(lockList[lockIndex].lockAddrSpace != currentThread->space){
		////printf("%s","Acquire_Syscall:Lock does not belong to the current user prog\n");
		kernelLockListLock->Release();
		return -1;
    }	
	
	lockList[lockIndex].usageCounter++;
	lockList[lockIndex].toBeDestroyed = false;
	
	kernelLockListLock->Release();
	lockList[lockIndex].lock->Acquire();
	
	rv = lockIndex;
	return rv;
}

//**************************************************************************************
//								RELEASE LOCK SYSCALL
//**************************************************************************************
int Release_Syscall(unsigned int lockIndex) {

//----------------------------------------------------------------------------------------
//									START	NETWORKING
//----------------------------------------------------------------------------------------	
#ifdef NETWORK
	
	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
	char *lockIndexStr = new char[30];
	char sendMessage[MaxMailSize]; 
	char receiveMessage[MaxMailSize]; 
	char receiveMessageCopy[MaxMailSize]; 
	bool success = 0;
	int rv = -1;	// index value of the Lock released
	
	// construct packet, mail header for message
    // To: Server Nachos machine, mailbox 0
    // From: Client machine, reply to: mailbox 1
    outPktHdr.to = clientMachineID;		
    outMailHdr.to = currentThread->mailBox+1;
    outMailHdr.from = currentThread->mailBox;
	outPktHdr.from = clientMachineID;	
	
	// Check for the maximum number of Ports on the machine are under use
	if(currentThread->mailBox > 60){
		////printf("Release_Syscall : Requesting invalid port number %d on machine %d\n", currentThread->mailBox, clientMachineID);
		return -1;
	}
	
	sprintf(sendMessage, "%d %d",releaseLockType, lockIndex);
	
	outMailHdr.length = strlen(sendMessage) + 1;	
	
	// Send the message to the server nachos machine with machine ID '0'
	success = postOffice->Send(outPktHdr, outMailHdr, sendMessage); 
	if ( !success ) {
		////printf("Release_Syscall : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
		interrupt->Halt();
	}
	////printf("Release_Syscall : Client sending the Release Lock %d request to the server\n", lockIndex);
	
	// Wait for the reply message from the server nachos machine
	postOffice->Receive(currentThread->mailBox, &inPktHdr, &inMailHdr, receiveMessage);
	////printf("Release_Syscall : Server replied for release lock request\n");
	fflush(stdout);
	strcpy(receiveMessageCopy, receiveMessage);
	lockIndexStr = strtok(receiveMessage, " ");
	if(strcmp(lockIndexStr, "FAILURE") == 0){
		// FAILURE message received from the server
		////printf("ReleaseLock_Syscall : %s\n", receiveMessageCopy);
		rv = -1;	
	}else{
		lockIndex = atoi(lockIndexStr); 
		////printf("ReleaseLock_Syscall : Released Lock Index received from server is %d\n", lockIndex);
		////printf("ReleaseLock_Syscall : Msg fron Server : %s\n", receiveMessageCopy);
		rv = lockIndex;
	}	
	return rv;
#endif // NETWORK
//----------------------------------------------------------------------------------------
//									END	NETWORKING
//----------------------------------------------------------------------------------------	
	
	if(kernelLockListLock == NULL){
		////printf("%s","Release_Syscall:Kernel Lock List Lock is NULL\n");
		return -1;
    }	
	kernelLockListLock->Acquire();
	
	if((lockIndex < 0) || lockIndex >= nextLockIndex){
		////printf("%s","Release_Syscall:Invalid Lock index\n");
		kernelLockListLock->Release();
		return -1;
    }	
	
	if(lockList[lockIndex].lock == NULL){
		////printf("%s","Release_Syscall:Lock does not exist\n");
		kernelLockListLock->Release();
		return -1;
    }
	
	if(lockList[lockIndex].lockAddrSpace != currentThread->space){
		////printf("%s","Release_Syscall:Lock does not belong to the current user prog\n");
		kernelLockListLock->Release();
		return -1;
    }	
	lockList[lockIndex].lock->Release();
	lockList[lockIndex].usageCounter--;	
	// Lock can be destroyed since its free
	if(lockList[lockIndex].usageCounter == 0)
		lockList[lockIndex].toBeDestroyed = true;
	
	kernelLockListLock->Release();
	rv = lockIndex;
	
	return rv;
}

//**************************************************************************************
//								DESTROY LOCK SYSCALL
//**************************************************************************************
int DestroyLock_Syscall(unsigned int lockIndex) {

//----------------------------------------------------------------------------------------
//									START	NETWORKING
//----------------------------------------------------------------------------------------	
#ifdef NETWORK
	
	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
	char *lockIndexStr = new char[30];
	char sendMessage[MaxMailSize]; 
	char receiveMessage[MaxMailSize]; 
	char receiveMessageCopy[MaxMailSize]; 
	bool success = 0;
	int rv = -1;	
	
	// construct packet, mail header for message
    // To: Server Nachos machine, mailbox 0
    // From: Client machine, reply to: mailbox 1
    outPktHdr.to = clientMachineID;		
    outMailHdr.to = currentThread->mailBox+1;
    outMailHdr.from = currentThread->mailBox;
	outPktHdr.from = clientMachineID;	
	// Check for the maximum number of Ports on the machine are under use
	if(currentThread->mailBox > 60){
		////printf("Release_Syscall : Requesting invalid port number %d on machine %d\n", currentThread->mailBox, clientMachineID);
		return -1;
	}
	
	sprintf(sendMessage, "%d %d",destroyLockType, lockIndex);
	outMailHdr.length = strlen(sendMessage) + 1;	
	
	// Send the message to the server nachos machine with machine ID '0'
	success = postOffice->Send(outPktHdr, outMailHdr, sendMessage); 
	if ( !success ) {
		////printf("DestroyLock_Syscall : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
		interrupt->Halt();
	}
	////printf("DestroyLock_Syscall : Client sending the Destroy Lock %d request to the server\n", lockIndex);
	
	// Wait for the reply message from the server nachos machine
	postOffice->Receive(currentThread->mailBox, &inPktHdr, &inMailHdr, receiveMessage);
	////printf("DestroyLock_Syscall : Server replied after destroy Lock request\n");
	fflush(stdout);
	
	strcpy(receiveMessageCopy, receiveMessage);
	lockIndexStr = strtok(receiveMessage, " ");
	if(strcmp(lockIndexStr, "FAILURE") == 0){
		// FAILURE message received from the server
		////printf("DestroyLock_Syscall : %s\n", receiveMessageCopy);
		rv = -1;	
	}else{
		lockIndex = atoi(lockIndexStr); 
		////printf("DestroyLock_Syscall : Destroyed Lock Index received from server is %d\n", lockIndex);
		////printf("DestroyLock_Syscall : Msg fron Server : %s\n", receiveMessageCopy);
		rv = lockIndex;
	}	
	return rv;
#endif // NETWORK
//----------------------------------------------------------------------------------------
//									END	NETWORKING
//----------------------------------------------------------------------------------------	
			
	if(kernelLockListLock == NULL){
		////printf("%s","DestroyLock_Syscall:Kernel Lock List Lock is NULL\n");
		return -1;
    }	
	kernelLockListLock->Acquire();
	
	if((lockIndex < 0) || lockIndex >= nextLockIndex){
		////printf("%s","DestroyLock_Syscall:Invalid Lock index\n");
		kernelLockListLock->Release();
		return -1;
    }	
	
	if(lockList[lockIndex].lock == NULL){
		////printf("%s","DestroyLock_Syscall:Lock does not exist\n");
		kernelLockListLock->Release();
		return -1;
    }

	if(lockList[lockIndex].lockAddrSpace != currentThread->space){
		////printf("%s","DestroyLock_Syscall:Lock does not belong to the current user prog\n");
		kernelLockListLock->Release();
		return -1;
    }	

	if(lockList[lockIndex].toBeDestroyed == false){
		////printf("%s","DestroyLock_Syscall:Lock under use!\n");
		kernelLockListLock->Release();
		return -1;
	}	
	
	delete lockList[lockIndex].lock;
	lockList[lockIndex].lock = NULL;
	lockList[lockIndex].lockAddrSpace = NULL;
	lockList[lockIndex].usageCounter = 0;
	lockList[lockIndex].toBeDestroyed = FALSE;
	return 1;
}

//**************************************************************************************
//								CREATE CV SYSCALL
//**************************************************************************************
int CreateCV_Syscall(unsigned int vaddr, int len, int uniqueId) {
	// Allocates a CV from the kernel CV list to the user prog
	
	int rv = -1;	// index value of the CV created
	char *CVName = new char[len+5];
	char *buffer = new char[len+1];
	
	if((vaddr+len) <= vaddr){
		////printf("%s", "CreateCV_Syscall:Invalid Virtual address length\n");
		delete[] CVName;
		return -1;
	}	
	
	if((vaddr < 0) || (vaddr > currentThread->space->spaceSize)){
		////printf("%s", "CreateCV_Syscall:Invalid virtual address\n");
		delete[] CVName;
		return -1;
	}	
	
	if((vaddr+len) > currentThread->space->spaceSize){
		////printf("%s", "CreateCV_Syscall:Trying to access Invalid Virtual address\n");
		delete[] CVName;
		return -1;
	}	
	
	if(len > MAX_CHARLEN){
		////printf("%s", "CreateCV_Syscall:Exceeding Max CV Name length\n");
		delete[] CVName;
		return -1;
	}	
	
	if (!CVName) {
		////printf("%s","CreateCV_Syscall:Can't allocate kernel buffer in CreateCV\n");
		delete[] CVName;
		return -1;
    }
	
	if (!buffer) {
		////printf("%s","CreateCV_Syscall:Can't allocate kernel buffer in CreateCV\n");
		delete[] buffer;
		delete[] CVName;
		return -1;
    }

    if(copyin(vaddr,len,buffer) == -1 ) {
		////printf("%s", "CreateCV_Syscall:Could not copy from the virtual address\n");
		delete[] CVName;
		delete[] buffer;
		return -1;
    }
	
	if(nextCVIndex > MAX_LOCKS){
		////printf("%s","CreateCV_Syscall:No more CVs can be created, count reached MAX\n");
		delete[] CVName;
		return -1;
    }	
	
	// If any unique id is passed from user program then append it to the CV name before passing it to the networking thread
	if(uniqueId == 0) {
		sprintf(CVName, "%s%c", buffer, '\0');
	}else{
		sprintf(CVName, "%s%d%c", buffer, uniqueId, '\0');
	}	
	
//----------------------------------------------------------------------------------------
//									START	NETWORKING
//----------------------------------------------------------------------------------------	
#ifdef NETWORK
	
	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
	char *cvIndexStr = new char[30];
	char sendMessage[MaxMailSize]; 
	char receiveMessage[MaxMailSize]; 
	char receiveMessageCopy[MaxMailSize]; 
	int cvIndex = -1;
	bool success = 0;
	
	// construct packet, mail header for message
    // To: Server Nachos machine, mailbox 0
    // From: Client machine, reply to: mailbox 1
    outPktHdr.to = clientMachineID;		
    outMailHdr.to = currentThread->mailBox+1;
    outMailHdr.from = currentThread->mailBox;
	outPktHdr.from = clientMachineID;	
	
	// Check for the maximum number of Ports on the machine are under use
	if(currentThread->mailBox > 60){
		////printf("CreateCV_Syscall : Requesting invalid port number %d on machine %d\n", currentThread->mailBox, clientMachineID);
		return -1;
	}
	
	sprintf(sendMessage, "%d %s",createCVType, CVName);
	
	outMailHdr.length = strlen(sendMessage) + 1;	
	// Send the message to the server nachos machine with machine ID '0'
	success = postOffice->Send(outPktHdr, outMailHdr, sendMessage); 

	if ( !success ) {
		////printf("CreateCV_Syscall : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
		interrupt->Halt();
	}
	////printf("CreateCV_Syscall : Client sending the message to the server\n");
	
	// Wait for the reply message from the server nachos machine
	postOffice->Receive(currentThread->mailBox, &inPktHdr, &inMailHdr, receiveMessage);
	fflush(stdout);
	strcpy(receiveMessageCopy, receiveMessage);
	cvIndexStr = strtok(receiveMessage, " ");
	if(strcmp(cvIndexStr, "FAILURE") == 0){
		// FAILURE message received from the server
		////printf("CreateCV_Syscall : %s\n", receiveMessageCopy);
		rv = -1;
	}else{
		cvIndex = atoi(cvIndexStr); 
		////printf("CreateCV_Syscall : CV Index received from server is %d\n", cvIndex);
		////printf("CreateCV_Syscall : Msg fron Server : %s\n", receiveMessageCopy);
		rv = cvIndex;
	}
	
	return rv;
	
#endif // NETWORK	
//----------------------------------------------------------------------------------------
//									END	NETWORKING
//----------------------------------------------------------------------------------------	

	// Acquire the Lock over the CV list to access 
	if(kernelCVListLock == NULL){
		////printf("%s","CreateCV_Syscall:Kernel CV List Lock is NULL\n");
		return -1;
    }	
	kernelCVListLock->Acquire();
	
	// Create the CV for the user prog
	CVList[nextCVIndex].condition = new Condition(CVName);
	// @
//	////printf("condition variable %d created\n", nextCVIndex);
	// Make the address space of the CV same as the currentThread
	CVList[nextCVIndex].conditionAddrSpace = currentThread->space;
	// Make the usage counter to zero for the new CV created
	CVList[nextCVIndex].usageCounter = 0;
	CVList[nextCVIndex].toBeDestroyed = true;
	
	rv = nextCVIndex;
	nextCVIndex++;
	kernelCVListLock->Release();
	delete[] CVName;
	
	// return the index of the CV created to the user program
	return rv;
}	

//**************************************************************************************
//								WAIT SYSCALL
//**************************************************************************************
int Wait_Syscall(unsigned int CVIndex, unsigned int lockIndex) {
	
//----------------------------------------------------------------------------------------
//									START	NETWORKING
//----------------------------------------------------------------------------------------	
#ifdef NETWORK
	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
	char *cvIndexStr = new char[30];
	char sendMessage[MaxMailSize]; 
	char receiveMessage[MaxMailSize]; 
	char receiveMessageCopy[MaxMailSize]; 
	bool success = 0;
	int rv = -1;	// index value of the Lock released
		
	// construct packet, mail header for message
    // To: Server Nachos machine, mailbox 0
    // From: Client machine, reply to: mailbox 1
   outPktHdr.to = clientMachineID;		
    outMailHdr.to = currentThread->mailBox+1;
    outMailHdr.from = currentThread->mailBox;
	outPktHdr.from = clientMachineID;
	
	// Check for the maximum number of Ports on the machine are under use
	if(currentThread->mailBox > 60){
		////printf("Wait_Syscall : Requesting invalid port number %d on machine %d\n", currentThread->mailBox, clientMachineID);
		return -1;
	}
	
	sprintf(sendMessage, "%d %d %d",waitCVType, CVIndex, lockIndex);
	
	outMailHdr.length = strlen(sendMessage) + 1;	
	// Send the message to the server nachos machine with machine ID '0'
	success = postOffice->Send(outPktHdr, outMailHdr, sendMessage); 

	if ( !success ) {
		////printf("Wait_Syscall : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
		interrupt->Halt();
	}
	////printf("Wait_Syscall : Client sending the wait request to the server\n");	

	// Wait for the reply message from the server nachos machine
	postOffice->Receive(currentThread->mailBox, &inPktHdr, &inMailHdr, receiveMessage);
	////printf("Wait_Syscall : Server replied after the CV %d wait request\n", CVIndex);
	fflush(stdout);
	strcpy(receiveMessageCopy, receiveMessage);
	cvIndexStr = strtok(receiveMessage, " ");
	if(strcmp(cvIndexStr, "FAILURE") == 0) {
		// FAILURE message received from the server
		////printf("Wait_Syscall : %s\n", receiveMessageCopy);
		rv = -1;
	}else{
		CVIndex = atoi(cvIndexStr);
		////printf("Wait_Syscall : CV index received from server is %d\n", CVIndex);
		////printf("Wait_Syscall : Msg fron Server : %s\n", receiveMessageCopy);
		rv = CVIndex;
	}
	return rv;	

#endif // NETWORK	
//----------------------------------------------------------------------------------------
//									END	NETWORKING
//----------------------------------------------------------------------------------------	
	if(kernelLockListLock == NULL){
		////printf("%s","Wait_Syscall:Kernel Lock List Lock is NULL\n");
		return -1;
    }	
	kernelLockListLock->Acquire();
	
	if((lockIndex < 0) || lockIndex >= nextLockIndex){
		////printf("%s","Wait_Syscall:Invalid Lock index\n");
		kernelLockListLock->Release();
		return -1;
    }	
	
	if(lockList[lockIndex].lock == NULL){
		////printf("%s","Wait_Syscall:Lock does not exist\n");
		kernelLockListLock->Release();
		return -1;
    }

	if(lockList[lockIndex].lockAddrSpace != currentThread->space){
		////printf("%s","Wait_Syscall:Lock does not belong to the current user prog\n");
		kernelLockListLock->Release();
		return -1;
    }	
	
	kernelLockListLock->Release();
	
	if(kernelCVListLock == NULL){
		////printf("%s","Wait_Syscall:Kernel CV List Lock is NULL\n");
		return -1;
    }	
	kernelCVListLock->Acquire();
	
	if((CVIndex < 0) || CVIndex >= nextCVIndex){
		////printf("%s","Wait_Syscall:Invalid CV index\n");
		kernelCVListLock->Release();
		return -1;
    }	
	
	if(CVList[CVIndex].condition == NULL){
		////printf("%s","Wait_Syscall:CV does not exist\n");
		kernelCVListLock->Release();
		return -1;
    }

	if(CVList[CVIndex].conditionAddrSpace != currentThread->space){
		////printf("%s","Wait_Syscall:CV does not belong to the current user prog\n");
		kernelCVListLock->Release();
		return -1;
    }	
	
	// Reacquire the Lock List Lock after validating CVIndex
	kernelLockListLock->Acquire();
	
	// increment the number of waiters on the CV
	CVList[CVIndex].usageCounter++;
	CVList[CVIndex].toBeDestroyed = false;
	
	kernelCVListLock->Release();
	kernelLockListLock->Release();
	
	CVList[CVIndex].condition->Wait(lockList[lockIndex].lock);
	
	return 1;
}

//**************************************************************************************
//								SIGNAL SYSCALL
//**************************************************************************************
int Signal_Syscall(unsigned int CVIndex, unsigned int lockIndex) {

//----------------------------------------------------------------------------------------
//									START	NETWORKING
//----------------------------------------------------------------------------------------	
#ifdef NETWORK
	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
	char *cvIndexStr = new char[30];
	char sendMessage[MaxMailSize]; 
	char receiveMessage[MaxMailSize]; 
	char receiveMessageCopy[MaxMailSize]; 
	bool success = 0;
	int rv = -1;	// index value of the Lock released
		
	// construct packet, mail header for message
    // To: Server Nachos machine, mailbox 0
    // From: Client machine, reply to: mailbox 1
    outPktHdr.to = clientMachineID;		
    outMailHdr.to = currentThread->mailBox+1;
    outMailHdr.from = currentThread->mailBox;
	outPktHdr.from = clientMachineID;
	// Check for the maximum number of Ports on the machine are under use
	if(currentThread->mailBox > 10){
		////printf("Signal_Syscall : Requesting invalid port number %d on machine %d\n", currentThread->mailBox, clientMachineID);
		return -1;
	}
	
	sprintf(sendMessage, "%d %d %d",signalCVType, CVIndex, lockIndex);
	
	outMailHdr.length = strlen(sendMessage) + 1;	
	// Send the message to the server nachos machine with machine ID '0'
	success = postOffice->Send(outPktHdr, outMailHdr, sendMessage); 
	if ( !success ) {
		////printf("Signal_Syscall : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
		interrupt->Halt();
	}
	////printf("Signal_Syscall : Client sending the signal request to the server\n");	

	// Wait for the reply message from the server nachos machine
	postOffice->Receive(currentThread->mailBox, &inPktHdr, &inMailHdr, receiveMessage);
	////printf("Signal_Syscall : Server replied after signal request\n");
	fflush(stdout);
	strcpy(receiveMessageCopy, receiveMessage);
	cvIndexStr = strtok(receiveMessage, " ");
	if(strcmp(cvIndexStr, "FAILURE") == 0) {
		// FAILURE message received from the server
		////printf("Signal_Syscall : %s\n", receiveMessageCopy);
		rv = -1;
	}else{
		CVIndex = atoi(cvIndexStr);
		////printf("Signal_Syscall : CV index received from server is %d\n", CVIndex);
		////printf("Signal_Syscall : Msg fron Server : %s\n", receiveMessageCopy);
		rv = CVIndex;
	}
	return rv;	

#endif // NETWORK	
//----------------------------------------------------------------------------------------
//									END	NETWORKING
//----------------------------------------------------------------------------------------	
	
	if(kernelLockListLock == NULL){
		////printf("%s","Signal_Syscall:Kernel Lock List Lock is NULL\n");
		return -1;
    }	
	kernelLockListLock->Acquire();
	
	if((lockIndex < 0) || lockIndex >= nextLockIndex){
		////printf("%s","Signal_Syscall:Invalid Lock index\n");
		kernelLockListLock->Release();
		return -1;
    }	
	
	if(lockList[lockIndex].lock == NULL){
		////printf("%s","Signal_Syscall:Lock does not exist\n");
		kernelLockListLock->Release();
		return -1;
    }

	if(lockList[lockIndex].lockAddrSpace != currentThread->space){
		////printf("%s","Signal_Syscall:Lock does not belong to the current user prog\n");
		kernelLockListLock->Release();
		return -1;
    }	
	
	kernelLockListLock->Release();
	
	if(kernelCVListLock == NULL){
		////printf("%s","Signal_Syscall:Kernel CV List Lock is NULL\n");
		return -1;
    }	
	kernelCVListLock->Acquire();
	
	if((CVIndex < 0) || CVIndex >= nextCVIndex){
		////printf("%s","Signal_Syscall:Invalid CV index\n");
		kernelCVListLock->Release();
		return -1;
    }	
	
	if(CVList[CVIndex].condition == NULL){
		////printf("%s","Signal_Syscall:CV does not exist\n");
		kernelCVListLock->Release();
		return -1;
    }

	if(CVList[CVIndex].conditionAddrSpace != currentThread->space){
		////printf("%s","Signal_Syscall:CV does not belong to the current user prog\n");
		kernelCVListLock->Release();
		return -1;
    }	
	
	
	// Reacquire the Lock List Lock after validating CVIndex
	kernelLockListLock->Acquire();
	
	if(CVList[CVIndex].usageCounter == 0){
/* @	 */
//		////printf("%s","Signal_Syscall:No waiters to signal\n");
		CVList[CVIndex].toBeDestroyed = true;
		kernelCVListLock->Release();
		kernelLockListLock->Release();
		return 1;
    }	
	
	CVList[CVIndex].usageCounter--;
	CVList[CVIndex].condition->Signal(lockList[lockIndex].lock);
	
	if(CVList[CVIndex].usageCounter == 0){
		CVList[CVIndex].toBeDestroyed = true;
		kernelCVListLock->Release();
		kernelLockListLock->Release();
		return 1;
    }
	
	kernelCVListLock->Release();
	kernelLockListLock->Release();
	
	return 1;
}

//**************************************************************************************
//								BROADCAST CV SYSCALL
//**************************************************************************************
int Broadcast_Syscall(unsigned int CVIndex, unsigned int lockIndex) {

//----------------------------------------------------------------------------------------
//									START	NETWORKING
//----------------------------------------------------------------------------------------	
#ifdef NETWORK
	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
	char *cvIndexStr = new char[30];
	char sendMessage[MaxMailSize]; 
	char receiveMessage[MaxMailSize]; 
	char receiveMessageCopy[MaxMailSize]; 
	bool success = 0;
	int rv = -1;	// index value of the cv
		
	// construct packet, mail header for message
    // To: Server Nachos machine, mailbox 0
    // From: Client machine, reply to: mailbox 1
    outPktHdr.to = clientMachineID;		
    outMailHdr.to = currentThread->mailBox+1;
    outMailHdr.from = currentThread->mailBox;
	outPktHdr.from = clientMachineID;	
	// Check for the maximum number of Ports on the machine are under use
	if(currentThread->mailBox > 60){
		////printf("Broadcast_Syscall : Requesting invalid port number %d on machine %d\n", currentThread->mailBox, clientMachineID);
		return -1;
	}
	
	sprintf(sendMessage, "%d %d %d",broadcastCVType, CVIndex, lockIndex);
	
	outMailHdr.length = strlen(sendMessage) + 1;	
	// Send the message to the server nachos machine with machine ID '0'
	success = postOffice->Send(outPktHdr, outMailHdr, sendMessage); 
	if ( !success ) {
		////printf("Broadcast_Syscall : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
		interrupt->Halt();
	}
	////printf("Broadcast_Syscall : Client sending the Broadcast request to the server\n");	

	// Wait for the reply message from the server nachos machine
	postOffice->Receive(currentThread->mailBox, &inPktHdr, &inMailHdr, receiveMessage);
	////printf("Broadcast_Syscall : Server replied after Broadcast request\n");
	fflush(stdout);
	strcpy(receiveMessageCopy, receiveMessage);
	cvIndexStr = strtok(receiveMessage, " ");
	if(strcmp(cvIndexStr, "FAILURE") == 0) {
		// FAILURE message received from the server
		////printf("Broadcast_Syscall : %s\n", receiveMessageCopy);
		rv = -1;
	}else{
		CVIndex = atoi(cvIndexStr);
		////printf("Broadcast_Syscall : Broadcast CV index received from server is %d\n", CVIndex);
		////printf("Broadcast_Syscall : Msg fron Server : %s\n", receiveMessageCopy);
		rv = CVIndex;
	}
	return rv;	

#endif // NETWORK	
//----------------------------------------------------------------------------------------
//									END	NETWORKING
//----------------------------------------------------------------------------------------	

	if(kernelLockListLock == NULL){
		////printf("%s","Broadcast_Syscall:Kernel Lock List Lock is NULL\n");
		return -1;
    }	
	kernelLockListLock->Acquire();
	
	if((lockIndex < 0) || lockIndex >= nextLockIndex){
		////printf("%s","Broadcast_Syscall:Invalid Lock index\n");
		kernelLockListLock->Release();
		return -1;
    }	
	
	if(lockList[lockIndex].lock == NULL){
		////printf("%s","Broadcast_Syscall:Lock does not exist\n");
		kernelLockListLock->Release();
		return -1;
    }

	if(lockList[lockIndex].lockAddrSpace != currentThread->space){
		////printf("%s","Broadcast_Syscall:Lock does not belong to the current user prog\n");
		kernelLockListLock->Release();
		return -1;
    }	
	
	kernelLockListLock->Release();
	
	if(kernelCVListLock == NULL){
		////printf("%s","Broadcast_Syscall:Kernel CV List Lock is NULL\n");
		return -1;
    }	
	kernelCVListLock->Acquire();
	
	if((CVIndex < 0) || CVIndex >= nextCVIndex){
		////printf("%s","Broadcast_Syscall:Invalid CV index\n");
		kernelCVListLock->Release();
		return -1;
    }	
	
	if(CVList[CVIndex].condition == NULL){

	////printf("%s","Broadcast_Syscall:CV does not exist\n");
		kernelCVListLock->Release();
		return -1;
    }

	if(CVList[CVIndex].conditionAddrSpace != currentThread->space){
		////printf("%s","Broadcast_Syscall:CV does not belong to the current user prog\n");
		kernelCVListLock->Release();
		return -1;
    }	
	
	// Reacquire the Lock List Lock after validating CVIndex
	kernelLockListLock->Acquire();
	
	if(CVList[CVIndex].usageCounter == 0){
		////printf("%s","Broadcast_Syscall:No waiters to Broadcast\n");
		CVList[CVIndex].toBeDestroyed = true;
		kernelCVListLock->Release();
		kernelLockListLock->Release();
		return 1;
    }	
	
	CVList[CVIndex].usageCounter = 0;
	CVList[CVIndex].condition->Broadcast(lockList[lockIndex].lock);
	
	kernelCVListLock->Release();
	kernelLockListLock->Release();
	
	return 1;
}

//**************************************************************************************
//								DESTROY CV SYSCALL
//**************************************************************************************
int DestroyCV_Syscall(unsigned int CVIndex) {

//----------------------------------------------------------------------------------------
//									START	NETWORKING
//----------------------------------------------------------------------------------------	
#ifdef NETWORK
	
	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
	char *CVIndexStr = new char[30];
	char sendMessage[MaxMailSize]; 
	char receiveMessage[MaxMailSize]; 
	char receiveMessageCopy[MaxMailSize]; 
	bool success = 0;
	int rv = -1;	
	
	// construct packet, mail header for message
    // To: Server Nachos machine, mailbox 0
    // From: Client machine, reply to: mailbox 1
    outPktHdr.to = clientMachineID;		
    outMailHdr.to = currentThread->mailBox+1;
    outMailHdr.from = currentThread->mailBox;
	outPktHdr.from = clientMachineID;
	// Check for the maximum number of Ports on the machine are under use
	if(currentThread->mailBox > 60){
		////printf("DestroyCV_Syscall : Requesting invalid port number %d on machine %d\n", currentThread->mailBox, clientMachineID);
		return -1;
	}
	
	sprintf(sendMessage, "%d %d",destroyCVType, CVIndex);
	outMailHdr.length = strlen(sendMessage) + 1;	
	
	// Send the message to the server nachos machine with machine ID '0'
	success = postOffice->Send(outPktHdr, outMailHdr, sendMessage); 
	if ( !success ) {
		////printf("DestroyCV_Syscall : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
		interrupt->Halt();
	}
	////printf("DestroyCV_Syscall : Client sending the Destroy CV %d request to the server\n", CVIndex);
	
	// Wait for the reply message from the server nachos machine
	postOffice->Receive(currentThread->mailBox, &inPktHdr, &inMailHdr, receiveMessage);
	////printf("DestroyCV_Syscall : Server replied after destroy CV request\n");
	fflush(stdout);
	
	strcpy(receiveMessageCopy, receiveMessage);
	CVIndexStr = strtok(receiveMessage, " ");
	if(strcmp(CVIndexStr, "FAILURE") == 0){
		// FAILURE message received from the server
		////printf("DestroyCV_Syscall : %s\n", receiveMessageCopy);
		rv = -1;	
	}else{
		CVIndex = atoi(CVIndexStr); 
		////printf("DestroyCV_Syscall : Destroyed CV Index received from server is %d\n", CVIndex);
		////printf("DestroyCV_Syscall : Msg fron Server : %s\n", receiveMessageCopy);
		rv = CVIndex;
	}	
	return rv;
#endif // NETWORK
//----------------------------------------------------------------------------------------
//									END	NETWORKING
//----------------------------------------------------------------------------------------		
	if(kernelCVListLock == NULL){
		////printf("%s","DestroyCV_Syscall:Kernel CV List Lock is NULL\n");
		return -1;
    }	
	kernelCVListLock->Acquire();
	
	if((CVIndex < 0) || CVIndex >= nextCVIndex){
		////printf("%s","DestroyCV_Syscall:Invalid CV index\n");
		kernelCVListLock->Release();
		return -1;
    }	
	
	if(CVList[CVIndex].condition == NULL){
		////printf("%s","DestroyCV_Syscall:CV does not exist\n");
		kernelCVListLock->Release();
		return -1;
    }

	if(CVList[CVIndex].conditionAddrSpace != currentThread->space){
		////printf("%s","DestroyCV_Syscall:CV does not belong to the current user prog\n");
		kernelCVListLock->Release();
		return -1;
    }	

	if(CVList[CVIndex].toBeDestroyed == false){
		////printf("%s","DestroyCV_Syscall:CV under use!\n");
		kernelCVListLock->Release();
		return -1;
	}	
	
	delete CVList[CVIndex].condition;
	CVList[CVIndex].condition = NULL;
	
	return 1;
}

//**************************************************************************************
//								YIELD SYSCALL
//**************************************************************************************
void Yield_Syscall() {

	currentThread->Yield();
	return;
	
}


void KernelFunc(unsigned int vaddr) {
// Set up all registers and switch to user mode to run the userprog
	int myPid = 0;
	
	machine->WriteRegister(PCReg, vaddr);	
	machine->WriteRegister(NextPCReg, (vaddr + 4));
	
	processTableLock->Acquire();	
	// loop through the entire process table to find the Pid of the current process
	for(int i = 0; i < processCount; i++) {
		if(processTable[i].addrSpace == currentThread->space){
			myPid = processTable[i].Pid;
			break;
		}	
	}	
	processTableLock->Release();
	
	machine->WriteRegister(StackReg, processTable[currentThread->pid].stackTops[currentThread->threadId]);
	currentThread->space->RestoreState();
	machine->Run();	
}

//**************************************************************************************
//								FORK SYSCALL
//**************************************************************************************
void Fork_Syscall(unsigned int vaddr) {
	
	// Parent process process id of the Thread to be created
	int myPid = -1;
	int oldNumPages = 0, newNumPages = 0;
	int freePageIndex = 0;
	AddrSpace *newSpace = NULL;
	pageTableWithOffset *ptoffset =  NULL;
	
	// $
//	////printf("thread id in fork = %d\n", currentThread->threadId);
	if((vaddr < 0) || (vaddr > currentThread->space->spaceSize)){
		////printf("%s", "Fork_Syscall:Invalid virtual address\n");
		return;
	}
	
	processTableLock->Acquire();	
	
	myPid = currentThread->pid;

	processTableLock->Release();
	
	if(pageTableLock[myPid] == NULL){
		////printf("%s", "Fork_Syscall:PageTable Lock is NULL\n");
		return;
	}

	pageTableLock[myPid]->Acquire();

	// Fetch the current size of the address space
	oldNumPages = currentThread->space->GetNumPages();
	newNumPages = oldNumPages + 8;
	
	// update the new number of pages into the address space of the current thread
	currentThread->space->SetNumPages(newNumPages);
	pageTableWithOffset *newptoffset = new pageTableWithOffset[newNumPages];
	
	ptoffset = currentThread->space->GetPageTableRef();
	
	for(int index = 0; index < oldNumPages; index++){
		newptoffset[index].whereisthepage = ptoffset[index].whereisthepage;
		newptoffset[index].offset = ptoffset[index].offset;
	}
	
	for(int index = oldNumPages; index < newNumPages; index++){
	
		newptoffset[index].whereisthepage = 2;
	
	}
	
	currentThread->space->DeletePageTable();
	
	// Make the new page table official 
	currentThread->space->SetPageTableRef(newptoffset);
	
	pageTableLock[myPid]->Release();
	
	Thread *t = new Thread("childThread");
	
	processTableLock->Acquire();
	
	processTable[myPid].threadCount++;
	
	processTable[myPid].activeThreadCount++;
	// $
//	////printf("incrementing activethreadcount to = %d\n", processTable[myPid].activeThreadCount);
	t->threadId = processTable[myPid].threadCount;
	t->pid = myPid;

	t->space = currentThread->space;
	
	processTable[myPid].stackTops[t->threadId] = ((newNumPages * PageSize) - 16);
	// $
//	////printf("child thread id in fork = %d and threadcount is %d\n", t->threadId, processTable[myPid].threadCount);
	processTableLock->Release();
	
	t->Fork((VoidFunctionPtr)KernelFunc, vaddr);
	
}


//**************************************************************************************
//								EXIT SYSCALL
//**************************************************************************************

void Exit_Syscall(int status) {
	
	currentThread->Finish();
	
}


//**************************************************************************************
//								EXEC SYSCALL
//**************************************************************************************
void ExecFunc()
{
	int stackStart = 0;
	
    currentThread->space->InitRegisters();		// set the initial register values
	
	stackStart = ((currentThread->space->GetNumPages()) * PageSize) - 16;
    machine->WriteRegister(StackReg, stackStart);
	
	currentThread->space->RestoreState();		// load page table register

    machine->Run();			// jump to the user progam
	
    ASSERT(FALSE);			// machine->Run never returns;
							// the address space exits
							// by doing the syscall "exit"
}					

//**************************************************************************************
//								EXEC SYSCALL
//**************************************************************************************
SpaceId	Exec_Syscall(unsigned int vaddr, unsigned int len) {

	Thread *t, *netThread;
	AddrSpace *space;
	int stackStart = 0;
	char *buf = new char[len+1];	// Kernel buffer to put the name in
	OpenFile *fd = NULL;					// The new open file
   	int id;							// The openfile id
	
	// $
//	////printf("thread id in exec = %d\n", currentThread->threadId);
	if((vaddr+len) <= vaddr){
		////printf("%s", "Exec_Syscall : Invalid Virtual address length\n");
		delete[] buf;
		return -1;
	}	
	
	if((vaddr < 0) || (vaddr > currentThread->space->spaceSize)){
		////printf("%s", "Exec_Syscall : Invalid virtual address\n");
		delete[] buf;
		return -1;
	}	
	
	if((vaddr+len) > currentThread->space->spaceSize){
		////printf("%s", "Exec_Syscall : Trying to access Invalid Virtual address\n");
		delete[] buf;
		return -1;
	}	
	
    if (!buf) {
		////printf("%s","Exec_Syscall : Can't allocate kernel buffer in Open\n");
		return -1;
    }
	
	if(copyin(vaddr,len,buf) == -1 ) {
		////printf("%s","Exec_Syscall : Bad pointer passed to Exec\n");
		delete[] buf;
		return -1;
    }
	
	buf[len]='\0';
	fd = fileSystem->Open(buf);	
		
	if (fd == NULL)
	{
	
		////printf("Exec_Syscall : Unable to open file %s\n", buf);
		return -1;
   	}
  
	// Create a new Address space for this new Process that will be created
  	space = new AddrSpace(fd);
	////printf("created addrspace\n");
	// Create a new thread (represents a process)
	t = new Thread(buf);
	netThread = new Thread("NetworkingThread");
	// Give the created address space to the new thread which is created
	t->space = space;
	
	// Update the process table with the new process details
	processTableLock->Acquire();
	
	// This value is never decremented
	processCount++;
	
	processTable[processCount].Pid = processCount;
	t->pid = processTable[processCount].Pid;
	//processTable[processCount].executable = fd;

	processTable[processCount].threadCount = 0;
	processTable[processCount].addrSpace = space;
	stackStart = ((t->space->GetNumPages()) * PageSize) - 16;

	// Every Process will have its own stack start location at the 
	// beginning of the stack top array in processTable	
	processTable[processCount].stackTops[0] = stackStart;
	processTable[processCount].activeThreadCount = 0;
	processTableLock->Release();
	
	activeProcessesLock->Acquire();
	// This value tells the current number of active processes in the system
	noOfActiveProcesses++;
	// $
//	////printf("incrementing noOfActiveProcesses to = %d\n", noOfActiveProcesses);
	activeProcessesLock->Release();
	t->threadId = 0;	
	
	// find the threadId for the networking thread
	processTableLock->Acquire();
	processTable[processCount].threadCount++;
	processTable[processCount].activeThreadCount++;
	netThread->threadId = processTable[processCount].threadCount;
	processTableLock->Release();
	
	// Find the mailbox number for this userprogram thread
	mailBoxCountLock->Acquire();
	mailBoxCount++;
	t->mailBox = mailBoxCount;
	// $
	////printf("userprog mail box = %d\n", t->mailBox);
	// Find the mailbox number its associated networking thread
	mailBoxCount++;
	netThread->mailBox = mailBoxCount;
	// $
	////printf("net thread mail box = %d\n", netThread->mailBox);
	netThread->Fork((VoidFunctionPtr)NetworkingThread, NULL);
	t->Fork((VoidFunctionPtr)ExecFunc, NULL);
	// $
	////printf("net thread mail box = %d\n", netThread->mailBox);
	mailBoxCountLock->Release();
	
	// Return the spaceId 
	return processTable[processCount].Pid;
}	

//**************************************************************************************
//								PRINT STATEMENT SYSCALL
//**************************************************************************************
void Print_Stmt_Syscall(unsigned int vaddr)
{
   int len = 150, i;
   char *buffer = new char[len];  // Kernel buffer to put the name in

   if (!buffer) return;
       //buffer exists
	
	for(i = 0; i < len; i++){	
		if( copyin(vaddr,1,&buffer[i]) == -1 ) {
			////printf("%s","Print_Stmt_Syscall : Bad pointer passed to Print\n");
				return;
		}else{		
			if(buffer[i] == '$')
				break;
			vaddr++;	
		}	
	}
		
    //statement with no access specifiers stored into buffer
   buffer[i]='\0';
   
printf(buffer);
}

//**************************************************************************************
//								PRINT STATEMENT WITH ONE ARGUMENT SYSCALL
//**************************************************************************************
void Print_1Arg_Syscall(unsigned int vaddr, int val)
{
   int len = 90, i = 0;
   char *buffer = new char[len]; 
   
   if (!buffer) return;
       //buffer exists
	   
   for(i = 0; i < len; i++){	
		if( copyin(vaddr,1, &buffer[i]) == -1 ) {
			////printf("%s","Print_Stmt_Syscall : Bad pointer passed to Print\n");
				return;
		}else{		
			if(buffer[i] == '$')
				break;
			vaddr++;	
		}	
	}
   // statement with 1 access specifiers stored into buffer
   buffer[i]='\0';
	
printf(buffer, val);
}


void Print_Str_Syscall(unsigned int vaddr1, unsigned int vaddr2) {

	int len = 90, i = 0;
	char *buffer1 = new char[len];
	char *buffer2 = new char[len];
	
	if (!buffer1) return;
	if (!buffer2) return;
      
	for(i = 0; i < len; i++){	
		if( copyin(vaddr1,1, &buffer1[i]) == -1 ) {
			////printf("%s","Print_Stmt_Syscall : Bad pointer passed to Print\n");
				return;
		}else{		
			if(buffer1[i] == '$')
				break;
			vaddr1++;	
		}	
	}
    buffer1[i]='\0';
	for(i = 0; i < len; i++){	
		if( copyin(vaddr2,1, &buffer2[i]) == -1 ) {
			////printf("%s","Print_Stmt_Syscall : Bad pointer passed to Print\n");
				return;
		}else{		
			if(buffer2[i] == '$')
				break;
			vaddr2++;	
		}	
	}
	buffer2[len]='\0';
	
printf(buffer1, buffer2);
}

//**************************************************************************************
//								PRINT STATEMENT WITH TWO ARGUMENT SYSCALL
//**************************************************************************************
void Print_2Arg_Syscall(unsigned int vaddr, int val1, int val2)
{
   int len=90, i = 0;
   char *buffer = new char[len];

   if (!buffer) return;
       //buffer exists
	
	for(i = 0; i < len; i++){	
		if( copyin(vaddr,1, &buffer[i]) == -1 ) {
			////printf("%s","Print_Stmt_Syscall : Bad pointer passed to Print\n");
				return;
		}else{		
			if(buffer[i] == '$')
				break;
			vaddr++;	
		}	
	}
   // statement with 2 access specifiers stored into buffer
   buffer[i]='\0';

printf(buffer,val1,val2);
}

//**************************************************************************************
//								PRINT STATEMENT WITH THREE ARGUMENT SYSCALL
//**************************************************************************************
void Print_3Arg_Syscall(unsigned int vaddr, int val1, int val2, int val3)
{
   int len=200, i = 0;
   char *buffer = new char[len];

   if (!buffer) return;
   for(i = 0; i < len; i++){	
		if( copyin(vaddr,1, &buffer[i]) == -1 ) {
			////printf("%s","Print_Stmt_Syscall : Bad pointer passed to Print\n");
				return;
		}else{		
			if(buffer[i] == '$')
				break;
			vaddr++;	
		}	
	}
   // statement with 2 access specifiers stored into buffer
   buffer[i]='\0';

printf(buffer,val1,val2,val3);
}


// @ Not used - can be removed
//**************************************************************************************
//								SPRINTF SYSCALL
//**************************************************************************************
void Sprint_Syscall(unsigned int vaddr1, unsigned int vaddr2, unsigned int vaddr3, int id)
{
	int len=50;
	char *buffer1 = new char[len];
	char *buffer2 = new char[len];
	char *buffer3 = new char[len];
	
	if (!buffer1) return;
	if (!buffer2) return;
	if (!buffer3) return;
	
	if( copyin(vaddr1,len,buffer1) == -1 ) {
      //////printf("%s","Sprint_Str+Int_Syscall : Bad pointer passed to Print\n");
       delete buffer1;
       return;
	}
	buffer1[len]='\0';
   
	if( copyin(vaddr2,len,buffer2) == -1 ) {
		////printf("%s","Sprint_Str+Int_Syscall : Bad pointer passed to Print\n");
		delete buffer2;
		return;
	}
	buffer2[len]='\0';
   
	if( copyin(vaddr3,len,buffer3) == -1 ) {
		////printf("%s","Sprint_Str+Int_Syscall : Bad pointer passed to Print\n");
		delete buffer3;
		return;
	}
	buffer3[len]='\0';
   
	sprintf(buffer1, buffer2, buffer3, id);

	////printf("concatenated string is %s\n", buffer1);	
	return;      
}


//**************************************************************************************
//								SCAN SYSCALL
//**************************************************************************************	
int Scan_Syscall(unsigned int vaddr)
{
   int len=2;
   int p;
   char *buffer = new char[len+1];        // Kernel buffer to put the name in
 
   if (!buffer) return -1;
       //buffer exists
   if( copyin(vaddr,len,buffer) == -1 ) {
      //////printf("%s","Bad pointer passed to Scan\n");
       delete buffer;
       return -1;
   }
   
   //access spacifier read into buffer  
   buffer[len]='\0';
   scanf(buffer,&p);

   return p;
} 


//**************************************************************************************
//								CREATE MV SYSCALL
//**************************************************************************************
int CreateMV_Syscall(unsigned int vaddr, int len, int initValue, int uniqueId) {

//----------------------------------------------------------------------------------------
//									START	NETWORKING
//----------------------------------------------------------------------------------------	
#ifdef NETWORK
	// Allocates a Monitor variable from the server to the user prog
	
	int rv = -1;	// index value of the MV created
	char *MVName = new char[len+5];
	char *buffer = new char[len+1];
	
	if((vaddr+len) <= vaddr){
		////printf("%s", "CreateMV_Syscall : Invalid Virtual address length\n");
		delete[] MVName;
		return -1;
	}	
	
	if(len > MAX_CHARLEN){
		////printf("%s", "CreateMV_Syscall : Exceeding Max MV Name length\n");
		delete[] MVName;
		return -1;
	}	
	
	if (!MVName) {
		////printf("%s","CreateMV_Syscall : Can't allocate kernel buffer for MV name\n");
		delete[] MVName;
		return -1;
    }
	
	if (!buffer) {
		////printf("%s","CreateMV_Syscall : Can't allocate kernel buffer for MV name\n");
		delete[] MVName;
		delete[] buffer;
		return -1;
    }
	
    if( copyin(vaddr,len,buffer) == -1 ) {
		////printf("%s", "CreateMV_Syscall : Could not copy from the virtual address\n");
		delete[] MVName;
		delete[] buffer;
		return -1;
    }
	
	// If any unique id is passed from user program then append it to the MV name before passing it to the networking thread
	if(uniqueId == 0) {
		sprintf(MVName, "%s%c", buffer, '\0');
	}else{
		sprintf(MVName, "%s%d%c", buffer, uniqueId, '\0');
	}	
	
	
	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
	char *MVIndexStr = new char[30];
	char sendMessage[MaxMailSize]; 
	char receiveMessage[MaxMailSize];
	char receiveMessageCopy[MaxMailSize];	
	int MVIndex = -1;
	bool success = 0;
	
	// construct packet, mail header for message
    // To: Server Nachos machine, mailbox 0
    // From: Client machine, reply to: mailbox 1
    outPktHdr.to = clientMachineID;				
    outMailHdr.to = currentThread->mailBox+1;
    outMailHdr.from = currentThread->mailBox;
	outPktHdr.from = clientMachineID;	
	
	// Check if maximum number of Ports on the machine are under use
	if(currentThread->mailBox > 60){
		////printf("CreateMV_Syscall : Requesting invalid port number %d on machine %d\n", currentThread->threadId, clientMachineID);
		return -1;
	}
	
	sprintf(sendMessage, "%d %s %d",createMVType, MVName, initValue);
	outMailHdr.length = strlen(sendMessage) + 1;	
	
	////printf("CreateMV_Syscall : Client sending the Create MV request to the server\n");
	// Send the message to the server nachos machine with machine ID '0'
	success = postOffice->Send(outPktHdr, outMailHdr, sendMessage); 

	if ( !success ) {
	 //////printf("CreateMV_Syscall : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
	  interrupt->Halt();
	}
	
	// Wait for the reply message from the server nachos machine
	////printf("CreateMV_Syscall : Client waiting to receive Create MV request reply\n");
	postOffice->Receive(currentThread->mailBox, &inPktHdr, &inMailHdr, receiveMessage);
	fflush(stdout);
	strcpy(receiveMessageCopy, receiveMessage);
	
	MVIndexStr = strtok(receiveMessage, " ");
	
	if(strcmp(MVIndexStr, "FAILURE") == 0){
		// FAILURE message received from the server
		////printf("CreateMV_Syscall : %s\n", receiveMessageCopy);
		rv = -1;	
	}else{
		MVIndex = atoi(MVIndexStr); 
		////printf("CreateMV_Syscall : MV Index received from server is %d\n", MVIndex);
		////printf("CreateMV_Syscall : Msg fron Server : %s\n", receiveMessageCopy);
		rv = MVIndex;
	}	
	return rv;
	
#endif // NETWORK
//----------------------------------------------------------------------------------------
//									END	NETWORKING
//----------------------------------------------------------------------------------------	
}


//**************************************************************************************
//								GET MV SYSCALL
//**************************************************************************************
int GetMV_Syscall(int MVIndex) {

//----------------------------------------------------------------------------------------
//									START	NETWORKING
//----------------------------------------------------------------------------------------	
#ifdef NETWORK
	// Fetches the value of a Monitor variable from the server to the user prog
	
	int rv = -1;	// value of the MV requested
	
	if (MVIndex < 0) {
		////printf("%s","GetMV_Syscall : Invalid MV Index received\n");
		return -1;
    }
	
	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
	char *MVIndexStr = new char[30];
	char sendMessage[MaxMailSize]; 
	char receiveMessage[MaxMailSize];
	char receiveMessageCopy[MaxMailSize];	
	bool success = 0;
	int MVValue = 0;
	
	// construct packet, mail header for message
    // To: Server Nachos machine, mailbox 0
    // From: Client machine, reply to: mailbox 1
    outPktHdr.to = clientMachineID;				
    outMailHdr.to = currentThread->mailBox+1;
    outMailHdr.from = currentThread->mailBox;
	outPktHdr.from = clientMachineID;	
	
	// Check if maximum number of Ports on the machine are under use
	if(currentThread->mailBox > 60){
		////printf("GetMV_Syscall : Requesting invalid port number %d on machine %d\n", currentThread->threadId, clientMachineID);
		return -1;
	}
	
	sprintf(sendMessage, "%d %d", getMVType, MVIndex);
	outMailHdr.length = strlen(sendMessage) + 1;	

	// Send the message to the server nachos machine with machine ID '0'
	////printf("GetMV_Syscall : Client sending the Get MV value request to the server\n");
	success = postOffice->Send(outPktHdr, outMailHdr, sendMessage); 

	if ( !success ) {
	 //////printf("GetMV_Syscall : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
	  interrupt->Halt();
	}
	
	// Wait for the reply message from the server nachos machine
	////printf("GetMV_Syscall : Client waiting to receive reply GetMV request\n");
	postOffice->Receive(currentThread->mailBox, &inPktHdr, &inMailHdr, receiveMessage);
	fflush(stdout);
	strcpy(receiveMessageCopy, receiveMessage);
	
	MVIndexStr = strtok(receiveMessage, " ");
	
	if(strcmp(MVIndexStr, "FAILURE") == 0){
		// FAILURE message received from the server
		////printf("GetMV_Syscall : %s\n", receiveMessageCopy);
		rv = -1;	
	}else{
		MVValue = atoi(MVIndexStr); 
		////printf("GetMV_Syscall : Value of the MV received from server is %d\n", MVValue);
		////printf("GetMV_Syscall : Msg fron Server : %s\n", receiveMessageCopy);
		rv = MVValue;
	}	
	return rv;
	
#endif // NETWORK
//----------------------------------------------------------------------------------------
//									END	NETWORKING
//----------------------------------------------------------------------------------------	
}

//**************************************************************************************
//								SET MV SYSCALL
//**************************************************************************************
int SetMV_Syscall(int MVIndex, int MVValue) {

//----------------------------------------------------------------------------------------
//									START	NETWORKING
//----------------------------------------------------------------------------------------	
#ifdef NETWORK
	// Sets the value of a Monitor variable in the server for the user prog
	
	int rv = -1;	// index value of the MV whose value is set by the server upon user request
	
	if (MVIndex < 0) {
		////printf("%s","SetMV_Syscall : Invalid MV Index received\n");
		return -1;
    }
	
	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
	char *MVIndexStr = new char[30];
	char sendMessage[MaxMailSize]; 
	char receiveMessage[MaxMailSize];
	char receiveMessageCopy[MaxMailSize];	
	bool success = 0;
	int retMVIndex = 0;
	
	// construct packet, mail header for message
    // To: Server Nachos machine, mailbox 0
    // From: Client machine, reply to: mailbox 1
    outPktHdr.to = clientMachineID;				
    outMailHdr.to = currentThread->mailBox+1;
    outMailHdr.from = currentThread->mailBox;
	outPktHdr.from = clientMachineID;	
	
	// Check if maximum number of Ports on the machine are under use
	if(currentThread->mailBox > 60){
		////printf("SetMV_Syscall : Requesting invalid port number %d on machine %d\n", currentThread->threadId, clientMachineID);
		return -1;
	}
	
	sprintf(sendMessage, "%d %d %d", setMVType, MVIndex, MVValue);
	outMailHdr.length = strlen(sendMessage) + 1;	

	// Send the message to the server nachos machine with machine ID '0'
	////printf("SetMV_Syscall : Client sending the Set MV value request to the server\n");
	success = postOffice->Send(outPktHdr, outMailHdr, sendMessage); 

	if ( !success ) {
	 //////printf("SetMV_Syscall : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
	  interrupt->Halt();
	}
	
	// Wait for the reply message from the server nachos machine
	////printf("SetMV_Syscall : Client waiting to receive reply for SetMV request\n");
	postOffice->Receive(currentThread->mailBox, &inPktHdr, &inMailHdr, receiveMessage);
	fflush(stdout);
	strcpy(receiveMessageCopy, receiveMessage);
	
	MVIndexStr = strtok(receiveMessage, " ");
	
	if(strcmp(MVIndexStr, "FAILURE") == 0){
		// FAILURE message received from the server
		////printf("SetMV_Syscall : %s\n", receiveMessageCopy);
		rv = -1;	
	}else{
		retMVIndex = atoi(MVIndexStr); 
		////printf("SetMV_Syscall : Value of the MV received from server is %d\n", retMVIndex);
		////printf("SetMV_Syscall : Msg fron Server : %s\n", receiveMessageCopy);
		rv = retMVIndex;
	}	
	return rv;
	
#endif // NETWORK
//----------------------------------------------------------------------------------------
//									END	NETWORKING
//----------------------------------------------------------------------------------------	
}


//**************************************************************************************
//								DESTROY MV SYSCALL
//**************************************************************************************
int DestroyMV_Syscall(int MVIndex) {

//----------------------------------------------------------------------------------------
//									START	NETWORKING
//----------------------------------------------------------------------------------------	
#ifdef NETWORK
	
	int rv = -1;	// index value of the MV whose value is set by the server upon user request
	
	if (MVIndex < 0) {
		////printf("%s","DestroyMV_Syscall : Invalid MV Index received\n");
		return -1;
    }
	
	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
	char *MVIndexStr = new char[30];
	char sendMessage[MaxMailSize]; 
	char receiveMessage[MaxMailSize];
	char receiveMessageCopy[MaxMailSize];	
	bool success = 0;
	int retMVIndex = 0;
	
	// construct packet, mail header for message
    // To: Server Nachos machine, mailbox 0
    // From: Client machine, reply to: mailbox 1
    outPktHdr.to = clientMachineID;				
    outMailHdr.to = currentThread->mailBox+1;
    outMailHdr.from = currentThread->mailBox;
	outPktHdr.from = clientMachineID;	
	
	// Check if maximum number of Ports on the machine are under use
	if(currentThread->mailBox > 60){
		////printf("DestroyMV_Syscall : Requesting invalid port number %d on machine %d\n", currentThread->threadId, clientMachineID);
		return -1;
	}
	
	sprintf(sendMessage, "%d %d", destroyMVType, MVIndex);
	outMailHdr.length = strlen(sendMessage) + 1;	

	// Send the message to the server nachos machine with machine ID '0'
	////printf("DestroyMV_Syscall : Client sending the Destroy MV request to the server\n");
	success = postOffice->Send(outPktHdr, outMailHdr, sendMessage); 

	if ( !success ) {
	 //////printf("DestroyMV_Syscall : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
	  interrupt->Halt();
	}
	
	// Wait for the reply message from the server nachos machine
	//printf("DestroyMV_Syscall : Client waiting for the DestroyMV reply\n");
	postOffice->Receive(currentThread->mailBox, &inPktHdr, &inMailHdr, receiveMessage);
	fflush(stdout);
	strcpy(receiveMessageCopy, receiveMessage);
	
	MVIndexStr = strtok(receiveMessage, " ");
	
	if(strcmp(MVIndexStr, "FAILURE") == 0){
		// FAILURE message received from the server
		//printf("DestroyMV_Syscall : %s\n", receiveMessageCopy);
		rv = -1;	
	}else{
		retMVIndex = atoi(MVIndexStr); 
		//printf("DestroyMV_Syscall : Value of the MV received from server is %d\n", retMVIndex);
		//printf("DestroyMV_Syscall : Msg fron Server : %s\n", receiveMessageCopy);
		rv = retMVIndex;
	}	
	return rv;
	
#endif // NETWORK
//----------------------------------------------------------------------------------------
//									END	NETWORKING
//----------------------------------------------------------------------------------------	
}


//**************************************************************************************
//								START SIMULATION SYSCALL
//**************************************************************************************
void StartSimulation_Syscall() {

//----------------------------------------------------------------------------------------
//									START	NETWORKING
//----------------------------------------------------------------------------------------	
#ifdef NETWORK
	
	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
	char sendMessage[MaxMailSize]; 
	char receiveMessage[MaxMailSize];
	char receiveMessageCopy[MaxMailSize];	
	char *netThreadRetVal = new char[MaxMailSize];
	bool success = 0;
	
	// construct packet, mail header for message
    // To: Associated networking thread machine, mailbox 0
    // From: Client machine, reply to: mailbox 1
    outPktHdr.to = clientMachineID;		
    outMailHdr.to = currentThread->mailBox + 1;
    outMailHdr.from = currentThread->mailBox;
	outPktHdr.from = clientMachineID;	
	
	// $
	//printf("StartSimulation_Syscall\n");
	// Check if maximum number of Ports on the machine are under use
	if(currentThread->mailBox > 60){
		//printf("StartSimulation_Syscall : Requesting invalid port number %d on machine %d\n", currentThread->threadId, clientMachineID);
		return;
	}
	
	strcpy(sendMessage, "Are you ready?");
	outMailHdr.length = strlen(sendMessage) + 1;	
	
	// $
	//printf("StartSimulation_Syscall : sending Are you ready? message to net thread\n");
	
	// Send the message to the associated networking thread on same machine with mailbox one more than the user program
	success = postOffice->Send(outPktHdr, outMailHdr, sendMessage); 

	if ( !success ) {
	 ////printf("StartSimulation_Syscall : The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
	  interrupt->Halt();
	}
	//printf("StartSimulation_Syscall : Client sending the StartSimulation request to the networking thread\n");
	
	// Wait for the reply message from the networking thread
	postOffice->Receive(currentThread->mailBox, &inPktHdr, &inMailHdr, receiveMessage);
	// $
	//printf("StartSimulation_Syscall : received reply message from net thread\n");
	fflush(stdout);
	strcpy(receiveMessageCopy, receiveMessage);
	netThreadRetVal = strtok(receiveMessage, " ");
	if(strcmp(netThreadRetVal, "FAILURE") == 0){
		// FAILURE message received from the server
		//printf("StartSimulation_Syscall : %s\n", receiveMessageCopy);
	
	}else{
		
		//printf("StartSimulation_Syscall : Msg from networking thread : %s\n", receiveMessageCopy);
		
	}	

#endif // NETWORK
//----------------------------------------------------------------------------------------
//									END	NETWORKING
//----------------------------------------------------------------------------------------	
}


//***************************************************************************************
//***************************************************************************************
//***************************************************************************************
//						PART 1 AND 2 CHANGES - PROJECT 3
//***************************************************************************************
//***************************************************************************************
//***************************************************************************************

//***************************************************************************************
//						EVICT A PAGE FROM PHYSICAL MEMORY
//***************************************************************************************
int evictAPage(int pageToEvict)
{
	AddrSpace *evictAddrSpace;
	int index = 0, vpn;
	int stackUnintialized = 0, numBytes = 128;
	int codeInitialize = 0;
	char *buf = new char[128];
	
		// reference IPT to check if dirty bit is it

		if(ipt[pageToEvict].processID != -1)
		{ 
			evictAddrSpace = processTable[ipt[pageToEvict].processID].addrSpace;
		}
		else
		{
			evictAddrSpace = mainProcess;
		}
		IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
		for(index = 0; index <4; index++)
		{
			if(machine->tlb[index].dirty !=  ipt[ machine->tlb[currentTLB].physicalPage].dirty)
			{
				ipt[machine->tlb[index].physicalPage].dirty = true;
			}	
		}
		
		for(index = 0; index <4; index++)
		{
		
			if(machine->tlb[index].physicalPage == pageToEvict)
			machine->tlb[index].valid = false;
		}
		
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts

		ipt[pageToEvict].valid = FALSE;
		
		if(ipt[pageToEvict].dirty)	
		{	
			// if evicting page is dirty
			// read from main memory
			// do I need a lock ?? 
			  
			swapFile->WriteAt(&(machine->mainMemory[(pageToEvict * PageSize)]), numBytes, position);
			vpn = ipt[pageToEvict].virtualPage;
			evictAddrSpace->updatePtOffset(vpn);
			position = position + numBytes;
		}	
		return pageToEvict;
}		


//***************************************************************************************
//					FIND THE INDEX OF THE PHYSICAL PAGE OF VPN FROM IPT
//***************************************************************************************
// This function finds the physical page index which has the required virtual page number 
int findIptIndex(int vpn)
{
	int index = 0;
	int PhysicalPage = -1;
	
	for(index = 0; index < NumPhysPages; index++)
	{ 	
		if(ipt[index].virtualPage == vpn && ipt[index].valid == true && ipt[index].processID == currentThread->pid)
		{ 
			PhysicalPage = ipt[index].physicalPage;
			return  PhysicalPage;
		}					
		}	
	return -1;	
}

//***************************************************************************************
//				UPDATE THE TLD AFTER FINDING A PHYSICAL PAGE FOR THE VPN PAGE
//***************************************************************************************
// updates the TLB from IPT
 
void updateTLB(int ppn)
{
		
		int currentPPN;
		
		currentPPN = machine->tlb[currentTLB].physicalPage;
		if(machine->tlb[currentTLB].dirty !=  ipt[currentPPN].dirty)
		{
			ipt[currentPPN].dirty = true;
			machine->tlb[currentTLB].valid = false;
		}	
		
		machine->tlb[currentTLB].virtualPage =  ipt[ppn].virtualPage;
		machine->tlb[currentTLB].physicalPage =  ppn;
		machine->tlb[currentTLB].valid =  ipt[ppn].valid;
		machine->tlb[currentTLB].readOnly =  ipt[ppn].readOnly;
		machine->tlb[currentTLB].use =  ipt[ppn].use;
		machine->tlb[currentTLB].dirty =  ipt[ppn].dirty;
		currentTLB = (currentTLB + 1) % TLBSize;	
}

//***************************************************************************************
//***************************************************************************************
//***************************************************************************************


void ExceptionHandler(ExceptionType which) {
    int type = machine->ReadRegister(2); // Which syscall?
    int rv=0; 	// the return value from a syscall
	
	// local variables for storing entries 
	int localVirtualPage = 0;
	int localPhysicalPage = 0;
	bool localvalid =  false;
	bool localreadOnly = false;
	bool localuse = false;
	bool localdirty = false;
	TranslationEntry* myPageTable;
	int index = 0;
	
	
    if ( which == SyscallException ) {
	switch (type) {
	    default:
		DEBUG('a', "Unknown syscall - shutting down.\n");
		
	    case SC_Halt:
		DEBUG('a', "Shutdown, initiated by user program.\n");
		interrupt->Halt();
		break;
		
	    case SC_Create:
		DEBUG('a', "Create syscall.\n");
		Create_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
		
	    case SC_Open:
		DEBUG('a', "Open syscall.\n");
		rv = Open_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
		
	    case SC_Write:
		DEBUG('a', "Write syscall.\n");
		Write_Syscall(machine->ReadRegister(4),
			      machine->ReadRegister(5),
			      machine->ReadRegister(6));
		break;
		
	    case SC_Read:
		DEBUG('a', "Read syscall.\n");
		rv = Read_Syscall(machine->ReadRegister(4),
			      machine->ReadRegister(5),
			      machine->ReadRegister(6));
		break;
		
	    case SC_Close:
		DEBUG('a', "Close syscall.\n");
		Close_Syscall(machine->ReadRegister(4));
		break;
		
		case SC_CreateLock:
		DEBUG('a', "CreateLock syscall.\n");
		rv = CreateLock_Syscall(machine->ReadRegister(4),
			      machine->ReadRegister(5),
				  machine->ReadRegister(6));
		break;	
		
		case SC_Acquire:
		DEBUG('a', "Acquire syscall.\n");
		rv = Acquire_Syscall(machine->ReadRegister(4));
		break;
		
		case SC_Release:
		DEBUG('a', "Release syscall.\n");
		rv = Release_Syscall(machine->ReadRegister(4));
		break;
		
		case SC_DestroyLock:
		DEBUG('a', "DestroyLock syscall.\n");
		rv = DestroyLock_Syscall(machine->ReadRegister(4));
		break;	
		
		case SC_CreateCV:
		DEBUG('a', "CreateCV syscall.\n");
		rv = CreateCV_Syscall(machine->ReadRegister(4),
				  machine->ReadRegister(5),
				  machine->ReadRegister(6));
		break;
		
		case SC_Wait:
		DEBUG('a', "Wait syscall.\n");
		rv = Wait_Syscall(machine->ReadRegister(4),
				  machine->ReadRegister(5));
		break;
		
		case SC_Signal:
		DEBUG('a', "Signal syscall.\n");
		rv = Signal_Syscall(machine->ReadRegister(4),
				  machine->ReadRegister(5));
		break;
		
		case SC_Broadcast:
		DEBUG('a', "Broadcast syscall.\n");
		rv = Broadcast_Syscall(machine->ReadRegister(4),
				  machine->ReadRegister(5));
		break;
		
		case SC_DestroyCV:
		DEBUG('a', "DestroyCV syscall.\n");
		rv = DestroyCV_Syscall(machine->ReadRegister(4));
		break;	
		
		case SC_Yield:
		DEBUG('a', "Yield syscall.\n");
		Yield_Syscall();
		break;		
		
		case SC_Fork:
		DEBUG('a', "Fork syscall.\n");
		Fork_Syscall(machine->ReadRegister(4));
		break;		
		
		case SC_Exit:
		DEBUG('a', "Exit syscall.\n");
		Exit_Syscall(machine->ReadRegister(4));
		break;		
		
		case SC_Scan:
		DEBUG('a', "Scan syscall.\n");
		rv = Scan_Syscall(machine->ReadRegister(4));
		break;
		
	 	case SC_Exec:
		DEBUG('a', "Exec syscall.\n");
		rv = Exec_Syscall(machine->ReadRegister(4), 
			machine->ReadRegister(5));
		break; 
		
		case SC_Print_Stmt:
		DEBUG('a', "Print_Stmt syscall.\n");
		Print_Stmt_Syscall(machine->ReadRegister(4));	
		break; 
		
		case SC_Print_1Arg:
		DEBUG('a', "Print_1Arg syscall.\n");
		Print_1Arg_Syscall(machine->ReadRegister(4),
			machine->ReadRegister(5));	
		break;
		
		case SC_Print_2Arg:
		DEBUG('a', "Print_2Arg syscall.\n");
		Print_2Arg_Syscall(machine->ReadRegister(4),
			machine->ReadRegister(5),
			machine->ReadRegister(6));	
		break;
		
		case SC_Print_3Arg:
		DEBUG('a', "Print_3Arg syscall.\n");
		Print_3Arg_Syscall(machine->ReadRegister(4),
			machine->ReadRegister(5),
			machine->ReadRegister(6),
			machine->ReadRegister(7));	
		break;
		
		case SC_Sprint:
		DEBUG('a', "Sprint syscall.\n");
		Sprint_Syscall(machine->ReadRegister(4),
			machine->ReadRegister(5),
			machine->ReadRegister(6),
			machine->ReadRegister(7));	
		break;
		
		case SC_Print_Str:
		DEBUG('a', "Print_Str syscall.\n");
		Print_Str_Syscall(machine->ReadRegister(4),
			machine->ReadRegister(5));	
		break;
		
		case SC_CreateMV:
		DEBUG('a', "CreateMV syscall.\n");
		rv = CreateMV_Syscall(machine->ReadRegister(4),
			machine->ReadRegister(5),
			machine->ReadRegister(6),
			machine->ReadRegister(7));	
		break;
		
		case SC_GetMV:
		DEBUG('a', "Get MV syscall.\n");
		rv = GetMV_Syscall(machine->ReadRegister(4));
		break;
		
		case SC_SetMV:
		DEBUG('a', "Set MV syscall.\n");
		rv = SetMV_Syscall(machine->ReadRegister(4),
			machine->ReadRegister(5));
		break;
		
		case SC_MVDestroy:
		DEBUG('a', "CreateMV syscall.\n");
		rv = DestroyMV_Syscall(machine->ReadRegister(4));
		break;
		
		case SC_StartSimulation:
		DEBUG('a', "StartSimulation syscall.\n");
		StartSimulation_Syscall();
		break;
	
	}

	// Put in the return value and increment the PC
	machine->WriteRegister(2,rv);
	machine->WriteRegister(PrevPCReg,machine->ReadRegister(PCReg));
	machine->WriteRegister(PCReg,machine->ReadRegister(NextPCReg));
	machine->WriteRegister(NextPCReg,machine->ReadRegister(PCReg)+4);
	return;
    } else if ( which == PageFaultException ) {
			
			// Register 39 stores the virtual page which caused the page fault
			int VPN = (machine->ReadRegister(39)) / PageSize;
		
			int ppn = findIptIndex(VPN);
			
			if(ppn == -1)
			{ 
				// VPN is not present
				// need to load VPN into memory
				ppn = currentThread->space->loadPageIntoIPT(VPN);
		
				IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
				updateTLB(ppn);
				(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
			}
			  
			if(ppn != -1)
			{
				IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts				
				updateTLB(ppn);
				(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
			}
	}
	else
	{
      cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
      interrupt->Halt(); 
    }
}
