main() {
    char argv[4][50],argc;
    char result;
    int i;
    enableInterrupts();
    interrupt(0x21, 0x22, &argc, 0, 0);
    // interrupt(0x21, 0x0, argv, 0, 0);
    for (i = 0; i < argc; i++) {
        // interrupt(0x21, 0x0, "asdwadwadwadwad", 0, 0);
        interrupt(0x21, 0x23, i, argv[i], 0);
    }
    // interrupt(0x21, 0x0, "finish for", 0, 0);
    interrupt(0x21, 0x0, argv, 0, 0);
    interrupt(0x21, 0x0, "\r\n", 0, 0);
    interrupt(0x21, 0x07, &result, 0, 0);
}