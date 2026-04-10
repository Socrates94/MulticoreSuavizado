
// Compilación: g++ -O3 fractalJulia.cpp -o fractalJulia && ./fractalJulia

#include <iostream>
#include <fstream>
#include <complex>
#include <cmath>
#include <algorithm>
#include <vector>

using namespace std;

/**
 * Genera un Conjunto de Julia 8K (8192x8192) en formato P5 (PGM Binario).
 */
int main() {
    const int width = 8192;
    const int height = 8192;
    const int max_iter = 500;

    // Constante compleja para el conjunto de Julia: -0.8 + 0.156i
    const complex<double> c(-0.8, 0.156);

    ofstream file("fractalJulia.pgm", ios::binary);
    if (!file.is_open()) {
        cerr << "Error al crear fractalJulia.pgm" << endl;
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
                // Renormalización para suavizado
                double log_zn = log(abs(z));
                double nu = log(log_zn / log(2.0)) / log(2.0);
                smooth_color = (double)n + 1.0 - nu;
            }

            int pixel_val = 0;
            if (n < max_iter) {
                // Mapeo sinusoidal
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

    cout << "Fractal 8K 'fractalJulia.pgm' generado con éxito (67 MB)." << endl;
    return 0;
}
