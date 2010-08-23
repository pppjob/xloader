/*******************************************************
 * Routine: delay
 * Description: spinning delay to use before udelay works
 ******************************************************/
void delay(unsigned long loops)
{
	__asm__ volatile ("1:\n" "subs %0, %1, #1\n"
			  "bne 1b":"=r" (loops):"0"(loops));
}

void udelay(unsigned long us)
{
	delay(us * 300); /* approximate */
}

void mdelay(unsigned long ms)
{
	udelay(1000); /* approximate */
}

