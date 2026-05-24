# Coprocessador ELM — Marco 2: Driver Linux (Assembly ARM)

Implementação do driver Linux em Assembly ARM para controle do coprocessador de inferência ELM (Extreme Learning Machine) embarcado na FPGA da DE1-SoC. A comunicação entre o HPS (ARM Cortex-A9) e a FPGA é feita via MMIO pelo barramento LWHPS2FPGA.

---

## Sumário

- [1. Levantamento de Requisitos](#1-levantamento-de-requisitos)
- [2. Softwares Utilizados](#2-softwares-utilizados)
- [3. Estrutura do Repositório](#3-estrutura-do-repositório)
- [4. Configuração do Ambiente e Compilação](#4-configuração-do-ambiente-e-compilação)
- [5. Mapeamento de Memória e PIOs](#5-mapeamento-de-memória-e-pios)
- [6. Conjunto de Instruções](#6-conjunto-de-instruções)
- [7. Execução e Testes](#7-execução-e-testes)
- [8. Análise dos Resultados](#8-análise-dos-resultados)

---

<details>
<summary><h2>1. Levantamento de Requisitos</h2></summary>

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
| Camada oculta | `h = activation(W_in · x + b)` — 128 neurônios, pesos em Q4.12 |
| Camada de saída | `y = β · h` — 10 neurônios |
| Predição | `pred = argmax(y)` — inteiro [0, 9] |

</details>

---

<details>
<summary><h2>2. Softwares Utilizados</h2></summary>

| Software | Versão | Finalidade |
|---|---|---|
| Intel Quartus Prime | 18.1 | Síntese, integração HPS↔FPGA e programação da FPGA |
| arm-linux-gnueabihf-gcc | >= 7.x | Compilação cruzada do driver Assembly + programa C |
| Linux (ARM) | Kernel >= 4.x | SO rodando no HPS da DE1-SoC |

**Hardware:** Terasic DE1-SoC (Cyclone V FPGA + ARM Cortex-A9 dual-core)

</details>

---

<details>
<summary><h2>3. Estrutura do Repositório</h2></summary>

```
Driver-Coprocessador/
│
├── src/                        # Código-fonte principal
│   ├── driver.s                # Driver em Assembly ARM (mapeamento, PIOs, carregamento, flags)
│   ├── api.h                   # Declarações da API C que expõe as funções do driver
│   ├── programa.c              # Interface interativa de linha de comando para controle e testes
│   └── colors.h                # Macros de cores ANSI para terminal
│
├── data/                       # Parâmetros da rede neural e imagens de teste
│   ├── W_in_q.mif              # Pesos quantizados (Q4.12) — formato MIF para inicialização da FPGA
│   ├── W_in_invertido.bin      # Pesos em binário (ordem invertida para carga sequencial pelo HPS)
│   ├── b_q.mif                 # Bias — formato MIF
│   ├── b_q_invertido.bin       # Bias em binário
│   ├── beta_q.mif              # Parâmetros β de batch normalization — formato MIF
│   ├── beta_q_invertido.bin    # Parâmetros β em binário
│   └── imagem_4.bin            # Imagem de teste padrão (dígito "4", 784 bytes)
│
├── Test/                       # Testes unitários e de integração do driver
│   ├── BiasInstruction.s       # Teste em Assembly: valida o envio isolado de instruções de bias
│   ├── LerBinEmC.c             # Teste em C: leitura e verificação dos arquivos binários de dados
│   └── testeInitFiles.c        # Teste em C: verifica inicialização do hardware e carregamento dos arquivos
│
├── utils/                      # Utilitários de apoio ao desenvolvimento
│   ├── BinToTer.py             # Script Python: lê um arquivo binário e imprime o valor do primeiro byte em decimal, hexadecimal e binário
│   ├── inout.s                 # Utilitário Assembly: rotinas auxiliares de entrada/saída (buffer de saída e syscalls)
│   └── mifToBin                # Conversor: transforma arquivos .mif em .bin para uso pelo HPS
│
└── quartus/                    # Projeto Quartus Prime (integração HPS↔FPGA)
    └── my_first_hps-fpga_base/
        ├── soc_system.qpf      # Arquivo de projeto Quartus
        ├── soc_system.qsf      # Atribuições de pinos e configurações
        ├── soc_system.qsys     # Descrição do sistema (Platform Designer)
        ├── ghrd_top.v          # Toplevel do projeto RTL
        ├── CoProcessor.v       # Módulo principal do coprocessador ELM
        ├── ELM_on_DE1_SoC.v   # Integração do ELM na plataforma DE1-SoC
        ├── aux_files/          # Módulos auxiliares do datapath
        │   ├── mac_first_layer.v      # MAC da primeira camada
        │   ├── mac_second_layer.v     # MAC da segunda camada
        │   ├── tanh_pwl_q4_12.v      # Ativação tanh por piecewise linear em Q4.12
        │   ├── reg_bank128.v          # Banco de registradores de 128 posições
        │   ├── reg_bank10.v           # Banco de registradores de 10 posições
        │   ├── lsu_controller.v       # Controlador de acesso à memória
        │   └── display_resultado.v    # Exibição do resultado nos LEDs/display
        ├── inference_unit/     # Unidade de inferência neural
        │   ├── first_layer.v          # Camada oculta (128 neurônios)
        │   ├── second_layer.v         # Camada de saída (10 neurônios)
        │   ├── argmax_iterativo.v     # Módulo argmax iterativo
        │   └── neural_unit.v          # Orquestrador das duas camadas
        ├── memorias/           # Arquivos MIF para inicialização das BRAMs
        │   ├── W_in_q.mif
        │   ├── b_q.mif
        │   └── beta_q.mif
        └── output_files/       # Arquivos gerados pela compilação do Quartus
            └── soc_system.sof  # Bitstream para gravação na FPGA
```

</details>

---

<details>
<summary><h2>4. Configuração do Ambiente e Compilação</h2></summary>

### 4.1 Pré-requisitos (máquina de desenvolvimento)

```bash
sudo apt-get update
sudo apt-get install gcc-arm-linux-gnueabihf
```

### 4.2 Compilação cruzada

```bash
arm-linux-gnueabihf-gcc -o coprocessador src/programa.c src/driver.s
```

### 4.3 Transferência para a DE1-SoC

```bash
scp coprocessador data/W_in_invertido.bin data/b_q_invertido.bin data/beta_q_invertido.bin data/imagem_4.bin \
    usuario@<IP_DA_PLACA>:~/coprocessador/
```

### 4.4 Programação da FPGA

1. Abra o projeto no Quartus Prime com os arquivos `.mif` em `quartus/my_first_hps-fpga_base/memorias/`.
2. Compile: **Processing → Start Compilation**.
3. Grave: **Tools → Programmer** → selecione `output_files/soc_system.sof` → **Start**.

</details>

---

<details>
<summary><h2>5. Mapeamento de Memória e PIOs</h2></summary>

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

</details>

---

<details>
<summary><h2>6. Conjunto de Instruções</h2></summary>

Os bits [2:0] de cada instrução carregam o opcode. Os campos de endereço e dado são inseridos em posições superiores por deslocamento lógico.

| Instrução | Opcode (bin) | Codificação (32 bits) |
|---|---|---|
| Store Image Pixel | — | `[pixel << 13] OR [addr << 3]` |
| Store Weight Address | `001` | `[addr << 3] OR 001` |
| Store Weight Data | `010` | `[dado << 3] OR 010` |
| Store Bias | `011` | `[dado << 10] OR [addr << 3] OR 011` |
| Store Beta | `100` | `[dado << 14] OR [addr << 3] OR 100` |
| Iniciar Inferência | `101` | `101` |
| NOP | `111` | `111` |

</details>

---

<details>
<summary><h2>7. Execução e Testes</h2></summary>

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

</details>

---

<details>
<summary><h2>8. Análise dos Resultados</h2></summary>

**Correção:** o coprocessador classificou corretamente `imagem_4.bin` (dígito 4) em todos os testes realizados, confirmando que a quantização Q4.12 preservou a acurácia da rede ELM original.

**Estabilidade:** 100 inferências consecutivas com 100% de acerto, validando a corretude do protocolo de handshaking (enable/done) e a ausência de corrupção de estado entre execuções.

**Desempenho estimado:** o número de clocks por inferência é calculado como `(N_inst x 5) + (128 x 18.844) + (2 x 18.844) + 12`, onde N_inst = 102.544 instruções de memória montadas e enviadas pelo driver, resultando em aproximadamente 2.964.932 clocks. A 50 MHz, isso equivale a cerca de 59 ms por inferência.

**Limitação identificada:** o driver utiliza busy-wait (`espera_done`) durante a inferência, bloqueando o HPS. Uma abordagem baseada em interrupções reduziria o overhead de CPU em execuções futuras.

</details>
