#define MEM_SIZE 1000
void mem_init ();
char *mem_get_value (char *var);
void mem_set_value (char *var, char *value);

typedef struct PCB {
    int pid;
    int length;
    int start;
    int pc;
    int score;
    int pageTable[20];
    int frame;
    int lineFile;
    int LRUIndex;
} PCB;

PCB newPCB (int pid, int length, int start);
