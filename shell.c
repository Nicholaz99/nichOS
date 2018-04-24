void getCommand(char * cmd, char * argv, char * argc);
int findCharInStr(char * buffer, char c);
int findCharInStr(char * buffer, char c);

main() {
    char command[17];
    char new_cmd[15];
    char argv[512];
    char argc;
    char curdir;
    int i;
    int stat;
    stat = -1;

    interrupt(0x21, 0x00, "\r\n$ ", 0x0, 0x0);

    getCommand(command, argv, &argc);

    interrupt(0x21, 0x0, "\r\n", 0,0);
    interrupt(0x21, 0x21, &curdir, 0, 0);

    if (command[0] == '.' && command[1] =='/') {
        for (i = 2; i < 17; i++) {
            new_cmd[i-2] = command[i];
        }
        //interrupt(0x21, 0x0, "MASUK\r\n", 0, 0);
        interrupt(0x21, (curdir << 8) | 0x06 , new_cmd, 0x2000, &stat);
    } else {
        interrupt(0x21, (0xFF << 8) | 0x06, command, 0x2000, &stat);
    }

    if (stat == -1) {
        interrupt(0x21, 0x07, &stat, 0, 0);
    }
}

void getCommand(char * cmd, char * argv, char * argc) {
    char curdir;
    char temp[512];
    int pos, i, count, j;
    interrupt(0x21, 0x1, temp, 0, 0);
    pos = findCharInStr(temp, ' ');
    count = countCharInStr(temp, ' ');

    i = 0;
    while (i < pos || (temp[i] != '\0' && pos == -1)) {
        cmd[i] = temp[i];
        i++;
    }
    cmd[i] = '\0';

    i = 0;
    while (temp[i+pos] != '\0') {
        argv[i] = temp[i+pos+1];
        i++;
    }

    *argc = (char) count;

    interrupt(0x21, 0x21, &curdir, 0, 0);
    interrupt(0x21, 0x20, curdir, *argc, argv);
}

int findCharInStr(char * buffer, char c) {
    int i = 0;
    while (buffer[i] != '\0') {
        if (c == buffer[i]) {
            return i;
        }
        i++;
    }

    return -1;
}

int countCharInStr(char * buffer, char c) {
    int i = 0;
    int count = 0;
    while (buffer[i] != '\0') {
        if (c == buffer[i]) {
            count++;
        }
        i++;
    }

    return count;
}