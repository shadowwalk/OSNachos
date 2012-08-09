#include "syscall.h"

int main() {
	
	
    /* Output statements for test case menu */
	Print_Stmt("\n\t\t\t\t--------------------------------------------------------------\n");
	Print_Stmt("\n\t\t			     PROJECT 4		  GROUP - 58\n\n");
	Print_Stmt("\t\t\t		     		Distributed Systems\n");
	Print_Stmt("\t\t\t		      >>>>>>>> TEST CASES PART 1 <<<<<<<<\n");
	Print_Stmt("\n\t\t\t\t--------------------------------------------------------------\n\n");
	
	
	Exec("../test/halt2", 13);
/* 	Exec("../test/halt2", 13); */
	Print_Stmt("came back here\n");
	/* Exec("../test/halt", 12); */
	
}