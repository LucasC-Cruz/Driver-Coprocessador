from PIL import Image

# Lê a imagem PNG

img = Image.open("Driver-Coprocessador/Test/3.png")

# Converte para escala de cinza (modo 'L' = 8 bits por pixel, 0-255)
img_gray = img.convert('L')

# Obtém os bytes dos pixels
pixels = img_gray.tobytes()

# Salva em arquivo binário
with open('pixels3.bin', 'wb') as f:
    f.write(pixels)

print("Imagem convertida e salva em pixels.bin")