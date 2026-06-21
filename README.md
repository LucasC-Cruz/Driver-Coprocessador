# Coprocessador ELM — Driver Linux (Assembly ARM) + Interface VGA/Mouse

Implementação do driver Linux em Assembly ARM para controle do coprocessador de inferência ELM (Extreme Learning Machine) embarcado na FPGA da DE1-SoC, agora estendida com exibição da imagem em um monitor VGA, painel de desenho via mouse USB, conversão automática de PNG para o formato binário do hardware e uma suíte de benchmark com geração de matriz de confusão e métricas de desempenho. A comunicação entre o HPS (ARM Cortex-A9) e a FPGA continua sendo feita via MMIO pelo barramento LWHPS2FPGA.

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

### Requisitos atendidos pela versão atual

- Integração do IP do coprocessador ao HPS via bridges HPS↔FPGA no Quartus (mantida da versão anterior).
- Driver Linux com rotinas críticas em Assembly ARM (`driver.s`) que permite:
  - Inicializar o hardware (mapeamento de memória via `/dev/mem`);
  - Enviar a imagem de entrada (784 pixels, 8 bits cada), pesos (`W_in`), bias (`b`) e parâmetros de normalização (`β`);
  - Iniciar a inferência e aguardar finalização por polling das flags de hardware (`done`/`busy`/`error`);
  - Ler o resultado da classificação e as flags de status;
  - Resetar o coprocessador e limpar flags de erro (`reset`, `clear_operation`).
- **Exibição em VGA:** a imagem que será classificada (predefinida ou desenhada) é renderizada em um monitor VGA de 320×240, com cada pixel da imagem 28×28 escalado para um bloco de 8×8 na tela.
- **Entrada via mouse:** painel de desenho interativo que lê eventos do dispositivo `/dev/input/event0` para permitir desenhar um dígito diretamente na tela VGA e enviá-lo ao coprocessador.
- **Conversão de imagem:** qualquer PNG 28×28 em escala de cinza pode ser convertido automaticamente para o formato binário de 784 bytes esperado pelo hardware, sem etapas manuais externas.
- **Suíte de benchmark:** execução de inferências em lote sobre um dataset de dígitos manuscritos (modo total, por amostragem aleatória ou filtrado por dígito), com geração de matriz de confusão, relatório de acertos por classe e métricas agregadas (acurácia, latência teórica, latência média real, vazão e desvio padrão de latência) em CSV.
- API documentada em `include/api.h`.

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

| Software / Biblioteca | Versão | Finalidade |
|---|---|---|
| Intel Quartus Prime | 18.1 | Síntese, integração HPS↔FPGA, módulo VGA e programação da FPGA |
| arm-linux-gnueabihf-gcc | >= 7.x | Compilação cruzada do driver Assembly + programa C |
| Linux (ARM) | Kernel >= 4.x | SO rodando no HPS da DE1-SoC, expõe `/dev/mem` e `/dev/input/eventX` |
| stb_image.h | single-header | Decodificação de PNG embarcada em `img2bin.c`, sem dependências externas |
| GNU Make | — | Build automatizado via `makefile` (substitui a chamada direta ao `gcc`) |

**Hardware:** Terasic DE1-SoC (Cyclone V FPGA + ARM Cortex-A9 dual-core), monitor VGA e mouse USB conectados à placa.

</details>

---

<details>
<summary><h2>3. Estrutura do Repositório</h2></summary>

