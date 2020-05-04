#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
  //sprawdź czy argument został podany
  if(argc < 2)
  {
    errno++;
    perror("Musisz podać ciąg znaków jako argument.");
    exit -1;
  }

  //zakończ jeśli argument jest o długości 1
  if(strlen(argv[1]) == 1)
  {
    printf("%d\t%s\n" ,getpid(), argv[1]);
    return 0;
  }

  //alokowanie pamięci na kolejne argumenty
  char* arg1 = (char*) malloc(1 * sizeof(char));
  char* arg2 = (char*) malloc(strlen(argv[1]) * sizeof(char));

  //tworzenie pierwszego argumentu o długości 1
  sprintf(arg1, "%c", argv[1][0]);

  //tworzenie drugiego argumentu z reszty znaków
  for(int i=1; i<strlen(argv[1]); i++)
  {
    char *temp = (char*) malloc(1 * sizeof(char));
    sprintf(temp, "%c", argv[1][i]);
    strcat(arg2, temp);
    free(temp);
  }

  //utworzenie nowych procesów
  pid_t child1 = fork();
  if(child1 == 0)
    execlp(argv[0], argv[0], arg1, (char *)NULL);

  pid_t child2 = fork();
  if(child2 == 0)
    execlp(argv[0], argv[0], arg2, (char *)NULL);

  //obsługa przypadku, w którym nie uda sie utworzyć procesu
  if(child1 < 0 || child2 < 0)
  {
    perror("Nie udało się utworzyć procesu.");
    exit -2;
  }

  //czekanie aż wszystkie procesy skończą swoją pracę.
  pid_t wpid;
  int status = 0;
  while ((wpid = wait(&status)) > 0);

  //wyświetlenie swojego id i argumentu
  printf("%d\t%s\n" ,getpid(), argv[1]);

  //zwalnianie pamięci
  free(arg1);
  free(arg2);
  return 0;
}
