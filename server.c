#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define MAX_CLIENTES 30
#define PUERTO 8080

void handle_client(int client_socket);

int main(int argc, char *argv[]) {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PUERTO);

    // Bind socket to port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Failed to bind socket");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, MAX_CLIENTES) < 0) {
        perror("Failed to listen");
        exit(EXIT_FAILURE);
    }

    printf("Servidor escuchando en el puerto %d...\n", PUERTO);

    // Accept incoming connections and handle them in separate processes
    while (1) {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Failed to accept connection");
            exit(EXIT_FAILURE);
        }

        printf("Nueva conexión aceptada\n");

        // Limit to 30 concurrent connections
        static int connection_count = 0;
        if (connection_count >= MAX_CLIENTES) {
            printf("Se alcanzó el máximo de conexiones. Cerrando nueva conexión.\n");
            close(client_socket);
            continue;
        }

        int pid = fork();
        if (pid == -1) {
            perror("Failed to create child process");
            close(client_socket);
            continue;
        } else if (pid == 0) {
            // Child process
            close(server_fd);
            handle_client(client_socket);
            exit(EXIT_SUCCESS);
        } else {
            // Parent process
            close(client_socket);
            // Increment connection count
            connection_count++;
            // Wait for a child process to finish if there are more than 30 connections
            if (connection_count >= MAX_CLIENTES) {
                wait(NULL);
                connection_count--;
            }
        }
    }

    return 0;
}

void handle_client(int client_socket) {
    char buffer[1024] = {0};
    char *root_response = "HTTP/1.0 200 OK\nContent-Type: text/plain\n\nCarlos Ballester's Server - Server v1.0";
    char *not_found_response = "HTTP/1.0 404 Not Found\nContent-Type: text/html\n\n<html><body><h1>404 Not Found</h1></body></html>";
    char *bad_request_response = "HTTP/1.0 400 Bad Request\nContent-Type: text/html\n\n<html><body><h1>400 Bad Request</h1></body></html>";
    char *internal_error_response = "HTTP/1.0 500 Internal Server Error\nContent-Type: text/html\n\n<html><body><h1>500 Internal Server Error</h1></body></html>";

    read(client_socket, buffer, 1024);
    printf("Recibido: %s\n", buffer);

    if (strstr(buffer, "GET / ") != NULL || strstr(buffer, "GET / HTTP/1.0") != NULL) {
        write(client_socket, root_response, strlen(root_response));
        printf("Respuesta de raíz enviada\n");
    } else if (strstr(buffer, "GET /test_form.html") != NULL) {
        FILE *file;
        char file_buffer[1024];
        int file_size;

        file = fopen("test_form.html", "r");
        if (file == NULL) {
            write(client_socket, not_found_response, strlen(not_found_response));
            printf("Respuesta de archivo no encontrado enviada\n");
        } else {
            fseek(file, 0, SEEK_END);
            file_size = ftell(file);
            rewind(file);

            fread(file_buffer, 1, file_size, file);
            fclose(file);

            char response_file[2048];
            sprintf(response_file, "HTTP/1.0 200 OK\nContent-Type: text/html\nContent-Length: %d\n\n%s", file_size, file_buffer);

            write(client_socket, response_file, strlen(response_file));
            printf("Respuesta de contenido de archivo enviada\n");
        }

    } else if(strstr(buffer, "POST /number.py") != NULL){
        FILE *file;
        char file_buffer[1024];
        int file_size;

        file = popen("python3 number.py", "r");
        if (file == NULL) {
            write(client_socket, not_found_response, strlen(not_found_response));
            printf("Respuesta de script de Python no encontrado enviada\n");
        } else {
            fseek(file, 0, SEEK_END);
            file_size = ftell(file);
            rewind(file);

            fread(file_buffer, 1, file_size, file);
            pclose(file);

            char response_python[2048];
            sprintf(response_python, "HTTP/1.0 200 OK\nContent-Type: text/html\nContent-Length: %d\n\n%s", file_size, file_buffer);

            write(client_socket, response_python, strlen(response_python));
            printf("Respuesta de salida de script de Python enviada\n");
        }
    } else{
        write(client_socket, bad_request_response, strlen(bad_request_response));
        printf("Respuesta de solicitud incorrecta enviada\n");
    }

    close(client_socket);
    printf("Socket de cliente cerrado\n");
}