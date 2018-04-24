#define MAIN
#include "proc.h"

#define MAX_BYTE 512
#define SECTOR_SIZE 512
#define MAX_FILES 32
#define MAX_FILENAME 15
#define MAX_DIRNAME 16
#define MAX_DIRS 16
#define MAX_SECTORS 16
#define DIR_ENTRY_LENGTH 16
#define FILE_ENTRY_LENGTH 16
#define MAP_SECTOR 0x100
#define DIRS_SECTOR 0x101
#define FILES_SECTOR 0x102
#define SECTORS_SECTOR 0x103
#define TRUE 1
#define FALSE 0
#define INSUFFICIENT_SECTORS 0
#define NOT_FOUND -1
#define INSUFFICIENT_DIR_ENTRIES -1
#define EMPTY 0x00
#define USED 0xFF
#define ARGS_SECTOR 512
#define SUCCESS 0
#define INSUFFICIENT_SEGMENTS -1

void handleInterrupt21(int AX, int BX, int CX, int DX);
int isDirExist(char *dir, char *current, char parentIndex);
void printString(char *string);
void readString(char *string, int disableProcessControls);
int mod(int a, int b);
int div(int a, int b);
void readSector(char *buffer, int sector);
void writeSector(char *buffer, int sector);
void clear(char *buffer, int length);
void writeFile(char *buffer, char *path, int *sectors, char parentIndex);
void executeProgram(char *path, int *result, char parentIndex, int isParalel);
int getCharByIndex(char *buffer, char c);
void findDirectory(char *path, int *result, char *parentIndex);
void readByFilename(char *buffer, char *filename, int parent, int *success);
char getIndex(char *name, char parent, int sector);
void readFile(char *buffer, char *path, int *result, char parentIndex);
void writeToFilename(char *buffer, char *filename, int *sectors, char parent);
void putArgs(char currDir, char argc, char *argv);
void getCurrDir(char *currDir);
void getArgc(char *argc);
void getArgv(char index, char *argv);
void deleteFile(char *path, int *result, char parentIndex);
void delFile(char *filename, char parent, int *success);
void deleteDirectory(char *path, int *success, char parentIndex);
void makeDirectory(char *path, int *result, char parentIndex);
void terminateProgram(int *result);
void copyStr(char *buffer, char *string);
char checkEmptySpace(int sector);
void handleTimerInterrupt(int segment, int stackPointer);
void yieldControl();
void sleep();
void pauseProcess(int segment, int *result);
void resumeProcess(int segment, int *result);
void killProcess(int segment, int *result);
void getPCBInfo(int idx, int *pid, int *state);
void setEmpty(char *current);
void parsingDir(char *path, char *current, int *sCount);
int isFileExist(char *files, char *current, char parentIndex);
int compareString(char *str1, char *str2, int l_str);

