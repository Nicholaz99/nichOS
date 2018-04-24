#define DIRS_SECTOR 0x101
#define FILES_SECTOR 0x102
#define MAX_FILES 32
#define MAX_FILENAME 15
#define MAX_DIRS 16
#define DIR_ENTRY_LENGTH 16
#define SECTOR_SIZE 512

void readSector(char * buffer, int sector);
int mod(int a, int b);
int div(int a, int b);

main () {
    char currDir;
    char files[SECTOR_SIZE];
    char dir[SECTOR_SIZE];
    char name[MAX_FILENAME];
    int i,j;
    char result;
    enableInterrupts();
    interrupt(0x21, 0x21, &currDir, 0, 0);

    readSector(files, FILES_SECTOR);
    readSector(dir, DIRS_SECTOR);

    for (i = 0; i < MAX_DIRS; i++) {
        // interrupt(0x21, 0x0, "iterateee", 0, 0);
        if (files[i*DIR_ENTRY_LENGTH] == currDir) {
            for (j = 0; j < MAX_FILENAME; j++) {
                name[j] = files[i*DIR_ENTRY_LENGTH+j+1];
            }
            if (name[0] != '\0') {
                // interrupt(0x21, 0x0, "here!!!", 0, 0);
                interrupt(0x21, 0x0, name, 0, 0);
                interrupt(0x21, 0x0, "\r\n", 0, 0);
            }
        }
        if (dir[i*DIR_ENTRY_LENGTH] == currDir) {
            for (j = 0; j < MAX_FILENAME; j++) {
                name[j] = dir[i*DIR_ENTRY_LENGTH+j+1];
            }
            if (name[0] != '\0') {
                // interrupt(0x21, 0x0, "finally get here", 0, 0);
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