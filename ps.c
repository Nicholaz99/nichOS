// State of process
#define DEFUNCT 0
#define RUNNING 1
#define STARTING 2
#define READY 3
#define PAUSED 4
// Maximal numbers
#define MAX_SEGMENTS 8

int mod(int a, int b);
int div(int a, int b);
void printPID(int pid);

int main()
{
	int success;
	int pid = 7;
	int state = 5;
	int i;
	enableInterrupts();
	interrupt(0x21, 0x0, "List of Process:", 0, 0);
	interrupt(0x21, 0x0, "\n\r", 0, 0);
	for (i = 0; i < MAX_SEGMENTS; i++)
	{
		interrupt(0x21, 0x24, i, &pid, &state);
		if (state != DEFUNCT)
		{
			if (state == READY)
			{
				printPID(pid);
				// interrupt(0x21, 0x0, "babiiiii", 0, 0);
				interrupt(0x21, 0x0, " READY :\n\r", 0, 0);
			}
			else if (state == STARTING)
			{
				printPID(pid);
				// interrupt(0x21, 0x0, "awww", 0, 0);
				interrupt(0x21, 0x0, " STARTING :\n\r", 0, 0);
			}
			else if (state == RUNNING)
			{
				printPID(pid);
				// interrupt(0x21, 0x0, "asas", 0, 0);
				interrupt(0x21, 0x0, " RUNNING :\n\r", 0, 0);
			}
			else if (state == PAUSED)
			{
				printPID(pid);
				// interrupt(0x21, 0x0, "Mie rica", 0, 0);
				interrupt(0x21, 0x0, " PAUSED :\n\r", 0, 0);
			}
		}
	}
	interrupt(0x21, 0x07, &success, 0, 0);
}

int mod(int a, int b)
{
	while (a >= b)
	{
		a = a - b;
	}
	return a;
}

int div(int a, int b)
{
	int q = 0;
	while (q * b <= a)
	{
		q = q + 1;
	}
	return q - 1;
}

void printPID(int pid)
{
	int num[1000];
	int i, n;
	int p = pid;

	if (pid == 0)
	{
		interrupt(0x10, 0xE00 + '0', 0, 0, 0);
	}
	else
	{
		if (pid < 0)
		{
			interrupt(0x10, 0xE00 + '-', 0, 0, 0);
			p = p * (-1);
		}
		n = 0;
		while (p != 0)
		{
			num[n] = mod(p, 10);
			p = div(p, 10);
			n++;
		}
		for (i = n - 1; i >= 0; i--)
		{
			interrupt(0x10, 0xE00 + ('0' + num[i]), 0, 0, 0);
		}
	}
}