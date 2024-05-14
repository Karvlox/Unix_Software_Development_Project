# Servidor Web

Este proyecto consiste en un servidor web simple implementado en C, capaz de manejar solicitudes HTTP GET y POST. También incluye ejemplos de clientes que interactúan con el servidor.

## Requisitos

Para compilar y ejecutar el servidor y los clientes, se necesita un entorno de desarrollo que admita el lenguaje C, como GCC. Además, el servidor asume que Python 3 está instalado en el sistema.

## Compilación

Para compilar el servidor, puedes utilizar el siguiente comando en la terminal:

```bash
gcc -o servidor servidor.c
```

Esto generará un archivo ejecutable llamado servidor.

## Ejecución del Servidor

Para ejecutar el servidor, simplemente utiliza el siguiente comando en la terminal:

```bash
./servidor
```

El servidor se ejecutará en el puerto especificado en el código (8080) y estará listo para recibir conexiones entrantes.

## Uso de los Clientes


### clientPython.c

Este cliente realiza una solicitud POST al servidor para ejecutar un script de Python y obtener su salida.


Para compilar y ejecutar el cliente, puedes seguir estos pasos:

```bash
gcc -o clientPython clientPython.c
./clientPython
```

### clientPath.c

Este cliente realiza una solicitud GET al servidor para obtener la página de inicio.


Para compilar y ejecutar el cliente, puedes seguir estos pasos:

```bash
gcc -o clientPath clientPath.c
./clientPath
```

### clientIndex.c

Este cliente realiza una solicitud GET al servidor para obtener una página HTML específica.


Para compilar y ejecutar el cliente, puedes seguir estos pasos:

```bash
gcc -o clientIndex clientIndex.c
./clientIndex
```

Archivos de Soporte


El proyecto también incluye dos archivos adicionales:

* number.py: Un simple script de Python que se ejecuta cuando se hace una solicitud POST al servidor.
* test_form.html: Una página HTML de ejemplo que puede ser solicitada al servidor.


## Notas Importantes

* El servidor utiliza la llamada al sistema fork para manejar múltiples conexiones simultáneas de manera eficiente. Cuando el servidor acepta una nueva conexión entrante, crea un nuevo proceso hijo utilizando fork. Este proceso hijo maneja la conexión con el cliente mientras que el proceso padre continúa aceptando nuevas conexiones.

* Al utilizar fork, cada proceso hijo tiene su propio espacio de memoria y recursos, lo que permite que múltiples conexiones se manejen de forma independiente sin interferir entre sí. Esto proporciona una manera eficaz de manejar varias solicitudes de clientes al mismo tiempo, lo que resulta en un servidor más escalable y robusto.

* Es importante tener en cuenta que el servidor está configurado actualmente para manejar un máximo de 30 conexiones simultáneas, y el uso de fork ayuda a garantizar que este límite se respete y que el servidor siga siendo receptivo incluso cuando está ocupado atendiendo a múltiples clientes.

* Todos los archivos de soporte (como el script de Python y las páginas HTML) están presentes en el directorio de trabajo del servidor, para usar otro tipo de archivo, este aun no tiene la capacidad de hacerlo de forma autodidacta, se tiene que modificar el archivo que recibe en el servidor.



### Autor 
- @Carlos Antonio Ballester Paniagua