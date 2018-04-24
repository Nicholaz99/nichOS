#define DIRS_SECTOR 0x101
#define FILES_SECTOR 0x102

void readSector(char * buffer, int sector);

int mod(int a, int b);

int div(int a, int b);


main () {
    char parent;
    char buffer[512];
    char buffer2[512];
    char name[15];
    int i,j;
    char result;
    enableInterrupts();
    //parent = 0xFF;

    interrupt(0x21, 0x21, &parent, 0, 0);
    //interrupt(0x21, 0x0, "\r\n$ ", 0x0, 0x0);

    readSector(buffer, FILES_SECTOR);
    readSector(buffer2, DIRS_SECTOR);

    for (i = 0; i < 512; i += 16) {
        if (buffer[i] == parent) {
            for (j = 0; j < 15; j++) {
                name[j] = buffer[i+j+1];
            }
            if (name[0] != '\0') {
                //interrupt(0x21, 0x0, "FILE :", 0 ,0);
                interrupt(0x21, 0x0, name, 0, 0);
                interrupt(0x21, 0x0, "\r\n", 0, 0);
            }
        }
        if (buffer2[i] == parent) {
            for (j = 0; j < 15; j++) {
                name[j] = buffer2[i+j+1];
            }
            if (name[0] != '\0') {
                //interrupt(0x21, 0x0, "DIR :", 0, 0);
                interrupt(0x21, 0x0, name, 0, 0);
                interrupt(0x21, 0x0, "\r\n", 0, 0);
            }
        }
    }

    interrupt(0x21, 0x07, result, 0, 0);
}

void readSector(char * buffer, int sector) {
    interrupt(0x13, 0x201, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
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