#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

void f(int x)
{
	if (x)
		f(x - 1);
}

int main()
{   
	int x = 3;
	f(x);
	return 0;
}
