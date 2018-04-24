#define MAIN
#include "proc.h"

#define MAX_BYTE 512
#define SECTOR_SIZE 512
#define MAX_FILES 32
#define MAX_FILENAME 15
#define MAX_SECTORS 16
#define DIR_ENTRY_LENGTH 16
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
void printString(char *string);
void readString(char *string, int disableProcessControls);
int mod(int a, int b);
int div(int a, int b);
void readSector(char *buffer, int sector);
void writeSector(char *buffer, int sector);
void clear(char *buffer, int length);
void writeFile(char *buffer, char *path, int *sectors, char parentIndex);
void executeProgram(char *path, int *result, char parentIndex, int isParalel);
int getCharFromString(char *buffer, char c);
void traversalDirectory(char *path, int *result, char *parentIndex);
void RekursifReadFile(char *buffer, char *filename, int parent, int *success);
char getIndex(char *name, char parent, int sector);
void readFile(char *buffer, char *path, int *result, char parentIndex);
void writeFileOnly(char *buffer, char *filename, int *sectors, char parent);
void putArgs(char currDirectory, char argc, char *argv);
void getcurrDirectory(char *currDirectory);
void getArgc(char *argc);
void getArgv(char index, char *argv);
void deleteFile(char *path, int *result, char parentIndex);
void delFile(char *filename, char parent, int *success);
void deleteDirectory(char *path, int *success, char parentIndex);
void makeDirectory(char *path, int *result, char parentIndex);
void terminateProgram(int *result);
void convertStringToBuffer(char *buffer, char *string);
char checkEmptySpace(int sector);
void handleTimerInterrupt(int segment, int stackPointer);
void yieldControl();
void sleep();
void pauseProcess(int segment, int *result);
void resumeProcess(int segment, int *result);
void killProcess(int segment, int *result);
void getPCBInfo(int idx, int *pid, int *state);

