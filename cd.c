#define DIRS_SECTOR 0x101
#define FILES_SECTOR 0x102

void readSector(char * buffer, int sector);

int mod(int a, int b);

int div(int a, int b);

char getIndex(char * name, char parent, int sector);

void traverseDir(char * path, int * result, char * parentIndex);

int findCharInStr(char * buffer, char c);

main() {
    int i;
    char curdir;
    char argc;
    char argv[8][50];
    char index;
    int stat;

    interrupt(0x21, 0x21, &curdir, 0, 0);
    interrupt(0x21, 0x22, &argc, 0, 0);
    for (i = 0; i < argc; ++i) {
        interrupt(0x21, 0x23, i, argv[i], 0);
    }
    interrupt(0x21, 0x0, argv[0], 0, 0);
    traverseDir(argv[0], &stat, &curdir);
    if (argv[0][0] != '\0') {
        index = getIndex(argv[0], curdir, DIRS_SECTOR);
    } else {
        index = curdir;
    }

    if (index != -2) {
        interrupt(0x21, 0x20, index, argc, argv);
    }

    interrupt(0x21, 0x07, &stat, 0, 0);

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

char getIndex(char * name, char parent, int sector) {
    char dir[512];
    int i, j, k, count;
    char result;

    readSector(dir, sector);

    for (i = 0; i < 512; i += 16) {
        if (dir[i] != parent) {
            continue;
        }
        if (dir[i+1] == '\0') {
            continue;
        }

        count = 0;
        if (dir[i] != parent) {
            continue;
        }
        for (j = 0; j < 15; j++) {
            if (name[j] == '\0' && dir[i+j+1] == '\0') {
                count = 15;
                break;
            }
            if (name[j] != dir[i+j+1]) {
                break;
            }
            count++;
        }
        result = count == 15;
        if (result) {
            return i/16;
        }
    }

    return -2;
}

void traverseDir(char * path, int * result, char * parentIndex) {
    char dir[512];
    int i, j, k, count;
    int len;
    int parent;

    len = findCharInStr(path, '/');
    //printString(path);

    if (len == 0) {
        while(path[i+len] != '\0') {
            path[i] = path[i+len+1];
            i++;
        }
        return traverseDir(path, result, parentIndex);
    }


    if (len != -1) {
        char name[15];
        for (i = 0; i < len; i++) {
            name[i] = path[i];
        }
        name[i] = '\0';

        if (len == 2 && name[0] == '.' && name[1] == '.') {
            readSector(dir, DIRS_SECTOR);
            *parentIndex = dir[*parentIndex*16];
        } else {
            *parentIndex = getIndex(name, *parentIndex, DIRS_SECTOR);
        }

        if (*parentIndex == -2) {
            *result = -1;
        } else {
            i = 0;
            while(path[i+len] != '\0') {
                path[i] = path[i+len+1];
                i++;
            }
            return traverseDir(path, result, parentIndex);
        }
    } else {
        *result = 0;
        return;
    }
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
