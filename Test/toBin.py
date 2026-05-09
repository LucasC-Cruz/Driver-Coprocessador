numero = int(input("Digite um número inteiro (0-65535): "))

dados = numero.to_bytes(4, 'big')

with open('output.bin', 'wb') as arquivo:
    arquivo.write(dados)

print("Dados salvos em 'output.bin'")

# queria depois decidir em qual local vai o path do arquivo escrito, vou ver isso dps

#  000000_0000000000000001_0000000_011