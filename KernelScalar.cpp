// Compilación: icpx -g -O3 -march=native -fno-inline -o scalar KernelScalar.cpp

#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <fstream>
#include <string>
#include <immintrin.h>
#include <algorithm>

using namespace std;

// Función para leer imagen PGM
void leer_pgm(const string& filename, vector<float>& buffer, int& w, int& h) {

    // Abrimos en modo binario por si acaso es P5
    ifstream file(filename, ios::binary); 
    if (!file.is_open()) {
        cerr << "Error: No se pudo abrir " << filename << endl;
        return;
    }
    
    string header;
    file >> header;
    
    if (header != "P2" && header != "P5") {
        cerr << "Error: El archivo no es un formato PGM válido (No es P2 ni P5)." << endl;
        return;
    }

    // Saltar los comentarios (líneas que empiezan con #)
    char c;
    while (file >> ws && (c = file.peek()) == '#') {
        string dummy;
        getline(file, dummy);
    }

    int max_val;
    file >> w >> h >> max_val;
    buffer.resize(w * h);
    
    if (header == "P2") {
        // Lectura de formato ASCII
        for (int i = 0; i < w * h; i++) {
            int pixel;
            file >> pixel;
            buffer[i] = static_cast<float>(pixel);
        }
    } else if (header == "P5") {
        // Lectura de formato Binario
        file.get(); // Consumir el salto de línea o espacio después del max_val
        vector<unsigned char> temp(w * h);
        file.read(reinterpret_cast<char*>(temp.data()), w * h);
        for (int i = 0; i < w * h; i++) {
            buffer[i] = static_cast<float>(temp[i]);
        }
    }
    file.close();
}

// Función para escribir PGM
void escribir_pgm(const string& filename, float* buffer, int w, int h) {

    ofstream file(filename);
    if (!file.is_open()) return;
    file << "P2\n" << w << " " << h << "\n255\n";
    for (int i = 0; i < w * h; i++) {
        int val = static_cast<int>(min(255.0f, buffer[i] * 2.0f));
        file << val << (i % w == w - 1 ? "\n" : " ");
    }
    file.close();

}

// Preprocesamiento para obtener gradientes reales usando máscaras de Sobel
void calcular_gradientes_sobel(const vector<float>& img, float* Gx, float* Gy, int w, int h) {

    int mx[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    int my[3][3] = {{ 1, 2, 1}, { 0, 0, 0}, {-1,-2,-1}};

    for (int y = 1; y < h - 1; y++) {

        for (int x = 1; x < w - 1; x++) {
            float sum_x = 0, sum_y = 0;

            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    float pixel = img[(y + ky) * w + (x + kx)];
                    sum_x += pixel * mx[ky + 1][kx + 1];
                    sum_y += pixel * my[ky + 1][kx + 1];
                }
            }

            Gx[y * w + x] = sum_x;
            Gy[y * w + x] = sum_y;
        }
    }
}

// KERNEL ESCALAR BASE
void sobel_scalar(const float* __restrict Gx, const float* __restrict Gy, float* __restrict Mag, int N) {
    
    for (int i = 0; i < N; i++) {
        // Cálculo secuencial uno a uno de la magnitud del gradiente
        Mag[i] = sqrtf(Gx[i] * Gx[i] + Gy[i] * Gy[i]);
    }
}

int main() {

    int WIDTH, HEIGHT;
    vector<float> raw_img;
    
    // 1. Cargar imagen original PGM
    leer_pgm("input.pgm", raw_img, WIDTH, HEIGHT);
    if(raw_img.empty()) return -1;

    const int ITERATIONS = 800; 
    const int N = WIDTH * HEIGHT;
    const int TOTAL_SIZE = N * ITERATIONS;

    // Reserva de memoria alineada
    float* Gx = (float*)_mm_malloc(TOTAL_SIZE * sizeof(float), 32);
    float* Gy = (float*)_mm_malloc(TOTAL_SIZE * sizeof(float), 32);
    float* Mag = (float*)_mm_malloc(TOTAL_SIZE * sizeof(float), 32);


    // AGREGA ESTO PARA LIMPIAR LA MEMORIA
    for (int i = 0; i < TOTAL_SIZE; i++) {
        Gx[i] = 0.0f;
        Gy[i] = 0.0f;
        Mag[i] = 0.0f;
    }


    // 2. Extraer los gradientes reales para la simulación
    calcular_gradientes_sobel(raw_img, Gx, Gy, WIDTH, HEIGHT);

    // 3. Replicar para estabilizar las métricas del procesador (Advisor)
    for (int k = 1; k < ITERATIONS; ++k) {
        int offset = k * N;
        for (int i = 0; i < N; ++i) {
            Gx[offset + i] = Gx[i];
            Gy[offset + i] = Gy[i];
        }
    }

    // 4. EJECUCIÓN Y MEDICIÓN DE TIEMPO
    auto start = chrono::high_resolution_clock::now();
    
    sobel_scalar(Gx, Gy, Mag, TOTAL_SIZE);
    
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> diff = end - start;
    cout << "Versión Escalar - Tiempo: " << diff.count() << " s\n";

    // 5. Generar imagen procesada real
    escribir_pgm("output_scalar.pgm", Mag, WIDTH, HEIGHT);
    cout << "Imagen 'output_scalar.pgm' generada correctamente.\n";

    _mm_free(Gx);
    _mm_free(Gy);
    _mm_free(Mag);

    return 0;
}