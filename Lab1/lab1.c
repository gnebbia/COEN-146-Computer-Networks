/*Anthony Fenzl
   Friday 2:15p lab
   Lab 1
*/

#include <stdio.h>

int main (int argc, char* argv[]) {
    if (argc != 3)
        return -1;
    FILE *src = fopen(argv[1], "rb");
    FILE *dest = fopen(argv[2], "wb");
    char buffer[10];
    int size;

    while (!feof(src)) {
        size = fread(buffer, sizeof(char), 10, src);
        fwrite(buffer, sizeof(char), size, dest);
    }

    fclose(src);
    fclose(dest);
    return 1;
}
