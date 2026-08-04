#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "types.h"

int main(int argc, char* argv[]){
    u8* data = malloc(8 * sizeof(u8));

    data[0] = 97;
    data[1] = 97;
    data[2] = 97;
    data[3] = 97;
    data[4] = 97;
    data[5] = 97;
    data[6] = 97;
    data[7] = 0;

    FILE * file;
    file = fopen("myfile.txt" , "wb");
    char buffer [100];

    if (!file)
        return 1;

    fwrite((const void *) data,  sizeof(u8), 8, file);

    fclose(file);
    // open file
    // write "hello world\n" to it
    // done

    free(data);

    return 0;
}

