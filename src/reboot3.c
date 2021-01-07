#include <stdio.h>

void *reboot3(int howto) {
	/* Let's just call reboot2 */
	/* It is defined in liblaunch */
	/* printf("libxpc reboot3 called with howto: %d\n", howto); */
	return reboot2(howto);
}
