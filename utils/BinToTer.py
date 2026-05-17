with open('Driver-Coprocessador/Test/3.png', 'rb') as f:
    data = f.read(1)
val = int.from_bytes(data, 'big', signed=False)
print(f'Decimal: {val}')
print(f'Hexadecimal: 0x{val:04X}')
print(f'Binário (8 bits): {val:08b}')