void main()
{
    int success;
    char args[SECTOR_SIZE];
    initializeProcStructures();
    readSector(args, ARGS_SECTOR);
    args[0] = 0xFF;
    writeSector(args, ARGS_SECTOR);
    makeInterrupt21();
    makeTimerInterrupt();
    executeProgram("shell", &success, 0xFF, TRUE);
    while (1)
        ;
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
        getcurrDirectory(BX);
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
    char c;
    int i;
    int success, parent;
    i = 0;
    c = interrupt(0x16, 0, 0, 0, 0);
    while (c != '\r')
    {
        // jika c == enter
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
            // Jika c == ctrl+z && bukan sedang shell, terminate program tersebut
            if (c == 3 && disableProcessControls == FALSE)
            {
                terminateProgram(&success);
            }
            // Jika c = ctrl+c && bukan sedang shell, resume shell & sleep program tersebut
            else if (c == 0x1A && disableProcessControls == FALSE)
            {
                setKernelDataSegment();
                parent = running->parentSegment;
                restoreDataSegment();
                resumeProcess(parent, &success);
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
void readSector(char *buffer, int sector)
{
    interrupt(0x13, 0x201, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
}
void writeSector(char *buffer, int sector)
{
    interrupt(0x13, 0x301, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
}
void clear(char *buffer, int length)
{
    int i;
    for (i = 0; i < length; ++i)
    {
        buffer[i] = EMPTY;
    }
}
void convertStringToBuffer(char *buffer, char *string)
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

void RekursifReadFile(char *buffer, char *filename, int parent, int *success)
{
    char dir[512];
    char file[512];
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
            *success = i / 16;
            readSector(file, SECTORS_SECTOR);
            for (k = 0; k < 16; k++)
            {
                if (file[i + k] == 0)
                    break;
                readSector(buffer + k * SECTOR_SIZE, file[i + k]);
            }
            return;
        }
    }
    *success = -1;
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

void traversalDirectory(char *path, int *result, char *parentIndex)
{
    char dir[512];
    char name[15];
    int i, j, k, count;
    int len;

    len = getCharFromString(path, '/');

    if (len == 0)
    {
        while (path[i + len] != '\0')
        {
            path[i] = path[i + len + 1];
            i++;
        }
        return traversalDirectory(path, result, parentIndex);
    }
    if (len != -1)
    {
        for (i = 0; i < len; i++)
        {
            name[i] = path[i];
        }
        name[i] = '\0';

        *parentIndex = getIndex(name, *parentIndex, DIRS_SECTOR);

        if (*parentIndex == 0xFF)
        {
            printString("Parent Index == 0xFF\r\n");
        }

        if (*parentIndex == 0xFE)
        {
            *result = -1;
            return;
        }
        else
        {
            i = 0;
            while (path[i + len] != '\0')
            {
                path[i] = path[i + len + 1];
                i++;
            }
            return traversalDirectory(path, result, parentIndex);
        }
    }
    else
    {
        *result = 0;
        return;
    }
}

void writeFileOnly(char *buffer, char *filename, int *sectors, char parent)
{
    char map[SECTOR_SIZE];
    char dir[SECTOR_SIZE];
    char sector[SECTOR_SIZE];
    char sectorBuffer[SECTOR_SIZE];
    int dirIndex;
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

    for (dirIndex = 0; dirIndex < MAX_FILES; ++dirIndex)
    {
        if (dir[dirIndex * DIR_ENTRY_LENGTH + 1] == '\0')
        {
            break;
        }
    }

    if (dirIndex < MAX_FILES)
    {
        for (i = 0, sectorCount = 0; i < MAX_BYTE && sectorCount < (count + 1) / SECTOR_SIZE + 1; ++i)
        {
            if (map[i] == EMPTY)
            {
                ++sectorCount;
            }
        }
        if (sectorCount < (count + 1) / SECTOR_SIZE + 1)
        {
            *sectors = INSUFFICIENT_SECTORS;
            return;
        }
        else
        {
            clear(dir + dirIndex * DIR_ENTRY_LENGTH, DIR_ENTRY_LENGTH);

            dir[dirIndex * DIR_ENTRY_LENGTH] = parent;
            for (i = 0; i < MAX_FILENAME; i++)
            {
                if (filename[i] != '\0')
                {
                    dir[dirIndex * DIR_ENTRY_LENGTH + i + 1] = filename[i];
                }
                else
                {
                    break;
                }
            }
            dir[dirIndex * DIR_ENTRY_LENGTH + i + 1] = '\0';

            filled = FALSE;
            for (i = 0, sectorCount = 0; i < MAX_BYTE && sectorCount < (count + 1) / SECTOR_SIZE + 1; ++i)
            {
                if (map[i] == EMPTY)
                {
                    map[i] = USED;
                    sector[dirIndex * DIR_ENTRY_LENGTH + sectorCount] = i;
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
        *sectors = INSUFFICIENT_DIR_ENTRIES;
        return;
    }

    writeSector(map, MAP_SECTOR);
    writeSector(dir, FILES_SECTOR);
    writeSector(sector, SECTORS_SECTOR);
}

void readFile(char *buffer, char *path, int *result, char parentIndex)
{
    char parentNew;
    char pathNew[100];
    char index, c;
    parentNew = parentIndex;
    convertStringToBuffer(pathNew, path);
    traversalDirectory(pathNew, result, &parentNew);
    if (*result != 0)
    {
        *result = -1;
        return;
    }
    index = getIndex(pathNew, parentNew, FILES_SECTOR);
    c = 0xFE;
    if (c == index)
    {
        *result = -1;
        return;
    }

    if (*result != -1)
    {
        RekursifReadFile(buffer, pathNew, parentNew, result);
    }
}

void writeFile(char *buffer, char *path, int *sectors, char parentIndex)
{
    char parentNew;
    char pathNew[100];
    int result;
    char c, index;
    char success;
    parentNew = parentIndex;
    if (!checkEmptySpace(FILES_SECTOR))
    {
        *sectors = -3;
        return;
    }
    convertStringToBuffer(pathNew, path);

    traversalDirectory(pathNew, &success, &parentNew);
    if (success != 0)
    {
        printString("GAGAL");
        *sectors = -1;
        return;
    }

    index = getIndex(pathNew, parentNew, FILES_SECTOR);

    c = 0xFE;
    if (c != index)
    {
        printString("UDAH ADA");
        *sectors = -2;
        return;
    }

    writeFileOnly(buffer, pathNew, sectors, parentNew);
}

void makeDirectory(char *path, int *result, char parentIndex)
{
    char buffer[512];
    char pathNew[100];
    char parent;
    int i, j;
    char index, curr;
    parent = parentIndex;

    convertStringToBuffer(pathNew, path);
    if (!checkEmptySpace(DIRS_SECTOR))
    {
        *result = -3;
        return;
    }

    traversalDirectory(pathNew, result, &parent);
    if (*result != 0)
    {
        *result = -1;
        return;
    }
    index = getIndex(pathNew, parent, DIRS_SECTOR);
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
                    buffer[j] = pathNew[j - i - 1];
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
    char pathNew[100];
    char index, curr;
    parent = parentIndex;
    convertStringToBuffer(pathNew, path);
    traversalDirectory(pathNew, result, &parent);
    if (*result != 0)
    {
        *result = -1;
        return;
    }
    index = getIndex(pathNew, parent, FILES_SECTOR);
    curr = 0xFE;
    if (curr == index)
    {
        *result = -1;
        return;
    }

    if (*result != -1)
    {
        delFile(pathNew, parent, result);
    }
}

void deleteDirectory(char *path, int *success, char parentIndex)
{
    char parent;
    char pathNew[100];
    char dirIndex;
    int i, j;
    char files[512];
    char name[16];
    char dir[512];
    char found;
    parent = parentIndex;
    convertStringToBuffer(pathNew, path);
    traversalDirectory(pathNew, success, &parent);

    if (*success != -1)
    {
        dirIndex = getIndex(pathNew, parent, DIRS_SECTOR);
        if (dirIndex == -2)
        {
            *success = -1;
            return;
        }

        readSector(dir, DIRS_SECTOR);
        for (i = 0; i < SECTOR_SIZE; i += 16)
        {
            if (dir[i] == dirIndex && dir[i + 1] != '\0')
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
                deleteDirectory(name, success, dirIndex);
            }
        }

        readSector(files, FILES_SECTOR);
        for (i = 0; i < SECTOR_SIZE; i += 16)
        {
            if (files[i] == dirIndex && files[i + 1] != '\0')
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
                delFile(name, dirIndex, success);
            }
        }

        readSector(dir, DIRS_SECTOR);
        dir[dirIndex * 16 + 1] = '\0';
        dir[dirIndex * 16] = '\0';
        writeSector(dir, DIRS_SECTOR);
        *success = 0;
    }
}

void putArgs(char currDirectory, char argc, char *argv)
{
    char args[SECTOR_SIZE];
    int i, j, p;
    clear(args, SECTOR_SIZE);

    args[0] = currDirectory;
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

void getcurrDirectory(char *currDirectory)
{
    char args[SECTOR_SIZE];
    readSector(args, ARGS_SECTOR);
    *currDirectory = args[0];
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
int getCharFromString(char *buffer, char c)
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
