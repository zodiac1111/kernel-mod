#include<stdio.h>
int main()
{
	struct st{
		char a;
		int b;
		char c;
		short d;
		double e;
	}st1;
	printf("sizeof %d\n",sizeof(st1));

	return 0;
}
