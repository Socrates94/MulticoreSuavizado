// Compilación: g++ -O3 -o fractal_pesado fractal_pesado.cpp
#include <iostream>
#include <fstream>
#include <complex>
#include <cmath>
#include <algorithm>
#include <vector>

using namespace std;

/**
 * Genera un Conjunto de Julia en resolución 8K (8192x8192).
 * Formato: P5 (PGM Binario) para mayor eficiencia ocupando solo ~67MB.
 * Este fractal es ideal para estresar algoritmos de suavizado en CPU (versión escalar).
 */
int main() {
    const int width = 8192;
    const int height = 8192;
    const int max_iter = 500;

    // Constante compleja para el conjunto de Julia: -0.8 + 0.156i
    const complex<double> c(-0.8, 0.156);

    ofstream file("fractal_pesado.pgm", ios::binary);
    if (!file.is_open()) {
        cerr << "Error al crear fractal_pesado.pgm" << endl;
        return -1;
    }

    // Encabezado PGM P5 (Binario)
    file << "P5\n" << width << " " << height << "\n255\n";

    // Reservamos memoria para optimizar la escritura
    vector<unsigned char> pixels(width * height);

    cout << "Generando fractal 8K (8192x8192)... paciencia." << endl;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Transformamos coordenadas: x [-1.5, 1.5], y [-1.5, 1.5]
            double zr = -1.5 + 3.0 * x / width;
            double zi = -1.5 + 3.0 * y / height;
            
            complex<double> z(zr, zi);
            
            int n = 0;
            while (abs(z) <= 2.0 && n < max_iter) {
                z = z * z + c;
                n++;
            }
            
            double smooth_color = 0.0;
            if (n < max_iter) {
                // Algoritmo de Renormalización para suavizado
                double log_zn = log(abs(z));
                double nu = log(log_zn / log(2.0)) / log(2.0);
                smooth_color = (double)n + 1.0 - nu;
            }

            int pixel_val = 0;
            if (n < max_iter) {
                // Mapeo sinusoidal para un look "heavy" y detallado
                pixel_val = static_cast<int>(255.0 * 0.5 * (1.0 + sin(0.15 * smooth_color + 3.0)));
                pixel_val = max(0, min(255, pixel_val));
            } else {
                pixel_val = 0;
            }
            
            pixels[y * width + x] = static_cast<unsigned char>(pixel_val);
        }
        if (y % 1000 == 0) cout << "Progreso: " << (y * 100 / height) << "%" << endl;
    }

    // Escribimos todo el buffer binario de una vez
    file.write(reinterpret_cast<char*>(pixels.data()), pixels.size());
    file.close();

    cout << "Fractal 8K 'fractal_pesado.pgm' generado con éxito (67 MB)." << endl;
    return 0;
}
