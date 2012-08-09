// nettest.cc 
//	Test out message delivery between two "Nachos" machines,
//	using the Post Office to coordinate delivery.
//
//	Two caveats:
//	  1. Two copies of Nachos must be running, with machine ID's 0 and 1:
//		./nachos -m 0 -o 1 &
//		./nachos -m 1 -o 0 &
//
//	  2. You need an implementation of condition variables,
//	     which is *not* provided as part of the baseline threads 
//	     implementation.  The Post Office won't work without
//	     a correct implementation of condition variables.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "system.h"
#include "network.h"
#include "post.h"
#include "interrupt.h"

#define LOCK_BUSY 0
#define LOCK_FREE 1

void ServerReg(int instances)
{	// instances --> number of networking threads

	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
	
	char *sendMessage = new char[MaxMailSize];
	char *sendInstanceInfo = new char[MaxMailSize]; 
	char *buffer = new char[MaxMailSize];
	char *machineId = new char[instances];
	char *mailBoxNumber = new char[instances];
	
	int instanceCount = 0, noOfMessages = 0, success = 0, mailBoxUnits = 0, mailBoxTens = 0, msgcount = 0;
	int loopCount = 1, flag = 0, count = 0, messageSize = 3, maxFileBytes = 40;
	printf("In serverReg function waiting for %d registration requests in a loop\n", instances);
	
    // Wait for the message from all of the networking thread
	for(instanceCount = 1; instanceCount <= instances; instanceCount++)
	{
		postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
		printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
		fflush(stdout);
		machineId[instanceCount] = inPktHdr.from;
		mailBoxNumber[instanceCount] = inMailHdr.from;
	}
	//add 1 to generate appropriate number of message count
	
		noOfMessages = ((instances*messageSize)/maxFileBytes) + 1;
		msgcount = noOfMessages;
	
	// SEND the "noOfMessages@noofnetthreads@" message to all networking threads 
	for(instanceCount = 1; instanceCount <= instances; instanceCount++)
	{
		// construct packet, mail header for message
		// To: Server Nachos machine, mailbox - one of the networking thread
		// From: Registration Server 0, reply to: mailbox 0
		outPktHdr.to = machineId[instanceCount];		
		outMailHdr.to = mailBoxNumber[instanceCount];
		outMailHdr.from = serverMailBoxNumber;
		outPktHdr.from = serverMachineID;	
		
		// Format the reply message to the networking threads waiting for the message
		sprintf(sendMessage, "%d%c%d%c", noOfMessages,'@', instances,'@');
		outMailHdr.length = strlen(sendMessage) + 1;
		// Send the message
		success = postOffice->Send(outPktHdr, outMailHdr, sendMessage); 
		
		if (!success)
		{
			printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
			interrupt->Halt();
		}
	}
	
	// Send the group table populating information to all the networking threads 
	while(msgcount != 0)
	{
		// divided by 10 to find the units and tens place of mailbox number
		
		for(count  = loopCount; count <= (loopCount + 12); count++)
		{
			if(count > instances)
			{
				flag = 1;
				break;
			}
				mailBoxUnits = ((mailBoxNumber[count])/10);
				mailBoxTens =  (mailBoxNumber[count])%10;
				sprintf(buffer,"%d%d%d", machineId[count], mailBoxUnits, mailBoxTens);		
				strcat(sendInstanceInfo, buffer);
		}	
		
		outMailHdr.length = strlen(sendInstanceInfo) + 1;
		if(flag == 1)
			strcat(sendInstanceInfo,"@");
		
		for(instanceCount = 1; instanceCount <= instances; instanceCount++)
		{
			// construct packet, mail header for message
			// To: Server Nachos machine, mailbox - one of the networking thread
			// From: Registration Server 0, reply to: mailbox 0
			outPktHdr.to = machineId[instanceCount];		
			outMailHdr.to = mailBoxNumber[instanceCount];
			outMailHdr.from = serverMailBoxNumber;
			outPktHdr.from = serverMachineID;		
			// send the message
			success = postOffice->Send(outPktHdr, outMailHdr, sendInstanceInfo); 
		
			if (!success)
			{
				printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
				interrupt->Halt();
			}
		}	
			msgcount--;
			loopCount = count + 1;
			
	}
	 
}


