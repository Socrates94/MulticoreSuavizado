// Compilación: g++ -O3 -o fractal fractal.cpp
#include <iostream>
#include <fstream>
#include <complex>

using namespace std;

int main() {
    int width = 1024;
    int height = 1024;
    int max_iter = 255;

    ofstream file("fractal.pgm");
    if (!file.is_open()) return -1;

    // Encabezado PGM P2 (ASCII)
    file << "P2\n" << width << " " << height << "\n255\n";

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Transformamos coordenadas de píxel a coordenadas complejas
            // Rango: x [-2.0, 1.0], y [-1.5, 1.5]
            double pr = -2.0 + 3.0 * x / width;
            double pi = -1.5 + 3.0 * y / height;
            
            complex<double> c(pr, pi);
            complex<double> z(0, 0);
            
            int n = 0;
            while (abs(z) <= 2.0 && n < max_iter) {
                z = z * z + c;
                n++;
            }
            
            file << n << " ";
        }
        file << "\n";
    }

    file.close();
    cout << "Fractal 'fractal.pgm' generado con éxito (1024x1024)." << endl;
    return 0;
}
