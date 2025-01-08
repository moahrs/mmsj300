/********************************************************************************
*    Programa    : mgi.c
*    Objetivo    : Monitor Graphic Interface
*    Criado em   : 06/03/2024
*    Programador : Moacir Jr.
*--------------------------------------------------------------------------------
* Data        Versão  Responsavel  Motivo
* 06/03/2023  0.1     Moacir Jr.   Criação Versão Beta
*--------------------------------------------------------------------------------*/

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "../mmsj300api.h"
#include "../monitor.h"
#include "../mmsjos.h"

void uasctohex(unsigned char a, unsigned char *s)
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

void uprintDiskError(unsigned char pError)
{
    printText("Error: ");

    switch( pError )
    {
      case S_NOTINIT   : printText("ArduinoFDC.begin() was not called"); break;
      case S_NOTREADY  : printText("Drive not ready"); break;
      case S_NOSYNC    : printText("No sync marks found"); break;
      case S_NOHEADER  : printText("Sector header not found"); break;
      case S_INVALIDID : printText("Data record has unexpected id"); break;
      case S_CRC       : printText("Data checksum error"); break;
      case S_NOTRACK0  : printText("No track 0 signal detected"); break;
      case S_VERIFY    : printText("Verify after write failed"); break;
      case S_READONLY  : printText("Disk is write protected"); break;
      default          : printText("Unknonwn error"); break;
    }

    printText("!\r\n\0");
}

//-----------------------------------------------------------------------------
int ufsSend4Byte(unsigned char vByte)
{
    unsigned char *pEnder = 0x200001;

    *pEnder = vByte & 0x0F;
    *(pEnder + 2) = (vByte >> 4) & 0x0F;

    return 1;
}

//-----------------------------------------------------------------------------
int ufsRec4Byte(unsigned char *pByte, unsigned char pTimeOut)
{
    unsigned char *pEnder = 0x200001;

    *pByte = *pEnder & 0x0F;
    *pByte |= (*(pEnder + 2) & 0x0F) << 4;

    return 1;
}

//-----------------------------------------------------------------------------
void udiskCmd (unsigned char *pCmd, unsigned char *pParam)
{
    unsigned long ix;
    int iy, pqtd;
    unsigned char pSendCmd[64];
    unsigned char *pEnder = 0x800000;
    char vbuffer [sizeof(long)*8+1];
    char buffer[10];
    unsigned char shex[4], vchr[2];
    unsigned char vByte = 0;
    unsigned char sqtdtam[10];

    printText("Opening Comm's...\r\n\0");

    // Limpar entrada arduino
    ufsSend4Byte(0x06);

    printText("Putting It All Together...\r\n\0");

    // Junta tudo pra envio
    ix = 0;
    while(pCmd[ix])
    {
        pSendCmd[ix] = pCmd[ix];
        ix++;
    }
    pSendCmd[ix++] = 0x06;  // ACK
    pSendCmd[ix] = 0x00;

    printText("Sending...\0");

    // Envia pro arduino
    ix = 0;
    while (pSendCmd[ix])
        ufsSend4Byte(pSendCmd[ix++]);

    printText("Ok\r\n\0");

    // Espera se comando foi entendido... Primeiro Byte é OK (= 0) ou Erro (<> 0)
    ufsRec4Byte(&vByte, 1);

    if (vByte > 0)
    {
        uprintDiskError(vByte);
        return;
    }

    // Apos ok, prepara recebimento ou envio de dados
    printText("Processing...\r\n\0");

    // Segundo e terceiro byte é a quantidade (em caso de r) ou somente um valor 0 ou 1
    if (pCmd[0] == 'B')
    {
        printText("Qty...\r\n\0");

        ufsRec4Byte(&vByte, 0);

        pqtd = vByte;
        pqtd = pqtd << 8;

        ufsRec4Byte(&vByte, 0);

        pqtd = pqtd | vByte;

        if (pqtd == 0)
        {
            ufsRec4Byte(&vByte, 0);

            if (vByte > 0)
            {
                uprintDiskError(vByte);

                return;
            }
        }

        printText("Rec...\r\n\0");
        for (ix = 0; ix < pqtd; ix += 1)
        {
            ufsRec4Byte(&vByte, 1);

            vBufDataDisk[ix] = vByte;
        }

        iy = 0;

        printText("Show...\r\n\0");

        for (ix = 0; ix < 128; ix += 1)
        {
            uasctohex(vBufDataDisk[ix], shex);
            printText(shex);

            printChar(' ', 1);

            if (iy == 7)
            {
                printText("\r\n\0");
                iy = -1;
            }

            iy++;

/*            for (iy = 0; iy < vcols; iy++)
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

            printText("\r\n\0");*/
        }
    }
    else if (pCmd[0] == 'X')
    {
        for(ix=0;ix < 512;ix++)
        {
            vByte = *(pEnder + ix);

            ufsSend4Byte(vByte);
        }
    }
    else
    {
        if (ufsRec4Byte(&vByte, 0) >= 0)
        {
            uasctohex(vByte, shex);
            printText(shex);
        }
        else
        {
            printText("Received Data Error...\r\n\0");
            return;
        }
    }

    printText("\r\n\0");
}

