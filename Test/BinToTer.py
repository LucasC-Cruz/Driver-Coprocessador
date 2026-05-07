with open('Driver-Coprocessador\temp\output.bin', 'rb') as f:
    data = f.read(2)
val = int.from_bytes(data, 'big', signed=False)
print(f'Decimal: {val}')
print(f'Hexadecimal: 0x{val:04X}')
print(f'Binário (16 bits): {val:016b}')