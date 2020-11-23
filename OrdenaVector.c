/* Ordenación de un vector
 * 
 * Compilación: gcc -W OrdenaVector.c -o OrdenaVector -fopenmp
 * Ejecución: ./OrdenaVector	
*/

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

void copiarVector(float Vdest[], float V[], int size)
{
  int i;
  for (i = 0; i < size; ++i)	Vdest[i] = V[i];
}

void printVector(float vector[], int size)
{
  for (int i = 0; i < size; ++i)	printf("%0.4f\t", vector[i]);
  printf("\n\n");
}

int estaOrdenado(float vector[], int size)
{
  int i=0;
  do i++; while ((vector[i-1]<=vector[i])&&(i<size-1));
  
  return ((i==size-1)&&(vector[i-1]<=vector[i]));	// TRUE (1) si el vector está ordenado y FALSE (0) en caso contrario
}

int vectoresIguales(float vecta[], float vectb[], int size)
{
	int i, ini, fin, iguales = TRUE, np = omp_get_num_procs();	// número de núcleos del microprocesador
	for (i=0; i<np; i++)
	{
		ini = i*size/np;	fin =(i+1)*size/np;
		int j=ini;
		while ((vecta[j] == vectb[j]) && (j<fin)) j++;
		iguales = iguales && (j==fin);
	}
	return iguales;
}


/* Función que mezcla dos fragmentos ordenados contiguos de un vector en un solo fragmento ordenado
 * Fragmentos ordenados de entrada: (vector[ini1]... vector[ini2-1]),		(vector[ini2]... vector[fin2])
 * Fragmento ordenado que contiene el resultado:	(vector[ini1]... vector[ini2-1], vector[ini2]... vector[fin2]) 
*/
void mezcla_ordenada(float vector[], int ini1, int ini2, int fin2)
{
	int i, j, k, terminado = 0; float temp;
	i = ini1; j = ini2;
	do
		{	// vector[ini1] <=...<= vector[j-1] AND vector[j] <=...<= vector[fin2] AND i < j
			while ((vector[i] <= vector[j]) && (i < j-1)) i++;
			if (vector[i] > vector[j])
				{
					/* Rotamos vector[i], vector[i+1], ...,vector[j-1], vector[j]
					 * para que vector[j] pase a la posición i y el resto se desplace una posición a la derecha */
					temp = vector[j];
					for (k=j; k>i ; k--) vector[k] = vector[k-1];
					vector[i] = temp;
					if (j==fin2) terminado = 1;
						// Hemos colocado en su posición el último elemento del segundo fragmento
					else {j++; i++;}
				}
			// vector[ini1] <=...<= vector[j-1] AND vector[j] <=...<= vector[fin2] AND i < j
			else terminado = 1;
				 /* i == j-1 AND vector[i] <= vector[j], 
				 * luego vector[j-1] <= vector[j] y todo queda ordenado*/
		}
	while (!terminado) ;
}

// Funciones que ordenan los size primeros elementos de un vector

void ord_secA(float vector[], int size)
{
	int incr, i, fin2;

	for (incr = 2; incr < 2*size; incr= 2*incr)	
		for (i= 0; i < (size-incr/2); i += incr)		// (i+incr/2) < size
		{	fin2 = min(size-1,i+incr-1);
			mezcla_ordenada(vector,i,i+incr/2,fin2);	
			/* Quedan ordenados incr componentes consecutivos del vector (índices i,i+1,...i+incr-1)
			 * (el último trozo ordenado puede ser menor: índices i, i+1,...size-1)
			*/
		}
		/* El último valor de incr cumple size <= incr < 2*size. 
		 * Esto conlleva que está ordenado todo el vector, al ser size <= incr */
}	// Fin de ord_secA


void ord_secB(float vector[], int size)
{
  int i,j; float x;
  
  for (int i = 1; i < size; i++)
  {
	 x = vector[i]; j = i-1;
	 while ((x<vector[j])&&(0<=j))
	 {
		 vector[j+1]=vector[j]; j--;
	 }
	 vector[j+1]=x;
  }
}	// Fin de ord_secB

void ord_secC(float vector[], int size)
{
   int list_length, i; float temp;
   
   for (list_length = size; list_length >= 2; list_length--)
   /* El siguiente for no se puede paralelizar ya que, por ejemplo, en la iteración i=0 
    * puede que haya que escribir en vector[1] y en la iteración siguiente (i=1) hay que leer vector[1].
    * Así pues, si se cumple la condición del "if" hay una dependencia leer después de escribir (RAW)
    * entre la iteración 0 y la 1.
    * [ En general, hay una dependencia RAW entre cualquier iteración, i=k,
    * (que no sea la última) y la siguiente respecto al operando vector[k+1], si se cumple la condición
    * del "if", cosa que puede ocurrir o no dependiendo de los datos de entrada).] 	*/
      for (i = 0; i < list_length-1; i++)
         if (vector[i] > vector[i+1])
         {
            temp = vector[i];
            vector[i] = vector[i+1];
            vector[i+1] = temp;
         }

}  // Fin de ord_secC

