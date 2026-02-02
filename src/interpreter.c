#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include "shellmemory.h"
#include "shell.h"

int MAX_ARGS_SIZE = 6;
char *command1[MAX_USER_INPUT][MAX_USER_INPUT];
char *command[100];
char *frameStore[100];
int pid = 0;
PCB readyQueue[MAX_USER_INPUT];

int badcommand () {
    printf ("Unknown Command\n");
    return 1;
}

// For source command only
int badcommandFileDoesNotExist () {
    printf ("Bad command: File not found\n");
    return 3;
}

int help ();
int quit ();
int set (char *var, char *value);
int print (char *var);
int source (char *script);
int echo (char *var);
int my_ls ();
int my_mkdir (char *dirname);
int my_touch (char *filename);
int my_cd (char *dirname);
int run (char *command, char *args[], int size);
int isAlphanumeric (const char *str);
void sort (char **arr, int n);
int codeLoading (char *script, int numFiles);
int exec (char *args[], int size);
void runSource ();
void SJFsort ();
void AGINGsort ();
void roundRobin (int r, int size, int numberFiles, int frameSize);
int count_lines (char *script);
int codeLoading1 (char *script);
int badcommandFileDoesNotExist ();
void loadFrame (int pid);

// Interpret commands and their arguments
int interpreter (char *command_args[], int args_size) {
    int i;

    if (args_size < 1 || args_size > MAX_ARGS_SIZE) {
        return badcommand ();
    }

    for (i = 0; i < args_size; i++) {   // terminate args at newlines
        command_args[i][strcspn (command_args[i], "\r\n")] = 0;
    }

    if (strcmp (command_args[0], "help") == 0) {
        //help
        if (args_size != 1)
            return badcommand ();
        return help ();

    } else if (strcmp (command_args[0], "quit") == 0) {
        //quit
        if (args_size != 1)
            return badcommand ();
        return quit ();

    } else if (strcmp (command_args[0], "set") == 0) {
        //set
        if (args_size != 3)
            return badcommand ();
        return set (command_args[1], command_args[2]);

    } else if (strcmp (command_args[0], "print") == 0) {
        if (args_size != 2)
            return badcommand ();
        return print (command_args[1]);

    } else if (strcmp (command_args[0], "source") == 0) {
        if (args_size != 2)
            return badcommand ();
        return source (command_args[1]);

    } else if (strcmp (command_args[0], "echo") == 0) {
        if (args_size != 2)
            return badcommand ();
        return echo (command_args[1]);

    } else if (strcmp (command_args[0], "ls") == 0) {
        if (args_size != 1)
            return badcommand ();
        return my_ls ();

    } else if (strcmp (command_args[0], "mkdir") == 0) {
        if (args_size != 2)
            return badcommand ();
        return my_mkdir (command_args[1]);

    } else if (strcmp (command_args[0], "touch") == 0) {
        if (args_size != 2)
            return badcommand ();
        return my_touch (command_args[1]);

    } else if (strcmp (command_args[0], "cd") == 0) {
        if (args_size != 2)
            return badcommand ();
        return my_cd (command_args[1]);
    } else if (strcmp (command_args[0], "run") == 0) {
        if (args_size < 2)
            return badcommand ();
        return run (command_args[1], &command_args[1], args_size);
    } else if (strcmp (command_args[0], "exec") == 0) {
        if (args_size > 6 || args_size < 3)
            return badcommand ();
        return exec (&command_args[1], args_size);
    } else
        return badcommand ();
}

int help () {

    // note the literal tab characters here for alignment
    char help_string[] = "COMMAND			DESCRIPTION\n \
help			Displays all the commands\n \
quit			Exits / terminates the shell with “Bye!”\n \
set VAR STRING		Assigns a value to shell memory\n \
print VAR		Displays the STRING assigned to VAR\n \
source SCRIPT.TXT	Executes the file SCRIPT.TXT\n ";
    printf ("%s\n", help_string);
    return 0;
}

