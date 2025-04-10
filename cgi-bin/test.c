#include <stdio.h>

int main(void) {
    // VERY IMPORTANT: must include a blank line after headers
    printf("Content-Type: text/html\r\n\r\n");
    printf("<html><body><h1>Hello from CGI!</h1></body></html>\n");
    return 0;
}
