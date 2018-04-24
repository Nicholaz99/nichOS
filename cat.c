#define MAX_FILE_ENTRY 512*16

void readString(char * string);
void clearString(char *buffer, int length);

main() {
    char argv[2][50],buffer[MAX_FILE_ENTRY];
    char currDir,argc,result;
    int i;
    char temp[50];
    enableInterrupts();
    interrupt(0x21, 0x21, &currDir, 0, 0);
    interrupt(0x21, 0x22, &argc, 0, 0);
    clearString(buffer, MAX_FILE_ENTRY);
    clearString(argv[0], 50);
    clearString(argv[1], 50);

    for (i = 0; i < argc; i++) {
        interrupt(0x21, 0x23, i, argv[i], 0);
    }

    if (argc >= 2 && argv[1][0] == '-' && argv[1][1] == 'w') {
        interrupt(0x21, 0x0, "Contents : ", 0, 0);
        readString(buffer);
        result=1;
        // interrupt(0x21, 0x0, "Read stringgggfasfdsa\n\r ", 0, 0);
        interrupt(0x21, (currDir << 8) | 0x05, buffer, argv[0], &result);
            
    }
    else {
        result=1;
        interrupt(0x21, (currDir << 8) | 0x04, buffer, argv[0], &result);
        interrupt(0x21, 0x0, buffer, 0, 0);
    }
    for (i = 0; i < 4; i++) {
        clearString(argv[i], 50);
    }
    // interrupt(0x21, 0x0, "FINISH CATTING MEOWWW\n\r ", 0, 0);
    interrupt(0x21, 0x07, &result, 0, 0);
}

void clearString(char *a, int n){
  int i;
  for(i = 0; i < n; ++i){
    a[i] = '\0';
  }
}

void readString(char * string) {
    char c;
	int i = 0;
	do
	{
		c = interrupt(0x16, 0, 0, 0, 0);
		if (c == '\b')
		{
			// interrupt(0x21, 0x0, "FINISH STARTING AWWW MEOWWW\n\r ", 0, 0);
            interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
			interrupt(0x10, 0xE00 + '\0', 0, 0, 0);
			interrupt(0x10, 0xE00 + '\b', 0, 0, 0);

			i--;
			string[i] = '\0';
		}
		else if (c == '\r')
		{
			interrupt(0x10, 0xE00 + '\n', 0, 0, 0);
			interrupt(0x10, 0xE00 + '\r', 0, 0, 0);
		}
		else
		{
			interrupt(0x10, 0xE00 + c, 0, 0, 0);
			string[i] = c;
			i++;
		}
	} while (c != '\r');
}