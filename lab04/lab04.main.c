#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <math.h>
#include <pthread.h>
#include <time.h>

//plik z którego pobierana będzie macierz
static FILE* file;

//przechowuje na ilu wątkach mają zostać wykonane obliczenia
static int numTh;

//ilość wierszy i kolumn w pobieranej macierzy oraz operacji na wątek
static int rows, cols, opperth, rest;

//struktura która będzie przekazywana do funkcji wywoływanej przez wątek
typedef struct forThread
{
  int** in_tab;
  int** out_tab;
  int thindx;
  int start_i;
  int start_j;
}forThread;

//funkcja odpowiadająca za wyświetlenie błędu i przerwanie programu
void print_error(char*);

//funkcja odpowiedzialna za policzenie średniej z 4 sąsiadów w tablicy
void* countFour(void*);

//funkcja odpowiedzialna za wyliczenie punktu startowego obliczeń w macierzy
void findStart(forThread*, int);

int main(int argc, char* argv[])
{
  //bosługa sytuacji z nieprawidłową ilością argumentów
  if(argc != 3)
    print_error("Nieprawidłowa ilość argumentów.");

  //pobranie informacji o ilości wątków i obsługa błędów
  numTh = atoi(argv[2]);
  if(numTh != 1 && numTh != 2 && numTh != 4 && numTh != 8)
    print_error("Nieprawidłowa ilość wątków.");

  //pobranie ścieżki do pliku i jego otworzenie
  char* path = argv[1];

  if ((file = fopen(path, "r")) == NULL)
    print_error("Nie udało się otworzyć pliku.");

  //określenie rozmiaru macierzy
  rows = 0, cols = 0;
  char* val;
  char* textline = NULL;
  size_t bufsize = 0;

  while(getline(&textline, &bufsize, file) > 0)
      rows++;

  val = strtok(textline, " ");
  while(val != NULL)
  {
    val = strtok(NULL, " ");
    cols++;
  }
  printf("Macierz: wierszy = %d,  kolumn = %d\n", rows, cols);

  if(rows%2 != 0 || cols%2 != 0)
    print_error("Nieprawidłowe wymiary macierzy.");

  //alokacja pamięci dla macierzy
  int **in_matrix;
  in_matrix = malloc(rows * sizeof(int*));
  for(int i=0; i<rows; i++)
    in_matrix[i] = malloc(cols * sizeof(int));

  int **out_matrix;
  out_matrix = malloc((rows/2) * sizeof(int*));
  for(int i=0; i<(rows/2); i++)
    out_matrix[i] = malloc((cols/2) * sizeof(int));

  //stworzenie wątków oraz wykonanie obliczeń umożliwiających
  //równy podział pracy
  //(blok ten znajduje się tutaj a nie niżej aby umożliwić działanie goto)
  int allcells = rows*cols;
  int numofoperations = allcells/4;
  opperth = numofoperations/numTh;
  rest = numofoperations%numTh;

  forThread thtb[numTh];
  pthread_t threads[numTh];

  //deklaracja struktury przechowującej czas
  struct timespec start, end;

  //pobranie macierzy z pliku
  //powrót na początek pliku
  fseek(file, 0, 0);
  //wyczyszczenie kontenerów
  bufsize = 0; textline = NULL;
  int i = 0, j = 0;
  bool errflag1 = false;
  while(getline(&textline, &bufsize, file) > 0)
  {
    val = strtok(textline, " ");
    while(val != NULL)
    {
      //jeśli w środku tablicy kolumna przekroczy oczekiwaną wartość
      //to zostanie wykonany skok do sekcji swalniania pamięci
      //a następnie powiadomienie o błędzie i zakończenie programu
      if(j >= cols)
      {
        errflag1 = true;
        goto mem;
      }
      in_matrix[i][j] = atoi(val);
      val = strtok(NULL, " ");
      j++;
    }
    //jak wyżej jednak tutaj w przypadku jeśli kolumn będzie za mało
    if(j < cols)
    {
      errflag1 = true;
      goto mem;
    }
    j = 0;
    i++;
  }
  //zamknięcie pliku
  fclose(file);

  //dokonanie obliczeń i pomiar czasu
  bool errflag2 = false;
  int check;
  clock_gettime(CLOCK_MONOTONIC, &start);

  for(int t=0; t<numTh; t++)
  {
    thtb[t].in_tab = in_matrix;
    thtb[t].out_tab = out_matrix;
    thtb[t].thindx = t;
    findStart(&thtb[t], opperth);

    check = pthread_create(&threads[t], NULL, countFour, &thtb[t]);
    if(check != 0)
    {
      errflag2 = true;
      goto mem;
    }
  }

  //poczekanie na wszystkie wątki
  for(int t=0; t<numTh; t++)
    pthread_join(threads[t], NULL);

  clock_gettime(CLOCK_MONOTONIC, &end);

  //zapisywanie wynikowej macierzy do pliku
  FILE *output;
  bool errflag3 = false;
  if ((output = fopen("output.txt", "wa")) == NULL)
  {
        errflag3 = true;
        goto mem;
  }
  for(i=0; i<(rows/2); i++)
  {
    for(j=0; j<(cols/2); j++)
      fprintf(output, " %d", out_matrix[i][j]);
    fprintf(output, "\n");
  }
  fclose(output);

  //zwolnienie pamięci
  mem:
  for(int i=0; i<rows; i++)
    free(in_matrix[i]);
  free(in_matrix);
  in_matrix = NULL;

  for(int i=0; i<(rows/2); i++)
    free(out_matrix[i]);
  free(out_matrix);
  out_matrix = NULL;

  if(errflag1 == true)
      print_error("Ilość kolumn w tablicy jest różna.");
  if(errflag2 == true)
      print_error("Nie udało się utworzyć wątku.");
  if(errflag3 == true)
      print_error("Nie udało się otworzyć pliku.");

  //policzenie czasu obliczeń
  double time;
  time = (end.tv_sec - start.tv_sec) * 1e9;
  time = (time + (end.tv_nsec - start.tv_nsec)) * 1e-9;
  printf("Czas obliczeń: %fs\n", time);
  printf("Wykorzystane wątki: %d\n", numTh);
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
  if(file)
    fclose(file);
  exit(-7);
}

//funkcja odpowiedzialna za wyliczenie punktu startowego obliczeń w macierzy
void findStart(forThread* st, int opperth)
{
  int operation = 0, i = 0, j = 0;
  while(opperth*st->thindx > operation)
  {
    operation++;
    j+=2;
    if(j >= cols)
    {
      i+=2;
      j=0;
    }
  }
  st->start_i = i;
  st->start_j = j;
}

//funkcja odpowiedzialna za policzenie średniej z 4 sąsiadów w tablicy
void* countFour(void* pills)
{
  forThread *tabs = (forThread*) pills;
  int operations = 0;
  if(tabs->thindx == numTh-1)
    opperth += rest;

  for(int i = tabs->start_i; i<rows; i+=2)
  {
    for(int j = tabs->start_j; j<cols; j+=2)
    {
      tabs->out_tab[i/2][j/2] = round
        ((tabs->in_tab[i][j] + tabs->in_tab[i][j+1] +
          tabs->in_tab[i+1][j] + tabs->in_tab[i+1][j+1]) / 4.0);
      operations++;
      if(operations == opperth)
        goto done;
    }
    tabs->start_j = 0;
  }

  done:
  pthread_exit(0);
}
