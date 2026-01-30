#!/bin/bash
# Setup CuteManga — instala dependências e compila.
# Execute este script DENTRO do MSYS do devkitPro (menu Iniciar: devkitPro → MSYS).

set -e

if [ -z "$DEVKITPRO" ]; then
    echo "ERRO: DEVKITPRO nao esta definido."
    echo "Abra o terminal 'MSYS' pelo menu Iniciar: devkitPro -> MSYS"
    echo "e execute este script de novo."
    exit 1
fi

echo "=== Instalando pacotes Switch (switch-dev + portlibs) ==="
pacman -S switch-dev switch-sdl2 switch-sdl2_ttf switch-sdl2_image switch-sdl2_gfx switch-curl switch-freetype --noconfirm

echo ""
echo "=== Compilando CuteManga ==="
make

echo ""
echo "=== Pronto! ==="
echo "Arquivo gerado: out/CuteManga.nro"
echo "Copie para o cartao SD do Switch em: SD:/switch/"
