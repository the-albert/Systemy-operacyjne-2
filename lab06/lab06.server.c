#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MSG_SIZE 128

//funkcja odpowiadająca za wyświetlenie błędu i przerwanie programu
void print_error(char*);

int main (int argc, char** argv)
{
  //zmienne
  int option;   //wywołana opcja
  int port;     //port serwera
  bool qf=false;//jeśli prawda wyłącz
  int sock;     //socket descriptor
  int incom;    //gniazdo połączenia
  socklen_t len;//wielkość struktury cliaddr
  char msg[MSG_SIZE];//odczytywany napis

  struct sockaddr cliaddr;     //adres klienta
  struct sockaddr_in servaddr; //adres serwera

  //sprawdzenie użytych opcji
  if(argc < 2 || argc > 3)
    print_error("Złe argumenty.");
  while((option = getopt(argc, argv, "p:q")) != -1)
  {
    switch(option)
    {
      case 'p': port = atoi(optarg);  break;
      case 'q': qf = true;            break;
    }
  }

  //wyszukanie i wyłączenie serwera
  if(qf == true)
  {

  }

  //utworzenie serwera
  //utworzenie gniazda
  if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    print_error("Nie udało się utworzyć gniazda.");
  //nadanie adresu
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);
  //powiązanie gniazda z adresem
  if(bind(sock, (struct sockaddr*) &servaddr, sizeof(servaddr)) == -1)
    print_error("Nie udało się powiązać gniazda.");
  //rozpoczęcie nasłuchiwania
  if(listen(sock, SOMAXCONN) == -1)
    print_error("Nie udało się rozpocząć nasłuchiwania.");
  //zaakceptowanie połączenia
  if((incom = accept(sock, (struct sockaddr*) &cliaddr, &len )) == -1)
    print_error("Nie udało się zaakceptować połączenia.");

  //odczytanie wiadomości
  read(incom, msg, sizeof(msg));
  printf("client says: %s\n", msg);
  write(incom, "dupa", 5);

  //zamknięcie gniazd
  close(sock);
  close(incom);
  return 0;
}

//funkcja odpowiadająca za wyświetlenie błędu i przerwanie programu
void print_error(char *err)
{
  if(errno == 0)
    errno++;

  char e[100] = "\nerror: ";
  strcat(e,err);
  perror(e);
  exit(-7);
}