int quit () {
    printf ("Bye!\n");
    exit (0);
}

int set (char *var, char *value) {
    char *link = " ";

    // Hint: If "value" contains multiple tokens, you'll need to loop through them, 
    // concatenate each token to the buffer, and handle spacing appropriately. 
    // Investigate how `strcat` works and how you can use it effectively here.

    mem_set_value (var, value);
    return 0;
}


int print (char *var) {
    printf ("%s\n", mem_get_value (var));
    return 0;
}

int source (char *script) {
    /*
       int errorCode = codeLoading1 (script);

       int fileSize = readyQueue[0].length;
       for (int j = 0; j < fileSize; j++) {
       parseInput (command1[0][j]);
       }

       for (int k = 0; k < fileSize; k++) {
       free (command1[0][k]);
       }

       pid = 0;

       return errorCode;
     */

    codeLoading (script, 1);
    loadFrame (0);
    loadFrame (0);
    roundRobin (2, 1, 1, FRAMESIZE);

}


int echo (char *var) {
    if (var != NULL && var[0] == '$') {
        char *res = mem_get_value (var + 1);
        if (strcmp (res, "Variable does not exist") == 0) {
            printf ("\n");
        } else {
            printf ("%s\n", res);
        }
    } else {
        printf ("%s\n", var);
    }

    return 0;
}

int my_ls () {
    DIR *dir = opendir (".");
    if (dir == NULL) {
        printf ("bruh");
        return 1;
    }

    struct dirent *entry;
    char **entries = NULL;
    int count = 0;

    while ((entry = readdir (dir)) != NULL) {
        char **temp = realloc (entries, (count + 1) * sizeof (char *));
        entries = temp;
        entries[count] = malloc (strlen (entry->d_name) + 1);
        strcpy (entries[count], entry->d_name);
        count++;
    }
    closedir (dir);

    sort (entries, count);

    for (int i = 0; i < count; i++) {
        printf ("%s\n", entries[i]);
        free (entries[i]);
    }
    free (entries);

    return 0;

}

int my_mkdir (char *dirname) {
    char directoryName[100];
    if (dirname[0] == '$') {
        char *name = dirname + 1;
        char *memName = mem_get_value (name);
        if (isAlphanumeric (name) == 1 || memName == NULL) {
            printf ("Bad command: my_mkdir\n");
            return 1;
        }
        strcpy (directoryName, memName);
    } else {
        if (isAlphanumeric (dirname) == 1) {
            printf ("Bad command: my_mkdir\n");
            return 1;
        }
        strcpy (directoryName, dirname);
    }

    if (mkdir (directoryName, 0777) == 0) {
        return 0;
    } else {
        printf ("Bad command: my_mkdir\n");
        return 1;
    }

}

int my_touch (char *filename) {
    FILE *file = fopen (filename, "w");
    if (file != NULL) {
        fclose (file);
        return 0;
    } else {
        printf ("Bad command: my_touch\n");
        return 1;
    }
}

int my_cd (char *dirname) {
    if (chdir (dirname) == 0) {
        return 0;
    } else {
        printf ("Bad command: my_cd\n");
        return 1;
    }
}

int run (char *command, char *args[], int size) {
    __pid_t pid = fork ();

    args[size - 1] = NULL;

    if (pid < 0) {
        printf ("Fork failed");
        return -1;
    }

    if (pid == 0) {
        if (execvp (command, args) == -1) {
            printf ("execvp failed");
            return -1;
        }
    } else {
        wait (NULL);
    }
    return 0;
}


int isAlphanumeric (const char *str) {
    for (int i = 0; i < strlen (str); i++) {
        char c = str[i];
        if ((c < 'A' || c > 'Z') && (c < 'a' || c > 'z')
            && (c < '0' || c > '9')) {
            return 1;
        }
    }
    return 0;
}

