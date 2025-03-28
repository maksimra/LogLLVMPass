#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

void f()
{
	int z = 35;
	double y = 65;
	z += y;
}

int main()
{   
	double x = 85;
	scanf("%lf", &x);
	x += 34;
	f();
	return 0;
}
