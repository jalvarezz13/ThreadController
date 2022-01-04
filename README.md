<img src="https://github.com/jalvarezz13/ThreadController/blob/master/images/logo.png" width="" height="80" align = "left">

# ThreadController
## Contenido
Proyecto para la paralelización de nucleos con Open MP, API del lenguaje C usada para la para la programación multiproceso de memoria compartida en múltiples plataformas y que permite añadir concurrencia a los programas escritos en C, C++ y Fortran sobre la base del modelo de ejecución fork-join. El programa consiste en la paralelización de diferentes algoritmos de ordenación de vectores y la medición y analisis de los tiempos de ejecución de los mismos.

<i>*Todos los resultado de la práctica se puede ver en la carpeta "Resultados".</i>

## Como usar
Contiene todo el código para la correcta ejecución del script. Para ello:
  1. Clona el repositorio con 
```
git clone https://github.com/jalvarezz13/ThreadController
```
  2. Compila ordenaVector.c y ordenaVectorOMP.c con GCC
```
gcc ordenaVector.c -o ordenaVector -fopenmp
```
```
gcc ordenaVectorOMP.c -o ordenaVectorOMP -fopenmp
```
  3. Ejecuta los scripts
```
./ordenaVector
```
```
./ordenaVectorOMP
```

## Tecnologías
![C](https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)
  
## Autores
<pre>Javier Álvarez Páramo  <a title="Linkedin" href="https://www.linkedin.com/in/javieralpa/"><img align="right" src="https://img.shields.io/badge/linkedin-%230077B5.svg?style=for-the-badge&logo=linkedin&logoColor=white"/></a></pre>
<pre>Sergio Jiménez Roncero  <a title="Linkedin" href="https://www.linkedin.com/in/sergiojimenezr/"><img align="right" src="https://img.shields.io/badge/linkedin-%230077B5.svg?style=for-the-badge&logo=linkedin&logoColor=white"/></a></pre>
