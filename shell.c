#define ARGS_CHAR_COMMAND 32
#define TOP_SEGMENT 0x2000
#define SECTOR_SIZE 512
#define TRUE 1
#define FALSE 0

int changeToNumber(char *dest);
int compareString(char *str, char *dest);

int main()
{
  char command[17], exec[15];
  char argv[SECTOR_SIZE], input[SECTOR_SIZE];
  char arguments[ARGS_CHAR_COMMAND];
  char argc;
  char currDir;
  int i, commandLength, count_arg;
  int result;
  int success;
  enableInterrupts();
  while (TRUE)
  {
    interrupt(0x21, 0x00, "\n\r$ ", 0, 0);
    interrupt(0x21, 0x1, input, TRUE, 0);
    i = 0;
    while (input[i] != '\0')
    {
      if (input[i] == ' ')
      {
        break;
      }
      i++;
    }
    commandLength = i;
    if (input[i] == '\0')
    {
      commandLength = -1;
    }
    i = 0;
    count_arg = 0;
    while (input[i] != '\0')
    {
      if (input[i] == ' ')
      {
        count_arg++;
      }
      i++;
    }
    i = 0;
    while (i < commandLength || (input[i] != '\0' && commandLength == -1))
    {
      command[i] = input[i];
      i++;
    }
    command[i] = '\0';
    i = 0;
    while (input[i + commandLength] != '\0')
    {
      argv[i] = input[i + commandLength + 1];
      i++;
    }
    argc = (char)count_arg;
    interrupt(0x21, 0x21, &currDir, 0, 0);
    interrupt(0x21, 0x20, currDir, argc, argv);
    interrupt(0x21, 0x00, "\r\n", 0, 0);
    // interrupt(0x21, 0xFF << 8 | 0x23, 0, arguments, 0);
    // interrupt(0x21, (currDir << 8) | 0x32, (changeToNumber(arguments) * 0x1000) + TOP_SEGMENT, &result, 0);
    // interrupt(0x21, 0xFF << 8 | 0x31, 0, 0, 0);
    interrupt(0x21, 0x21, &currDir, 0, 0);

    result = -1;
    if (command[0] == '.' && command[1] == '/')
    {
      for (i = 0; i < 15; i++)
      {
        exec[i] = command[i + 2];
      }
      interrupt(0x21, (currDir << 8) | 0x06, exec, &result, FALSE);
    }
    else if (compareString(command, "pause"))
    {
      interrupt(0x21, 0xFF << 8 | 0x23, 0, arguments, 0);
      interrupt(0x21, (currDir << 8) | 0x32, (changeToNumber(arguments) * 0x1000) + TOP_SEGMENT, &result, 0);
      interrupt(0x21, 0xFF << 8 | 0x31, 0, 0, 0);
    }
    else if (compareString(command, "resume"))
    {
      // interrupt(0x21, 0xFF << 8 | 0x23, 0, arguments, 0);
      // interrupt(0x21, (currDir << 8) | 0x32, (changeToNumber(arguments) * 0x1000) + TOP_SEGMENT, &result, 0);
      // interrupt(0x21, 0xFF << 8 | 0x31, 0, 0, 0);
      interrupt(0x21, 0xFF << 8 | 0x23, 0, arguments, 0);
      interrupt(0x21, (currDir << 8) | 0x33, (changeToNumber(arguments) * 0x1000) + TOP_SEGMENT, &result, 0);
      interrupt(0x21, 0xFF << 8 | 0x31, 0, 0, 0);
    }
    else if (compareString(command, "kill"))
    {
      interrupt(0x21, 0xFF << 8 | 0x23, 0, arguments, 0);
      interrupt(0x21, (currDir << 8) | 0x34, (changeToNumber(arguments) * 0x1000) + TOP_SEGMENT, &result, 0);
      interrupt(0x21, 0xFF << 8 | 0x31, 0, 0, 0);
      // interrupt(0x21, 0xFF << 8 | 0x23, 0, arguments, 0);
      // interrupt(0x21, (currDir << 8) | 0x32, (changeToNumber(arguments) * 0x1000) + TOP_SEGMENT, &result, 0);
      // interrupt(0x21, 0xFF << 8 | 0x31, 0, 0, 0);
    }
    else
    {
      //Parallel Shell
      if (argc > 0)
      {
        interrupt(0x21, 0xFF << 8 | 0x23, argc - 1, arguments, 0);
        if (compareString(arguments, "&"))
        {
          interrupt(0x21, (0xFF << 8) | 0x06, command, &result, TRUE);
        }
        else
        {
          interrupt(0x21, (0xFF << 8) | 0x06, command, &result, FALSE);
        }
      }
      else
      {
        interrupt(0x21, (0xFF << 8) | 0x06, command, &result, FALSE);
      }
    }
  }
  return 0;
}

int changeToNumber(char *dest)
{
  int result = 0;
  int i = 0;
  while (dest[i] != '\0')
  {
    result *= 10;
    result += dest[i] - '0';
    i++;
  }
  return result;
}

int compareString(char *str, char *dest)
{
  int i;
  i = 0;
  while (str[i] != '\0' || dest[i] != '\0')
  {
    if (str[i] != dest[i])
    {
      break;
    }
    i++;
  }
  if (str[i] != dest[i])
  {
    return FALSE;
  }
  else
  {
    return TRUE;
  }
}
