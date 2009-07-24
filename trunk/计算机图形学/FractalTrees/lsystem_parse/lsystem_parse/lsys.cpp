
#include <stdlib.h> 
#include <stdio.h> 

FILE *yyin; 
extern "C"
extern int yyparse(); 
 
int main(int argc, char* argv[]) 
{ 
	
 
	printf("Compiling...!\n"); 
	/*if((yyin=fopen("wc.l","rt"))==NULL){ 
			printf("can not open file wc.l\n") ; 
			getchar();
			exit(1); 
		} */
	if (yyparse()==1){ 
		fprintf(stderr,"parser error\n"); 
		exit(1); 
	} 
	printf("yyparse() completed successfully!\n"); 
	return 0; 

} 
 

