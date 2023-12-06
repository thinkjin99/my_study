#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

#define PORT 8080
#define MAX_MESSAGE_LEN 128
struct termios initial_settings, new_settings;

struct my_socket
{
    int fd;
    struct sockaddr_in address;
    char read_buffer[MAX_MESSAGE_LEN];
    char write_buffer[MAX_MESSAGE_LEN];
};

void reset_terminal()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &initial_settings);
}

void set_terminal()
{
    tcgetattr(STDIN_FILENO, &initial_settings);
    new_settings = initial_settings;
    new_settings.c_lflag &= ~ICANON; // 정규 모드 비활성화
    new_settings.c_lflag &= ~ECHO;   // 에코 비활성화
    tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);
    atexit(reset_terminal);
}

void back_space(char *write_buffer, char letter_bytes[MAX_MESSAGE_LEN], char *letter_index, char *byte_len)
{
    for (int i = 1; i <= letter_bytes[*letter_index]; i++)
    {
        write_buffer[*byte_len - i] = '\0'; // if (byte_len > 0)
        // *byte_len -= 1;
    }
    *byte_len -= letter_bytes[*letter_index];
    *letter_index -= 1;
}

void send_msg(int fd, char *write_buffer)
{
    write(fd, write_buffer, MAX_MESSAGE_LEN);
    memset(write_buffer, 0, MAX_MESSAGE_LEN);
}

void save_input_to_buffer(ssize_t rx_len, char *write_buffer, char *input, char *byte_len)
{
    for (int i = 0; i < rx_len; i++)
    {
        write_buffer[*byte_len] = input[i];
        *byte_len += 1;
    }
}

void read_socket(int fd, char *read_buffer)
{
    struct timeval timeout;
    fd_set readFds;

    // recive time out config
    // Set 1ms timeout counter

    timeout.tv_sec = 0;
    timeout.tv_usec = 1000;
    memset(read_buffer, 0, MAX_MESSAGE_LEN);

    // while (1)
    // {
    FD_ZERO(&readFds);
    FD_SET(STDIN_FILENO, &readFds);
    FD_SET(fd, &readFds);
    select(fd + 1, &readFds, NULL, NULL, &timeout);

    if (FD_ISSET(fd, &readFds))
    {
        if (read(fd, read_buffer, MAX_MESSAGE_LEN) > 0)
        {
            printf("\r\033[K");
            printf("%s\n\n", read_buffer);
            memset(read_buffer, 0, MAX_MESSAGE_LEN);
        }
        else
        {
            printf("Fail to read data\n");
            exit(0);
        }
    }
}

void read_input(int fd, char *write_buffer, char *letter_bytes, char *letter_index, char *byte_len)
{
    char input[3] = {
        0,
    };

    ssize_t rx_len = read(STDIN_FILENO, input, 3);

    if (rx_len > 0)
    {
        char first_ch = input[0];
        if (first_ch == 127)
        {
            back_space(write_buffer, letter_bytes, letter_index, byte_len);
            return;
        }

        else if (first_ch == '\n')
        {
            if (strcmp(write_buffer, "exit") == 0)
                exit(0);

            send_msg(fd, write_buffer);
            printf("\n\n");
            // memset(write_buffer, 0, MAX_MESSAGE_LEN);
            memset(letter_bytes, 0, MAX_MESSAGE_LEN);

            *byte_len = 0;
            *letter_index = 0;
            return;
        }

        else
        {
            save_input_to_buffer(rx_len, write_buffer, input, byte_len);
            letter_bytes[*letter_index] = rx_len;
            *letter_index += 1;
        }
    }
}

void run(struct my_socket socket)
{
    char byte_len = 0;
    char letter_index = 0;
    char letter_bytes[128] = {
        0,
    };

    int flags = fcntl(STDIN_FILENO, F_GETFL);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    while (1)
    {
        printf("\r\033[K");
        printf("\r>> %s", socket.write_buffer);
        fflush(stdout);
        read_input(socket.fd, socket.write_buffer, letter_bytes, &letter_index, &byte_len);
        read_socket(socket.fd, socket.read_buffer);
    }
}

int main()
{
    int client_socket;
    // 클라이언트 소켓 생성
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;
    pthread_t read_msg, write_msg, render_msg;

    if (client_socket == -1)
    {
        perror("Socket creation failed");
        exit(1);
    }

    // 서버 주소 설정
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    // 54.180.134.95
    server_address.sin_addr.s_addr = inet_addr("52.78.89.10"); // 서버 IP 주소
    // server_address.sin_addr.s_addr = inet_addr("127.0.0.1"); // 서버 IP 주소

    struct my_socket socket;
    socket.fd = client_socket;
    socket.address = server_address;
    memset(socket.write_buffer, 0, MAX_MESSAGE_LEN);
    memset(socket.read_buffer, 0, MAX_MESSAGE_LEN);

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        perror("Connection failed\n");
        close(client_socket);
        exit(1);
    }

    printf("Server connected: %s:%d\n", inet_ntoa(server_address.sin_addr), ntohs(server_address.sin_port));

    // 닉네임 입력
    printf("Input your name: ");
    char name[20] = {
        0,
    };
    scanf("%s", name);
    write(socket.fd, name, strlen(name));

    printf("\n<<< Chat started!! Enter 'exit' to quit >>>\n\n");

    set_terminal();
    run(socket);
    close(client_socket);
}
