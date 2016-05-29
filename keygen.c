#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>


/*function: modulo
 * takes two integers
 * returns integers
 * a custom mod function to handle negatives
 * ref: http://stackoverflow.com/questions/4003232/how-to-code-a-modulo-operator-in-c-c-obj-c-that-handles-negative-numbers?lq=1
 * ref2: http://stackoverflow.com/questions/14997165/fastest-way-to-get-a-positive-modulo-in-c-c
 */
int modulo(int a, int b){
    int mod = a % b;
    if (mod < 0) {
      mod = mod + b;
    }
    return mod;
}

int main(int argc, char const *argv[]) {

    if (argc < 2) {
        printf("Usage: <keygen> <number> %s\n", argv[0]);
        exit(1);
    }

    srand(time(NULL));
    int length = atoi(argv[1]);
    char key[length];
    char randChar;

    int i;
    for (i = 0; i < length; i++) {
        randChar = " ABCDEFGHIJKLMNOPQRSTUVWXYZ"[modulo(random(), 27)];
        key[i] = randChar;
    }
    //place null terminator at end so it is a true string
    key[length] = '\0';

    //print key to screen
    printf("%s\n", key);

    return 0;
}
