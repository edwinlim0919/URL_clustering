#include <stdio.h>


void read_urls() {
    char *url_filename = "URLS.txt";
    FILE *url_file = fopen(url_filename, "r");

    if (url_file == NULL) {
        printf("Error: could not open file %s", url_filename);
    }

    const unsigned MAX_LENGTH = 1024;
    char buffer[MAX_LENGTH];

    while (fgets(buffer, MAX_LENGTH, url_file)) {
        printf("%s", buffer);
    }

    printf("\n");
    fclose(url_file);
    return;
}


int main(void) {
    read_urls();
    return 0;
}