#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

int f(int x)
{
	if (x)
		f(x - 1);
	return 8;
}

int main()
{   
	int x = 3;
	x = f(x);
	return 0;
}
