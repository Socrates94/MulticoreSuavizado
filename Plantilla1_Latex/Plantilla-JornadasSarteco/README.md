# Análisis de Rendimiento y Vectorización SIMD: Operador Sobel

Este repositorio contiene el código fuente en LaTeX y los recursos gráficos del artículo de investigación científico: **"Análisis de Rendimiento y Vectorización SIMD de un Kernel Numérico para Detección de Bordes basado en el Operador Sobel"**.

## Descripción del Proyecto
El proyecto evalúa empíricamente el impacto del paralelismo a nivel de datos (SIMD) en procesadores modernos. Se desarrollaron cuatro implementaciones en C++ de un kernel de detección de bordes (Operador Sobel) para contrastar el rendimiento entre la ejecución secuencial y la vectorizada:
1. **Escalar:** Línea base secuencial (`-fno-vectorize`).
2. **Automática:** Vectorización delegada al compilador (`-O3`, `__restrict`).
3. **Guiada:** Paralelismo forzado mediante directivas OpenMP (`#pragma omp simd`).
4. **Explícita:** Inyección manual de intrínsecas AVX2 (`<immintrin.h>`).

El análisis de rendimiento y la detección de cuellos de botella se realizaron utilizando el modelo **Roofline** a través de Intel Advisor.

## Tecnologías y Herramientas
* **Lenguaje:** C++
* **Compilador:** `icpx` (Intel oneAPI Base Toolkit)
* **Perfilado de Hardware:** Intel Advisor
* **Paralelismo:** SIMD (AVX2), OpenMP
* **Documentación:** LaTeX (Plantilla formato IEEE / Jornadas Sarteco)

## Conclusiones Principales
El perfilado de hardware demostró que, a pesar de lograr una correcta emisión de instrucciones de 256 bits y alcanzar un rendimiento matemático de **9.50 GFLOPS**, el *speedup* global de la aplicación es estadísticamente nulo. El algoritmo posee una intensidad aritmética críticamente baja (0.069 FLOP/Byte), lo que satura inmediatamente el bus de memoria DRAM (~29 GB/s), clasificando el problema como estrictamente **Memory-Bound**.

## Estructura del Documento LaTeX
El documento está diseñado de forma modular para facilitar su edición y compilación:

* `main.tex`: Fichero principal(incluye el 1. titulo, autores, afiliación, datos de contacto, resumen y palabras clave) que orquesta la plantilla y los paquetes.
* `02-Resumen.tex`: Abstract y Keywords.
* `03-Introduccion.tex`: Contexto del procesamiento de imágenes.
* `04-MarcoTeorico.tex`: Fundamentos matemáticos, formato PGM y modelo SIMD.
* `05-Metodologia.tex`: Especificaciones del hardware (Ryzen 9, 128GB RAM) y parámetros de compilación.
* `06-Resultados.tex`: Tiempos de ejecución y gráficas Roofline.
* `07-Discusion.tex`: Análisis del fenómeno de inanición de datos.
* `08-Conclusiones.tex`: Diagnóstico final y trabajo futuro.
* `biblio.bib`: Referencias bibliográficas.
* `/img/`: Carpeta que contiene las capturas de la terminal y los reportes de Intel Advisor.

## Cómo compilar
se ocupo un plugin de vscode(o forks de vscode) para compilar latex y generar el pdf de forma automatica.
