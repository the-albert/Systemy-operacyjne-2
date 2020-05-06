#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>

//funkcja odpowiadająca za wyświetlenie błędu i przerwanie programu
void print_error(char*);

int main (int argc, char** argv)
{
  //zmienne
  int option;   //wywołana opcja

  //sprawdzenie użytych opcji
  if(argc < 2)
    print_error("Za mało argumentów.");
  while(option = getopt(argc, argv, "p:q"))
  {
    switch(option)
    {
      case: break;
      case: break;
    }
  }
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
