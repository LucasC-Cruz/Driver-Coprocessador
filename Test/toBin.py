


try:
    with open('output.bin', 'wb') as arquivo:
        for i in range(0, 3):
            numero = int(input("Digite um número inteiro (0-65535): "))
            dados = numero.to_bytes(2, 'big')
            arquivo.write(dados)
except FileNotFoundError:
    print("Erro na abertura do arquivo")




print("Dados salvos em 'output.bin'")

# queria depois decidir em qual local vai o path do arquivo escrito, vou ver isso dps

#  000000_0000000000000001_0000000_011