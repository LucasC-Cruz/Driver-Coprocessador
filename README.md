# Coprocessador ELM — Marco 2: Driver Linux (Assembly ARM)

## Visão Geral

Implementação do driver Linux em Assembly ARM para controle do coprocessador de inferência ELM (Extreme Learning Machine) embarcado na FPGA da DE1-SoC. A comunicação entre o HPS (ARM Cortex-A9) e a FPGA é feita via MMIO pelo barramento LWHPS2FPGA.

---

## 1. Levantamento de Requisitos

### Requisitos do Marco 2 (conforme especificação)

- Integrar o IP do coprocessador ao HPS via bridges HPS↔FPGA no Quartus.
- Implementar driver Linux com rotinas críticas em Assembly ARM que permita:
  - Inicializar o hardware (mapeamento de memória via `/dev/mem`);
  - Enviar a imagem de entrada (784 pixels, 8 bits cada);
  - Enviar pesos (`W_in`), bias (`b`) e parâmetros de normalização (`β`);
  - Iniciar a inferência;
  - Aguardar finalização por polling das flags de hardware;
  - Ler o resultado da classificação e as flags de status.
- Demonstração de estabilidade: classificar uma imagem conhecida repetidamente sem falhas.
- API definida e documentada (protótipos em `api.h`).

### Arquitetura da Rede ELM

| Estágio | Operação |
|---|---|
| Entrada | Vetor de 784 pixels (imagem 28×28, 8 bits/pixel) |
| Camada oculta | `h = activation(W_in · x + b)` — 32 neurônios, pesos em Q4.12 |
| Camada de saída | `y = β · h` — 10 neurônios |
| Predição | `pred = argmax(y)` — inteiro [0, 9] |

---

## 2. Softwares Utilizados

| Software | Versão | Finalidade |
|---|---|---|
| Intel Quartus Prime | 18.1 | Síntese, integração HPS↔FPGA e programação da FPGA |
| `arm-linux-gnueabihf-gcc` | >= 7.x | Compilação cruzada do driver Assembly + programa C |
| Python 3 | >= 3.6 | Geração dos arquivos `.bin` de pesos, bias e beta |
| NumPy | >= 1.18 | Quantização e exportação dos parâmetros da rede |
| Linux (ARM) | Kernel >= 4.x | SO rodando no HPS da DE1-SoC |

**Hardware:** Terasic DE1-SoC (Cyclone V FPGA + ARM Cortex-A9 dual-core)

---

## 3. Arquivos do Projeto

| Arquivo | Descrição |
|---|---|
| `driver.s` | Driver em Assembly ARM: mapeamento de memória, PIOs, carregamento de dados, flags |
| `api.h` | Declarações da API C que expõe as funções do driver |
| `programa.c` | Interface interativa de linha de comando para controle e testes |
| `colors.h` | Macros de cores ANSI para terminal |
| `W_in_invertido.bin` | Pesos quantizados (Q4.12, 16 bits, 100.352 entradas) |
| `b_q_invertido.bin` | Bias quantizados (16 bits, 128 entradas) |
| `beta_q_invertido.bin` | Parâmetros β de batch normalization (16 bits, 1.280 entradas) |
| `imagem_4.bin` | Imagem de teste padrão (dígito "4", 784 bytes) |

---

## 4. Configuração do Ambiente e Compilação

### 4.1 Pré-requisitos (máquina de desenvolvimento)

```bash
sudo apt-get update
sudo apt-get install gcc-arm-linux-gnueabihf
```

### 4.2 Compilação cruzada

```bash
arm-linux-gnueabihf-gcc -o coprocessador programa.c driver.s
```

### 4.3 Transferência para a DE1-SoC

```bash
scp coprocessador W_in_invertido.bin b_q_invertido.bin beta_q_invertido.bin imagem_4.bin \
    usuario@<IP_DA_PLACA>:~/coprocessador/
```

### 4.4 Programação da FPGA