//-----------------------------------------------------------------------------
void drawLine(int X1, int Y1, int X2, int Y2, int color)
{
    int x,y,addx,addy,dx,dy;
    long P;
    unsigned int i;
    int ix, iy;

    dx = abs(X2 - X1);
    dy = abs(Y2 - Y1);
    x = X1;
    y = Y1;

    if(X1 > X2)
        addx = -1;
    else
        addx = 1;

    if(Y1 > Y2)
        addy = -1;
    else
        addy = 1;


    if(dx >= dy)
    {
        P = (2*dy) - dx;

        for(i = 1; i <= (dx +1); i++)
        {
            vdp_plot_hires(x, y, color, 0);

            if(P < 0)
            {
                P = P + (2*dy);
                x = (x + addx);
            }
            else
            {
                P = P+(2*dy) - (2*dx);
                x = x + addx;
                y = y + addy;
            }
        }
    }
    else
    {
        P = (2*dx) - dy;

        for(i = 1; i <= (dy +1); i++)
        {
            vdp_plot_hires(x, y, color, 0);

            if (P < 0)
            {
                P = P + (2*dx);
                y = y + addy;
            }
            else
            {
                P = P + (2*dx) - (2*dy);
                x = x + addx;
                y = y + addy;
            }
        }
    }
}

//-------------------------------------------------------------------
void DrawHLine(int X1, int X2, int y_pos, int color)
{
    int ix;

    for (ix = X1; ix <= X2; ix++)
        vdp_plot_hires(ix, y_pos, color, 0);
}

//-------------------------------------------------------------------
void DrawVLine(int Y1, int Y2, int x_pos, int color)
{
    int iy;

    for (iy = Y1; iy <= Y2; iy++)
        vdp_plot_hires(x_pos, iy, color, 0);
}

//-----------------------------------------------------------------------------
void drawBox(int X1, int Y1, int X2, int Y2, int color)
{
    int ix, iy;

    for (iy = Y1; iy <= Y2; iy++)
        for (ix = X1; ix <= X2; ix++)
            vdp_plot_hires(ix, iy, color, 0);

}

//-----------------------------------------------------------------------------
void drawRect(int X1, int Y1, int X2, int Y2, int color)
{
    DrawHLine(X1,X2,Y1,color);
    DrawHLine(X1,X2,Y2,color);
    DrawVLine(Y1,Y2,X1,color);
    DrawVLine(Y1,Y2,X2,color);
}

//-----------------------------------------------------------------------------
void drawCircle(int x, int y, int radius, int color, int fill)
{
    int a_,b_,P;
    a_ = 0;
    b_ = radius;
    P = 1 - radius;

    while (a_ <= b_)
    {
        if(fill == 1)
        {
             drawBox(x-a_,y-b_,x+a_,y+b_,color);
             drawBox(x-b_,y-a_,x+b_,y+a_,color);
        }
        else
        {
             vdp_plot_hires(a_+x, b_+y, color, 0);
             vdp_plot_hires(b_+x, a_+y, color, 0);
             vdp_plot_hires(x-a_, b_+y, color, 0);
             vdp_plot_hires(x-b_, a_+y, color, 0);
             vdp_plot_hires(b_+x, y-a_, color, 0);
             vdp_plot_hires(a_+x, y-b_, color, 0);
             vdp_plot_hires(x-a_, y-b_, color, 0);
             vdp_plot_hires(x-b_, y-a_, color, 0);
        }
        if (P < 0 )
        {
            P = (P + 3) + (2* a_);
            a_ ++;
        }
        else
        {
            P = (P + 5) + (2* (a_ - b_));
            a_ ++;
            b_ --;
        }
    }
}

//-----------------------------------------------------------------------------
void doMenuBar(void)
{
    drawBox(0,0,255,10,*fgcolor);
    vdp_set_cursor(0, 1);
    *fgcolor = VDP_DARK_RED;
    *bgcolor = VDP_WHITE;
    printText("Menu");
    *fgcolor = VDP_WHITE;
    *bgcolor = VDP_BLACK;
}

//-----------------------------------------------------------------------------
void doStatusBar(void)
{
    drawBox(0,181,255,191,*fgcolor);
}

//-----------------------------------------------------------------------------
// Principal
//-----------------------------------------------------------------------------
void main(void)
{
    unsigned char *blin = vbuf;
    unsigned char vRetInput;

    printText("T4\r\n\0");
    udiskCmd("T4","");
    printText("R0,1,0\r\n\0");
    udiskCmd("R0,1,0","");
    printText("B\r\n\0");
    udiskCmd("B","");
    printText("R0,2,1\r\n\0");
    udiskCmd("R0,2,1","");
    printText("B\r\n\0");
    udiskCmd("B","");

/*    vdp_init(VDP_MODE_G2, 0x0, 1, 0);
    vdp_set_bdcolor(VDP_BLACK);

    doMenuBar();
    doStatusBar();

    while (1)
    {
        vRetInput = inputLine(1,'@');

        if (vRetInput == 0x1B)  // ESC
        {
            break;
        }
    }

    vdp_init(VDP_MODE_TEXT, (*fgcolor<<4) | (*bgcolor & 0x0f), 0, 0);

    clearScr();
    */
}