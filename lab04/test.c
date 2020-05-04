#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#define MAX 20

struct dupa
{
  int **table;
  int row;
  int col;
};

void* madafaka(void* bad)
{
  struct dupa *tasty = (struct dupa*) bad;

  tasty->table[tasty->row][tasty->col] = 5;

  pthread_exit(0);
}

int main(int argc, char** argv)
{
  struct dupa d1, d2;

  int **tab;
  tab = malloc(sizeof(tab) * MAX);
  for(int i=0; i<MAX; i++)
    tab[i] = malloc(sizeof(tab[i]) * MAX);

  for(int i=0; i<MAX; i++)
  {
    for(int j=0; j<MAX; j++)
    {
      printf("%d ", tab[i][j]);
    }
    printf("\n");
  }
  printf("\n");

  int numofcells = MAX*MAX;
  int half = numofcells/2;
  int row =

  d1.table = tab;
  d1.row = 0;
  d1.col = 0;

  d2.table = tab;
  d2.row = MAX-1;
  d2.col = MAX-1;

  pthread_t t1,t2;

  pthread_create(&t1, NULL, madafaka, &d1);
  pthread_create(&t2, NULL, madafaka, &d2);

  pthread_join(t1, NULL);
  pthread_join(t2, NULL);

  for(int i=0; i<MAX; i++)
  {
    for(int j=0; j<MAX; j++)
    {
      printf("%d ", tab[i][j]);
    }
    printf("\n");
  }

  for(int i=0; i<MAX; i++)
    free(tab[i]);
   free(tab);

  return 0;
}
