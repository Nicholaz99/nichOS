#define MAX_BYTE 512
#define SECTOR_SIZE 512
#define MAX_FILES 32
#define MAX_FILENAME 15
#define MAX_SECTORS 16
#define MAX_DIRNAME 16
#define DIR_ENTRY_LENGTH 16
#define FILE_ENTRY_LENGTH 16
#define SECTORS_ENTRY_LENGTH 16
#define MAX_DIRS 16
#define MAP_SECTOR 0x100
#define DIRS_SECTOR 0x101
#define FILES_SECTOR 0x102
#define SECTORS_SECTOR 0x103
#define TRUE 1
#define FALSE 0
#define INSUFFICIENT_SECTORS 0
#define INSUFFICIENT_ENTRIES -3
#define NOT_FOUND -1
#define INSUFFICIENT_DIR_ENTRIES -1
#define EMPTY 0x00
#define USED 0xFF
#define ARGS_SECTOR 512
#define ALREADY_EXISTS -2
#define SUCCESSRESULT 0

void handleInterrupt21 (int AX, int BX, int CX, int DX);
int compareString(char *str1, char *str2, int l_str);
void printString(char *string);
void readString(char *string);
int mod(int a, int b);
int div(int a, int b);
void readSector(char *buffer, int sector);
void writeSector(char *buffer, int sector);
void clear(char *buffer, int length);
void writeFile(char *buffer, char *path, int *sectors, char parentIndex);
void executeProgram(char *path, int segment, int *result, char parentIndex);
char getIndex(char * name, char parent, int sector);
void readFile(char *buffer, char *path, int *result, char parentIndex);
void writeFileOnly(char *buffer, char *filename, int *sectors, char parent);
void putArgs (char curdir, char argc, char * argv);
void getCurdir (char *curdir);
void getArgc (char *argc);
void setEmpty(char *current);
void getArgv (char index, char *argv);
void deleteFile(char *path, int *result, char parentIndex);
void deleteFileOnly(char* filename, char parent, int* success);
void deleteDirectory(char *path, int *success, char parentIndex);
void makeDirectory(char *path, int *result, char parentIndex);
void terminateProgram (int *result);
int strlen(char *buf);
void parsingDir(char *path, char *current, int *sCount);
int isDirExist(char *dir, char *current, char parentIndex);
int isFileExist(char *files, char *current, char parentIndex);

void main() {
  char stat, stat2, stat3, stat4;
  char name[8];
  char name2[8];
  char args[SECTOR_SIZE];
  char c;
  makeInterrupt21();
  stat = 1;
  readSector(args, ARGS_SECTOR);
  args[0] = 0xFF;
  writeSector(args, ARGS_SECTOR);
  executeProgram("shell", 0x2000, &stat, 0xFF);
  while(1);
}

int strlen(char *buf)
{
  int i=0;
  while(buf[i] != '\0')
  {
    i++;
  }
  return i;
}


void handleInterrupt21 (int AX, int BX, int CX, int DX) {
  char AL, AH;
  AL = (char) (AX);
  AH = (char) (AX >> 8);

  switch (AL) {
    case 0x00:
    printString(BX);
    break;
    case 0x01:
    readString(BX);
    break;
    case 0x02:
    readSector(BX, CX);
    break;
    case 0x03:
    writeSector(BX, CX);
    break;
    case 0x04:
    readFile(BX, CX, DX, AH);
    break;
    case 0x05:
    writeFile(BX, CX, DX, AH);
    break;
    case 0x06:
    executeProgram(BX, CX, DX, AH);
    break;
    case 0x07:
    terminateProgram(BX);
    break;
    case 0x08:
    makeDirectory(BX, CX, AH);
    break;
    case 0x09:
    deleteFile(BX, CX, AH);
    break;
    case 0x0A:
    deleteDirectory(BX, CX, AH);
    break;
    case 0x20:
    putArgs(BX, CX, DX);
    break;
    case 0x21:
    getCurdir(BX);
    break;
    case 0x22:
    getArgc(BX);
    break;
    case 0X23:
    getArgv(BX, CX);
    break;
    default:
    printString("Invalid interrupt");
  }
}

