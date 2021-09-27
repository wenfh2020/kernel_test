#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main() {
    write(1, "hello world", strlen("hello world"));
    pause();
    return 0;
}
