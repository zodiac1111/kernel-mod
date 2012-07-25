#include <stdio.h>
23/07/12 20:41:56

char *strA()
{
	char str[] = "hello world!";
	return str;
}

int main()
{
	printf("%s\n", strA());
	return 0;
}
