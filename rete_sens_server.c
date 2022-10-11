/*
 * usage: tcpserver <port>
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/i2c-dev.h>

#define I2C_ADDR_T1 0x48		/* Indirizzo dispositivo 1 */
#define I2C_ADDR_T2 0x49		/* Indirizzo dispositivo 2 */

#define DS1624_START 0xEE
#define DS1624_READ  0xAA

char temp[2];


/* Socket */
Int parentfd; /* socket padre*/
struct sockaddr_in serveraddr; /* indirizzo del server*/
struct sockaddr_in clientaddr; /* indirizzo del client */
int init_socket(int portno);
int connect_tcp_client();
int write_tcp_client();

/* I2C */
static const char *device = "/dev/i2c-1";	/* I2C bus */
int read_temp();  /* legge la temperatura */
int i2c_addr;     /* legge l'indirizzo del sensore */

/*
 * error - Si visualizza il messaggio di errore
 */
void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char **argv)
{
   printf("Rete sensori di temperatura\n");
   int portno; /* Porta d'ascolto */
   int client;
    /*
    * Controlla la linea di comando
    */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    portno = atoi(argv[1]);
    printf("Server connesso sulla porta: %d\n", portno);
    init_socket(portno);

    while(1)
    {
        client = connect_tcp_client();
        read_temp();
	write_tcp_client(client);
    }
    return 0;
}

int init_socket(int portno)
{
    int optval; /* Valore del flag setsockopt */

    /*
    * socket: crea il socket padre
    */
    parentfd = socket(AF_INET, SOCK_STREAM, 0);
    if (parentfd < 0) error("ERROR opening socket");

    /* setsockopt: Pratico metodo di debug che ci permette
    * di riavviare immediatemnte il server dopo averlo chiuso;
    * altrimenti bisognerà aspettare 20 secondi.
    * Elimina "Errore nel collegamento: porta già in uso" errore.
    */
    optval = 1;
    setsockopt(parentfd, SOL_SOCKET, SO_REUSEADDR,
	     (const void *)&optval , sizeof(int));
    /*
    * Costruisce l'indirizzo IP del server
    */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    /* Questo è l'indirizzo IP */
    serveraddr.sin_family = AF_INET;
    /* Lascia che il sistema calcoli il vostro indirizzo IP */
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    /* Questa è la porta in ascolto */
    serveraddr.sin_port = htons((unsigned short)portno);
    /*
    * bind: Associa il socket padre ad una porta
    */
    if (bind(parentfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
        error("ERROR on binding");
    /*
    * listen: rende questo socket pronto ad accettare richieste di connessione
    */
    if (listen(parentfd, 5) < 0) /* autorizza 5 richieste di mettersi in coda */
        error("ERROR on listen");
    return 0;
}

int connect_tcp_client()
{
    int childfd; /* socket figlio */
    socklen_t clientlen = sizeof(clientaddr);
    /*
    * accept: aspetta una richiesta di connessione
    */
    childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen);
    if (childfd < 0) error("ERROR on accept");
    /*
    * read: legge la stringa di input del client
    */
    bzero(temp, 2);
    int n = read(childfd, temp, 1);
    if (n < 0) printf("ERROR reading from socket\n");
    
    if(temp[0] == 1)
    {
        i2c_addr = I2C_ADDR_T1;
        printf("Ricevuto: %d\n", temp[0]);

    }
    else if(temp[0] == 2)
    {
        i2c_addr = I2C_ADDR_T2;
        printf("Ricevuto: %d\n", temp[0]);

    }
    else printf("Codice comando non valido: %d\n", temp[0]);
    return childfd;
}

int write_tcp_client(int client)
{
    int n = write(client, temp, 2);
    printf("Trasmessa temperatura\n");
    close(client);
    return 0;
}

int read_temp()
{
    int fd;
    int val;
    float temperatura;
    char start = DS1624_START;
    char read_conv = DS1624_READ;
    /* apre il dispositivo I2C */
    if ((fd = open(device, O_RDWR)) < 0) error ("Can't open I2C device");
    if (ioctl(fd, I2C_SLAVE, i2c_addr) < 0) error ("Can't talk to slave");
    if (write(fd, &start, 1) < 1 ) 
    {
       if(i2c_addr == I2C_ADDR_T1)
          printf ("Il sensore T1 non risponde \n"); 
       else if(i2c_addr == I2C_ADDR_T1)
          printf ("Il sensore T2 non risponde \n");
       temp[0] = 0;
       temp[1] = 0;
    }
    else
    {
	sleep(1);
	if (write(fd, &read_conv, 1) < 1) printf("Failed to write read convertion\n");
	else
        {
           read(fd, temp, 2);
           val = (temp[0]<<8) | temp[1];
   	   temperatura = val / 256.0;
           if(temperatura > 128.0) temperatura -= 256.0;
	   printf("Letta temperatura %d %d %2.1f\n", temp[0], temp[1], temperatura);
        }
    }
    close(fd);
    return 0;
}