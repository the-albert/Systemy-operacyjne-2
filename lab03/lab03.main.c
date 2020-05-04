#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

volatile static bool optf = false;

//funkcja odpowiadająca za działanie podczas wywołania Ctrl+C
static void handler(int num)
{
    if(num == SIGINT)
        optf = false;
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

int main(int argc, char *argv[])
{
    //obsługa Ctrl+C przy pomocy sigaction
      struct sigaction signal;
      signal.sa_flags = 0;
      signal.sa_handler = handler;
      if(sigaction(SIGINT, &signal, NULL) < 0)
        print_error("Nie udało się zainicjować sigaction.");

    //zmienne przechowujące argumenty opcji -n i -s
    int nline = 10;
    float sec = 1.0;

    //sprawdzenie jakie opcje i ich wartości zostały użyte
    int option;
    bool nflag = false;
    while((option = getopt(argc, argv, "fn:s:")) != -1)
    {
        switch(option)
        {
          case 'n':
            if(optarg[0] == '+')
              nflag = true;
            nline = atoi(optarg);
            if(nline < 0)
              nline *= -1;
          break;

          case 'f':
            optf = true;
          break;

          case 's':
            if(optf == false)
              print_error("Opcja s funkcjonuje tylko w połączeniu z opcją f.");
            sec = atof(optarg);
          break;
        }
    }

    //obsługa, w którym zostało podane za dużo lub żadna ścieżka do pliku
    int index = optind;
    if(argv[++index] != NULL || argv[optind] == NULL)
      print_error("Nie prawidłowa ilość argumentów argumantów.");

    //pobranie podanej ścieżki do pliku
    char *path = argv[optind];

    //próba otwarcia pliku i obsłużenie błędów
    FILE *file;
    if ((file = fopen(path, "r")) == NULL)
      print_error("Nie udało się otworzyć pliku.");

    //wejście w pętlę która jest nieskończona jeśli została podana opcja -f
    do
    {

      //sprawdzenie ile plik zawiera linii
      int counter = 0;
      char c;
      while((c = fgetc(file)) != EOF)
      {
        if(c == '\n')
          counter ++;
      }

      //przywrócenie pozycji na początek pliku
      fseek(file, 0, 0);

      //ustawienie startowej linii
      int start = counter - nline;
      if(nflag == true)
        start = nline-1;
      if(nline > counter)
        start = 0;

      //wypisywanie linii z pliku na ekran
      char *textline = NULL;
      size_t bufsize = 0;
      counter = 0;
      while(getline(&textline, &bufsize, file) > 0)
      {
        if(counter >= start)
          printf("%s", textline);
        counter++;
      }

      //utworzenie struktury stat
      struct stat stats;
      fstat(fileno(file), &stats);

      //obsługa instrukcji -f
      while(optf == true)
      {
        time_t old = stats.st_mtime;
        fclose(file);
        usleep(sec * 1000000);

        if ((file = fopen(path, "r")) == NULL)
          print_error("Nie udało się otworzyć pliku.");

        fstat(fileno(file), &stats);
        time_t new = stats.st_mtime;

        if(new > old)
          {
            const char *CLEAR_SCREEN_ANSI = "\e[1;1H\e[2J";
            write(STDOUT_FILENO, CLEAR_SCREEN_ANSI, 12);
            break;
          }
      }

      //zwolnienie pamięci
      free(textline);
    }while(optf == true);

    //zamknięcie pliku
    fclose(file);
  return 0;
}
