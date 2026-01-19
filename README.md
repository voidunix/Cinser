# 🪐 Tervia Cinser OS

![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)
![Year: 2026](https://img.shields.io/badge/Year-2026-blueviolet)
![Status: In_Development](https://img.shields.io/badge/Status-Development-orange)

O **Tervia Cinser** é um sistema operacional moderno desenvolvido pela **Tervia Corporation**. Este projeto visa explorar conceitos avançados de kernel e arquitetura de sistemas, mantendo a filosofia de software livre.

---

## 🚀 Como Compilar e Executar

O projeto utiliza um sistema de automação baseado em `Makefile`. Certifique-se de ter as dependências instaladas antes de começar.

### 📦 Pré-requisitos (Linux)
Para compilar o núcleo e as ferramentas em C e Assembly, você precisará dos seguintes pacotes (exemplo para distribuições baseadas em Debian/Ubuntu):

```bash
sudo apt update
sudo apt install build-essential nasm qemu-system-x86 mtools crossbuild-essential-i686-linux-gnu
