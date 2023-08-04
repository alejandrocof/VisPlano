PLANO

Plano es un programa que se ejecuta en línea de comandos para la creación de imágenes png de la visualización de uno o varios planos de corte a partir de los datos de un volumen de simulación sísmica.

**********************************************************

DEPENDENCIAS

Para compilar se requiere la biblioteca de OpenMP que permite ejecutar varios procesos y añadir concurrencia a los programas escritos en C, C++ y Fortran. Para instalar OpenMP ejecute en la terminal:

sudo apt-get install libomp-dev

**********************************************************
 
COMPILAR
Se incluye un Makefile para compilar el proyecto, sólo ejecute

make

también se puede usar el script Compilar462

./Compilar462

**********************************************************

DIRECTORIO

El programa se ejecuta sobre un directorio que deberá contener los subdirectorios imagenes y dentro de ella tres carpetas de nombre X, Y y Z respectivamente, en el ejemplo representado en la imagen el programa se debería ejecutar estando dentro de la carpeta Plano

**********************************************************

CREACIÓN DE IMÁGENES

Para crear las imágenes ejecute el programa Plano con las opciones y los valores requeridos, por ejemplo:

./Plano -r /alicefs/mcg_g/mcg/OLD-Cerjan/SismoOaxaca090720/ -vx -min -0.02 -max 0.02 -px 200 550 -py 225 575 -pz 0 100 250 297

a continuación se describen cada una de las opciones de Plano:

Ruta
-r "./ruta" (único y obligatorio)
donde "./ruta" es la ruta donde se encuentra el archivo input.dat y se encuentra el directorio result

Paso
-step i (único y opcional)
donde i es el paso de la simulación que se desea hacer su visualización, si no se agrega -step se genera la visualización de todos los pasos.

Máximos y mínimos
-min x
define el mínimo (obligatorio)
-max x
define el máximo (obligatorio)
*se pueden dar varios valores mínimos o máximos
una vez que se definen los máximos y mínimos los planos que se agreguen se visualizarán con los últimos valores definidos.

Componentes
-vx (opcional)
selecciona la componente x
-vy (opcional)
selecciona la componente y
-vz (opcional)
selecciona la componente z
*por omisión se visualizará la componente x (Vx)

Planos a visualizar
-px i
agrega el plano i'esimo para visualizar, se pueden agregar tantos planos como se desee 
-py i
agrega el plano i'esimo para visualizar, se pueden agregar tantos planos como se desee 
-pz i
agrega el plano i'esimo para visualizar, se pueden agregar tantos planos como se desee 

**********************************************************

CREACIÓN DE ANIMACIÓN

Para generar una animación a partir de las imágenes obtenidas puede usar mencoder para poner todas las imágenes juntas y crear un video, primero instale mencoder si aún no lo tiene instalado:

sudo apt-get install mencoder 

Primero indique la ubicación de las imágenes que serán usadas para construir el video
 "mf://./ruta/*.png" -mf
Indique a cuántos cuadros por segundo se generará el video, por ejemplo si el dt=0.03s de la simulación, entonces esto corresponde al tiempo entre cada imagen, entonces en un segundo se tendrá que desplegar 1/dt=1/0.03=33.33 imágenes, de ahí tendremos que: 
fps=33
Luego se debe indicar el nombre del video que se creará 
-o nombrevideo.avi
y finalmente los codecs de video a usar
-ovc lavc -lavcopts vcodec=mpeg4

Ejemplo:

mencoder "mf://./X/Vx3D-X200*.png" -mf fps=33 -o videoX200.avi -ovc lavc -lavcopts vcodec=mpeg4

Se incluye un pequeño script que hace uso de mencoder, para generar una animación simplemente ejecute:

CreaVideo

*Nota: el script CreaVideo deberá modificarse de acuerdo a sus necesidades, úselo simplemente como una guía de como construir su propio script.

