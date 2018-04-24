void clear(char *buffer, int length);

main() {
    char argc;
    char argv[2][50];
    char curdir;
    int stat;

    interrupt(0x21, 0x21, &curdir, 0, 0);
    interrupt(0x21, 0x22, &argc, 0, 0);

    clear(argv[0], 50);
    clear(argv[1], 50);

    if (argc == 1) {
        interrupt(0x21, 0x23, 0, argv[0], 0);
        interrupt(0x21, (curdir << 8) | 0x09, argv[0], &stat, 0x0);
    } else if (argc > 1) {
        interrupt(0x21, 0x23, 0, argv[0], 0);
        interrupt(0x21, 0x23, 1, argv[1], 0);
        //interrupt(0x21, 0x0, argv[1], 0, 0);
        if (argv[1][0] == '-' && argv[1][1] == 'd') {
            interrupt(0x21, (curdir << 8) | 0x0A, argv[0], &stat, 0x0);
        }
    }

    interrupt(0x21, 0x07, &stat, 0, 0);
}

void clear(char *buffer, int length){
  int i;
  for(i = 0; i < length; ++i){
    buffer[i] = 0x00;
  }
}