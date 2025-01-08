/********************************************************************************
*    Programa    : hello.c
*    Objetivo    : Hello para testes
*    Criado em   : 13/10/2014
*    Programador : Moacir Jr.
*--------------------------------------------------------------------------------
* Data        Versão  Responsavel  Motivo
* 13/10/2014  0.1     Moacir Jr.   Criação Versão Beta
*--------------------------------------------------------------------------------*/

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "../mmsj300api.h"
#include "../monitor.h"

//-----------------------------------------------------------------------------
// Principal
//-----------------------------------------------------------------------------
void main(void)
{
    unsigned char *linhacomando = "DUMPW";
    unsigned iy = 5;

    // mostra msgs na tela
    printText("Hellooooooooo...\n\0");

    if (strcmp(linhacomando,"DUMP") == 0 && iy == 4)
    {
        printText("DUMP\n\0");
    }
    else if (strcmp(linhacomando,"DUMPS") == 0 && linhacomando[4] == 'S' && iy == 5)
    {
        printText("DUMPS\n\0");
    }
    else if (strcmp(linhacomando,"DUMPW") == 0 && linhacomando[4] == 'W'  && iy == 5)
    {
        printText("DUMPW\n\0");
    }

}