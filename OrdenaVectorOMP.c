/* Ordenación de un vector
 *
 * Compilación: gcc -W OrdenaVectorOMP.c -o OrdenaVectorOMP -fopenmp
 * Ejecución: ./OrdenaVectorOMP	
*/

/*
 * Autores:
 * - Sergio Jiménez Roncero
 * - Javier Álvarez Páramo
 */

/*
 * Procesador:
 * - Intel Core i7-7850H (2.20GHz a 4.1GHz (Turbo Boost))
 * - 6 Cores - 12 Hilos
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define M 100			// ==> Rango de valores de los componentes: [0, M[
#define VECT_SIZE 20000 // N.º componentes del vector que se quiere ordenar
#define NM 5			// N.º de métodos de ordenación que se llamarán desde el programa principal
#define min(a, b) ((a) < (b) ? a : b)
#define FALSE 0
#define TRUE 1

// Variables globales
float vini[VECT_SIZE], vord0[VECT_SIZE], vord[VECT_SIZE]; // vector desordenado y vector que se ordenará, respectivamente

void copiarVector(float Vdest[], float V[], int size)
{
	int i;
	#pragma omp parallel for schedule(dynamic)
	for (i = 0; i < size; ++i)
		Vdest[i] = V[i];
}

void printVector(float vector[], int size)
{
	/* 
   * Este bucle for no se puede paralelizar porque al distribuir las distintas iteraciones entre los distintos hilos
   * pueden existir hilos que tarden menos que otros y en consecuencia algunas iteraciones se impriman antes que otras
   * alterando el orden original de vector.
   */
	for (int i = 0; i < size; ++i)
		printf("%0.4f\t", vector[i]);
	printf("\n\n");
}

int estaOrdenado(float vector[], int size)
{
	/* Comenzamos a paralelizar el bucle y para ello la primera transformación que hicimos fue pasar de un bucle do-while,
	 * a un bucle for cuya condición era la que este bucle while tiene. Nos dimos cuenta que OpenMP no permitía este tipo de condiciones
	 * al desconocer el numero de iteraciones que realiza. Entonces convertimo el bucle en uno cuya cabecera era for(i=1; i<size; i++) y con
	 * if nos permitía saber si el orden del vector era creciente y en ese caso continuar o por el contrario salirnos con la sentencia "break".
	 * Lo que nos llevó a una segunda apreciación y es que dicha llamada no está permitida al manejar varios hilos de ejecución.
	 * La CONCLUSIÓN FINAL fue que llegamos a la conclusión de que este bucle no es paralelizable.
	 */
	int i = 0;
	do
		i++;
	while ((vector[i - 1] <= vector[i]) && (i < size - 1));

	return ((i == size - 1) && (vector[i - 1] <= vector[i])); // TRUE (1) si el vector está ordenado y FALSE (0) en caso contrario
}

int vectoresIguales(float vecta[], float vectb[], int size)
{
	int i, ini, fin, iguales = TRUE, np = omp_get_num_procs(); // número de núcleos del microprocesador
	#pragma omp parallel for schedule(dynamic) private(ini, fin)
	for (i = 0; i < np; i++)
	{
		ini = i * size / np;
		fin = (i + 1) * size / np;
		int j = ini;
		while ((vecta[j] == vectb[j]) && (j < fin))
			j++;

		#pragma omp critical
		{
			iguales = iguales && (j == fin);
		}
	}
	return iguales;
}

