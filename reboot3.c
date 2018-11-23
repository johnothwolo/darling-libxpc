#include <stdio.h>

void *reboot3(int howto) {
	printf("libxpc reboot3 called with howto: %d\n", howto);
	return NULL;
}
