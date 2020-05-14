#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>

//funkcja odpowiadająca za wyświetlenie błędu i przerwanie programu
void print_error(char*);

int main(int argc, char** argv)
{
  //zmienne
  bool killa = false; //true jeśli uruchomiony jest killall
  DIR* proc;          //plik z którego pobiorę info o procesach
  struct dirent* dir; //struktura z informacjami o pliku z katalogu
  char path[20];      //ścieżka do pliku stat
  int fdescript;      //file descriptor pliku stat
  char line[100];     //linia z pliku stat
  int i=0;            //iterator
  bool check=false;   //czy znaleziono jakieś procesy
  memset(path, 0, sizeof(path));
  memset(line, 0, sizeof(line));
  //sprawdzenie czy został podany argument (nazwa programu)
  if(argc < 2)
    print_error("Musisz podać nazwę programu jako argument.");
  //sprawdzenie czy uruchomione zostało dowiązanie killall
  if(strcmp(argv[0], "./killall") == 0)
    killa = true;

  //otwarcie katalogu ./proc
  if((proc = opendir("/proc")) == NULL)
    print_error("Nie udało się otworzyć katallogu.");

  //wejście w pętle do przeszukania katalogu proc
  while((dir = readdir(proc)) != NULL)
  {
    //sprawdzenie czy to katalog reprezentujący proces >1000
    if(dir->d_type == DT_DIR && atoi(dir->d_name) > 1000)
    {
        //utworzenie ścieżki do kat stat
        strcat(path, "/proc/");
        strcat(path, dir->d_name);
        strcat(path, "/stat");

          //próba otworzenia pliku
          if((fdescript = open(path, O_RDONLY)) != -1)
          {
            //odczytanie i sprawdzenie czy plik dotyczy szukanego procesu
            int check = read(fdescript, line, sizeof(line)-1);
            //wykonanie dla każdego podanego argumentu
            for(i=1; i<argc; i++)
            {
              if(strstr(line, argv[i]) != NULL)
              {
                  check = true;
                  printf("%s ", dir->d_name);
                  //obsługa killall
                  if(killa == true)
                    kill(atoi(dir->d_name), SIGINT);
              }
            }
            close(fdescript);
          }
        //wyczyszczenie pamięci na nową ścieżkę
        memset(path, 0, sizeof(path));
    }
  }

  //new line po wyświetleniu pid
  if(check)
    printf("\n");

  //zamknięcie katalogu i zakończenie
  closedir(proc);
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
