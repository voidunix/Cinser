#pragma once
#include <stdint.h>

// Inicializa uma IDT minima (todas as entradas apontam para um handler que trava em HLT)
void idt_init(void);
