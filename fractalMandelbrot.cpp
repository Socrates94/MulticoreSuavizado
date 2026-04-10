// Compilar: g++ -O3 -o fractalMandelbrot fractalMandelbrot.cpp && ./fractalMandelbrot

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

int main() {
    // Imagen 8K para stress test
    int width = 7680; 
    int height = 4320;
    int max_iter = 255;
    
    // Coordenadas matemáticas para encuadrar perfectamente el Fractal de Mandelbrot
    // Evita que la imagen se estire ajustando el área visible a las proporciones 16:9 (8K)
    double center_re = -0.75;
    double center_im = 0.0;
    double height_range = 2.5;
    double width_range = height_range * (double)width / height;
    
    double min_re = center_re - width_range / 2.0;
    double max_re = center_re + width_range / 2.0;
    double min_im = center_im - height_range / 2.0;
    double max_im = center_im + height_range / 2.0;
    
    // Buffer para guardar la imagen
    vector<unsigned char> img(width * height);
    
    cout << "Generando Mandelbrot 8K (" << width << "x" << height << ")..." << endl;
    
    // Generación paralela con OpenMP
    #pragma omp parallel for schedule(dynamic)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Mapeando las coordenadas X e Y a coordenadas del Plano Complejo
            double cr = min_re + (max_re - min_re) * x / width;
            double ci = min_im + (max_im - min_im) * y / height;
            
            // z(n+1) = z(n)^2 + c
            double zr = 0.0;
            double zi = 0.0;
            int iter = 0;
            
            // Criterio de escape
            while (zr * zr + zi * zi <= 4.0 && iter < max_iter) {
                double temp = zr * zr - zi * zi + cr; // Parte real
                zi = 2.0 * zr * zi + ci;              // Parte imaginaria
                zr = temp;
                iter++;
            }
            
            // Asignando color según los cálculos.
            // Si llega a max_iter (no escapó), es el interior del conjunto (Negro)
            // Si escapó rápido, será un tono más blanco/gris
            unsigned char pixel_val = 0;
            if (iter < max_iter) {
                // Mapeo de color con contraste
                pixel_val = static_cast<unsigned char>((iter * 8) % 256);
            }
            
            img[y * width + x] = pixel_val;
        }
    }
    
    // Escribir archivo en formato PGM binario (P5) puro
    string filename = "fractalMandelbrot.pgm";
    ofstream file(filename, ios::binary);
    if (!file) {
        cerr << "Error al crear el archivo " << filename << endl;
        return 1;
    }
    
    file << "P5\n" << width << " " << height << "\n255\n";
    file.write(reinterpret_cast<char*>(img.data()), width * height);
    file.close();
    
    cout << "¡Éxito! Fractal de Mandelbrot guardado como '" << filename << "' (" << width * height / 1024 / 1024 << " MB)" << endl;
    return 0;
}