/* Función que mezcla dos fragmentos ordenados contiguos de un vector en un solo fragmento ordenado
 * Fragmentos ordenados de entrada: (vector[ini1]... vector[ini2-1]),		(vector[ini2]... vector[fin2])
 * Fragmento ordenado que contiene el resultado:	(vector[ini1]... vector[ini2-1], vector[ini2]... vector[fin2]) 
*/
void mezcla_ordenada(float vector[], int ini1, int ini2, int fin2)
{
	int i, j, k, terminado = 0;
	float temp;
	i = ini1;
	j = ini2;
	do
	{													// vector[ini1] <=...<= vector[j-1] AND vector[j] <=...<= vector[fin2] AND i < j
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

// Funciones que ordenan los size primeros elementos de un vector

void ord_parA(float vector[], int size)
{
	int incr, i, fin2;
	/*
	 * Este bucle for externo no se puede paralelizar ya que todavía no están ordenados los componentes del vector ya sean en grupos que 
	 * la variable 'incr' dicta (2,4,8...) y cuyo orden dependen de que el bucle interno, cuyas iteraciones si han sido paralelizadas, termine correctamente.
	 * El algoritmo mezcla ordenada requiere que cada mitad de vector esté ordenada a su vez y eso implica que tenga que haber terminado la iteración previa.
	 */
	for (incr = 2; incr < 2 * size; incr = 2 * incr)
		#pragma omp parallel for schedule(static) private(fin2)
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

void ord_parB(float vector[], int size)
{
	int i, j;
	float x;
	/* 
	 * Este bucle for no se puede paralelizar porque requiere que las anteriores posiciones con respecto a i estén ordenadas. Nos basamos en un bucle en 
	 * el cual cada iteración utiliza x posiciones de vector, + 1 posición añadida en cada iteración. Si lanzamos 8 hilos por ejemplo, al ejecutar dicho bucle 
	 * trabajaran en comun en un mismo vector desordenado y en el que probablemente aparecerán inconsistencias.
	 * En nuestra opinión, este algoritmo debe desarrollarse de manera necesariamente secuencial.
	 * 
	 * (R=read; W=write)
	 * Para i=1 
	 * R = vector[1] 
	 * R = vector[0]
	 * W = vector[1]
	 * W = vector[2]
	 * 
	 * Para i=2
	 * R = vector[2] 
	 * R = vector[1]
	 * W = vector[2]
	 * W = vector[3]
	 * 
	 * Como vemos en este esquema simple se produce una dependencia RAW entre la iteración i=j e i=j+1 ya que en la iteración i=j, dentro del bucle while la primera iteración
	 * escribiría en el vector[j+1], que en el caso de la siguiente iteracion (i=j+1) leerá dicha posición del vector al hacer "x=vector[j+1]". Por esta dependencia loop-carried
	 * este bucle no es paralelizable con dicha implementación.
	 */
	for (int i = 1; i < size; i++)
	{
		x = vector[i];
		j = i - 1;
		while ((x < vector[j]) && (0 <= j))
		{
			vector[j + 1] = vector[j];
			j--;
		}
		vector[j + 1] = x;
	}
} // Fin de ord_secB

void ord_parC(float vector[], int size)
{
	int list_length, i;
	float temp;
	
	/* 
	 * Cada iteración tiene por objetivo encontrar el número mayor, y dejarlo apartado en la última posición del vector de tamaño N para esa iteración i=j; para que, 
	 * de esta forma, en la siguiente iteración (i=j+1), con un vector de tamaño N-1, ya no se contemple ese número grande. Por esa razón, una iteración debe haberse completado 
	 * y haber retirado su número más grande del campo de visión del vector de la iteración posterior, antes de que esta comience. Si no fuese así, y una iteración posterior comenzase 
	 * antes de que la anterior previa hubiese terminado, podría ocurrir que, ambas iteraciones escojan el mismo número más grande, y el algoritmo, por tanto no cumpla su cometido. 
	 * Como curiosidad, la complejidad temporal de este algoritmo viene acotada por O(n^2) porque hay n iteraciones y posiciones, y por cada una, se recorre (n-i) posiciones.
	 */
	for (list_length = size; list_length >= 2; list_length--)
		/* 
		* El siguiente for no se puede paralelizar ya que, por ejemplo, en la iteración i=0 
		* puede que haya que escribir en vector[1] y en la iteración siguiente (i=1) hay que leer vector[1].
		* Así pues, si se cumple la condición del "if" hay una dependencia leer después de escribir (RAW)
		* entre la iteración 0 y la 1.
		* [ En general, hay una dependencia RAW entre cualquier iteración, i=k,
		* (que no sea la última) y la siguiente respecto al operando vector[k+1], si se cumple la condición
		* del "if", cosa que puede ocurrir o no dependiendo de los datos de entrada).] 	
		*/
		for (i = 0; i < list_length - 1; i++)
			if (vector[i] > vector[i + 1])
			{
				temp = vector[i];
				vector[i] = vector[i + 1];
				vector[i + 1] = temp;
			}

} // Fin de ord_secC

void ord_parD(float vector[], int size)
{
	int phase, i;
	float temp;

	/*
	 * El siguiente for no se puede paralelizar ya que entre una fase y la siguiente hay dependencias RAW:
     * por ejemplo, un fase puede escribir en vector[1] y la siguiente lee vector[1]
	 */
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

void ord_parDm(float vector[], int size)
{
	int phase, i;
	float temp;

	/*
	 * El siguiente for no se puede paralelizar ya que entre una fase y la siguiente hay dependencias RAW:
     * por ejemplo, un fase puede escribir en vector[1] y la siguiente lee vector[1]
	 */
	for (phase = 0; phase < size; phase++)
	{
		#pragma omp parallel for private(temp)
		for (i = 1; i < size - (phase % 2); i += 2)
			if (vector[i - 1 + (phase % 2)] > vector[i + (phase % 2)])
			{
				temp = vector[i];
				vector[i] = vector[i - 1 + 2 * (phase % 2)];
				vector[i - 1 + 2 * (phase % 2)] = temp;
			}
	}
} // Fin de ord_parDm

int main()
{
	int i;
	double t;

	// 1. Dar valores aleatorios al vector en el intervalo [0, M[
	srand((unsigned)time(NULL));
	float faux;
	for (i = 0; i < VECT_SIZE; i++)
	{
		faux = (float)(rand() % RAND_MAX) / RAND_MAX;
		// faux se distribuye con igual probabilidad a lo largo del intervalo [0,1[
		vini[i] = M * faux;
	}

	// 2. Imprimir vector desordenado (solo si el número de componentes no es muy grande)
	if (VECT_SIZE <= 400)
	{
		printf("\nVector antes de ser ordenado: \n");
		printVector(vini, VECT_SIZE);
	}

	// 3. Para el primer método de ordenación, copiar vini en vord0, ordenar vord0 y comprobar que el vector queda ordenado
	//	  Para el resto de métodos, copiar vini en vord, ordendar vord y comprobar que vord queda igual que vord0
	//	  Para todos los métodos medir e imprimir el tiempo de copiar y ordenar el vector

	for (i = 0; i < NM; i++)
	{
		t = omp_get_wtime();
		printf("=================================================================\n");
		switch (i)
		{
		case 0:
			copiarVector(vord0, vini, VECT_SIZE); // vord0 <-- vini
			printf("Ordenando por el método paralelo A\n");
			ord_parA(vord0, VECT_SIZE);
			printf("\nTiempo empleado por método paralelo A: %0.8f milisegundos\n", 1000 * (omp_get_wtime() - t));
			if (estaOrdenado(vord0, VECT_SIZE))
				printf("\nEl vector obtenido por el método paralelo A está ordenado\n");
			else
				printf("\nEl vector obtenido por el método paralelo A no está ordenado\n");
			break;
		case 1:
			copiarVector(vord, vini, VECT_SIZE); // vord <-- vini
			printf("Ordenando por el método paralelo B\n");
			ord_parB(vord, VECT_SIZE);
			printf("\nTiempo empleado por método paralelo B: %0.8f milisegundos\n", 1000 * (omp_get_wtime() - t));
			if (vectoresIguales(vord0, vord, VECT_SIZE))
				printf("\nEl vector obtenido por el método paralelo B coincide con el del método paralelo A\n");
			else
				printf("\nEl vector obtenido por el método paralelo B no coincide con el del método paralelo A\n");
			break;
		case 2:
			copiarVector(vord, vini, VECT_SIZE); // vord <-- vini
			printf("Ordenando por el método paralelo C\n");
			ord_parC(vord, VECT_SIZE);
			printf("\nTiempo empleado por método paralelo C: %0.8f milisegundos\n", 1000 * (omp_get_wtime() - t));
			if (vectoresIguales(vord0, vord, VECT_SIZE))
				printf("\nEl vector obtenido por el método paralelo C coincide con el del método paralelo A\n");
			else
				printf("\nEl vector obtenido por el método paralelo C no coincide con el del método paralelo A\n");
			break;
		case 3:
			copiarVector(vord, vini, VECT_SIZE); // vord <-- vini
			printf("Ordenando por el método paralelo D\n");
			ord_parD(vord, VECT_SIZE);
			printf("\nTiempo empleado por método paralelo D: %0.8f milisegundos\n", 1000 * (omp_get_wtime() - t));
			if (vectoresIguales(vord0, vord, VECT_SIZE))
				printf("\nEl vector obtenido por el método paralelo D coincide con el del método paralelo A\n");
			else
				printf("\nEl vector obtenido por el método paralelo D no coincide con el del método paralelo A\n");
			break;
		case 4:
			copiarVector(vord, vini, VECT_SIZE); // vord <-- vini
			printf("Ordenando por el método paralelo D mejorado\n");
			ord_parDm(vord, VECT_SIZE);
			printf("\nTiempo empleado por método paralelo D mejorado: %0.8f milisegundos\n", 1000 * (omp_get_wtime() - t));
			if (vectoresIguales(vord0, vord, VECT_SIZE))
				printf("\nEl vector obtenido por el método paralelo D mejorado coincide con el del método paralelo A\n");
			else
				printf("\nEl vector obtenido por el método paralelo D mejorado no coincide con el del método paralelo A\n");
			break;
		}
	}

	// 4. Imprimir vector ordenado (solo si el número de componentes no es muy grande)
	printf("=================================================================\n");
	if (VECT_SIZE <= 400)
	{
		printf("\nVector ordenado: \n");
		printVector(vord, VECT_SIZE);
	}
	printf("\n");
}
