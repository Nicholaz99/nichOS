void getCommand(char * cmd, char * argv, char * argc);
void strToBuff(char * buffer, char * str);
int strEqual(char * a, char * b);

main() {
    char ch[2];
    char command[17];
    char new_cmd[17];
    char argv[512];
    char argc;
    char curdir;
    int stat;
    int i;
    enableInterrupts();
    while (1) {
        stat = -1;
        interrupt(0x21, 0x00, "\r\n$ ", 0x0, 0x0);
        getCommand(command, argv, &argc);
        interrupt(0x21, 0x0, "\r\n", 0,0);
        interrupt(0x21, 0x21, &curdir, 0, 0);

        interrupt(0x21, 0x23, 0, ch, 0);

        if (strEqual(command, "resume")) {
            interrupt(0x21, 0x33, ((ch[0] - '0')*4096)+8192, &stat, 0);
        } else if (strEqual(command, "ps")) {
            interrupt(0x21, 0x35, 0,0,0);
        } else if (strEqual(command, "pause")) {
            interrupt(0x21, 0x32, ((ch[0] - '0')*4096)+8192, &stat, 0);
        } else if (strEqual(command, "kill")) {
            interrupt(0x21, 0x34, ((ch[0] - '0')*4096)+8192, &stat, 0);
        } else if (command[0]=='.' && command[1]=='/') {
            interrupt(0x21, (0xFF << 8) | 0x06, command+2, &stat, 0);
        }else {
            // strToBuff(&new_cmd, &command);
            interrupt(0x21, (0xFF << 8) | 0x06 , command, &stat, 0);
        }
    }
}

int strEqual(char * a, char * b) {
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0') {
        if (a[i] != b[i]) {
            return 0;
        }
        i++;
    }

    if (a[i] != b[i]) {
        return 0;
    } else {
        return 1;
    }
}

void getCommand(char * cmd, char * argv, char * argc) {
    char curdir;
    char temp[512];
    int pos, i, count, j;
    interrupt(0x21, 0x1, temp, 1, 0);
    pos = -1;
    i = 0;
    while (temp[i] != '\0') {
        if (' ' == temp[i]) {
            pos = i;
        }
        i++;
    }

    count = 0;
    i = 0;
    while (temp[i] != '\0') {
        if (' ' == temp[i]) {
            count++;
        }
        i++;
    }

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

/*int findCharInStr(char * buffer, char c) {
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
}*/