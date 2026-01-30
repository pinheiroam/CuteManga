# Setup — CuteManga (Nintendo Switch Homebrew)

Este guia descreve como configurar o ambiente de desenvolvimento para compilar o **CuteManga** no Windows, usando **devkitPro**, **devkitA64** e **libnx**.

> **Atenção:** Desenvolver e instalar homebrews exige que o Switch esteja desbloqueado, o que pode levar ao banimento pela Nintendo. Use por sua conta e risco.

---

## 1. Configurar o ambiente de desenvolvimento

### 1.1 Instalar o devkitPro (Windows)

1. **Baixe o instalador gráfico**
   - Acesse: [devkitPro Installer (Releases)](https://github.com/devkitPro/installer/releases)
   - Baixe a versão mais recente do instalador para Windows.

2. **Execute o instalador**
   - É necessária conexão com a internet.
   - Siga as etapas do assistente.
   - **Importante:** marque a opção **Switch Development** (desenvolvimento para Nintendo Switch) para instalar **devkitA64** e **libnx**.

3. **Concluir a instalação**
   - Ao terminar, o instalador configura o **DEVKITPRO** (em geral em `C:\devkitPro`).
   - Abra o **MSYS** pelo menu Iniciar: **devkitPro → MSYS** (ou similar).  
     Todas as compilações devem ser feitas **nesse terminal**, onde `DEVKITPRO` já está definido.

### 1.2 Instalar ferramentas e bibliotecas para Switch (pacman)

**Importante:** Use o comando **`pacman`** (não `dkp-pacman`) — no Windows o instalador do devkitPro usa o MSYS2, onde o gerenciador de pacotes é o `pacman` normal, já configurado com os repositórios devkitPro.

1. Abra o **MSYS do devkitPro** pelo menu Iniciar: **devkitPro → MSYS** (ou **MSYS2 MSYS**).
2. No terminal que abrir, rode:

```bash
# Ferramentas Switch (devkitA64 + libnx)
pacman -S switch-dev --noconfirm

# Bibliotecas usadas pelo CuteManga (SDL2, imagens, fontes, rede)
pacman -S switch-sdl2 switch-sdl2_ttf switch-sdl2_image switch-sdl2_gfx switch-curl switch-freetype --noconfirm
```

Ou execute o script de setup do projeto (dentro do MSYS do devkitPro):

```bash
cd /c/Users/marila/Documents/Projetos/CuteManga
bash setup-devkitpro.sh
```

Se o pacman pedir dependências adicionais (por exemplo libpng, libjpeg, libwebp, mbedtls), aceite a instalação. O projeto usa `-lpng -lwebp -ljpeg -lcurl -lmbedtls -lmbedx509 -lmbedcrypto -lz -lnx` no Makefile; essas libs costumam vir como dependências dos pacotes acima.

---

## 2. MuPDF (para suporte a CBZ)

O CuteManga usa **MuPDF** para abrir arquivos **CBZ** (comics em ZIP). O MuPDF não está nos portlibs do devkitPro; é preciso compilá-lo a partir do código-fonte.

**Dependências do MuPDF (portlibs):** para a compilação do MuPDF são usadas as libs do Switch já instaladas (zlib, libjpeg, freetype). JBIG2 está desativado nesta build (`jbig2=no`) porque não há pacote `switch-jbig2dec` nos portlibs do devkitPro.

1. **Clonar o MuPDF** na pasta do projeto (uma vez):
   ```bash
   cd /c/Users/marila/Documents/Projetos/CuteManga
   git clone --depth 1 https://github.com/ArtifexSoftware/mupdf.git libs/mupdf
   ```

2. **Compilar o MuPDF para Switch** (no MSYS do devkitPro):
   ```bash
   make -C libs -f Makefile.mupdf
   ```
   As bibliotecas serão geradas em `libs/mupdf/lib/`. Se der erro, confira se o MuPDF ainda usa Makefile (versões muito novas podem usar só CMake).

3. Depois disso, o `make` principal do projeto vai encontrar o MuPDF e linkar o suporte a CBZ.

---

## 3. Libnx

A **libnx** é a biblioteca principal para interagir com o hardware do Switch (vídeo, controles, sistema de arquivos, teclado, etc.). Ela é instalada junto com **switch-dev** (pacote `switch-dev`). Não é necessário instalar libnx separadamente.

---

## 4. Compilar o projeto

1. **Abrir o MSYS do devkitPro** (não use o PowerShell ou CMD normais, a menos que você configure `DEVKITPRO` manualmente neles).

2. **Ir até a pasta do projeto:**
   ```bash
   cd /c/Users/marila/Documents/Projetos/CuteManga
   ```
   (Ajuste o caminho se o projeto estiver em outro lugar.)

3. **Compilar:**
   ```bash
   make
   ```

4. **Resultado**
   - Se tudo estiver certo, o executável será gerado em:
     - **`out/CuteManga.nro`** — este é o arquivo que você copia para o Switch.

- Se aparecer *"Please set DEVKITPRO in your environment"*, você não está no MSYS do devkitPro. Use sempre o atalho **devkitPro → MSYS**.
- Se aparecer *"dkp-pacman: command not found"*, use **`pacman`** em vez de `dkp-pacman` — no Windows o devkitPro usa o pacman do MSYS2, já configurado com os repositórios devkitPro.
- Se aparecer *"No rule to make target '.../source/main.cpp'"* (com um caminho de outro usuário/pasta), a pasta **build** tem arquivos de dependência gerados em outro PC. Rode **`make clean`** no MSYS e depois **`make`** de novo. Se não tiver `make` no PATH fora do MSYS, apague manualmente as pastas **build** e **out** e rode **`make`** no MSYS.
- Se aparecer *"freetype-config: command not found"* ou *"cannot find -lmbedtls"*, o Makefile já foi ajustado: usa **pkg-config** para freetype e não liga mbedtls (o switch-curl das portlibs atuais não usa mbedtls). Atualize o projeto e compile de novo.

---

## 4. Testar no Nintendo Switch

1. **Preparar o cartão SD**
   - No cartão SD do Switch, crie a pasta **`switch`** na raiz (se ainda não existir):
     - `SD:/switch/`

2. **Copiar o .nro**
   - Copie o arquivo **`out/CuteManga.nro`** para **`SD:/switch/`**.

3. **Executar no Switch**
   - Insira o cartão SD no Switch desbloqueado.
   - Inicie o **Homebrew Menu** (método depende do seu CFW, por exemplo via album ou título específico).
   - Selecione **CuteManga** na lista e execute.

### Uso do app (resumo)

- Crie uma pasta **`CuteManga`** na raiz do cartão SD e coloque dentro as pastas dos seus mangás/comics (cada pasta = um título).
- Evite nomes com caracteres especiais (ñ, á, í, etc.); use apenas caracteres latinos simples no nome das pastas.

---

## 5. (Opcional) Testar via rede com nxlink

O projeto inclui **makewin.bat**, que usa **nxlink** para enviar o .nro ao Switch pela rede:

1. No MSYS (na pasta do projeto), após `make`, você pode rodar:
   ```bash
   nxlink -a <IP_DO_SWITCH> -s out/CuteManga.nro
   ```
2. No **makewin.bat** o IP está fixo como `192.168.0.9`. Edite o arquivo e coloque o IP do seu Switch (configurado em Rede).
3. No Switch, inicie **nxlink** (ou o receiver que seu CFW usa) antes de rodar o comando no PC.

Isso evita ter que copiar o .nro manualmente para o SD a cada teste.

---

## Resumo rápido

| Etapa              | Ação |
|--------------------|------|
| Ambiente           | Instalar devkitPro (instalador Windows) com **Switch Development**. |
| Libnx + ferramentas| No MSYS: `dkp-pacman -S switch-dev`. |
| Portlibs           | No MSYS: `dkp-pacman -S switch-sdl2 switch-sdl2_ttf switch-sdl2_image switch-sdl2_gfx switch-curl switch-freetype`. |
| Compilar           | No MSYS: `cd` até o projeto e `make`. |
| Saída              | `out/CuteManga.nro`. |
| Testar             | Copiar `CuteManga.nro` para `SD:/switch/` e abrir pelo Homebrew Menu. |

Para mais detalhes e exemplos oficiais, consulte a [documentação da libnx](https://switchbrew.org/wiki/Libnx) e a [wiki da devkitPro](https://devkitpro.org/wiki/).
