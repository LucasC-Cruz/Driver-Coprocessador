with open('Driver-Coprocessador/temp/output.bin', 'rb') as f:
    data = f.read(4)
val = int.from_bytes(data, 'big', signed=False)
print(f'Decimal: {val}')
print(f'Hexadecimal: 0x{val:04X}')
print(f'Binário (16 bits): {val:032b}')