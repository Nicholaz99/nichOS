main() {
    char argv[4][50];
    char argc;
    int i;
    char stat;
    enableInterrupts();

    interrupt(0x21, 0x22, &argc, 0, 0);
    //interrupt(0x10, 0xE00 + argc + '0', 0, 0, 0);
    for (i = 0; i < argc; i++) {
        interrupt(0x21, 0x23, i, argv[i], 0);
        //interrupt(0x21, 0x0, argv[i], 0, 0);
    }

    interrupt(0x21, 0x0, argv[0], 0, 0);
    interrupt(0x21, 0x0, "\r\n", 0, 0);
    interrupt(0x21, 0x07, &stat, 0, 0);
}