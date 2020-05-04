#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>

volatile static bool sigcheck = false;

static void handler(int num)
{
    if(num == SIGINT)
        sigcheck = true;
}


int main(int argc, char *argv[])
{
  //deklaracja maski sygnałów i zablokowanie TSTP
  sigset_t sigmaskset;
  sigemptyset(&sigmaskset);
  sigaddset(&sigmaskset, SIGTSTP);
  sigprocmask(SIG_BLOCK, &sigmaskset, NULL);


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
    {
      setpgid(0,0);
      execlp(argv[0], argv[0], arg1, (char *)NULL);
    }

  pid_t child2 = fork();
  if(child2 == 0)
    {
      setpgid(0,0);
      execlp(argv[0], argv[0], arg2, (char *)NULL);
    }

  //obsługa przypadku, w którym nie uda sie utworzyć procesu
  if(child1 < 0 || child2 < 0)
  {
    perror("Nie udało się utworzyć procesu.");
    exit -2;
  }

  //deklaracja struktury sigaction
    struct sigaction signal;
    signal.sa_mask = sigmaskset;
    signal.sa_flags = 0;
    signal.sa_handler = handler;
    if(sigaction(SIGINT, &signal, NULL) < 0)
      perror("\nsigaction error.\n");

  //wejście do nieskończonej pętli
  while(1)
  {
    //sprawdzenie czy został wysłany sygnał
    if(sigcheck == true)
    {
      //przekazanie sygnału dalej
      kill(child1, SIGINT);
      kill(child2, SIGINT);
      //poczekanie aż wszystkie dzieci skończą swoją pracę
      int status = 0;
      if(waitpid(child1, &status, 0) > 0 && waitpid(child2, &status, 0) > 0)
        break;
    }
  }

  //wyświetlenie swojego id i argumentu
  printf("\n%d\t%s " ,getpid(), argv[1]);

  //zwalnianie pamięci
  free(arg1);
  free(arg2);

  //wyświetlenie czy został użyty Ctrl+Z
  sigset_t sigused;
  sigpending(&sigused);
  if(sigismember(&sigused, SIGTSTP) == 1)
    printf("\nPodczas działania programu został zablokowany SIGTSTP(Ctrl+Z).\n");

  //odblokowanie SIGTSTP
  sigprocmask(SIG_UNBLOCK, &sigmaskset, NULL);
  return 0;
}
