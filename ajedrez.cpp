//icpx -g -O3 -march=native -fno-inline -o ajedrez ajedrez.cpp

#include <iostream>
#include <fstream>

using namespace std;

int main() {
    int width = 256;
    int height = 256;
    int square_size = 32; 
    
    ofstream file("ajedrez.pgm");
    if (!file.is_open()) return -1;

    // Encabezado PGM P2 (ASCII)
    file << "P2\n" << width << " " << height << "\n255\n";
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Lógica matemática para crear el patrón de ajedrez
            bool is_white = ((x / square_size) % 2) == ((y / square_size) % 2);
            file << (is_white ? 255 : 0) << " ";
        }
        file << "\n";
    }
    
    file.close();
    cout << "Imagen sintética 'ajedrez.pgm' generada con éxito." << endl;
    return 0;
}