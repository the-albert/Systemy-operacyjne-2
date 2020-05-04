#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

//funkcja odpowiadająca za wyświetlenie błędu i przerwanie programu
void print_error(char*);

int main(int argc, char** argv)
{
  //zmienne
    bool killa = false; //true jeśli uruchomiony jest killall

  //sprawdzenie czy został podany argument (nazwa programu)
  if(argc < 2)
    print_error("Musisz podać nazwę programu jako argument.");
  //sprawdzenie czy uruchomione zostało dowiązanie killall
  if(strcmp(argv[0], "./killall") == 0)
    killa = true;

  

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