void ord_secD(float vector[], int size)
{
   int phase, i; float temp;

   for (phase = 0; phase < size; phase++) 
      if (phase % 2 == 0)
      { // Fase par
         for (i = 1; i < size; i += 2) 
            if (vector[i-1] > vector[i])
            {
               temp = vector[i];
               vector[i] = vector[i-1];
               vector[i-1] = temp;
            }
      } else
      { // Fase impar
         for (i = 1; i < size-1; i += 2)
            if (vector[i] > vector[i+1])
            {
               temp = vector[i];
               vector[i] = vector[i+1];
               vector[i+1] = temp;
            }
      }
}  // Fin de ord_secD

int main()
{
	int i; double t;
	
	// 1. Dar valores aleatorios al vector en el intervalo [0, M[
	srand((unsigned)time(NULL));
	float faux;
	for (i=0; i < VECT_SIZE; i++)	{
		faux = (float)(rand()%RAND_MAX)/RAND_MAX;
		// faux se distribuye con igual probabilidad a lo largo del intervalo [0,1[
		vini[i] = M *faux;
	}
		
	// 2. Imprimir vector desordenado (solo si el número de componentes no es muy grande)
	if (VECT_SIZE <= 400)	{
		printf("\nVector antes de ser ordenado: \n");
		printVector(vini,VECT_SIZE);
	}
	
	// 3. Para el primer método de ordenación, copiar vini en vord0, ordenar vord0 y comprobar que el vector queda ordenado
	//	  Para el resto de métodos, copiar vini en vord, ordendar vord y comprobar que vord queda igual que vord0
	//	  Para todos los métodos medir e imprimir el tiempo de copiar y ordenar el vector

	
	for (i=0; i<NM; i++)
	{
		t = omp_get_wtime();
		printf("=================================================================\n");
		switch (i)
		{
			case	0:
				copiarVector(vord0, vini, VECT_SIZE);	// vord0 <-- vini
				printf("Ordenando por el método secuencial A\n");
				ord_secA(vord0,VECT_SIZE);
				printf("\nTiempo empleado por método secuencial A: %0.8f milisegundos\n",1000*(omp_get_wtime()-t));
				if (estaOrdenado(vord0,VECT_SIZE)) printf("\nEl vector obtenido por el método secuencial A está ordenado\n");
				else printf("\nEl vector obtenido por el método secuencial A no está ordenado\n");
				break;
			case	1:
				copiarVector(vord, vini, VECT_SIZE);	// vord <-- vini
				printf("Ordenando por el método secuencial B\n");
				ord_secB(vord,VECT_SIZE);
				printf("\nTiempo empleado por método secuencial B: %0.8f milisegundos\n",1000*(omp_get_wtime()-t));
				if (vectoresIguales(vord0, vord, VECT_SIZE))
					printf("\nEl vector obtenido por el método secuencial B coincide con el del método secuencial A\n");
				else printf("\nEl vector obtenido por el método secuencial B no coincide con el del método secuencial A\n");			
				break;
			case	2:
				copiarVector(vord, vini, VECT_SIZE);	// vord <-- vini			
				printf("Ordenando por el método secuencial C\n");
				ord_secC(vord,VECT_SIZE);
				printf("\nTiempo empleado por método secuencial C: %0.8f milisegundos\n",1000*(omp_get_wtime()-t));
				if (vectoresIguales(vord0, vord, VECT_SIZE))
					printf("\nEl vector obtenido por el método secuencial C coincide con el del método secuencial A\n");
				else printf("\nEl vector obtenido por el método secuencial C no coincide con el del método secuencial A\n");
				break;
			case	3:
				copiarVector(vord, vini, VECT_SIZE);	// vord <-- vini			
				printf("Ordenando por el método secuencial D\n");
				ord_secD(vord,VECT_SIZE);
				printf("\nTiempo empleado por método secuencial D: %0.8f milisegundos\n",1000*(omp_get_wtime()-t));
				if (vectoresIguales(vord0, vord, VECT_SIZE))
					printf("\nEl vector obtenido por el método secuencial D coincide con el del método secuencial A\n");
				else printf("\nEl vector obtenido por el método secuencial D no coincide con el del método secuencial A\n");			
				break;				
		}
	}

	
	// 4. Imprimir vector ordenado (solo si el número de componentes no es muy grande)
	printf("=================================================================\n");
	if (VECT_SIZE <= 400)	{
		printf("\nVector ordenado: \n");
		printVector(vord,VECT_SIZE);
	}
	printf("\n");
}
