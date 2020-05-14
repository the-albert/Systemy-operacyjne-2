#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MSG_SIZE 128 //maksymalna wielkość wiadomości

//funkcja odpowiadająca za wyświetlenie błędu i przerwanie programu
void print_error(char*);

int main (int argc, char** argv)
{
  //zmienne
  int option;   //wywołana opcja
  char ip[32];  //adres ip
  int port;     //port
  int operation;//rodzaj operacji
  bool f=false; //flaga pomocnicza
  int sock;     //socket descriptor
  char opts[MSG_SIZE-1];  //napis użytkownika
  char msg[MSG_SIZE]; //wysyłany napis

  struct sockaddr_in servaddr;  //adres serwera

  //sprawdzenie użytych opcji
  if(argc < 9)
    print_error("Za mało argumentów.");
  while((option = getopt(argc, argv, "a:p:s:o:")) != -1)
  {
    f = true;
    switch(option)
    {
      case 'a':
          snprintf(ip, strlen(optarg)+1, "%s", optarg);
      break;
      case 'p':
          port = atoi(optarg);
      break;
      case 's':
          snprintf(opts, strlen(optarg)+1, "%s", optarg);
      break;
      case 'o':
            if((strcmp("tolower", optarg)) == 0)
              operation = 1;
            else if((strcmp("toupper", optarg)) == 0)
              operation = 2;
            else if((strcmp("invert", optarg)) == 0)
              operation = 3;
            else
              print_error("Nie ma takiej operacji.");
      break;
      case '?': print_error("Nieprawidłowa opcja.");  break;
      default:  print_error("Nieprawidłowa opcja.");  break;
    }
  }
  if(f == false)
    print_error("Nieprawidłowe argumenty.");

  //utworzenie gniazda
  if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    print_error("Nie udało się utworzyć gniazda.");
  //nadanie adresu
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr(ip);
  servaddr.sin_port = htons(port);
  //połączenie z serwerem
  if(connect(sock, (struct sockaddr*) &servaddr, sizeof(servaddr)) == -1)
    print_error("Nie udało się połączyć z serwerem.");

  //napisanie wiadomości
  sprintf(msg, "%d%s", operation, opts);
  write(sock, msg, sizeof(msg));
  //odebranie wiadomości
  bzero(msg, sizeof(msg));
  read(sock, msg, sizeof(msg));
  printf("\nServer says: %s\n", msg);

  //zamknięcie gniazd
  close(sock);
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
