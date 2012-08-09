#include "syscall.h"

int main() {
	
	
	int userInput = 0;
		char buf[3];
	
    /* Output statements for test case menu */
	Print_Stmt("\n\t\t\t\t--------------------------------------------------------------\n");
	Print_Stmt("\n\t\t			     PROJECT 4		  GROUP - 58\n\n");
	Print_Stmt("\t\t\t		     		Distributed Systems\n");
	Print_Stmt("\t\t\t		      >>>>>>>> TEST CASES PART 1 <<<<<<<<\n");
	Print_Stmt("\n\t\t\t\t--------------------------------------------------------------\n\n");
	
	Print_Stmt(" 1 > Create Lock networking Test\n");
	Print_Stmt(" 2 > Acquire and release integrated\n");
	Print_Stmt(" 3 > Create CV networking Test\n");
	Print_Stmt(" 4 > Wait-Signal synchronization Test\n");
	Print_Stmt(" 5 > Broadcast networking Test\n");
	Print_Stmt(" 6 > Create Monitor Variable Test\n");
	Print_Stmt(" 7 > Get and Set Monitor Variable Test\n");
	Print_Stmt("\n\nEnter the index of the testcase you want to run here ->");
    
	userInput = Scan("%d"); 

	 	switch(userInput){
		
		case 1:			
				Exec("../test/halt", 12);

				break;
		case 2:			
				Print_Stmt("\nCHOOSE THE NUMBER FOR SPECIFIC TEST TO RUN\n");
				Print_Stmt(" 1> See the single instance Acquire&Release test\n");
				Print_Stmt(" 2> Make this instance Acquirelock \n");
				Print_Stmt(" 3> Make this instance Acquire then Releaselock \n");
				userInput = Scan("%d");
				switch(userInput){
		
				case 1:			Exec("../test/halt", 12);
					break;
				case 2:			Exec("../test/Acquirelock", 19);
					break;	
				case 3:			Exec("../test/Releaselock", 19);
					break;	
				default: Write("\n Invalid STEP entered by the user !!\n\n", 41, ConsoleOutput);	
					break;	
				}
				break;

				break;
		case 3:			Exec("../test/halt", 12);
				break;
		
		case 4:
				Print_Stmt("\nCHOOSE THE NUMBER FOR SPECIFIC TEST TO RUN\n");
				Print_Stmt(" 1> See the single instance wait&signal test\n");
				Print_Stmt(" 2> Make this instance wait \n");
				Print_Stmt(" 3> Make this instance signal \n");
				userInput = Scan("%d");
				switch(userInput){
		
				case 1:			Exec("../test/halt", 12);
					break;
				case 2:			Exec("../test/CVwait", 14);
					break;	
				case 3:			Exec("../test/CVsignal", 16);
					break;	
				default: Write("\n Invalid STEP entered by the user !!\n\n", 41, ConsoleOutput);	
					break;	
				}
				break;

		case 5:
				Exec("../test/broadcast",17);
				break;
	
		case 6:
				Exec("../test/halt",12);
				break;
		
		case 7:
				Exec("../test/MVop",12);
				break;
				
				
	}
	
	Print_Stmt("came back here\n");
	/* Exec("../test/halt", 12); */
	

	
	
	
	
}