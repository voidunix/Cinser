# 🪐 Tervia Cinser — README

![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)
![Since: 2026](https://img.shields.io/badge/Since-2026-rebeccapurple)
![Boot: GRUB%20%2B%20Multiboot](https://img.shields.io/badge/Boot-GRUB%20%2B%20Multiboot-orange)
![Arch: i386](https://img.shields.io/badge/Arch-i386-blue)

A sovereign computing environment developed by **Tervia Corporation**.  
This page documents the **Makefile workflow** used to build, clean, and run **Tervia Cinser**.

---

## 📦 Prerequisites (Linux)

Install the core toolchain:

```bash
sudo apt update
sudo apt install build-essential nasm grub-pc-bin xorriso qemu-system-x86 mtools
```

> If you use a different distro, install equivalents for: **gcc**, **binutils**, **make**, **nasm**, **grub-mkrescue**, **xorriso**, and **qemu**.

---

## 🧭 Project Build Layout

After building, generated files typically go to a `build/` directory (or the directory defined by your Makefile):

```text
build/
├── obj/        # Object files (.o)
├── kernel.bin  # Linked kernel binary
└── cinser.iso  # Bootable ISO image
```

Keeping outputs separated helps avoid accidental commits and makes cleanup easy.

---

## 🚀 Build Commands

### ✅ `make all` — Full Build

Builds everything from source:

- Assemble `.s` files (NASM)
- Compile `.c` files (freestanding C)
- Link with the project linker script
- Create a bootable ISO (GRUB + Multiboot)

```bash
make all
```

**Use this when:**
- You changed low-level boot code
- You want a clean, consistent rebuild
- You’re preparing a test ISO

---

### 🧹 `make clean` — Clean Outputs

Removes build artifacts (objects, kernel, ISO):

```bash
make clean
```

**Use this when:**
- You changed linker scripts
- You suspect stale objects are causing weird bugs
- You want a completely fresh build

---

### ▶️ `make run` — Run in Emulator

Builds (if needed) and launches the system in an emulator (commonly QEMU):

```bash
make run
```

**Use this when:**
- You’re iterating quickly
- You want fast boot tests
- You’re debugging early kernel output

> If your Makefile uses VirtualBox instead of QEMU, the target name may still be `run`—check the Makefile for the exact command.

---

## 🧠 Tips for Debugging Builds

- **If it boots yesterday but not today:** run `make clean && make all`
- **If it crashes instantly:** ensure Multiboot header + flags are correct, and your entrypoint matches the linker script
- **If text prints but then freezes:** confirm IDT/ISR setup before enabling anything that can fault

---

## 📜 License

Licensed under **GNU GPLv3**.

➡️ Read the full license text here: **[LICENSE](./LICENSE)**

---

## 🌌 Final Note

A kernel build system isn’t just automation — it’s part of the architecture.  
If you can build it reliably, you can evolve it safely.
