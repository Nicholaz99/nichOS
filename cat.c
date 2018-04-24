void readString(char * string);
void clear(char *buffer, int length);

main() {
    char argc;
    char argv[2][50];
    char curdir;
    char stat;
    int i;
    char buffer[512*16];
    char temp[50];

    interrupt(0x21, 0x21, &curdir, 0, 0);
    interrupt(0x21, 0x22, &argc, 0, 0);
    clear(buffer, 512*16);
    for (i = 0; i < 2; i++) {
        clear(argv[i], 50);
    }



    for (i = 0; i < argc; i++) {
        //interrupt(0x21, 0x0, "DI CAT\r\n", 0, 0);
        interrupt(0x21, 0x23, i, argv[i], 0);
        //interrupt(0x21, 0x23, i +'0', 0x0, 0);
        //interrupt(0x21, 0x0, argv[i], 0, 0);
    }

    if (argc >= 2 && argv[1][0] == '-' && argv[1][1] == 'w') {
        interrupt(0x21, 0x0, argv[1], 0, 0);
        //interrupt(0x21, 0x0, "MASUK\r\n", 0, 0);
        readString(buffer);
        //interrupt(0x21, 0x0, "\r\n", 0,0);
        //interrupt(0x21, 0x0, buffer, 0, 0);
        stat = 1;
        //interrupt(0x21, 0x0, "\r\n", 0, 0);
        //interrupt(0x21, 0x0, argv[0], 0, 0);
        interrupt(0x21, (curdir << 8) | 0x05, buffer, argv[0], &stat);
    } else {
        //interrupt(0x21, 0x0, argv[0], 0, 0);
        stat = 1;
        interrupt(0x21, (curdir << 8) | 0x04, buffer, argv[0], &stat);
        interrupt(0x21, 0x0, buffer, 0, 0);
    }

    for (i = 0; i < 4; i++) {
        clear(argv[i], 50);
    }

    interrupt(0x21, 0x07, &stat, 0, 0);
}

void clear(char *buffer, int length){
  int i;
  for(i = 0; i < length; ++i){
    buffer[i] = 0x00;
  }
}

void readString(char * string) {
    int i = 0;
    int count;
    char c;
    count = 0;
    c = interrupt(0x16, 0, 0, 0, 0);
    while (c != '\r' || count != 1) {
        if (c == '\b' && i > 0) {
            interrupt(0x10, 0xE00 + '\b', 0,0,0);
            interrupt(0x10, 0xE00 + '\0',0,0,0);
            interrupt(0x10, 0xE00 + '\b', 0,0,0);
            i--;
            c = interrupt(0x16, 0, 0, 0, 0);
        } else {
            if (c != '\r') {
                count = 0;
                string[i] = c;
            } else {
                interrupt(0x10, 0xE00 + '\r', 0, 0, 0);
                string[i] = '\r';
                string[i+1] = '\n';
                i++;
                //interrupt(0x10, 0xE00 + '\n', 0, 0, 0);
                count = 1;
            }
            interrupt(0x10, 0xE00 + string[i], 0, 0, 0);
            c = interrupt(0x16, 0, 0, 0, 0);
            i++;
        }
    }
    string[i] = '\0';
}