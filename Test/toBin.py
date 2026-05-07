numero = int(input("Digite um número inteiro (0-65535): "))

dados = numero.to_bytes(2, 'big')

with open('output.bin', 'wb') as arquivo:
    arquivo.write(dados)

print("Dados salvos em 'output.bin'")

# queria depois decidir em qual local vai o path do arquivo escrito, vou ver isso dps