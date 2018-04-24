int div(int a, int b);
int mod(int a, int b);
int strlen(char * buff);
int strcmp(char * buff, char * buff2);
int strToInt(char * buff);
void intToStr(int x, char * buff);

main() {
    char buff1[8];
    char buff2[8];
    char result[8];
    char cmd[8];
    int a ,b;
    int stat;
    interrupt(0x21, 0x0, "\r\nBilangan 1 : \r\n", 0x0, 0x0);
    interrupt(0x21, 0x1, buff1, 0x0, 0x0);
    interrupt(0x21, 0x0, "\r\nBilangan 2 : \r\n", 0x0, 0x0);
    interrupt(0x21, 0x1, buff2, 0x0, 0x0);
    interrupt(0x21, 0x0, "\r\nOperator (+, -, *, /, %) : \r\n", 0x0, 0x0);
    interrupt(0x21, 0x1, cmd, 0x0, 0x0);
    interrupt(0x21, 0x0, "\r\n", 0x0, 0x0);

    a = strToInt(buff1);
    b = strToInt(buff2);

    if (strcmp(cmd, "+")) {
        intToStr(a+b, result);
    } else if (strcmp(cmd, "-")) {
        intToStr(a-b, result);
    } else if (strcmp(cmd, "*")) {
        intToStr(a*b, result);
    } else if (strcmp(cmd, "/")) {
        intToStr(div(a,b), result);
    } else {
        intToStr(mod(a, b), result);
    }
    interrupt(0x21, 0x0, "Result : ", 0,0);
    interrupt(0x21, 0x0,result, 0, 0);
    interrupt(0x21, 0x07, &stat, 0, 0);

}

int mod(int a, int b) {
    while(a >= b){
      a = a - b;
    }
    return a;
  }

int div(int a, int b) {
    int q = 0;
    while(q*b <=a) {
        q = q+1;
    }
    return q-1;
}

int strlen(char * buff) {
    int i = 0;
    while (buff[i] != '\0') {
        i++;
    }

    return i;
}

int strcmp(char * buff, char * buff2) {
    int i;
    if (strlen(*buff) != strlen(*buff2)) {
        return 0;
    }
    for (i = 0; i < strlen(buff); i++) {
        if (buff[i] != buff2[i]) {
            return 0;
        }
    }

    return 1;
}
int strToInt(char * buff) {
    int j = 1;
    int i = 0;
    int start = 0;
    int result = 0;

    if (buff[0] == '-') {
	start = 1;
	j = -1;
    }
    for (i = 0; i < strlen(buff)-start; i++) {
        result += (buff[strlen(buff)-1-i] - '0') * j;
        j *= 10;
    }

    return result;
}

void intToStr(int x, char * buff) {
    int i = 0;
    int y = 0;
    int j = 0;
    int start = 0;
    char temp[10];
    if (x > 0) {
        y = x;
    } else if (x < 0) {
        buff[0] = '-';
        i = 1;
	start = 1;
        y = -x;
    } else {
        buff[0] = '0';
        buff[1] = '\0';
    }
    while (y != 0) {
        temp[i] = mod(y, 10) + '0';
        y = div(y - mod(y, 10), 10);
        i++;
    }

    buff[i] = '\0';
    for (j = start; j < i; j++) {
	if (start) {
		buff[j] = temp[i-j];
	} else {
		buff[j] = temp[i-j-1];
	}
    }
}
