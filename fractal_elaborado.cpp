// Compilación: g++ -O3 -o fractal_elaborado fractal_elaborado.cpp
#include <iostream>
#include <fstream>
#include <complex>
#include <cmath>
#include <algorithm>

using namespace std;

/**
 * Función para generar un conjunto de Julia con coloreado suavizado.
 * El coloreado suavizado elimina el efecto de bandas y crea transiciones graduales.
 */
int main() {
    const int width = 1024;
    const int height = 1024;
    const int max_iter = 1000;

    // Constante compleja para el conjunto de Julia: -0.7 + 0.27015i (muy estética)
    const complex<double> c(-0.7, 0.27015);

    ofstream file("fractal_elaborado.pgm");
    if (!file.is_open()) {
        cerr << "Error al abrir el archivo fractal_elaborado.pgm" << endl;
        return -1;
    }

    // Encabezado PGM P2 (ASCII)
    file << "P2\n" << width << " " << height << "\n255\n";

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Transformamos coordenadas de píxel al plano complejo
            // Rango: x [-1.5, 1.5], y [-1.5, 1.5]
            double zr = -1.5 + 3.0 * x / width;
            double zi = -1.5 + 3.0 * y / height;
            
            complex<double> z(zr, zi);
            
            int n = 0;
            // Algoritmo de escape: iteramos hasta que |z| > 2 o max_iter
            while (abs(z) <= 2.0 && n < max_iter) {
                z = z * z + c;
                n++;
            }
            
            double smooth_color = 0.0;
            if (n < max_iter) {
                // Algoritmo de Renormalización para suavizado
                // log(log(|z|)) / log(2)
                double log_zn = log(abs(z));
                double nu = log(log_zn / log(2.0)) / log(2.0);
                smooth_color = (double)n + 1.0 - nu;
            } else {
                smooth_color = 0.0;
            }

            // Mapeo del valor suave a un rango de 0-255
            // Usamos una función seno o un mapeo lineal para que se vea bien
            int pixel_val = 0;
            if (n < max_iter) {
                // El mapeo puede ajustarse para resaltar detalles. 
                // Usamos una escala logarítmica o lineal simple multiplicada por un factor.
                pixel_val = static_cast<int>(255.0 * 0.5 * (1.0 + sin(0.1 * smooth_color)));
                pixel_val = max(0, min(255, pixel_val));
            } else {
                pixel_val = 0; // El centro del fractal suele ser negro o blanco
            }
            
            file << pixel_val << " ";
        }
        file << "\n";
    }

    file.close();
    cout << "Fractal elaborado 'fractal_elaborado.pgm' generado con éxito (1024x1024)." << endl;
    return 0;
}