1. Abra o projeto no Quartus Prime com os arquivos `.mif` no diretório do projeto.
2. Compile: **Processing → Start Compilation**.
3. Grave: **Tools → Programmer** → selecione o `.sof` → **Start**.

---

## 5. Mapeamento de Memória e PIOs

Base: `0xFF200` (LWHPS2FPGA), página de `0x5000` bytes mapeada via `mmap2` sobre `/dev/mem`.

| Registrador | Offset | Direção | Descrição |
|---|---|---|---|
| `PIO_INSTRUCTION` | `0x00` | Escrita | Instrução a enviar ao coprocessador |
| `PIO_ENABLE` | `0x10` | Escrita | Pulso 1 para 0 para disparar a instrução |
| `PIO_RESULTADO` | `0x20` | Leitura | Resultado da inferência (classe 0–9) |
| `PIO_FLAG_DONE` | `0x30` | Leitura | Operação concluída |
| `PIO_FLAG_BUSY` | `0x40` | Leitura | Coprocessador ocupado |
| `PIO_FLAG_ERROR` | `0x50` | Leitura | Erro na operação |
| `PIO_CLR_OP` | `0x60` | Escrita | Limpa flag de erro |
| `PIO_RESET_COP` | `0x70` | Escrita | Reset do coprocessador |
| `PIO_CONFIRMAR` | `0x80` | Leitura | Leitura da última instrução (debug) |

---

## 6. Conjunto de Instruções

| Instrução | Opcode | Codificação (32 bits) |
|---|---|---|
| Store Image Pixel | — | `[pixel << 13] OR [addr << 3]` |
| Store Weight Address | `0x1` | `[addr << 3] OR 0x1` |
| Store Weight Data | `0x2` | `[dado << 3] OR 0x2` |
| Store Bias | `0x3` | `[dado << 10] OR [addr << 3] OR 0x3` |
| Store Beta | `0x4` | `[dado << 14] OR [addr << 3] OR 0x4` |
| Iniciar Inferência | `0x5` | `5` |
| NOP | `0x7` | `7` |

---

## 7. Execução e Testes

### 7.1 Execução

O acesso a `/dev/mem` requer privilégios de root:

```bash
sudo ./coprocessador
```

O programa carrega automaticamente pesos, bias, beta e a imagem padrão na inicialização, depois exibe o menu interativo.

### 7.2 Teste de Inferência Única (Opção 1)

Selecione `[1]` no menu. Saída esperada para `imagem_4.bin`:

```
Resultado inferência: 4
Flag de done: 1
Flag de busy: 0
Flag de erro: 0
```

### 7.3 Teste de Estabilidade (Opção 3)

Executa N inferências consecutivas e calcula a taxa de acerto:

```
Resultado esperado: 4
Número de inferências: 100
...
Acertou 100 de 100
Taxa de acerto: 100.000000 porcento
```

### 7.4 Script de Teste Automatizado

```bash
#!/bin/bash
# Executa 100 inferências e verifica taxa de acerto
OUTPUT=$(sudo ./coprocessador << EOF
3
4
100
0
EOF
)
echo "$OUTPUT" | grep "Acertou"
echo "$OUTPUT" | grep "Taxa de acerto"
```

---

## 8. Análise dos Resultados

**Correção:** o coprocessador classificou corretamente `imagem_4.bin` (dígito 4) em todos os testes realizados, confirmando que a quantização Q4.12 preservou a acurácia da rede ELM original.

**Estabilidade:** 100 inferências consecutivas com 100% de acerto, validando a corretude do protocolo de handshaking (enable/done) e a ausência de corrupção de estado entre execuções.

**Desempenho estimado:** o número de clocks por inferência é calculado como `(N_inst x 5) + (32 x 18.844) + (2 x 18.844) + 12`, o que resulta em aproximadamente 648.028 clocks. A 50 MHz, isso equivale a cerca de 13 ms por inferência.

**Limitação identificada:** o driver utiliza busy-wait (`espera_done`) durante a inferência, bloqueando o HPS. Uma abordagem baseada em interrupções reduziria o overhead de CPU em execuções futuras.