// for Extra Credit


void KernelFunction()
{


	PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
	int index = 0;
	int packetNo = 0;
	char *sendMessage = new char[MaxMailSize];
	char *sendAckMessage = new char[MaxMailSize];
	char *buffer = new char[MaxMailSize];
	char separator = ':';
	int success = 0;
	int loop = 0;
	// seding message to the other machine
	loop = extra;
	Delay(10);
	printf("Extra = %d\n", extra);
		for(index = 0; index < extra; index++)
		{	printf("In for extra = %d\n", extra);
			if(index != clientId)
			{	
				outPktHdr.to = index;
				outMailHdr.to = 0;
				outMailHdr.from = 0;
				outPktHdr.from = clientId;	
				
				for(packetNo = 0; packetNo <5; packetNo++) 
				{
					sprintf(sendMessage,"%d%c%d%c%d", index,separator, packetNo,separator,clientId);
					outMailHdr.length = strlen(sendMessage) + 1;
					
					printf("Sending Message %s to Client %d\n", sendMessage,outPktHdr.to);
					success = postOffice->Send(outPktHdr, outMailHdr, sendMessage); 
					if (!success)
					{	
						printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
						//interrupt->Halt();
					}
						
				}  					
			}
		}
		int nextseqnum[5];
		for(int g=0;g<5;g++)
		{
		   nextseqnum[g]=0;	
		}
	// receiving packets or Acks messages from other machines
	while(1)
	{
		postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
		char *tempBuffer = new char[40];
		strcpy(tempBuffer,buffer);
		
		
		
		char *content = strtok(buffer, ":");
		char *pNo = strtok(NULL, ":");
		int packNo = atoi(pNo);
		
		if((strcmp(content,"ACK")) != 0)
		{
			   
			   
			   if(packNo>nextseqnum[inPktHdr.from])
			   {
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = 0;
					outMailHdr.from = 0;
					outPktHdr.from = clientId;
					sprintf(sendAckMessage,"%s%c%d","ACK",separator,nextseqnum[inPktHdr.from] -1);
					
					outMailHdr.length=strlen(sendAckMessage)+1;		
					
					printf("Ack Message %s sent for Sequence Number %d from Client %d to client %d\n",sendAckMessage,nextseqnum[inPktHdr.from] -1,clientId, outPktHdr.to);	
					success = postOffice->Send(outPktHdr,outMailHdr,sendAckMessage);
					if(!success)
					{
						printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
						
					}
			   }
			   else
			   {
				if(nextseqnum[inPktHdr.from] == packNo)
				{
					printf("Received Message %s sequence number %d from %d client\n",tempBuffer,packNo,inPktHdr.from);
					nextseqnum[inPktHdr.from]++;
				}
				
												
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = 0;
					outMailHdr.from = 0;
					outPktHdr.from = clientId;
					sprintf(sendAckMessage,"%s%c%d","ACK",separator,packNo);
					
					outMailHdr.length=strlen(sendAckMessage)+1;		
					
					printf("Ack Message %s sent for Sequence Number %d from Client %d to client %d\n",sendAckMessage,packNo,clientId, outPktHdr.to);	
					success = postOffice->Send(outPktHdr,outMailHdr,sendAckMessage);
					if(!success)
					{
						printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
						
					}
				
				}				
		}
		else
		{
			char *Qmsg = new char[40];
			char *buf=new char[40];
			int sequenceNo, cliId;
				int flag = 0;
			while(!(msgQueue->IsEmpty()))              
			{						
				Qmsg=(char*)msgQueue->Remove();
				 
				strcpy(buf,Qmsg);
				
				char *tempclientId=strtok(buf,":");
				char *seq=strtok(NULL,":");
				sequenceNo=atoi(seq);
				cliId=atoi(tempclientId); 
				
				if(packNo == sequenceNo && inPktHdr.from == cliId)
				{
					printf("Got Ack for Sequence number %d from Machine %d Mailbox 0\n",sequenceNo,inPktHdr.from);
					detail[cliId].sequenceNum = 1;
					break;                            
				}
				else
				{
					msgQueue->Append((void *)Qmsg);
					if(flag == 21)
						break;
						flag++;
				}

			}
							
		}		
		content = NULL;
		pNo = NULL;
	}	
}	




