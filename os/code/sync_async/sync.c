#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

ssize_t read_file(int fd, char *buffer)
{
    ssize_t bytesRead = read(fd, buffer, 32);
    if (bytesRead == -1)
    {
        perror("Failed to read file");
    }
    return bytesRead;
}

ssize_t none_block_read(int fd, char *buffer)
{
    // Set the file descriptor to non-blocking mode
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    ssize_t bytesRead = read_file(fd, buffer);
    if (bytesRead == -1)
    {
        if (errno == EWOULDBLOCK || errno == EAGAIN)
        {
            // Non-blocking read returned with no data available
            printf("Still reading...\n");
        }
    }
    return bytesRead;
}

int sync_none_block_test(int fd, int fd2, char *buffer, char *buffer2)
{
    ssize_t bytesRead, bytesRead2;
    while (1)
    {
        bytesRead = none_block_read(fd, buffer);
        printf("Check Large is end\n");
        bytesRead2 = none_block_read(fd2, buffer2);
        printf("Check Samll is end\n");

        if (bytesRead == 0)
        {
            printf("Large fished!\n");
            break;
        }
        else if (bytesRead2 == 0)
        {
            printf("Small fished!\n");
            break;
        }
        printf("Read %zd bytes: %.*s\n", bytesRead, (int)bytesRead, buffer);
        printf("Read %zd bytes: %.*s\n", bytesRead2, (int)bytesRead2, buffer2);
    }
    return 0;
}

int main()
{
    int fd = open("large.txt", O_RDONLY);
    int fd2 = open("small.txt", O_RDONLY); // 논블락킹 read
    char buffer[1024], buffer2[1024];
    if (fd == -1)
    {
        perror("Failed to open file");
        return 1;
    }
    sync_none_block_test(fd, fd2, buffer, buffer2);
    read_file(fd, buffer);
    close(fd);
    close(fd2);
    return 0;
}
