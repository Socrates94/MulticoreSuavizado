#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <chrono>
#include <cuda_runtime.h>

using namespace std;

// Estructura para manejar la imagen PGM
struct PGMImage {
    int width, height, max_val;
    vector<int> data;
};

// Función para leer PGM (P2)
bool leer_pgm(const string& filename, PGMImage& img) {
    ifstream file(filename);
    if (!file.is_open()) return false;

    string magic;
    file >> magic;
    if (magic != "P2") return false;

    file >> img.width >> img.height >> img.max_val;
    img.data.resize(img.width * img.height);
    for (int i = 0; i < img.width * img.height; i++) {
        file >> img.data[i];
    }
    return true;
}

// Función para escribir PGM (P2)
void escribir_pgm(const string& filename, const PGMImage& img) {
    ofstream file(filename);
    file << "P2\n" << img.width << " " << img.height << "\n" << img.max_val << "\n";
    for (int i = 0; i < img.height; i++) {
        for (int j = 0; j < img.width; j++) {
            file << img.data[i * img.width + j] << " ";
        }
        file << "\n";
    }
}

// KERNEL DE CUDA: Suavizado 3x3
__global__ void smoothingKernel(const int* input, int* output, int width, int height) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x < width && y < height) {
        int sum = 0;
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                int nx = max(0, min(x + j, width - 1));
                int ny = max(0, min(y + i, height - 1));
                sum += input[ny * width + nx];
            }
        }
        output[y * width + x] = sum / 9;
    }
}

int main() {
    string input_file;
    cout << "Ingresa el nombre de la imagen a procesar (ej. fractal.pgm o ajedrez.pgm): ";
    cin >> input_file;
    
    PGMImage img;

    if (!leer_pgm(input_file, img)) {
        cerr << "Error al leer " << input_file << endl;
        return 1;
    }

    int size = img.width * img.height * sizeof(int);
    int *d_input, *d_output;

    // Reservar memoria en la GPU
    cudaMalloc(&d_input, size);
    cudaMalloc(&d_output, size);

    // Copiar datos del host a la GPU
    cudaMemcpy(d_input, img.data.data(), size, cudaMemcpyHostToDevice);

    // Configuracion de hilos y bloques
    dim3 blockSize(16, 16);
    dim3 gridSize((img.width + blockSize.x - 1) / blockSize.x, 
                  (img.height + blockSize.y - 1) / blockSize.y);

    int iterations = 100;
    cout << "Aplicando suavizado (CUDA) sobre " << input_file << " (" << iterations << " iteraciones)..." << endl;

    auto start = chrono::high_resolution_clock::now();

    // Ejecutar el kernel
    for (int i = 0; i < iterations; i++) {
        smoothingKernel<<<gridSize, blockSize>>>(d_input, d_output, img.width, img.height);
        // Swap para la siguiente iteracion (opcional segun logica, aqui output -> input si queremos acumulativo)
        cudaMemcpy(d_input, d_output, size, cudaMemcpyDeviceToDevice);
    }

    // Esperar a que la GPU termine
    cudaDeviceSynchronize();

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> diff = end - start;

    // Copiar resultado de vuelta a la CPU
    cudaMemcpy(img.data.data(), d_output, size, cudaMemcpyDeviceToHost);

    string output_file = input_file;
    size_t pos = output_file.find(".pgm");
    if (pos != string::npos) {
        output_file.replace(pos, 4, "_blurred_cuda.pgm");
    } else {
        output_file += "_blurred_cuda.pgm";
    }

    cout << "Tiempo de ejecucion (CUDA): " << diff.count() << " segundos" << endl;
    cout << "Imagen guardada como: " << output_file << endl;

    escribir_pgm(output_file, img);

    // Liberar memoria
    cudaFree(d_input);
    cudaFree(d_output);

    return 0;
}
