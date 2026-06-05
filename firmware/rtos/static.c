#include "FreeRTOS.h"
#include "task.h"

/**
 * @brief Encargada de reservar la memoria necesaria para la tarea de servicio de timers
 * @see Barry, R. "Mastering the FreeRTOS Real Time Kernel", Sección 3.4.2.1
 * 
 * @param ppxTimerTaskTCBBuffer referencia al Task Control Block del Tmr Svc
 * @param ppxTimerTaskStackBuffer referencia al stack del Tmr Svc
 * @param pulTimerTaskStackSize cantidad de palabras del stack del Tmr Svc
 */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize) {
    // Task Control Block de la tarea
    static StaticTask_t xTimerTaskTCB;
    // Se crea un array con el tamaño del stack
    static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];
    // Se asignan las referencias a los parámetros
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

/**
 * @brief Encargada de reservar la memoria necesaria para la tarea IDLE
 * @see Barry, R. "Mastering the FreeRTOS Real Time Kernel", Sección 3.4.2.2
 * 
 * @param ppxIdleTaskTCBBufer referencia al Task Control Block de la tarea IDLE
 * @param ppxIdleTaskStackBuffer referencia al stack de la tarea IDLE
 * @param pulIdleTaskStackSize cantidad de palabras del stack de la tarea IDLE
 */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize ) {
    // Task Control Block de la tarea
    static StaticTask_t xIdleTaskTCB;
    // Se crea un array con el tamaño del stack
    static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];
    // Se asignan las referencias a los parámetros
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}