```
Driver-Coprocessador/
│
├── src/                          # Código-fonte principal
│   ├── driver.s                  # Driver em Assembly ARM (mapeamento, PIOs, carregamento, flags)
│   ├── programa.c                # Interface interativa de linha de comando (menus, benchmark)
│   ├── vga.c                     # Renderização da imagem 28x28 no monitor VGA
│   ├── mouse.c                   # Painel de desenho via eventos do mouse (/dev/input/eventX)
│   └── img2bin.c                 # Conversão de PNG 28x28 para o binário de 784 bytes do hardware
│
├── include/                      # Cabeçalhos da API e dos módulos C
│   ├── api.h                     # Declarações da API exposta pelo driver.s
│   ├── pio.h                     # Offsets e ponteiros dos PIOs de VGA
│   ├── vga.h                     # Protótipos de vga.c
│   ├── mouse.h                   # Protótipos de mouse.c
│   ├── img2bin.h                 # Protótipos de img2bin.c
│   ├── colors.h                  # Macros de cores ANSI para o terminal
│   └── stb_image.h               # Biblioteca de terceiros (single-header) para leitura de PNG
│
├── assets/                       # Dados da rede neural, imagens de teste e resultados de benchmark
│   ├── W_in_invertido.bin        # Pesos em binário (ordem invertida para carga sequencial pelo HPS)
│   ├── b_q_invertido.bin         # Bias em binário
│   ├── beta_q_invertido.bin      # Parâmetros β em binário
│   ├── predef_6.bin              # Imagem de teste padrão carregada na inicialização (dígito "6")
│   ├── desenho.bin                # Última imagem salva pelo painel de desenho via mouse
│   ├── image.bin                  # Última imagem convertida de PNG via img2bin
│   ├── teste.png                  # PNG de exemplo para testar a conversão
│   ├── arquivos_1_bin.txt         # Lista mestre (caminho;rótulo;) usada pelos benchmarks total/sorteio
│   ├── 1/, 2/                     # Amostras de imagens por dígito (.png) para o benchmark por pasta
│   ├── BPasta/                    # Saída do benchmark filtrado por dígito (csv + matriz de confusão)
│   ├── BSorteio/                  # Saída do benchmark por amostragem aleatória
│   └── Btotal/                    # Saída do benchmark sobre o dataset completo
│
├── build/                        # Objetos (.o) e executável final, gerados pelo makefile
│
├── Test/
│   └── teste vga/                # Snapshot de desenvolvimento da versão com VGA (mesma estrutura de src/include)
│
├── binarios.zip, test.zip        # Datasets completos (dígitos 0-9) compactados, usados nos benchmarks
├── makefile                       # Build automatizado (compila tudo em src/*.c e src/*.s)
│
└── quartus/                      # Projeto Quartus Prime (integração HPS↔FPGA)
    └── my_first_hps-fpga_base/
        ├── soc_system.qpf/.qsf/.qsys   # Projeto, pinagem e sistema (Platform Designer)
        ├── ghrd_top.v                  # Toplevel do projeto RTL (inclui pinagem VGA)
        ├── CoProcessor.v               # Módulo principal do coprocessador ELM
        ├── ELM_on_DE1_SoC.v            # Integração do ELM na plataforma DE1-SoC
        ├── aux_files/                  # MACs, ativação, bancos de registradores, LSU, display
        ├── inference_unit/             # Camadas da rede (first_layer, second_layer, argmax, neural_unit)
        ├── modulo_vga/                 # vga_driver.v e controller_vga_to_sd.v — geração do sinal VGA
        ├── ip/                         # debounce, edge_detect, intr_capturer, hps_reset
        ├── pll01/                      # PLL para geração do clock de pixel do VGA
        ├── memorias/                   # MIFs de inicialização das BRAMs (W_in_q, b_q, beta_q, imagem_4)
        └── output_files/               # soc_system.sof — bitstream para gravação na FPGA
```

> **Nota:** as pastas `1/` e `2/` em `assets/` contêm apenas uma amostra do dataset (100 imagens cada). O conjunto completo (dígitos 0–9) usado pelos benchmarks está nos arquivos `binarios.zip`/`test.zip` e deve ser extraído na placa em `assets/` ou no caminho referenciado por `arquivos_1_bin.txt`.

</details>

---

<details>
<summary><h2>4. Configuração do Ambiente e Compilação</h2></summary>

### 4.1 Pré-requisitos (máquina de desenvolvimento)

```bash
sudo apt-get update
sudo apt-get install gcc-arm-linux-gnueabihf make
```

### 4.2 Compilação cruzada

A compilação agora é feita via `makefile`, que descobre automaticamente os arquivos `.c` e `.s` em `src/`, gera os objetos em `build/` e linka com `-lrt -lm` (necessárias para `clock_gettime` do benchmark e funções matemáticas como `pow`/`sqrt`):

```bash
make           # gera build/exec
make clean     # remove a pasta build/
```

### 4.3 Transferência para a DE1-SoC

```bash
scp -r build/exec assets/ makefile \
    usuario@<IP_DA_PLACA>:~/coprocessador/
```

> Lembre-se de extrair `binarios.zip`/`test.zip` na placa caso vá usar os modos de benchmark com o dataset completo (0–9).

### 4.4 Programação da FPGA

1. Abra o projeto no Quartus Prime em `quartus/my_first_hps-fpga_base/`.
2. Compile: **Processing → Start Compilation**.
3. Grave: **Tools → Programmer** → selecione `output_files/soc_system.sof` → **Start**.
4. Conecte um monitor VGA e um mouse USB à DE1-SoC antes de iniciar o programa.

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
| `PIO_VGA` | `0x90` | Escrita | Comando de pixel para o controlador VGA (posição, RGB, enable) |
| `PIO_VGA_DONE` | `0xA0` | Leitura | Confirmação de que o pixel foi escrito no framebuffer |

O comando de pixel do VGA é montado diretamente em `vga.c` (sem passar pelo conjunto de instruções do coprocessador) e segue o layout de bits consumido por `controller_vga_to_sd.v`:

```
pixel_cmd = posx[8:0] | (posy[7:0] << 9) | (red[2:0] << 17) | (green[2:0] << 20) | (blue[2:0] << 23) | (enable << 26)
```

</details>

---

<details>
<summary><h2>6. Conjunto de Instruções</h2></summary>

Os bits [2:0] de cada instrução carregam o opcode do coprocessador. Os campos de endereço e dado são inseridos em posições superiores por deslocamento lógico. Este conjunto não foi alterado em relação à versão anterior do driver.

