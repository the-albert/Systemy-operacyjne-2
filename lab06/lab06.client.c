#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

//funkcja odpowiadająca za wyświetlenie błędu i przerwanie programu
void print_error(char*);

//zmienne globalne
char* string; //wysyłany napis

int main (int argc, char** argv)
{
  //zmienne
  int option;   //wywołana opcja
  char ip[32];  //adres ip
  int port;     //port
  int operation;//rodzaj operacji
  bool f=false; //flaga pomocnicza

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
          string = (char*) malloc((strlen(optarg)+1)*sizeof(char));
          snprintf(string, strlen(optarg)+1, "%s", optarg);
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

  printf("ip: %s, port: %d, napis: %s, operacja: %d", ip, port, string, operation);

  //zwalnianie pamięci
    free(string);
  return 0;
}

//funkcja odpowiadająca za wyświetlenie błędu i przerwanie programu
void print_error(char *err)
{
  if(errno == 0)
    errno++;
  if(string != NULL)
    free(string);

  char e[100] = "\nerror: ";
  strcat(e,err);
  perror(e);
  exit(-7);
}