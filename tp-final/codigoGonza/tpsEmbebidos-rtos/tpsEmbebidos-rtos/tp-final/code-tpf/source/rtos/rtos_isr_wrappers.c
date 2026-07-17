#include <os.h>

/*
 * MCUXpresso startup_mk64f12.c expects a symbol named PendSV_Handler.
 * The Micrium ARMv7-M port implements the real handler as OS_CPU_PendSVHandler().
 *
 * This wrapper overrides the weak default PendSV_Handler from startup_mk64f12.c
 * and branches directly to the Micrium assembly context-switch handler.
 */
void PendSV_Handler(void) __attribute__((naked));

void PendSV_Handler(void)
{
    __asm volatile ("b OS_CPU_PendSVHandler");
}
