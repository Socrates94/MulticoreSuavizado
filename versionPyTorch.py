import torch
import torch.nn.functional as F
import numpy as np
import time
import os

def read_pgm(filename):
    """Lector PGM (P5)."""
    with open(filename, 'rb') as f:
        # Leer cabecera
        magic = f.readline().decode().strip()
        if magic != 'P5':
            raise ValueError(f"Formato {magic} no soportado. Este script requiere P5 (binario).")
        
        # Saltar comentarios
        line = f.readline()
        while line.startswith(b'#'):
            line = f.readline()
        
        # Leer dimensiones
        width, height = map(int, line.decode().split())
        
        # Leer valor máximo
        max_val = int(f.readline().decode().strip())
        
        # Leer datos binarios
        data = np.frombuffer(f.read(), dtype=np.uint8).reshape((height, width))
    return data.astype(np.float32), width, height, max_val

def write_pgm(filename, data, width, height, max_val):
    """Escritor PGM (P5)."""
    with open(filename, 'wb') as f:
        f.write(f"P5\n{width} {height}\n{max_val}\n".encode())
        # Asegurarse de que los datos estén en el rango 0-255 y sean uint8
        data_clamped = np.clip(data, 0, 255).astype(np.uint8)
        f.write(data_clamped.tobytes())

def main():
    input_file = input("Ingresa el nombre de la imagen a procesar (ej. fractal_pesado.pgm): ")
    
    if not os.path.exists(input_file):
        print(f"Error: El archivo {input_file} no existe.")
        return

    # 1. Cargar la imagen
    print(f"Cargando {input_file}...")
    try:
        img_np, width, height, max_val = read_pgm(input_file)
    except Exception as e:
        print(f"Error al leer la imagen: {e}")
        return

    # 2. Preparar PyTorch y mover a CUDA
    if not torch.cuda.is_available():
        print("CUDA no está disponible. Usando CPU (será más lento).")
        device = torch.device('cpu')
    else:
        print("CUDA detectado. Usando GPU...")
        device = torch.device('cuda')

    # Formato tensor [B=1, C=1, H, W]
    img_tensor = torch.from_numpy(img_np).to(device).unsqueeze(0).unsqueeze(0)

    # Kernel suavizado 3x3
    kernel = torch.ones((1, 1, 3, 3), device=device) / 9.0

    # 3. Aplicar suavizado (100 iteraciones)
    iterations = 100
    print(f"Aplicando suavizado ({iterations} iteraciones)...")
    
    # Sincronización para medición
    if device.type == 'cuda':
        torch.cuda.synchronize()
        
    start_time = time.time()

    # Ejecución de las iteraciones
    with torch.no_grad():
        for i in range(iterations):
            # Padding replicado (equivalente a clamp)
            img_tensor = F.pad(img_tensor, (1, 1, 1, 1), mode='replicate')
            img_tensor = F.conv2d(img_tensor, kernel)

    if device.type == 'cuda':
        torch.cuda.synchronize()
        
    end_time = time.time()

    total_time = end_time - start_time
    print(f"Versión PyTorch (CUDA) - Tiempo ({iterations} iteraciones): {total_time:.6f} s")

    # 4. Guardar resultado
    output_file = input_file.replace(".pgm", "_blurred_pytorch.pgm")
    print(f"Guardando resultado en {output_file}...")
    
    # Mover de vuelta a CPU y numpy
    img_out = img_tensor.squeeze().cpu().numpy()
    write_pgm(output_file, img_out, width, height, max_val)
    print("¡Listo!")

if __name__ == "__main__":
    main()
