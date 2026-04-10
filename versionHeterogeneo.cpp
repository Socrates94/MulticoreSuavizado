// Cargar variables de entorno: source /opt/intel/oneapi/setvars.sh
// Compilación: icpx -fiopenmp -fopenmp-targets=spir64 -O3 -march=native -o heterogeneo_v versionHeterogeneo.cpp && ./heterogeneo_v

#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <fstream>
#include <string>
#include <immintrin.h>
#include <algorithm>
#include <omp.h>

using namespace std;

// Lector PGM (P2/P5)
void leer_pgm(const string& filename, vector<float>& buffer, int& w, int& h) {
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

    char c;
    while (file >> ws && (c = file.peek()) == '#') {
        string dummy;
        getline(file, dummy);
    }

    int max_val;
    file >> w >> h >> max_val;
    buffer.resize(w * h);
    
    if (header == "P2") {
        for (int i = 0; i < w * h; i++) {
            int pixel;
            file >> pixel;
            buffer[i] = static_cast<float>(pixel);
        }
    } else if (header == "P5") {
        file.get(); 
        vector<unsigned char> temp(w * h);
        file.read(reinterpret_cast<char*>(temp.data()), w * h);
        for (int i = 0; i < w * h; i++) {
            buffer[i] = static_cast<float>(temp[i]);
        }
    }
    file.close();
}

// Escritor PGM (P2)
void escribir_pgm(const string& filename, float* buffer, int w, int h) {
    ofstream file(filename);
    if (!file.is_open()) return;
    file << "P2\n" << w << " " << h << "\n255\n";
    for (int i = 0; i < w * h; i++) {
        int val = static_cast<int>(min(255.0f, max(0.0f, buffer[i])));
        file << val << (i % w == w - 1 ? "\n" : " ");
    }
    file.close();
}

// Kernel  suavizado heterogéneo (Box Filter 3x3 con Offloading)
void suavizado_heterogeneo(float* __restrict input, float* __restrict output, int w, int h, int iterations) {
    int N = w * h;
    
    // Offloading a GPU OpenMP
    #pragma omp target data map(to: input[0:N]) map(tofrom: output[0:N])
    {
        for (int i = 0; i < iterations; i++) {
            // Paralelización en device
            #pragma omp target teams distribute parallel for collapse(2)
            for (int y = 0; y < h; y++) {
                for (int x = 0; x < w; x++) {
                    float sum = 0.0f;
                    for (int ky = -1; ky <= 1; ky++) {
                        for (int kx = -1; kx <= 1; kx++) {
                            // CUDA/OpenMP device min/max
                            int nx = (x + kx < 0) ? 0 : ((x + kx > w - 1) ? w - 1 : x + kx);
                            int ny = (y + ky < 0) ? 0 : ((y + ky > h - 1) ? h - 1 : y + ky);
                            sum += input[ny * w + nx];
                        }
                    }
                    output[y * w + x] = sum / 9.0f;
                }
            }
            // Retroalimentación en device
            #pragma omp target teams distribute parallel for
            for (int n = 0; n < N; n++) {
                input[n] = output[n];
            }
        }
    } // Sincronización al final del bloque
}

int main() {
    int WIDTH, HEIGHT;
    vector<float> input_img;
    string input_file;
    
    cout << "Ingresa el nombre de la imagen a procesar (ej. fractal.pgm o ajedrez.pgm): ";
    cin >> input_file;
    
    // 1. Cargar imagen original PGM
    leer_pgm(input_file, input_img, WIDTH, HEIGHT);
    if(input_img.empty()) {
        cerr << "Error: Asegúrate de que " << input_file << " exista." << endl;
        return -1;
    }

    const int N = WIDTH * HEIGHT;
    float* output_img = (float*)_mm_malloc(N * sizeof(float), 32);

    // Inicializar salida
    for (int i = 0; i < N; i++) output_img[i] = input_img[i];

    // 2. EJECUCIÓN Y MEDICIÓN DE TIEMPO
    cout << "Versión Heterogénea (GPU Offloading) - Buscando dispositivos..." << endl;
    
    auto start = chrono::high_resolution_clock::now();
    
    const int ITERATIONS = 100;
    suavizado_heterogeneo(input_img.data(), output_img, WIDTH, HEIGHT, ITERATIONS);
    
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> diff = end - start;
    cout << "Versión Heterogénea Suavizado - Tiempo (" << ITERATIONS << " iteraciones): " << diff.count() << " s\n";

    // 3. Generar imagen procesada
    string output_file = input_file;
    size_t pos = output_file.find(".pgm");
    if (pos != string::npos) {
        output_file.replace(pos, 4, "_blurred_heterogeneo.pgm");
    } else {
        output_file += "_blurred_heterogeneo.pgm";
    }

    escribir_pgm(output_file, output_img, WIDTH, HEIGHT);
    cout << "Imagen '" << output_file << "' generada correctamente.\n";

    _mm_free(output_img);

    return 0;
}
