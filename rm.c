main() {
    char argc;
    char argv[2][50];
    char currDir;
    int result,i;
    enableInterrupts();
    interrupt(0x21, 0x21, &currDir, 0, 0);
    interrupt(0x21, 0x22, &argc, 0, 0);

    i=0;
    for(i=0;i<50;i++){
        argv[0][i]='\0';
        argv[1][i]='\0';
    }
    interrupt(0x21, 0x23, 0, argv[0], 0);
    if (argc == 1) {
        // interrupt(0x21, 0x0, "Delete file....", 0,0);
        interrupt(0x21, (currDir << 8) | 0x09, argv[0], &result, 0x0);
    }
    else if (argc > 1) {
        interrupt(0x21, 0x23, 1, argv[1], 0);
        // interrupt(0x21, 0x0, "Deleter dir..... ", 0,0);
        if (argv[1][0] == '-' && argv[1][1] == 'd') {
            interrupt(0x21, (currDir << 8) | 0x0A, argv[0], &result, 0x0);
        }
    }
    interrupt(0x21, 0x07, &result, 0, 0);
}
