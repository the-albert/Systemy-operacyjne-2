#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <poll.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>

#define MSG_SIZE 128 //maksymalna wielkość wiadomości

//funkcja odpowiadająca za wyświetlenie błędu i przerwanie programu
void print_error(char*);

int main (int argc, char** argv)
{
  //zmienne
  int i, j;      //iterator
  int option;    //wywołana opcja
  int port;      //port serwera
  bool qf=false; //jeśli prawda wyłącz
  int serv_sock; //socket descriptor serwera
  int incom;     //gniazdo połączenia
  int size=100;  //wielkość tablicy sockets
  int returned=1;//zwrócona wartość
  int nfds=1;    //aktywne gniazda
  int cur_size;  //aktywne gniazda w danym momencie
  void* tmp;     //tymczasowy wskaźnik do realokacji
  char msg[MSG_SIZE];   //odczytywany napis
  char bck[MSG_SIZE];   //odsyłany napis
  bool stop_serv=false; //zatrzymaj serwer jeśli true

  struct pollfd* sockets;      //wskaźnik na tablicę struktur poll
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
  if((serv_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    print_error("Nie udało się utworzyć gniazda.");
  //ustawienie możliwości użycia adresu podczas time_period
  if((setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &returned, sizeof(int)))==-1)
    print_error("Nie udało się ustawić opcji gniazda.");
  //sprawia gniazdo jest non-blocking czyli połączenie nie będzie zatrzymywane
  //przy takich funkcjach jak recv(), rzucą one EWOULDBLOCK na errno
  returned = 1;
  if((ioctl(serv_sock, FIONBIO, &returned)) == -1)
    print_error("Nie udało się ustawić funkcji non-blocking");

  //nadanie adresu
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);
  //powiązanie gniazda z adresem
  if(bind(serv_sock, (struct sockaddr*) &servaddr, sizeof(servaddr)) == -1)
    print_error("Nie udało się powiązać gniazda.");
  //rozpoczęcie nasłuchiwania
  if(listen(serv_sock, SOMAXCONN) == -1)
    print_error("Nie udało się rozpocząć nasłuchiwania.");
  //alokacja pamięci oraz przypisanie nasłuchującego gniazda
  sockets = malloc(size * sizeof(struct pollfd));
  bzero(sockets, sizeof(sockets));
  sockets[0].fd = serv_sock;
  sockets[0].events = POLLIN;

  //wejście do pętli serwera
    do
  {
    //inicjacja poll
    returned = poll(sockets, nfds, 180000);
    if(returned == -1)
    {
        perror("\nerror: Inicjacja poll nie powiodła się.");
        break;
    }
    else if(returned == 0)
    {
        printf("\nMinął czas bezczynności, wyłączanie serwera...\n");
        break;
    }

    //przejżenie wszystkich gniazd
    cur_size = nfds;
    for (i = 0; i < cur_size; i++)
    {
      //jeśli nieaktywne przejdź dalej
      if(sockets[i].revents == 0)
        continue;

      //spodziewamy się danych do odczytu od klienta więc jeśli zwracany
      //event nie jest POLLIN działanie serwera zostaje przerwane
      if(sockets[i].revents != POLLIN)
      {
        printf("\nerror: Niespodziewany zwracany event\n");
        stop_serv = true;
        break;
      }
      //jeśli jest to gniazdo nasłuchujące
      if (sockets[i].fd == serv_sock)
      {
        //zaakceptowanie wszystkich oczekujących połączeń
        do
        {
          //akceptuje połączenie, jeśli accept zwróci -1 i errno
          //zostanie ustawione na EWOULDBLOCK znaczy, że nie ma więcej
          //oczekujących połączeń, jeśli inny błąd serwer zostaje zakończony
          incom = accept(serv_sock, NULL, NULL);
          if (incom == -1)
          {
            if (errno != EWOULDBLOCK)
            {
              perror("error: Nie udało się zaakceptować połączenia.");
              stop_serv = true;
            }
            break;
          }

          //dodanie przechwyconego połączenia do struktury
          sockets[nfds].fd = incom;
          sockets[nfds].events = POLLIN;
          nfds++;

          //realokacja pamięci
          if(nfds > (size - 10))
          {
            if((tmp = realloc(sockets, size*2*sizeof(struct pollfd))) == NULL)
            {
              perror("\nerror: Nieudana realokacja pamięci.");
              stop_serv = true;
              break;
            }
            sockets = tmp;
            size *= 2;
          }
        } while (incom != -1);
      }

      //jeśli jest to gniazdo do odczytu
      else
      {
        //odczytanie informacji z gniazda
        returned = recv(sockets[i].fd, msg, sizeof(msg), 0);
        if(returned == -1)
            perror("\nerror: Nie udało się odczytać wiadomości.");
        else if(returned == 0)
            printf("\nPołączenie z klientem zostało zerwane.\n");
        //odpowiednie sformatowanie wiadomości
        else
        {
          if(msg[0] == '1')
            for(j=1; j<strlen(msg); j++)
              bck[j-1] = tolower(msg[j]);
          else if(msg[0] == '2')
            for(j=1; j<strlen(msg); j++)
              bck[j-1] = toupper(msg[j]);
          else if(msg[0] == '3')
            for(j=1; j<strlen(msg); j++)
              bck[strlen(msg)-j-1] = msg[j];
          else
            printf("\nNierozpoznawalna operacja.\n");
        }

        //odesłanie wiadomości
        if((send(sockets[i].fd, bck, sizeof(bck), 0)) == -1)
          perror("\nNie udało się odesłać wiadomości.");

        //zamknięcie gniazda
        close(sockets[i].fd);
        sockets[i].fd = -1;

        bzero(msg, sizeof(msg));
        bzero(bck, sizeof(bck));
      }
    }
    //kompresja tablicy
    for (i = 0; i < nfds; i++)
    {
      if (sockets[i].fd == -1)
      {
        for(j = i; j < nfds; j++)
        {
          sockets[j].fd = sockets[j+1].fd;
        }
        i--;
        nfds--;
      }
    }

  } while (stop_serv == false); //sprawdzenie czy zakończyć działanie serwera

  //pozamykanie gniazd i zwolnienie pamięci
  for (i = 0; i < nfds; i++)
  {
    if(sockets[i].fd >= 0)
      close(sockets[i].fd);
  }
  close(serv_sock);
  close(incom);
  free(sockets);

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
