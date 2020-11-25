#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>


#define M 100			// ==> Rango de valores de los componentes: [0, M[
#define VECT_SIZE 50000	// N.º componentes del vector que se quiere ordenar
#define NM 4			// N.º de métodos de ordenación que se llamarán desde el programa principal
#define min(a,b) ((a)<(b)? a:b)
#define FALSE 0
#define TRUE 1


// Variables globales
float vini[VECT_SIZE], vord0[VECT_SIZE], vord[VECT_SIZE];	// vector desordenado y vector que se ordenará, respectivamente

void ord_parD(float vector[], int size)
{
	int phase, i;
	float temp;
	/* El siguiente for no se puede paralelizar ya que entre una fase y la siguiente hay dependencias RAW:
    * por ejemplo, un fase puede escribir en vector[1] y la siguiente lee vector[1]*/
	for (phase = 0; phase < size; phase++)
		if (phase % 2 == 0)
		{ // Fase par
			#pragma omp parallel for private(temp)
			for (i = 1; i < size; i += 2)
				if (vector[i - 1] > vector[i])
				{
					temp = vector[i];
					vector[i] = vector[i - 1];
					vector[i - 1] = temp;
				}
		}
		else
		{ // Fase impar
			#pragma omp parallel for private(temp)
			for (i = 1; i < size - 1; i += 2)
				if (vector[i] > vector[i + 1])
				{
					temp = vector[i];
					vector[i] = vector[i + 1];
					vector[i + 1] = temp;
				}
		}
} // Fin de ord_parD

void ord_parD2(float vector[], int size)
{
	int phase, i;
	float temp;
	/* El siguiente for no se puede paralelizar ya que entre una fase y la siguiente hay dependencias RAW:
    * por ejemplo, un fase puede escribir en vector[1] y la siguiente lee vector[1]*/
	for (phase = 0; phase < size; phase++)
		if (phase % 2 == 0)
		{ // Fase par
			#pragma omp parallel for schedule(dynamic) private(temp)
			for (i = 1; i < size; i += 2)
				if (vector[i - 1] > vector[i])
				{
					temp = vector[i];
					vector[i] = vector[i - 1];
					vector[i - 1] = temp;
				}
		}
		else
		{ // Fase impar
			#pragma omp parallel for schedule(dynamic) private(temp)
			for (i = 1; i < size - 1; i += 2)
				if (vector[i] > vector[i + 1])
				{
					temp = vector[i];
					vector[i] = vector[i + 1];
					vector[i + 1] = temp;
				}
		}
} // Fin de ord_parD

int main(){
  float v[] = {44,54,13,42,95,19,32};
  float v2[] = {44,54,13,42,95,19,32};
  float t1 = omp_get_wtime();
  ord_parD(v, 7);
  printf("Ha tardado: %f\n", omp_get_wtime()-t1);
  float t2 = omp_get_wtime();
  ord_parD2(v2, 7);
  printf("Ha tardado: %f\n", omp_get_wtime()-t2);
  for(int i=0; i<7; i++){
    printf("%f, ", v[i]);
  }

  return 0;
}

// 25/11/2020: COMENTAR MEZCLA ORDENADA Y ORD SEC A 