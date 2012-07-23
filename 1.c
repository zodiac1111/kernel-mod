#include <stdio.h>

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