void sort (char **arr, int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (strcmp (arr[j], arr[j + 1]) > 0) {
                char *temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}


int codeLoading1 (char *script) {
    char temp[MAX_USER_INPUT];

    FILE *file = fopen (script, "rt");
    if (file == NULL) {
        badcommandFileDoesNotExist ();
        return 1;
    }

    int i = 0;
    while (fgets (temp, MAX_USER_INPUT - 1, file)) {
        command1[pid][i] = malloc (strlen (temp) + 1);
        if (command1[pid][i] == NULL) {
            fclose (file);
            return 1;
        }
        strcpy (command1[pid][i], temp);
        i++;
    }
    PCB pcb = newPCB (pid, i, 0);
    readyQueue[pid] = pcb;
    pid++;

    fclose (file);

    return 0;
}


int codeLoading (char *script, int numFiles) {
    char temp[MAX_USER_INPUT];

    PCB pcb = newPCB (pid, count_lines (script), pid * 3);
    readyQueue[pid] = pcb;

    FILE *file = fopen (script, "rt");
    if (file == NULL) {
        badcommandFileDoesNotExist ();
        return 1;
    }

    int i = pid * 3;
    int j = 0;
    readyQueue[pid].pageTable[j] = i;
    while (fgets (temp, MAX_USER_INPUT - 1, file)) {

        command[i] = malloc (strlen (temp) + 1);
        if (command[i] == NULL) {
            fclose (file);
            return 1;
        }
        strcpy (command[i], temp);
        i++;

        if (i % 3 == 0) {
            i += (numFiles - 1) * 3;
            j++;
            readyQueue[pid].pageTable[j] = i;
        }
    }

    pid++;

    fclose (file);

    return 0;
}

int count_lines (char *script) {
    FILE *file = fopen (script, "rt");
    if (file == NULL) {
        badcommandFileDoesNotExist ();
        return 1;
    }

    int lines = 0;
    int ch;
    int prev_char = '\0';

    while ((ch = fgetc (file)) != EOF) {
        if (ch == '\n') {
            lines++;
        }
        prev_char = ch;
    }

    if (prev_char != '\n' && prev_char != '\0') {
        lines++;
    }

    fclose (file);
    return lines;
}

int exec (char *args[], int size) {
    int background = 0;
    int s = 2;
    /*
       if (strcmp (args[size - 2], "#") == 0) {
       background = 1;
       s = 3;
       }
     */
    for (int i = 0; i < size - s; i++) {
        codeLoading (args[i], size - s);
        loadFrame (i);
        loadFrame (i);
    }

    //for (int i=0; i < 50; i++){
    //printf("\n%s", command[i]);
    //}
    //printf("\n%s", command[1]);
    //printf("\n%s", command[2]);


    char *policy = args[size - s];

    if (strcmp (policy, "FCFS") == 0) {
        runSource ();
    } else if (strcmp (policy, "SJF") == 0) {
        SJFsort ();
        runSource ();
    } else if (strcmp (policy, "RR") == 0) {
        roundRobin (2, pid, size - s, FRAMESIZE);
    } else if (strcmp (policy, "RR30") == 0) {
        roundRobin (30, pid, size - s, FRAMESIZE);
    } else if (strcmp (policy, "AGING") == 0) {
        int done = 1;
        int counter = 0;
        while (1) {
            done = 1;

            for (int i = 0; i < pid; i++) {
                if (readyQueue[i].pc < readyQueue[i].length) {
                    done = 0;
                }
            }

            if (done == 1) {
                break;
            }

            AGINGsort ();

            if (command1[readyQueue[counter].pid][readyQueue[counter].pc] ==
                NULL) {
                printf ("Error: NULL command at pid=%d, pc=%d\n",
                        readyQueue[counter].pid, readyQueue[counter].pc);
                break;
            }

            parseInput (command1[readyQueue[counter].pid]
                        [readyQueue[counter].pc]);

            readyQueue[counter].pc = readyQueue[counter].pc + 1;

            for (int j = counter + 1; j < pid; j++) {
                if (readyQueue[j].score > 0) {
                    readyQueue[j].score = readyQueue[j].score - 1;
                }
            }

            if (readyQueue[counter].pc >= readyQueue[counter].length) {
                readyQueue[counter].score = -1;
                counter++;
            }
        }
        pid = 0;

    } else {
        printf ("\n");
    }
}

void runSource () {
    int size = pid;
    for (int i = 0; i <= size; i++) {
        int fileSize = readyQueue[i].length;
        for (int j = 0; j < fileSize; j++) {
            parseInput (command1[readyQueue[i].pid][j]);
        }

        for (int k = 0; k < fileSize; k++) {
            free (command1[readyQueue[i].pid][k]);
        }
    }
    pid = 0;
}

void SJFsort () {
    char temp[MAX_ARGS_SIZE][MAX_ARGS_SIZE];

    for (int i = 0; i < pid - 1; i++) {
        for (int j = 0; j < pid - i - 1; j++) {
            if (readyQueue[j].length > readyQueue[j + 1].length) {
                PCB tempPCB = readyQueue[j];
                readyQueue[j] = readyQueue[j + 1];
                readyQueue[j + 1] = tempPCB;
            }
        }
    }
}

void AGINGsort () {
    for (int i = 0; i < pid - 1; i++) {
        for (int j = 0; j < pid - i - 1; j++) {
            if (readyQueue[j].score > readyQueue[j + 1].score) {
                PCB tempPCB = readyQueue[j];
                readyQueue[j] = readyQueue[j + 1];
                readyQueue[j + 1] = tempPCB;
            }
        }
    }
}

void roundRobin (int r, int size, int numberFiles, int frameSize) {
    int done = 1;
    int evict = 0;
    int k = 1;

    while (1) {
        done = 1;

        for (int k = 0; k < size; k++) {
            if (readyQueue[k].lineFile < readyQueue[k].length) {
                done = 0;
            }
        }

        if (done == 1) {
            break;
        }


        for (int i = 0; i < size; i++) {

            for (int j = 0;
                 j < r && readyQueue[i].lineFile < readyQueue[i].length; j++) {

                if (readyQueue[i].pc >= frameSize) {

                    printf ("Page fault! Victim page contents:\n\n");
                    for (int k = 0; k < 3; k++) {
                        printf ("%s", frameStore[evict + k]);
                    }


                    evict += 3;
                    printf ("\nEnd of victim page contents.\n");
                    loadFrame (i);

                    if (readyQueue[i].lineFile == readyQueue[i].length - 1) {
                        frameSize += k * 3;
                        k++;
                    } else {
                        frameSize += 3;
                    }

                    break;
                }

                if (frameStore[readyQueue[i].pc] == NULL) {
                    printf ("Page fault!\n");
                    loadFrame (i);
                    break;
                }

                char temp[MAX_USER_INPUT];
                strcpy (temp, frameStore[readyQueue[i].pc]);
                parseInput (frameStore[readyQueue[i].pc]);
                strcpy (frameStore[readyQueue[i].pc], temp);

                readyQueue[i].pc++;
                readyQueue[i].lineFile++;

                if (readyQueue[i].pc % 3 == 0) {
                    readyQueue[i].pc += (numberFiles - 1) * 3;
                }
            }
        }
    }
    pid = 0;
}

/*
void replaceFrame(int pid) {

    for (int i = readyQueue[pid].pageTable[readyQueue[pid].LRUIndex]; i < readyQueue[pid].pageTable[readyQueue[pid].LRUIndex] + 3; i++) {
        frameStore[i] = command[];
    }
}
*/
void loadFrame (int pid) {

    for (int i = readyQueue[pid].pageTable[readyQueue[pid].frame];
         i < readyQueue[pid].pageTable[readyQueue[pid].frame] + 3; i++) {
        frameStore[i] = command[i];
        //printf("\n%s", frameStore[i]);
    }

    readyQueue[pid].frame++;

}
