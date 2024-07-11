#include "../shm.c"

int main() {
    printf("%s", tcp_connect_shm_read());
}
