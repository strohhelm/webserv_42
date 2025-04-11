#include <stdio.h>

int main(int argc, char** argv, char** envp) {
    // VERY IMPORTANT: must include a blank line after headers
    printf("Content-Type: text/html\r\n\r\n");
    printf("<html><body><h1>CGI GET REQUEST!</h1></body></html>\n");
    return 0;
}
