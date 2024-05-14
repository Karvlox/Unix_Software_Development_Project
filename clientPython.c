#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
 
#define PORT 8080
 
int main(int argc, char *argv[]) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char *message = "POST /number.py HTTP/1.0\r\n\r\n";
    char buffer[1024] = {0};
 
    // Crear socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
 
    // Convertir dirección IP y comprobar validez
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
 
    // Conectar al servidor
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }
 
    // Enviar mensaje al servidor
    send(sock, message, strlen(message), 0);
    printf("Message sent\n");
 
    // Leer respuesta del servidor
    read(sock, buffer, 1024);
    printf("Response: %s\n", buffer);
 
    return 0;
}