int compareString(char *str1, char *str2, int l_str)
{
  int i=0;
  for (i=0;i<l_str;i++)
  {
    if (str1[i]!=str2[i]) return FALSE;
    if(str1[i]==0) break;
  }
  return TRUE;
}

void printString(char *string) {
  int i = 0;
  while (string[i] != '\0') {
    interrupt(0x10, 0xE00 + string[i], 0, 0, 0);
    i++;
  }
}

void readString(char * string) {
  int i = 0;
  char c = interrupt(0x16, 0, 0, 0, 0);
  if (c != '\r'){
    while (c != '\r') {
     if (c == '\b' && i > 0) {
      interrupt(0x10, 0xE00 + '\b', 0,0,0);
      interrupt(0x10, 0xE00 + '\0',0,0,0);
      interrupt(0x10, 0xE00 + '\b', 0,0,0);
      i--;
      c = interrupt(0x16, 0, 0, 0, 0);
    } else {
      string[i] = c;
      interrupt(0x10, 0xE00 + string[i], 0, 0, 0);
      c = interrupt(0x16, 0, 0, 0, 0);
      i++;
    }
  }
}

string[i] = '\0';
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

void readSector(char * buffer, int sector) {
  interrupt(0x13, 0x201, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
}

void writeSector(char * buffer, int sector) {
  interrupt(0x13, 0x301, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
}

void clear(char *buffer, int length){
  int i;
  for(i = 0; i < length; ++i){
    buffer[i] = EMPTY;
  }
}

char checkEmptyEntrySlot(int sector) {
  char buffer[512];
  int i;
  readSector(buffer, sector);

  for (i = 1; i < 512; i += 16) {
    if (buffer[i] == '\0') {
      return TRUE;
    }
  }

  return FALSE;
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

void readFile(char *buffer, char *path, int *result, char parentIndex) {
  char dir[SECTOR_SIZE], files[SECTOR_SIZE], sectors[SECTOR_SIZE], map[SECTOR_SIZE];
  int i=0,j=0,z, sCount = 0;
  char current[MAX_DIRNAME];
  readSector(dir,DIRS_SECTOR);
  setEmpty(&current);
  parsingDir(path, &current, &sCount);
  if (sCount != 0)
  {
    i = isDirExist(&dir, &current, parentIndex);
    if (i >= MAX_DIRS)
    {
      printString("GA ADA DIRNYA\n\r");
      *result = NOT_FOUND;
      return;
    }
    readFile(buffer, path+sCount, result, i);
  }
  else
  {
    readSector(files, FILES_SECTOR);
    i = isFileExist(&files, &current, parentIndex);
    if (i >= MAX_FILES)
    {
      printString("FILE GA NEMU\n\r");
      *result = NOT_FOUND;
      return;
    }
    else
    {
      readSector(sectors, SECTORS_SECTOR);
      for(j=0;j<MAX_SECTORS&&sectors[i*FILE_ENTRY_LENGTH+j]!='\0';j++)
      {
        readSector(buffer+j*SECTOR_SIZE, sectors[i*FILE_ENTRY_LENGTH+j]);
      }
      *result = SUCCESSRESULT;
    }
  }
}

void writeFile(char *buffer, char *path, int *sectors, char parentIndex) {
  int i, j, k, emptySectorCount, fileIndex, dirNameLength;

  int neededSectorCount, fileIndexFound, done, dirNameFound, fileNameFound, dirFound, fileFound;

  char dirName[MAX_DIRNAME];
  char dirParent;
  
  char map[SECTOR_SIZE];
  char files[SECTOR_SIZE];
  char sectors[SECTOR_SIZE];
  char dirs[SECTOR_SIZE];

  neededSectorCount = strlen(buffer) / SECTOR_SIZE + 1;
  readSector(map, MAP_SECTOR);
  for (i = 50, emptySectorCount = 0; i < SECTOR_SIZE; i++) {
    if (map[i] == EMPTY) {
      emptySectorCount++;
    }
  }
  if (emptySectorCount < neededSectorCount) {
    *sectors = INSUFFICIENT_SECTORS;
    return;
  }
  readSector(files, FILES_SECTOR);
  fileIndexFound = FALSE; 
  for (i = 0; i < SECTOR_SIZE / FILE_ENTRY_LENGTH; i++) {
    if (files[i * FILE_ENTRY_LENGTH + 1] == '\0') {
      fileIndexFound = TRUE;
      fileIndex = i;
      break;
    }
  }
  if (!fileIndexFound) {
    *sectors = INSUFFICIENT_ENTRIES;
    return;
  }
  readSector(dirs, DIRS_SECTOR);

  i = 0;
  done = FALSE;
  dirNameLength = 0;

  while (!done) {
    j = 0;
    while (path[i] != '/' && path[i] != '\0') {
      dirName[j] = path[i];
      dirNameLength++;
      i++;
      j++;
    }

    if (path[i] == '/') {
      dirFound = FALSE;
      for (j = 0; j < SECTOR_SIZE / DIR_ENTRY_LENGTH; j++) {
        if (dirs[j * DIR_ENTRY_LENGTH] == parentIndex) {
          dirNameFound = TRUE;
          for (k = 0; k < dirNameLength; k++) {
            if (dirs[j * DIR_ENTRY_LENGTH + k + 1] != dirName[k]) {
              dirNameFound = FALSE;
              break;
            }
          }
          if (dirNameFound) {
            dirFound = TRUE;
            parentIndex = j;
            dirNameLength = 0;
            break;
          }
        }
      }
      if (!dirFound) {
        *sectors = NOT_FOUND;
        return;
      } else {
        i++;
      }
    } else {
      fileFound = FALSE;
      for (j = 0; j < SECTOR_SIZE / FILE_ENTRY_LENGTH; j++) {
        if (files[j * DIR_ENTRY_LENGTH] == parentIndex) {
          fileNameFound = TRUE;
          for (k = 0; k < dirNameLength; k++) {
            if (files[j * FILE_ENTRY_LENGTH + k + 1] != dirName[k]) {
              fileNameFound = FALSE;
              break;
            }
          }
          if (fileNameFound) {
            *sectors = ALREADY_EXISTS;
            return;
          }
        }
      }
      done = TRUE;
    }
  }
  dirName[dirNameLength] = '\0';
  dirNameLength += 1;
  files[fileIndex * FILE_ENTRY_LENGTH] = parentIndex;
  for (i = 0; i < dirNameLength; i++) {
    files[fileIndex * FILE_ENTRY_LENGTH + i + 1] = dirName[i];
  }
  writeSector(files, FILES_SECTOR);
  readSector(sectors, SECTORS_SECTOR);
  for (i = 0; i < neededSectorCount; i++) {
    j = 0;
    while (map[j] != EMPTY) {
      j++;
    }
    map[j] = USED;
    sectors[fileIndex * SECTORS_ENTRY_LENGTH + i] = j;
    writeSector(buffer + i * SECTOR_SIZE, j);
  }
  writeSector(map, MAP_SECTOR);
  writeSector(sectors, SECTORS_SECTOR);
  *sectors = SUCCESSRESULT;
}
void parsingDir(char *path, char *current, int *sCount)
{
  int i =0;
  while (path[i] != '\0')
  {
    if (path[i]=='/')
    {
      *sCount=i+1;
      break;
    }
    else
    {
      current[i]=path[i];
    }
    i++;
  }
}

void setEmpty(char *current)
{
  int z=0;
  for(z=0; z<=MAX_DIRNAME;z++)
  {
    current[z]='\0';
  }
}

int isDirExist(char *dir, char *current, char parentIndex)
{
  int i=0;
  for (i = 0; i < MAX_DIRS; i++)
  {
    if (dir[i * DIR_ENTRY_LENGTH] == parentIndex)
    {
      if (compareString(current, dir+i * DIR_ENTRY_LENGTH +1, MAX_DIRNAME))
      {
        break;
      }
    }
  }
  return i;
}

int isFileExist(char *files, char *current, char parentIndex)
{
  int i =0;
  for (i=0;i < MAX_FILES;i++)
  {
    if (files[i * FILE_ENTRY_LENGTH] == parentIndex)
    {
      if (compareString(current, files +i *FILE_ENTRY_LENGTH+1, MAX_FILENAME))
      {
        break;
      }
    }
  }
  return i;
}

void makeDirectory(char *path, int *result, char parentIndex) {
  char dir[SECTOR_SIZE];
  char files[SECTOR_SIZE];
  char current[MAX_DIRNAME];
  int i=0,j=0,z, sCount = 0;
  readSector(dir, DIRS_SECTOR);
  setEmpty(&current);
  for (i=0;i<MAX_DIRS;i++)
  {
    if (dir[i*DIR_ENTRY_LENGTH+1] == '\0')
    {
      break;
    }
  }
  if (i >=MAX_DIRS)
  {
    *result = INSUFFICIENT_ENTRIES;
    return;
  }
  parsingDir(path, &current, &sCount);
  if (sCount!=0)
  {
    i = isDirExist(&dir, &current, parentIndex);
    if (i >= MAX_DIRS)
    {
      *result = NOT_FOUND;
      return;
    }
    makeDirectory(path+sCount, result, i);
  }
  else
  {
    readSector(files, FILES_SECTOR);
    i = isFileExist(&files, &current, parentIndex);
    if (i < MAX_FILES)
    {
      *result = ALREADY_EXISTS;
      return;
    }
    for(i=0;i<MAX_DIRS;i++)
    {
      if(dir[i * DIR_ENTRY_LENGTH+1]=='\0')
      {
        dir[i*DIR_ENTRY_LENGTH]=parentIndex;
        for(j=0;j<MAX_DIRNAME;j++)
        {
          dir[i*DIR_ENTRY_LENGTH+1 +j] = current[j];
        }
        writeSector(dir, DIRS_SECTOR);
        *result = SUCCESSRESULT;
        return;
      }
    }
  }
}


void deleteFile(char *path, int *result, char parentIndex) {
  char dir[SECTOR_SIZE], files[SECTOR_SIZE], sectors[SECTOR_SIZE], map[SECTOR_SIZE];
  int i=0,j=0,z, sCount = 0;
  char current[MAX_DIRNAME];
  readSector(dir,DIRS_SECTOR);
  setEmpty(&current);
  parsingDir(path, &current, &sCount);
  if (sCount != 0)
  {
    i = isDirExist(&dir, &current, parentIndex);
    if (i >= MAX_DIRS)
    {
      *result = NOT_FOUND;
      return;
    }
    deleteFile(path+sCount, result, i);
  }
  else
  {
    readSector(files, FILES_SECTOR);
    i = isFileExist(&files, &current, parentIndex);
    if (i >= MAX_FILES)
    {
      *result = NOT_FOUND;
      return;
    }
    else
    {
      readSector(sectors, SECTORS_SECTOR);
      readSector(map, MAP_SECTOR);
      for (j=0; j< 16;j++)
      {
        if (files[i*FILE_ENTRY_LENGTH+j] == '\0') break;
        map[files[i*FILE_ENTRY_LENGTH+j]] = EMPTY;
        files[i*FILE_ENTRY_LENGTH+j] = '\0';
      }
      writeSector(map, MAP_SECTOR);
      writeSector(dir, DIRS_SECTOR);
      writeSector(files, FILES_SECTOR);
      *result = SUCCESSRESULT;
    }
  }

}


void deleteDirectory(char *path, int *success, char parentIndex) {
  char dir[SECTOR_SIZE], files[SECTOR_SIZE], map[SECTOR_SIZE],name[MAX_DIRNAME];
  char current[MAX_DIRNAME];
  int i=0,sCount=0,j=0,idx;
  readSector(dir, DIRS_SECTOR);
  setEmpty(&current);
  parsingDir(path, &current, &sCount);
  idx = isDirExist(&dir, &current, parentIndex);
  if (idx>=MAX_DIRS)
  {
    *success= NOT_FOUND;
    return;
  }
  if (sCount!=0)
  {
    if (i >= MAX_DIRS)
    {
      *success = NOT_FOUND;
      return;
    }
    deleteDirectory(path+sCount, success, idx);
  }
  else
  {
    readSector(files, FILES_SECTOR);
    for (i=0; i< MAX_DIRS;i++)
    {
      if (dir[i * DIR_ENTRY_LENGTH] == idx)
      {
        deleteDirectory(dir+i*DIR_ENTRY_LENGTH+1, success, idx);
        readSector(dir, DIRS_SECTOR);
      }
    }
    for (i=0;i<MAX_FILES;i++)
    {
      if (files[i*FILE_ENTRY_LENGTH] == idx)
      {
        deleteFile(files+i*FILE_ENTRY_LENGTH+1, success, idx);
        readSector(files, FILES_SECTOR);
      }
    }
    for (i=0;i<MAX_DIRS;i++)
    {
      if(dir[i * DIR_ENTRY_LENGTH] == parentIndex && compareString(current, dir+i*DIR_ENTRY_LENGTH+1, MAX_DIRNAME))
      {
        for (j=i;j<MAX_DIRNAME;j++)
        {
          dir[i*DIR_ENTRY_LENGTH+j]='\0';
        }
      }
    }
    writeSector(dir, DIRS_SECTOR);
    *success =SUCCESSRESULT;
  }
}

void putArgs (char curdir, char argc, char * argv) {
  char args[SECTOR_SIZE];
  int i, j, p;
  clear(args, SECTOR_SIZE);

  args[0] = curdir;
  args[1] = argc;
  i = 0;
  j = 0;
  for (p = 2; p < SECTOR_SIZE; p++) {
    args[p] = argv[i];
    if (args[p] == '\0') {
      break;
    }
    i++;
  }

  writeSector(args, ARGS_SECTOR);
}

void getCurdir (char *curdir) {
  char args[SECTOR_SIZE];
  readSector(args, ARGS_SECTOR);
  *curdir = args[0];
}


void getArgc (char *argc) {
  char args[SECTOR_SIZE];
  readSector(args, ARGS_SECTOR);
  *argc = args[1];
}

void getArgv (char index, char *argv) {
  char args[SECTOR_SIZE];
  int i, j, p;
  clear(args, SECTOR_SIZE);
  readSector(args, ARGS_SECTOR);

  i = 0;
  j = 0;
  for (p = 2; p < SECTOR_SIZE; p++) {
    if (args[p] == ' ') {
      if (i == index) {
        break;
      } else {
        i++;
      }
    }
    if (i == index && args[p] != ' ') {
      argv[j] = args[p];
      j++;
    }

    if (args[p] == '\0') {
      break;
    }
  }
}

void executeProgram(char *path, int segment, int *result, char parentIndex){
  int i;
  char buffer[MAX_SECTORS * SECTOR_SIZE];
  readFile(buffer, path, result, parentIndex);
  if (*result == 0){
    for (i = 0; i < (MAX_SECTORS * SECTOR_SIZE); i++){
      putInMemory(segment, i, buffer[i]);
    }
    launchProgram(segment);
  }
}

void terminateProgram (int *result) {
  char shell[6];
  shell[0] = 's';
  shell[1] = 'h';
  shell[2] = 'e';
  shell[3] = 'l';
  shell[4] = 'l';
  shell[5] = '\0';
  executeProgram(shell, 0x2000, result, 0xFF);
}
