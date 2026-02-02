#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "shell.h"
#include "interpreter.h"
#include "shellmemory.h"

int parseInput (char ui[]);

// Start of everything
int main (int argc, char *argv[]) {
    printf ("Frame Store Size = %d; Variable Store Size = %d\n", FRAMESIZE,
            VARMEMSIZE);
    fflush (stdout);

    char prompt = '$';          // Shell prompt
    char userInput[MAX_USER_INPUT];     // user's input stored here
    int errorCode = 0;          // zero means no error, default

    //init user input
    for (int i = 0; i < MAX_USER_INPUT; i++) {
        userInput[i] = '\0';
    }

    //init shell memory
    mem_init ();
    while (1) {

        int mode = isatty (STDIN_FILENO);
        if (mode == 1) {
            printf ("%c ", prompt);
        }
        // here you should check the unistd library 
        // so that you can find a way to not display $ in the batch mode
        fgets (userInput, MAX_USER_INPUT - 1, stdin);
        errorCode = parseInput (userInput);

        if (feof (stdin)) {
            quit ();
        }

        if (errorCode == -1)
            exit (99);          // ignore all other errors
        memset (userInput, 0, sizeof (userInput));
    }

    return 0;
}

int wordEnding (char c) {
    // You may want to add ';' to this at some point,
    // or you may want to find a different way to implement chains.
    return c == '\0' || c == '\n' || c == ' ';
}

int parseInput (char inp[]) {

    if (strlen (inp) > 1000) {
        printf ("Error: Input exceeds 1000 characters.\n");
        return 1;
    }

    char *commandList[10];
    int commandCount = 0;
    char *command = strtok (inp, ";");

    while (commandCount < 10 && command != NULL) {

        while (*command == ' ') {
            command++;
        }

        int length = strlen (command);
        while (length > 0
               && (command[length - 1] == ' '
                   || command[length - 1] == '\n')) {
            command[length - 1] = '\0';
            length--;
        }

        if (strlen (command) > 0) {
            commandList[commandCount] = strdup (command);
            commandCount++;
        }

        command = strtok (NULL, ";");
    }

    int errorCode = 0;
    for (int i = 0; i < commandCount; i++) {
        char *words[100];
        int w = 0;
        char *word = strtok (commandList[i], " ");

        while (word != NULL && w < 100) {
            words[w] = strdup (word);
            w++;
            word = strtok (NULL, " ");
        }

        if (w > 0) {
            errorCode = interpreter (words, w);
            if (errorCode == 1) {
                return 1;
            }
        }
    }
    return errorCode;
}
