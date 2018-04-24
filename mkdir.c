main() {
    char argc;
    char argv[2][50];
    char curdir;
    int stat;
    enableInterrupts();

    interrupt(0x21, 0x21, &curdir, 0, 0);
    interrupt(0x21, 0x22, &argc, 0, 0);

    if (argc >= 1) {
        interrupt(0x21, 0x23, 0, argv[0], 0);
        interrupt(0x21, (curdir << 8) | 0x08, argv[0], &stat, 0x0);
    }

    interrupt(0x21, 0x07, &stat, 0, 0);
}