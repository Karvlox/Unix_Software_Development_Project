#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void handle_root_request(int client_socket)
{
    // La ruta solicitada es la raíz ("/")
    // Responder con información del servidor
    char *server_info = "HTTP/1.1 200 OK\nContent-Type: text/plain\n\nServer: MyServer v1.0";
    send(client_socket, server_info, strlen(server_info), 0);
}

void handle_about_request(int client_socket)
{
    // La ruta solicitada es "/about"
    // Responder con información sobre el servidor o cualquier otra información relevante
    char *about_info = "HTTP/1.1 200 OK\nContent-Type: text/plain\n\nThis is the about page of MyServer.";
    send(client_socket, about_info, strlen(about_info), 0);
}

void handle_contact_request(int client_socket)
{
    // La ruta solicitada es "/contact"
    // Responder con información de contacto o cualquier otra información relevante
    char *contact_info = "HTTP/1.1 200 OK\nContent-Type: text/plain\n\nYou can contact us at example@example.com.";
    send(client_socket, contact_info, strlen(contact_info), 0);
}

int main()
{
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Crear socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configurar opciones del socket
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Vincular socket al puerto
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Escuchar conexiones entrantes
    if (listen(server_fd, 5) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // Aceptar conexiones entrantes y manejar solicitudes
    while (1)
    {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        char buffer[BUFFER_SIZE] = {0};
        read(client_socket, buffer, BUFFER_SIZE);
        printf("Received: %s\n", buffer);

        // Parsear la solicitud HTTP para determinar el método y la ruta solicitada
        char method[10]; // Array para almacenar el método (GET o POST)
        char path[100];  // Array para almacenar la ruta solicitada

        // Leer la primera línea de la solicitud HTTP
        char *first_line = strtok(buffer, "\n");
        sscanf(first_line, "%s %s", method, path);

        // Imprimir el método y la ruta para propósitos de depuración
        printf("Method: %s, Path: %s\n", method, path);

        // Manejar la solicitud según la ruta solicitada
        if (strcmp(path, "/") == 0)
        {
            handle_root_request(client_socket);
        }
        else if (strcmp(path, "/about") == 0)
        {
            handle_about_request(client_socket);
            char *server_info = "HTTP/1.1 200 OK\nContent-Type: text/plain\n\nServer: MyServer About v1.0";
        }
        else if (strcmp(path, "/contact") == 0)
        {
            handle_contact_request(client_socket);
            char *server_info = "HTTP/1.1 200 OK\nContent-Type: text/plain\n\nServer: MyServer conteact v1.0";
        }
        else
        {
            // La ruta solicitada no coincide con ninguna ruta conocida
            // Responder con código de estado 404 (Not Found)
            char *not_found_response = "HTTP/1.1 404 Not Found\nContent-Type: text/plain\n\n404 Not Found";
            send(client_socket, not_found_response, strlen(not_found_response), 0);
        }

        close(client_socket);
    }

    return 0;
}