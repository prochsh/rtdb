#include<stdio.h>
int main(){
#define STRING(sz) #sz
	char *sz = "hello" "\t" "world" "\n"
		"my name is ranpanf\n";
	printf(sz);
	return  0;
}
