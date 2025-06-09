#include <stdio.h>
#include <stdlib.h>
#include "pin_mux.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "enet.h"

/* Mensajes */
/*
 *M: "No todo lo que es oro reluce..."
 *R: "...Ni todos los que vagan están perdidos."
 *M: "Aún en la oscuridad..."
 *R: "...brilla una luz."
 *M: "¿Qué es la vida?"
 *R: "Nada más que un breve caminar a la luz del sol."
 *M: "No temas a la oscuridad..."
 *R: "...pues en ella se esconden las estrellas."
 *M: "Hasta los más pequeños..."
 *R: "...pueden cambiar el curso del futuro."
 *M: "No digas que el sol se ha puesto..."
 *R: "...si aún te queda la luna."
 *M: "El coraje se encuentra..."
 *R: "...en los lugares más inesperados."
 *M: "No todos los tesoros..."
 *R: "...son oro y plata."
 *M: "Es peligroso..."
 *R: "...cruzar tu puerta."
 *M: "Un mago nunca llega tarde..."
 *R: "...ni pronto, Frodo Bolsón. Llega precisamente cuando se lo propone."
 *M: "Aún hay esperanza..."
 *R: "...mientras la Compañía permanezca fiel."
 *M: "El mundo está cambiando..."
 *R: "...Siento que algo se avecina."
 *M: "Las raíces profundas..."
 *R: "...no alcanzan las heladas."
 *M: "No se puede..."
 *R: "...pasar."
 *M: "Y sobre todo..."
 *R: "...cuidado con el Anillo."
 *M: "De las cenizas, un fuego..."
 *R: "...se despertará."
 */

const char *messages[] = {
    "No todo lo que es oro reluce...",
    "Aún en la oscuridad...",
    "¿Qué es la vida?",
    "No temas a la oscuridad...",
    "Hasta los más pequeños...",
    "No digas que el sol se ha puesto...",
    "El coraje se encuentra...",
    "No todos los tesoros...",
    "Es peligroso...",
    "Un mago nunca llega tarde...",
    "Aún hay esperanza...",
    "El mundo está cambiando...",
    "Las raíces profundas...",
    "No se puede...",
    "Y sobre todo...",
    "De las cenizas, un fuego..."
};
const uint8_t mess_t = sizeof(messages) / sizeof(messages[0]);

int main(void)
{
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    init_ENET();

    for (volatile int j = 0; j < 8000000; j++)
    {
        __asm volatile ("nop");
    }


    while (1)
    {
    }
}
