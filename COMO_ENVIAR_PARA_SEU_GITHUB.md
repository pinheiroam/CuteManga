# Como enviar este projeto para o seu GitHub

## 1. Criar o fork no GitHub

1. Acesse o repositório original: **https://github.com/darkxex/CuteManga**
2. Clique no botão **"Fork"** (canto superior direito).
3. O GitHub vai criar uma cópia do projeto na sua conta: `https://github.com/pinheiroam/CuteManga`

## 2. Adicionar o seu repositório como remote e enviar

No terminal (PowerShell ou CMD), dentro da pasta do projeto:

```powershell
cd c:\Users\marila\Documents\Projetos\CuteManga
```

**Opção A – Enviar para o fork (seu usuário):**

```powershell
# Adicionar seu GitHub como remote
git remote add meu-github https://github.com/pinheiroam/CuteManga.git

# Ver o que será enviado
git status

# Adicionar todas as alterações (incluindo MuPDF, etc.)
git add .
git commit -m "CBZ support, portrait mode, improvements"

# Enviar para o seu GitHub (branch master)
git push -u meu-github master
```

**Opção B – Usar o seu fork como novo origin:**

```powershell
git remote set-url origin https://github.com/pinheiroam/CuteManga.git

git add .
git commit -m "CBZ support, portrait mode, improvements"
git push -u origin master
```

## 3. Se o repositório no seu GitHub ainda não existir

1. No GitHub: **New repository** (novo repositório).
2. Nome sugerido: **CuteManga** (pode ser outro).
3. **Não** marque “Initialize with README” se você já tem código local.
4. Depois use os comandos da Opção B acima (já com o usuário pinheiroam).

## Observação
- Na primeira vez que rodar `git push`, o GitHub pode pedir login (usuário/senha ou token).
- Se usar autenticação em dois fatores, use um **Personal Access Token** em vez da senha.
