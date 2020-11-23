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

void mezcla_ordenada(float vector[], int ini1, int ini2, int fin2)
{
	int i, j, k, terminado = 0;
	float temp;
	i = ini1;
	j = ini2;
	do
	{ // vector[ini1] <=...<= vector[j-1] AND vector[j] <=...<= vector[fin2] AND i < j
		while ((vector[i] <= vector[j]) && (i < j - 1)) // 2a condicion para que no adelante i a j (solo se produce al final)
			i++;
		if (vector[i] > vector[j])
		{
			/* Rotamos vector[i], vector[i+1], ...,vector[j-1], vector[j]
					 * para que vector[j] pase a la posición i y el resto se desplace una posición a la derecha */
			temp = vector[j];
      #pragma omp parallel for schedule(dynamic)
			for (k = j; k > i; k--)
				vector[k] = vector[k - 1];
			vector[i] = temp;
			if (j == fin2)
				terminado = 1;
			// Hemos colocado en su posición el último elemento del segundo fragmento
			else
			{
				j++;
				i++;
			}
		}
		// vector[ini1] <=...<= vector[j-1] AND vector[j] <=...<= vector[fin2] AND i < j
		else
			terminado = 1;
		/* i == j-1 AND vector[i] <= vector[j], 
				 * luego vector[j-1] <= vector[j] y todo queda ordenado*/
	} while (!terminado);
}

void ord_secA(float vector[], int size)
{
	int incr, i, fin2;

	for (incr = 2; incr < 2 * size; incr = 2 * incr)
    #pragma omp parallel for schedule(dynamic) private(fin2)
		for (i = 0; i < (size - incr / 2); i += incr) // (i+incr/2) < size
		{
			fin2 = min(size - 1, i + incr - 1);
			mezcla_ordenada(vector, i, i + incr / 2, fin2);
			/* Quedan ordenados incr componentes consecutivos del vector (índices i,i+1,...i+incr-1)
			 * (el último trozo ordenado puede ser menor: índices i, i+1,...size-1)
			*/
		}
	/* El último valor de incr cumple size <= incr < 2*size. 
		 * Esto conlleva que está ordenado todo el vector, al ser size <= incr */
} // Fin de ord_secA


int main(){
  float v[] = {44,54,13,42,95,19,32};
  ord_secA(v, 7);
  // mezcla_ordenada(v, 0, 5, 9);
  for(int i=0; i<7; i++){
    printf("%f, ", v[i]);
  }
  return 0;
}

// 25/11/2020: COMENTAR MEZCLA ORDENADA Y ORD SEC A 