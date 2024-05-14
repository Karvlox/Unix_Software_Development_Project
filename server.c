#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <curl/curl.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_THREADS 30
#define DOCUMENT_ROOT "./www"

void handle_root_request(int client_socket)
{
    char *server_info = "HTTP/1.0 200 OK\nContent-Type: text/plain\n\nServer: MyServer v1.0";
    send(client_socket, server_info, strlen(server_info), 0);
}

void handle_about_request(int client_socket)
{
    char *about_info = "HTTP/1.0 200 OK\nContent-Type: text/plain\n\nThis is the about page of MyServer.";
    send(client_socket, about_info, strlen(about_info), 0);
}

void handle_contact_request(int client_socket)
{
    char *contact_info = "HTTP/1.0 200 OK\nContent-Type: text/plain\n\nYou can contact us at example@example.com.";
    send(client_socket, contact_info, strlen(contact_info), 0);
}

void handle_file_request(int client_socket, char *path)
{
    // Construct full path
    char full_path[100];
    snprintf(full_path, sizeof(full_path), "%s/%s", DOCUMENT_ROOT, path);

    // Open the file
    int fd = open(full_path, O_RDONLY);
    if (fd == -1)
    {
        // If the file cannot be opened, return a 404 error
        perror("Error opening file");
        char *not_found_response = "HTTP/1.0 404 Not Found\nContent-Type: text/plain\n\n404 Not Found";
        send(client_socket, not_found_response, strlen(not_found_response), 0);
        return;
    }

    // Check if the file is a regular file
    struct stat file_stat;
    if (fstat(fd, &file_stat) == -1 || !S_ISREG(file_stat.st_mode))
    {
        close(fd);
        char *not_found_response = "HTTP/1.0 404 Not Found\nContent-Type: text/plain\n\n404 Not Found";
        send(client_socket, not_found_response, strlen(not_found_response), 0);
        return;
    }

    // Send HTTP headers
    char headers[BUFFER_SIZE];
    snprintf(headers, sizeof(headers), "HTTP/1.0 200 OK\nContent-Type: text/html\nContent-Length: %ld\n\n", (long)file_stat.st_size);
    send(client_socket, headers, strlen(headers), 0);

    // Send file contents
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer, BUFFER_SIZE)) > 0)
    {
        send(client_socket, buffer, bytes_read, 0);
    }

    // Close file descriptor
    close(fd);
}

void *handle_client(void *arg)
{
    int client_socket = *((int *)arg);
    char buffer[BUFFER_SIZE] = {0};
    read(client_socket, buffer, BUFFER_SIZE);
    printf("Received: %s\n", buffer);

    char method[10];
    char path[100];

    char *first_line = strtok(buffer, "\n");
    sscanf(first_line, "%s %s", method, path);

    printf("Method: %s, Path: %s\n", method, path);

    if (strcmp(method, "GET") == 0)
    {
        if (strcmp(path, "/") == 0)
        {
            handle_root_request(client_socket);
        }
        else if (strcmp(path, "/about") == 0)
        {
            handle_about_request(client_socket);
        }
        else if (strcmp(path, "/contact") == 0)
        {
            handle_contact_request(client_socket);
        }
        else
        {
            handle_file_request(client_socket, path);
        }
    }
    else if (strcmp(method, "POST") == 0)
    {
        if (strstr(path, ".py") != NULL)
        {
            // Ejecutar script de Python
            FILE *fp;
            char command[150]; // Incrementamos el tamaño del buffer command
            snprintf(command, sizeof(command), "python3 %s", path);

            // Abrir proceso para ejecutar el script de Python
            fp = popen(command, "r");
            if (fp == NULL)
            {
                // Si la llamada a popen() falla, envía una respuesta de error al cliente
                char *internal_error_response = "HTTP/1.0 500 Internal Server Error\nContent-Type: text/plain\n\n500 Internal Server Error";
                send(client_socket, internal_error_response, strlen(internal_error_response), 0);
                // Cierra el socket del cliente y finaliza la ejecución del hilo
                close(client_socket);
                pthread_exit(NULL);
            }

            // Leer salida del script de Python
            char output[BUFFER_SIZE];
            fgets(output, sizeof(output), fp);

            // Cerrar proceso
            pclose(fp);

            // Enviar encabezados HTTP
            char headers[BUFFER_SIZE];
            snprintf(headers, sizeof(headers), "HTTP/1.0 200 OK\nContent-Type: text/plain\nContent-Length: %ld\n\n", (long)strlen(output));
            send(client_socket, headers, strlen(headers), 0);

            // Enviar salida del script de Python
            send(client_socket, output, strlen(output), 0);
        }
        else
        {
            char *bad_request_response = "HTTP/1.0 400 Bad Request\nContent-Type: text/plain\n\n400 Bad Request";
            send(client_socket, bad_request_response, strlen(bad_request_response), 0);
        }

        close(client_socket);
        pthread_exit(NULL);
    }
    else
    {
        // Manejo de solicitud incorrecta (400 Bad Request)
        char *bad_request_response = "HTTP/1.0 400 Bad Request\nContent-Type: text/plain\n\n400 Bad Request";
        send(client_socket, bad_request_response, strlen(bad_request_response), 0);

        close(client_socket);
        pthread_exit(NULL);
    }
}

int main()
{
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    pthread_t threads[MAX_THREADS];
    int thread_count = 0;

    while (1)
    {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        if (pthread_create(&threads[thread_count], NULL, handle_client, (void *)&client_socket) != 0)
        {
            perror("Thread creation failed");
            close(client_socket);
        }

        thread_count++;

        if (thread_count >= MAX_THREADS)
        {
            // No crear más hilos si hemos alcanzado el límite
            break;
        }
    }

    // Esperar a que todos los hilos terminen antes de salir
    for (int i = 0; i < thread_count; i++)
    {
        pthread_join(threads[i], NULL);
    }

    return 0;
}