| Instrução | Opcode (bin) | Codificação (32 bits) |
|---|---|---|
| Store Image Pixel | — | `[pixel << 13] OR [addr << 3]` |
| Store Weight Address | `001` | `[addr << 3] OR 001` |
| Store Weight Data | `010` | `[dado << 3] OR 010` |
| Store Bias | `011` | `[dado << 10] OR [addr << 3] OR 011` |
| Store Beta | `100` | `[dado << 14] OR [addr << 3] OR 100` |
| Iniciar Inferência | `101` | `101` |
| Status (não utilizada no hardware) | `110` | `110` |
| NOP | `111` | `111` |

</details>

---

<details>
<summary><h2>7. Execução e Testes</h2></summary>

### 7.1 Execução

O acesso a `/dev/mem` e a `/dev/input/eventX` requer privilégios de root:

```bash
sudo ./build/exec
```

Na inicialização o programa mapeia a memória, mapeia os PIOs de VGA, e carrega automaticamente bias, beta e pesos na memória do coprocessador. Em seguida exibe o menu principal:

```
[1] Realizar a inferencia de imagem definida
[2] Realizar a inferencia da imagem desenhada na tela
[3] Benchmark
[4] Outras Opções
[0] para sair
```

### 7.2 Inferência de imagem definida (Opção 1)

Solicita o caminho de um PNG 28×28 em escala de cinza, converte-o automaticamente para binário (`img2bin`), envia a imagem para o coprocessador, exibe-a no monitor VGA e roda a inferência. O usuário informa o valor esperado e o programa indica se houve acerto, além das flags de `done`/`busy`/`error` e do número estimado de clocks da operação.

### 7.3 Inferência via desenho no VGA (Opção 2)

Abre um painel de desenho 28×28 renderizado no monitor VGA, controlado pelo mouse:

- **Botão esquerdo:** desenha (incrementa o tom dos pixels sob o cursor);
- **Botão direito:** apaga (decrementa o tom dos pixels sob o cursor);
- **Botão do meio (scroll):** salva o desenho em `assets/desenho.bin`, encerra o painel e dispara a inferência automaticamente.

### 7.4 Benchmark (Opção 3)

Submenu com três modos de avaliação, todos gerando CSV de matriz de confusão, relatório de acertos por classe e métricas agregadas (acurácia, latência teórica, latência média real, vazão e desvio padrão de latência):

| Modo | Fonte das imagens | Saída |
|---|---|---|
| 1 — Total | Todas as imagens listadas em `assets/arquivos_1_bin.txt` | `assets/Btotal/` |
| 2 — Sorteio | N imagens sorteadas aleatoriamente da lista | `assets/BSorteio/` |
| 3 — Por pasta | Imagens filtradas por um dígito específico (0–9), em um número de iterações configurável | `assets/BPasta/` |

### 7.5 Outras opções (Opção 4)

Submenu de baixo nível para depuração manual do driver: resetar o coprocessador, limpar flag de erro, enviar NOP, enviar uma instrução personalizada em binário, enviar pixel/peso/bias/beta isoladamente, confirmar a última instrução enviada, e reenviar os arquivos pré-carregados (imagem padrão, bias, beta ou pesos).

</details>

---

<details>
<summary><h2>8. Análise dos Resultados</h2></summary>

**LINK PARA SLIDE: https://docs.google.com/presentation/d/1NIGzjEhVZVlr4j5ys-Hxj6DoKCVKzw56rjIRBb6FDaM/edit?slide=id.g3ec6ca3519e_4_6#slide=id.g3ec6ca3519e_4_6


Os três modos de benchmark já possuem resultados registrados em `assets/` a partir de execuções na placa:

**Benchmark total** (`assets/Btotal/`, dataset completo): acurácia geral de **66,61%**, vazão de **≈59,7 inferências/segundo** e latência média real de **≈16,8 ms** por inferência (latência teórica estimada pelos clocks de hardware: **≈2,3 ms**). A acurácia varia bastante por classe — dígitos como `1` (86,61%) e `7` (78,50%) são bem reconhecidos, enquanto `5` (14,46%) concentra a maior parte dos erros, sendo frequentemente confundido com `3` e `8` segundo a matriz de confusão.

**Benchmark por sorteio** (`assets/BSorteio/`, amostragem aleatória do dataset): acurácia de **64,67%**, consistente com o resultado do benchmark total, confirmando que a amostragem aleatória é representativa do desempenho geral da rede.

**Benchmark por pasta** (`assets/BPasta/`, execução registrada para o dígito `7`): acurácia de **77,33%** sobre as iterações daquele dígito, em linha com a taxa de acerto observada para a classe `7` no benchmark total.

**Estabilidade:** em todos os três modos a latência média real e a vazão permanecem praticamente idênticas (~16,8 ms / ~59,7 inferências por segundo), o que indica um protocolo de handshaking (`enable`/`done`) estável e sem degradação de desempenho entre execuções consecutivas, independentemente do dígito classificado.

**Observação sobre acurácia:** a diferença entre a latência teórica (baseada na contagem de clocks do hardware) e a latência real média sugere que parte do tempo medido em software é dominado por overhead do driver (chamadas de sistema, leitura de arquivo a cada imagem) e não apenas pelo tempo de computação da FPGA.

</details>
