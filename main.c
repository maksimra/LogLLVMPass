#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

void f()
{
	int x = 67;
	printf("%d\n", x);
	return;
}

int main()
{   
	double x = 85;
	x += 56;
	float y = 65;
	f();
	return y;
}