void main()
{
  char stat, stat2, stat3, stat4;
  char name[8];
  char name2[8];
  char args[SECTOR_SIZE];
  char c;
  initializeProcStructures();
  readSector(args, ARGS_SECTOR);
  args[0] = 0xFF;
  writeSector(args, ARGS_SECTOR);
  makeInterrupt21();
  makeTimerInterrupt();
  executeProgram("shell", &stat, 0xFF, TRUE);
  while (1);
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

void clear(char *buffer, int length)
{
  int i;
  for (i = 0; i < length; ++i)
  {
    buffer[i] = EMPTY;
  }
}

void copyStr(char *buffer, char *string)
{
  int i;
  i = 0;
  while (string[i] != '\0')
  {
    buffer[i] = string[i];
    i++;
  }
  buffer[i] = '\0';
}

int getCharByIndex(char *buffer, char c)
{
  int i = 0;
  while (buffer[i] != '\0')
  {
    if (c == buffer[i])
    {
      return i;
    }
    i++;
  }
  return -1;
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

void setEmpty(char *current)
{
  int z=0;
  for(z=0; z<=MAX_DIRNAME;z++)
  {
    current[z]='\0';
  }
}

void getArgc(char *argc)
{
  char args[SECTOR_SIZE];
  readSector(args, ARGS_SECTOR);
  *argc = args[1];
}

void getArgv(char index, char *argv)
{
  char args[SECTOR_SIZE];
  int i, j, p;
  clear(args, SECTOR_SIZE);
  readSector(args, ARGS_SECTOR);

  i = 0;
  j = 0;
  for (p = 2; p < SECTOR_SIZE; p++)
  {
    if (args[p] == ' ')
    {
      if (i == index)
      {
        break;
      }
      else
      {
        i++;
      }
    }
    if (i == index && args[p] != ' ')
    {
      argv[j] = args[p];
      j++;
    }

    if (args[p] == '\0')
    {
      break;
    }
  }
}

void handleTimerInterrupt(int segment, int stackPointer)
{
  struct PCB *currPCB;
  struct PCB *nextPCB;

  setKernelDataSegment();
  currPCB = getPCBOfSegment(segment);
  currPCB->stackPointer = stackPointer;
  if (currPCB->state != PAUSED)
  {
    currPCB->state = READY;
    addToReady(currPCB);
  }

  do
  {
    nextPCB = removeFromReady();
  } while (nextPCB != NULL && (nextPCB->state == DEFUNCT || nextPCB->state == PAUSED));

  if (nextPCB != NULL)
  {
    nextPCB->state = RUNNING;
    segment = nextPCB->segment;
    stackPointer = nextPCB->stackPointer;
    running = nextPCB;
  }
  else
  {
    running = &idleProc;
  }
  restoreDataSegment();

  returnFromTimer(segment, stackPointer);
}

void yieldControl()
{
  interrupt(0x08, 0, 0, 0, 0);
}

void sleep()
{
  setKernelDataSegment();
  running->state = PAUSED;
  restoreDataSegment();
  yieldControl();
}

void pauseProcess(int segment, int *result)
{
  struct PCB *pcb;
  int res;

  setKernelDataSegment();
  pcb = getPCBOfSegment(segment);
  if (pcb != NULL && pcb->state != PAUSED)
  {
    pcb->state = PAUSED;
    res = SUCCESS;
  }
  else
  {
    res = NOT_FOUND;
  }
  restoreDataSegment();

  *result = res;
}

void resumeProcess(int segment, int *result)
{
  struct PCB *pcb;
  int res;

  setKernelDataSegment();
  pcb = getPCBOfSegment(segment);
  if (pcb != NULL && pcb->state == PAUSED)
  {
    pcb->state = READY;
    addToReady(pcb);
    res = SUCCESS;
  }
  else
  {
    res = NOT_FOUND;
  }
  restoreDataSegment();

  *result = res;
}

void killProcess(int segment, int *result)
{
  struct PCB *pcb;
  int res;

  setKernelDataSegment();
  pcb = getPCBOfSegment(segment);
  if (pcb != NULL)
  {
    releaseMemorySegment(pcb->segment);
    releasePCB(pcb);
    res = SUCCESS;
  }
  else
  {
    res = NOT_FOUND;
  }
  restoreDataSegment();

  *result = res;
}

void executeProgram(char *path, int *result, char parentIndex, int isParalel)
{
  struct PCB *pcb;
  int segment;
  int i, fileIndex;
  char buffer[MAX_SECTORS * SECTOR_SIZE];
  readFile(buffer, path, result, parentIndex);

  if (*result != NOT_FOUND)
  {
    setKernelDataSegment();
    segment = getFreeMemorySegment();
    restoreDataSegment();
    fileIndex = *result;
    // printString("IS IT HERE YET??\n");
    if (segment != NO_FREE_SEGMENTS)
    {
      setKernelDataSegment();
      pcb = getFreePCB();
      pcb->index = fileIndex;
      pcb->state = STARTING;
      pcb->segment = segment;
      pcb->stackPointer = 0xFF00;
      pcb->parentSegment = running->segment;
      addToReady(pcb);
      restoreDataSegment();
      // printString("IS IT HERE YET??\n");
      for (i = 0; i < SECTOR_SIZE * MAX_SECTORS; i++)
      {
        putInMemory(segment, i, buffer[i]);
      }
      initializeProgram(segment);
      if (isParalel == 0)
      {
        sleep();
      }
    }
    else
    {
      // printString("OH SHIT??\n");
      *result = INSUFFICIENT_SEGMENTS;
    }
  }
}
void terminateProgram(int *result)
{
  int parentSegment;
  setKernelDataSegment();
  parentSegment = running->parentSegment;
  releaseMemorySegment(running->segment);
  releasePCB(running);
  restoreDataSegment();
  if (parentSegment != NO_PARENT)
  {
    resumeProcess(parentSegment, result);
  }
  yieldControl();
}

void getPCBInfo(int idx, int *pid, int *state)
{
  int seg;
  int st;
  setKernelDataSegment();
  seg = pcbPool[idx].segment;
  st = pcbPool[idx].state;
  restoreDataSegment();

  *pid = (seg >> 12) - 2;
  *state = st;
}

void handleInterrupt21(int AX, int BX, int CX, int DX)
{
  char AL, AH;
  AL = (char)(AX);
  AH = (char)(AX >> 8);

  switch (AL)
  {
    case 0x00:
    printString(BX);
    break;
    case 0x01:
    readString(BX, CX);
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
    executeProgram(BX, CX, AH, DX);
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
    getCurrDir(BX);
    break;
    case 0x22:
    getArgc(BX);
    break;
    case 0X23:
    getArgv(BX, CX);
    break;
    case 0x24:
    getPCBInfo(BX, CX, DX);
    break;
    case 0x30:
    yieldControl();
    break;
    case 0x31:
    sleep();
    break;
    case 0x32:
    pauseProcess(BX, CX);
    break;
    case 0x33:
    resumeProcess(BX, CX);
    break;
    case 0x34:
    killProcess(BX, CX);
    break;

    default:
    printString("Invalid interrupt");
  }
}

void printString(char *string)
{
  int i = 0;
  while (string[i] != '\0')
  {
    interrupt(0x10, 0xE00 + string[i], 0, 0, 0);
    i++;
  }
}

void readString(char *string, int disableProcessControls)
{
  int i;
  int stat, parent;
  char c = interrupt(0x16, 0, 0, 0, 0);
  i=0;
  while (c != '\r')
  {
    if (c == '\b')
    {
      interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
      interrupt(0x10, 0xE00 + '\0', 0, 0, 0);
      interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
      string[i] = '\0';
      i--;
    }
    else
    {
      if (c == 0x03 && disableProcessControls == FALSE)
      {
        terminateProgram(&stat);
      } 
      else if (c == 0x1A && disableProcessControls == FALSE)
      {
        setKernelDataSegment();
        parent = running->parentSegment;
        restoreDataSegment();
        resumeProcess(parent, &stat);
        sleep();
      }
      else
      {
        interrupt(0x10, 0xE00 + c, 0, 0, 0);
        string[i] = c;
        i++;
      }
    }

    c = interrupt(0x16, 0, 0, 0, 0);
  }
  string[i] = '\0';
  interrupt(0x10, 0xE00 + '\n', 0, 0, 0);
  interrupt(0x10, 0xE00 + '\r', 0, 0, 0);
}

void readSector(char *buffer, int sector)
{
  interrupt(0x13, 0x201, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
}

void writeSector(char *buffer, int sector)
{
  interrupt(0x13, 0x301, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
}

char checkEmptySpace(int sector)
{
  char buffer[512];
  int i;
  readSector(buffer, sector);

  for (i = 1; i < 512; i += 16)
  {
    if (buffer[i] == '\0')
    {
      return TRUE;
    }
  }
  return FALSE;
}

void readByFilename(char *buffer, char *filename, int parent, int *success)
{
  char dir[512];
  char file[512];
  int i, j, k, count;

  readSector(dir, FILES_SECTOR);

  // printString("IS IT HERE YET??\n");
  for (i = 0; i < 512; i += 16)
  {
    count = 0;
    if (dir[i] != parent)
    {
      continue;
    }
    if (dir[i + 1] == '\0')
    {
      continue;
    }
    for (j = 0; j < 15; j++)
    {
      // printString("IS IT HERE YET??\n");
      if (filename[j] == '\0' && dir[i + j + 1] == '\0')
      {
        count = 15;
        break;
      }
      if (filename[j] != dir[i + j + 1])
      {
        break;
      }
      count++;
    }
    if (count == 15)
    {
      *success = i / 16;
      readSector(file, SECTORS_SECTOR);
      for (k = 0; k < 16; k++)
      {
        // printString("IS IT HERE YET??\n");
        if (file[i + k] == 0)
          break;
        readSector(buffer + k * SECTOR_SIZE, file[i + k]);
      }
      return;
    }
  }
  *success = -1;
}

char getIndex(char *name, char parent, int sector)
{
  char dir[512];
  int i, j, k, count;
  char result;
  readSector(dir, sector);

  for (i = 0; i < 512; i += 16)
  {
    if (dir[i] != parent)
    {
      continue;
    }
    if (dir[i + 1] == '\0')
    {
      continue;
    }

    count = 0;
    if (dir[i] != parent)
    {
      continue;
    }
    for (j = 0; j < 15; j++)
    {
      if (name[j] == '\0' && dir[i + j + 1] == '\0')
      {
        count = 15;
        break;
      }
      if (name[j] != dir[i + j + 1])
      {
        break;
      }
      count++;
    }
    result = count == 15;
    if (result)
    {
      return i / 16;
    }
  }
  return -2;
}

void findDirectory(char *path, int *result, char *parentIndex)
{
  char dir[512];
  char name[15];
  int i, j, k, count;
  int len;

  len = getCharByIndex(path, '/');

  if (len == 0)
  {
    // printString("IS IT HERE YET??\n");
    while (path[i + len] != '\0')
    {
      path[i] = path[i + len + 1];
      i++;
    }
    return findDirectory(path, result, parentIndex);
  }
  if (len != -1)
  {
    for (i = 0; i < len; i++)
    {
      name[i] = path[i];
    }
    name[i] = '\0';

    *parentIndex = getIndex(name, *parentIndex, DIRS_SECTOR);

    if (*parentIndex == 0xFE)
    {
      *result = -1;
      return;
    }
    else
    {
      i = 0;
      // printString("IS IT HERE YET??\n");
      while (path[i + len] != '\0')
      {
        path[i] = path[i + len + 1];
        i++;
      }
      return findDirectory(path, result, parentIndex);
    }
  }
  else
  {
    *result = 0;
    return;
  }
}

void writeToFilename(char *buffer, char *filename, int *sectors, char parent)
{
  char map[SECTOR_SIZE];
  char dir[SECTOR_SIZE];
  char sector[SECTOR_SIZE];
  char sectorBuffer[SECTOR_SIZE];
  int dirIdx;
  char filled;
  int count;
  int i, j, sectorCount;

  readSector(map, MAP_SECTOR);
  readSector(dir, FILES_SECTOR);
  readSector(sector, SECTORS_SECTOR);

  count = 0;
  i = 0;
  while (buffer[i] != '\0')
  {
    count++;
    i++;
  }

  for (dirIdx = 0; dirIdx < MAX_FILES; ++dirIdx)
  {
    if (dir[dirIdx * DIR_ENTRY_LENGTH + 1] == '\0')
    {
      break;
    }
  }

  if (dirIdx < MAX_FILES)
  {
    for (i = 0, sectorCount = 0; i < MAX_BYTE && sectorCount < (count + 1) / SECTOR_SIZE + 1; ++i)
    {
      if (map[i] == EMPTY)
      {
        ++sectorCount;
      }
    }
    // printString("IS IT HERE YET??\n");
    if (sectorCount < (count + 1) / SECTOR_SIZE + 1)
    {
      *sectors = INSUFFICIENT_SECTORS;
      return;
    }
    else
    {
      clear(dir + dirIdx * DIR_ENTRY_LENGTH, DIR_ENTRY_LENGTH);
      dir[dirIdx * DIR_ENTRY_LENGTH] = parent;
      for (i = 0; i < MAX_FILENAME; i++)
      {
        if (filename[i] != '\0')
        {
          dir[dirIdx * DIR_ENTRY_LENGTH + i + 1] = filename[i];
        }
        else
        {
          break;
        }
      }
      dir[dirIdx * DIR_ENTRY_LENGTH + i + 1] = '\0';
      filled = FALSE;
      for (i = 0, sectorCount = 0; i < MAX_BYTE && sectorCount < (count + 1) / SECTOR_SIZE + 1; ++i)
      {
        if (map[i] == EMPTY)
        {
          map[i] = USED;
          sector[dirIdx * DIR_ENTRY_LENGTH + sectorCount] = i;
          clear(sectorBuffer, SECTOR_SIZE);
          for (j = 0; j < SECTOR_SIZE; ++j)
          {
            sectorBuffer[j] = buffer[sectorCount * SECTOR_SIZE + j];
            if (sectorBuffer[j] == '\0')
            {
              filled = TRUE;
            }
          }
          writeSector(sectorBuffer, i);
          if (filled)
          {
            break;
          }
          ++sectorCount;
        }
      }
    }
  }
  else
  {
    // printString("OH MY LORDDD NOT ENOUGH DIR??\n");
    *sectors = INSUFFICIENT_DIR_ENTRIES;
    return;
  }
  writeSector(map, MAP_SECTOR);
  writeSector(dir, FILES_SECTOR);
  writeSector(sector, SECTORS_SECTOR);
}

void readFile(char *buffer, char *path, int *result, char parentIndex) {
  char nparent;
  char npath[100];
  char index, c;
  nparent = parentIndex;
  copyStr(npath, path);
  // printString("Start looking for dir??\n");
  findDirectory(npath, result, &nparent);
  if (*result != 0)
  {
    *result = -1;
    return;
  }
  index = getIndex(npath, nparent, FILES_SECTOR);
  c = 0xFE;
  if (c == index)
  {
    *result = -1;
    return;
  }
  if (*result != -1)
  {
    // printString("Already last path??\n");
    readByFilename(buffer, npath, nparent, result);
  }
}

void writeFile(char *buffer, char *path, int *sectors, char parentIndex)
{
  char nparent;
  char npath[100];
  int result;
  char c, index;
  char success;
  nparent = parentIndex;
  if (!checkEmptySpace(FILES_SECTOR))
  {
    *sectors = -3;
    return;
  }
  copyStr(npath, path);

  findDirectory(npath, &success, &nparent);
  if (success != 0)
  {
    *sectors = -1;
    return;
  }
  index = getIndex(npath, nparent, FILES_SECTOR);
  c = 0xFE;
  if (c != index)
  {
    *sectors = -2;
    return;
  }
  writeToFilename(buffer, npath, sectors, nparent);
}

void getCurrDir(char *currDir)
{
  char args[SECTOR_SIZE];
  readSector(args, ARGS_SECTOR);
  *currDir = args[0];
}

void makeDirectory(char *path, int *result, char parentIndex)
{
  char buffer[512];
  char npath[100];
  char parent;
  int i, j;
  char index, curr;
  parent = parentIndex;

  copyStr(npath, path);
  if (!checkEmptySpace(DIRS_SECTOR))
  {
    *result = -3;
    return;
  }

  findDirectory(npath, result, &parent);
  if (*result != 0)
  {
    *result = -1;
    return;
  }
  index = getIndex(npath, parent, DIRS_SECTOR);
  curr = -2;
  if (curr != index)
  {
    *result = -2;
    return;
  }

  if (*result == 0)
  {
    *result = 0;
    readSector(buffer, DIRS_SECTOR);
    for (i = 0; i < SECTOR_SIZE; i += 16)
    {
      if (buffer[i + 1] == '\0')
      {
        buffer[i] = parent;
        for (j = i + 1; j < i + 15; j++)
        {
          buffer[j] = npath[j - i - 1];
          if (buffer[j] == '\0')
          {
            break;
          }
        }
        writeSector(buffer, DIRS_SECTOR);
        break;
      }
    }
  }
}

void deleteFile(char *path, int *result, char parentIndex)
{
  char parent;
  char npath[100];
  char index, curr;
  parent = parentIndex;
  copyStr(npath, path);
  findDirectory(npath, result, &parent);
  if (*result != 0)
  {
    *result = -1;
    return;
  }
  index = getIndex(npath, parent, FILES_SECTOR);
  curr = 0xFE;
  if (curr == index)
  {
    *result = -1;
    return;
  }

  if (*result != -1)
  {
    delFile(npath, parent, result);
  }
}

void delFile(char *filename, char parent, int *success)
{
  char dir[512];
  char file[512];
  char map[512];
  int i, j, k, count;

  readSector(dir, FILES_SECTOR);

  for (i = 0; i < 512; i += 16)
  {
    count = 0;
    if (dir[i] != parent)
    {
      continue;
    }
    if (dir[i + 1] == '\0')
    {
      continue;
    }
    for (j = 0; j < 15; j++)
    {
      if (filename[j] == '\0' && dir[i + j + 1] == '\0')
      {
        count = 15;
        break;
      }
      if (filename[j] != dir[i + j + 1])
      {
        break;
      }
      count++;
    }
    if (count == 15)
    {
      *success = 0;
      dir[i + 1] = '\0';
      dir[i] = '\0';
      readSector(file, SECTORS_SECTOR);
      readSector(map, MAP_SECTOR);
      for (k = 0; k < 16; k++)
      {
        if (file[i + k] == '\0')
        {
          break;
        }
        map[file[i + k]] = EMPTY;
        file[i + k] = '\0';
      }
      writeSector(map, MAP_SECTOR);
      writeSector(dir, FILES_SECTOR);
      writeSector(file, SECTORS_SECTOR);
      break;
    }
  }
}

void deleteDirectory(char *path, int *success, char parentIndex)
{
  char parent;
  char npath[100];
  char dirIdx;
  int i, j;
  char files[512];
  char name[16];
  char dir[512];
  char found;
  parent = parentIndex;
  copyStr(npath, path);
  findDirectory(npath, success, &parent);

  if (*success != -1)
  {
    dirIdx = getIndex(npath, parent, DIRS_SECTOR);
    if (dirIdx == -2)
    {
      *success = -1;
      return;
    }

    readSector(dir, DIRS_SECTOR);
    for (i = 0; i < SECTOR_SIZE; i += 16)
    {
      if (dir[i] == dirIdx && dir[i + 1] != '\0')
      {
        for (j = i + 1; j < i + 15; j++)
        {
          name[j - i - 1] = dir[j];
          if (dir[j] == '\0')
          {
            break;
          }
        }
        printString(name);
        deleteDirectory(name, success, dirIdx);
      }
    }

    readSector(files, FILES_SECTOR);
    for (i = 0; i < SECTOR_SIZE; i += 16)
    {
      if (files[i] == dirIdx && files[i + 1] != '\0')
      {
        for (j = i + 1; j < i + 15; j++)
        {
          name[j - i - 1] = files[j];
          if (files[j] == '\0')
          {
            break;
          }
        }
        printString(name);
        delFile(name, dirIdx, success);
      }
    }

    readSector(dir, DIRS_SECTOR);
    dir[dirIdx * 16 + 1] = '\0';
    dir[dirIdx * 16] = '\0';
    writeSector(dir, DIRS_SECTOR);
    *success = 0;
  }
}

void putArgs(char currDir, char argc, char *argv)
{
  char args[SECTOR_SIZE];
  int i, j, p;
  clear(args, SECTOR_SIZE);

  args[0] = currDir;
  args[1] = argc;
  i = 0;
  j = 0;
  for (p = 2; p < SECTOR_SIZE; p++)
  {
    args[p] = argv[i];
    if (args[p] == '\0')
    {
      break;
    }
    i++;
  }
  writeSector(args, ARGS_SECTOR);
}
