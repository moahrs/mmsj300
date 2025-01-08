/********************************************************************************
*    Programa    : dumpw.c
*    Objetivo    : Dump Memoria em Janela
*    Criado em   : 13/10/2014
*    Programador : Moacir Jr.
*--------------------------------------------------------------------------------
* Data        Versão  Responsavel  Motivo
* 06/04/2023  0.1     Moacir Jr.   Criação Versão Beta
*--------------------------------------------------------------------------------*/

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "../mmsj300api.h"
#include "../monitor.h"

//-----------------------------------------------------------------------------
int hex2int(char ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    return -1;
}

//-----------------------------------------------------------------------------
unsigned long pow(int val, int pot)
{
    int ix;
    int base = val;

    if (val != 0)
    {
        if (pot == 0)
            val = 1;
        else if (pot == 1)
            val = base;
        else
        {
            for (ix = 0; ix <= pot; ix++)
            {
                if (ix >= 2)
                    val *= base;
            }
        }
    }

    return val;
}

//-----------------------------------------------------------------------------
unsigned long hexToLong(char *pHex)
{
    int ix;
    unsigned char ilen = strlen(pHex) - 1;
    unsigned long pVal = 0;

    for (ix = ilen; ix >= 0; ix--)
    {
        pVal += hex2int(pHex[ilen - ix]) * pow(16, ix);
    }

    return pVal;
}

//-----------------------------------------------------------------------------
void asctohex(unsigned char a, unsigned char *s)
{
     unsigned char c;
     c = (a >> 4) & 0x0f;
     if (c <= 9) c+= '0'; else c += 'a' - 10;
     *s++ = c;
     c = a & 0x0f;
     if (c <= 9) c+= '0'; else c += 'a' - 10;
     *s++ = c;
     *s = 0;
}

//-----------------------------------------------------------------------------
// Principal
//-----------------------------------------------------------------------------
void main(void)
{
    unsigned char ptype = 0x00;
    unsigned char *pender = 0x00800000; //hexToLong(pEnder);
    unsigned char *blin = vbuf;
    unsigned char pEnder[10];
    unsigned long vqtd = 128, ix;
    unsigned long vcols = 8;
    int iy;
    unsigned char shex[4], vchr[2];
    unsigned char pbytes[16];
    char vbuffer [sizeof(long)*8+1];
    char buffer[10];
    int i=0;
    int j=0;
    unsigned char vRetInput;

    clearScr();
    printText("             DUMPW v0.1                \r\n\0");
    printChar(218,1);
    for (ix = 0; ix < 5; ix++)
        printChar(196,1);
    printChar(194,1);
    for (ix = 0; ix < 23; ix++)
        printChar(196,1);
    printChar(194,1);
    for (ix = 0; ix < 7; ix++)
        printChar(196,1);
    printChar(191,1);
    printText("\r\n");
    printChar(179,1);
    printText("Addr ");
    printChar(179,1);
    printText("         Bytes         ");
    printChar(179,1);
    printText(" ASCII ");
    printChar(179,1);
    printText("\r\n");
    printChar(192,1);
    for (ix = 0; ix < 5; ix++)
        printChar(196,1);
    printChar(193,1);
    for (ix = 0; ix < 23; ix++)
        printChar(196,1);
    printChar(193,1);
    for (ix = 0; ix < 7; ix++)
        printChar(196,1);
    printChar(217,1);
    printText("\r\n");

    vdp_set_cursor(0, 20);
    printChar(218,1);
    for (ix = 0; ix < 37; ix++)
        printChar(196,1);
    printChar(191,1);
    printText("\r\n");
    printChar(179,1);
    printText(" <-:Prev  ->:Next  <");
        printChar(217,1);
    printText(":Addr  ESC:Exit ");
    printChar(179,1);
    printText("\r\n");
    printChar(192,1);
    for (ix = 0; ix < 37; ix++)
        printChar(196,1);
    printChar(217,1);
    printText("\r\n");

    if (*vdpMaxCols == 32)
        vcols = 4;

    while (1)
    {
        vdp_set_cursor(0, 4);

        for (ix = 0; ix < vqtd; ix += vcols)
        {
            ltoa (pender,vbuffer,16);
            for (i=0; i<(6-strlen(vbuffer));i++) {
                buffer[i]='0';
            }
            for(j=0;j<strlen(vbuffer);j++){
                buffer[i] = vbuffer[j];
                i++;
                buffer[i] = 0x00;
            }

            printText(buffer);
            printChar(':', 1);

            for (iy = 0; iy < vcols; iy++)
                pbytes[iy] = *pender++;

            for (iy = 0; iy < vcols; iy++)
            {
                asctohex(pbytes[iy], shex);
                printText(shex);

                if ((vcols - iy) >= 2)
                    printChar(' ', 1);
            }

            printText("|\0");

            for (iy = 0; iy < vcols; iy++)
            {
                if (pbytes[iy] >= 0x20)
                {
                    vchr[0] = pbytes[iy];
                    vchr[1] = 0x00;
                    printText(vchr);
                }
                else
                    printChar('.', 1);
            }

            printText("\r\n\0");
        }

        while (1)
        {
            vRetInput = inputLine(1,'@');

            if (vRetInput == 0x12)  // LeftArrow (18)
            {
                pender = pender - 256;
                
                if (pender > 0x00FFFFFF)
                    pender = 0x00FFFF00;

                break;
            }
            else if (vRetInput == 0x14)  // RightArrow (20)
            {
                if (pender > 0x00FFFFFF)
                    pender = 0x00000000;

                break;
            }
            else if (vRetInput == 0x0D)  // Enter (13)
            {
                vdp_set_cursor(1, 21);
                printText(" Address(HEX):                       \0");

                vdp_set_cursor(16, 21);

                vRetInput = inputLine(6,'$');

                vdp_set_cursor(1, 21);
                printText(" <-:Prev  ->:Next  <");
                    printChar(217,1);
                printText(":Addr  ESC:Exit ");

                if (vRetInput != 0x1B)
                {
                    blin = vbuf;
                    pender = hexToLong(blin);
                    
                    if (pender > 0x00FFFF00)
                        pender = 0x00FFFF00;

                    break;
                }
            }
            else if (vRetInput == 0x1B)  // ESC
            {
                break;
            }
        }
        
        if (vRetInput == 0x1B)  // ESC
        {
            break;
        }
    }

    clearScr();
}
