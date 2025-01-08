/********************************************************************************
*    Programa    : mgui.c
*    Objetivo    : MMSJ300 Graphical User Interface
*    Criado em   : 25/07/2023
*    Programador : Moacir Jr.
*--------------------------------------------------------------------------------
* Data        Versao  Responsavel  Motivo
* 25/07/2023  0.1     Moacir Jr.   Criacao Versao Beta
*    ...       ...       ...            ...
* 03/01/2025  0.5a    Moacir Jr.   Troca de cores e ajustes de tela
*--------------------------------------------------------------------------------
*
*--------------------------------------------------------------------------------
* To do
*
*--------------------------------------------------------------------------------
*
*********************************************************************************/
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "../mmsj300api.h"
#include "../monitor.h"
#include "../mmsjos.h"
#include "mgui.h"

#define versionMgui "0.5a"
#define __EM_OBRAS__ 1

//-----------------------------------------------------------------------------
// Principal
//-----------------------------------------------------------------------------
void main(void)
{
    startMGI();
}

//-----------------------------------------------------------------------------
void clearScrW(unsigned char color)
{
    unsigned int ix, iy;

    color &= 0x0F;

    setWriteAddress(*pattern_table);
    for (iy = 0; iy < 192; iy++)
    {
        for (ix = 0; ix < 32; ix++)
            *vvdgd = 0x00;
    }
    setWriteAddress(*color_table);
    for (iy = 0; iy < 192; iy++)
    {
        for (ix = 0; ix < 32; ix++)
            *vvdgd = color;
    }
}

//-----------------------------------------------------------------------------
void setWriteAddress(unsigned int address)
{
    *vvdgc = (unsigned char)(address & 0xff);
    *vvdgc = (unsigned char)(0x40 | (address >> 8) & 0x3f);
}

//-----------------------------------------------------------------------------
void setReadAddress(unsigned int address)
{
    *vvdgc = (unsigned char)(address & 0xff);
    *vvdgc = (unsigned char)((address >> 8) & 0x3f);
}

//-----------------------------------------------------------------------------
// VDP Functions
//-----------------------------------------------------------------------------
void setRegisterGui(unsigned char registerIndex, unsigned char value)
{
    *vvdgc = value;
    *vvdgc = (0x80 | registerIndex);
}

//-----------------------------------------------------------------------------
void vdp_set_cursor_pos_gui(unsigned char direction)
{
    unsigned char pMoveIdX = 6, pMoveIdY = 8;

    switch (direction)
    {
        case VDP_CSR_UP:
            vdp_set_cursor(*videoCursorPosColX, *videoCursorPosRowY - pMoveIdY);
            break;
        case VDP_CSR_DOWN:
            vdp_set_cursor(*videoCursorPosColX, *videoCursorPosRowY + pMoveIdY);
            break;
        case VDP_CSR_LEFT:
            vdp_set_cursor(*videoCursorPosColX - pMoveIdX, *videoCursorPosRowY);
            break;
        case VDP_CSR_RIGHT:
            vdp_set_cursor(*videoCursorPosColX + pMoveIdX, *videoCursorPosRowY);
            break;
    }
}

int vdp_init_gui(unsigned char mode, unsigned char color, unsigned char big_sprites, unsigned char magnify)
{
    unsigned int i, j;
    unsigned char ix;
    unsigned char *tempFontes = videoFontes;

    *vdp_mode = mode;
    *sprite_size_sel = big_sprites;

    // Clear Ram
    setWriteAddress(0x0);
    for (i = 0; i < 0x3FFF; i++)
        *vvdgd = 0;

    switch (mode)
    {
        case VDP_MODE_G1:
            setRegisterGui(0, 0x00);
            setRegisterGui(1, 0xC0 | (big_sprites << 1) | magnify); // Ram size 16k, activate video output
            setRegisterGui(2, 0x05); // Name table at 0x1400
            setRegisterGui(3, 0x80); // Color, start at 0x2000
            setRegisterGui(4, 0x01); // Pattern generator start at 0x800
            setRegisterGui(5, 0x20); // Sprite attriutes start at 0x1000
            setRegisterGui(6, 0x00); // Sprite pattern table at 0x000
            *sprite_pattern_table = 0;
            *pattern_table = 0x800;
            *sprite_attribute_table = 0x1000;
            *name_table = 0x1400;
            *color_table = 0x2000;
            *color_table_size = 32;
            // Initialize pattern table with ASCII patterns
            setWriteAddress(*pattern_table + 0x100);
            for (i = 0; i < 1784; i++)  // era 768
            {
                tempFontes = *videoFontes + i;
                *vvdgd = *tempFontes;
            }
            break;

        case VDP_MODE_G2:
            setRegisterGui(0, 0x02);
            setRegisterGui(1, 0xC0 | (big_sprites << 1) | magnify); // Ram size 16k, Disable Int, 0=8x8 and 1=16x16 Sprites, mag 0=off/1=on, activate video output
            setRegisterGui(2, 0x0E); // Name table at 0x3800
            setRegisterGui(3, 0xFF); // Color, start at 0x2000             // segundo manual, deve ser 7F para 0x0000 ou FF para 0x2000
            setRegisterGui(4, 0x03); // Pattern generator start at 0x0000   // segundo manual, deve ser 03 para 0x0000 ou 07 para 0x2000
            setRegisterGui(5, 0x76); // Sprite attributes start at 0x3B00
            setRegisterGui(6, 0x03); // Sprite pattern table at 0x1800
            *name_table = 0x3800;
            *color_table = 0x2000;
            *pattern_table = 0x0000;
            *sprite_attribute_table = 0x3B00;
            *sprite_pattern_table = 0x1800;
            *color_table_size = 0x1800;
            *vdpMaxCols = 255;
            *vdpMaxRows = 191;
            setWriteAddress(*name_table);
            for (i = 0; i < 768; i++)  // era 768
                *vvdgd = i & 0xFF;
            break;

        case VDP_MODE_MULTICOLOR:
            setRegisterGui(0, 0x00);
            setRegisterGui(1, 0xC8 | (big_sprites << 1) | magnify); // Ram size 16k, Multicolor
            setRegisterGui(2, 0x05); // Name table at 0x1400
            // setRegisterGui(3, 0xFF); // Color table not available
            setRegisterGui(4, 0x01); // Pattern table start at 0x800
            setRegisterGui(5, 0x76); // Sprite Attribute table at 0x1000
            setRegisterGui(6, 0x03); // Sprites Pattern Table at 0x0
            *pattern_table = 0x800;
            *name_table = 0x1400;
            *vdpMaxCols = 63;
            *vdpMaxRows = 47;
            setWriteAddress(*name_table); // Init name table
            for (j = 0; j < 24; j++)
                for (i = 0; i < 32; i++)
                    *vvdgd = (i + 32 * (j / 4));

            break;

        case VDP_MODE_TEXT:
            setRegisterGui(0, 0x00);
            setRegisterGui(1, 0xD2); // Ram size 16k, Disable Int
            setRegisterGui(2, 0x02); // Name table at 0x800
            setRegisterGui(4, 0x00); // Pattern table start at 0x0
            *pattern_table = 0x00;
            *name_table = 0x800;
            *vdpMaxCols = 39;
            *vdpMaxRows = 23;
            setWriteAddress(*pattern_table + 0x100);
            for (i = 0; i < 1784; i++)  // era 768
            {
                tempFontes = *videoFontes + i;
                *vvdgd = *tempFontes;
            }
            vdp_textcolor(VDP_WHITE, VDP_BLACK);
            break;
        default:
            return VDP_ERROR; // Unsupported mode
    }

    setRegisterGui(7, color);
    return VDP_OK;
}

//-----------------------------------------------------------------------------
void vdp_write_gui(unsigned char chr)
{
    unsigned int name_offset = *videoCursorPosRowY * (*vdpMaxCols + 1) + *videoCursorPosColX; // Position in name table
    unsigned int pattern_offset = name_offset << 3;                    // Offset of pattern in pattern table
    unsigned short i, ix, iy, xf;
    unsigned short vAntX, vAntY;
    unsigned char *tempFontes = videoFontes;
    unsigned long vEndFont, vEndPart;
    unsigned short posX, posY, modX, modY, offset, offsetmodX, posmodX;
    unsigned char lineChar, pixel, color;

    vEndPart = chr - 32;
    vEndPart = vEndPart << 3;
    vAntY = *videoCursorPosRowY;
    for (i = 0; i < 8; i++)
    {
        vEndFont = *videoFontes;
        vEndFont += vEndPart + i;
        tempFontes = vEndFont;
        lineChar = *tempFontes;
        lineChar = (lineChar & 0xFC);

/*        vAntX = *videoCursorPosColX;
        for (ix = 7; ix >= 2; ix--)
        {
            vdp_plot_hires(*videoCursorPosColX, *videoCursorPosRowY, ((*tempFontes >> ix) & 0x01) ? *fgcolor : 0, *bgcolor);
            *videoCursorPosColX = *videoCursorPosColX + 1;
        }

        *videoCursorPosColX = vAntX;*/

        ix = *videoCursorPosColX;
        iy = *videoCursorPosRowY;
        xf = ix + 6;
        offsetmodX = 0;

        while (ix < xf)
        {
            posX = (int)(8 * (ix / 8));
            posY = (int)(256 * (iy / 8));
            modX = (int)(ix % 8);
            modY = (int)(iy % 8);

            offset = posX + modY + posY;

            setReadAddress(*pattern_table + offset);
            setReadAddress(*pattern_table + offset);
            pixel = *vvdgd;
            setReadAddress(*color_table + offset);
            setReadAddress(*color_table + offset);
            color = *vvdgd;

            if (modX > 2 || (modX == 0 && ix > *videoCursorPosColX))   // Parcial com bits dos 6 bits no proximo Byte
            {
                if (ix == *videoCursorPosColX)  // Posicao inicial
                {
                    posmodX = (8 - modX);
                    pixel = ((pixel & (0xFF << posmodX)) | (lineChar >> modX));
                    offsetmodX = posmodX;
                }
                else
                {
                    posmodX = (6 - offsetmodX);
                    pixel = ((pixel & (0xFF >> posmodX)) | (lineChar << offsetmodX));
                }

                ix += posmodX;
            }
            else    // Total, com 6 bits no mesmo Byte
            {
                lineChar = lineChar >> modX;

                switch (modX)
                {
                    case 0:
                        pixel = pixel & 0x03;
                        break;
                    case 1:
                        pixel = pixel & 0x81;
                        break;
                    case 2:
                        pixel = pixel & 0xC0;
                        break;
                }

                pixel = pixel | lineChar;

                ix += 6;
            }

            color = (*bgcolor & 0x0F) | (*fgcolor << 4);

            setWriteAddress(*pattern_table + offset);
            *vvdgd = (pixel);
            setWriteAddress(*color_table + offset);
            *vvdgd = (color);
        }

        *videoCursorPosRowY = *videoCursorPosRowY + 1;
    }

    *videoCursorPosRowY = vAntY;
}

//-----------------------------------------------------------------------------
// Graphic Interface Functions
//-----------------------------------------------------------------------------
void writesxy(unsigned short x, unsigned short y, unsigned char sizef, unsigned char *msgs, unsigned short pcolor, unsigned short pbcolor) {
    unsigned char ix = 10, xf;

    vdp_set_cursor(x,y);
    vdp_textcolor(pcolor, pbcolor);

    while (*msgs) {
        if (*msgs >= 0x20 && *msgs < 0x7F)
        {
            vdp_write_gui(*msgs);
            vdp_set_cursor_pos_gui(VDP_CSR_RIGHT);
        }
        *msgs++;
    }
}

//-----------------------------------------------------------------------------
void writecxy(unsigned char sizef, unsigned char pbyte, unsigned short pcolor, unsigned short pbcolor) {
    vdp_set_cursor(*pposx,*pposy);
    vdp_write_gui(pbyte);

    *pposx = *pposx + sizef;

    if ((*pposx + sizef) > *vxgmax)
        *pposx = *pposx - sizef;
}

//-----------------------------------------------------------------------------
void locatexy(unsigned short xx, unsigned short yy) {
    *pposx = xx;
    *pposy = yy;
}

//-----------------------------------------------------------------------------
void SaveScreen(unsigned short xi, unsigned short yi, unsigned short pwidth, unsigned short pheight) {
    unsigned short xf, yf;
    unsigned int ix, iy;
    unsigned int offset, posX, posY, modY, saveOffSet, saveOffSetAnt;

    // Manter leitura rapida de 8 pixels (1 pixel por Byte)
    if ((xi & 0x0F) < 0x08)
        xi = xi - (xi & 0x0F);
    else
        xi = (xi - (xi & 0x0F)) + 0x08;

    // Define Final
    xf = (xi + pwidth);
    yf = (yi + pheight);

    if (xf > 255)
        xf = 255;

    if (yf > 191)
        yf = 191;

    saveOffSet = 0;

    for (iy = yi; iy <= yf; iy++)
    {
        ix = xi;
        saveOffSetAnt = saveOffSet;
        while (ix <= xf)
        {
            posX = (int)(8 * (ix / 8));
            posY = (int)(256 * (iy / 8));
            modY = (int)(iy % 8);
            offset = posX + modY + posY;

            setReadAddress(*pattern_table + offset);
            setReadAddress(*pattern_table + offset);

            *(saverPat + saveOffSet) = *vvdgd;
            saveOffSet = saveOffSet + 1;
            ix += 8;
        }

        ix = xi;
        saveOffSet = saveOffSetAnt;
        while (ix <= xf)
        {
            posX = (int)(8 * (ix / 8));
            posY = (int)(256 * (iy / 8));
            modY = (int)(iy % 8);
            offset = posX + modY + posY;

            setReadAddress(*color_table + offset);
            setReadAddress(*color_table + offset);

            *(saverCor + saveOffSet) = *vvdgd;
            saveOffSet = saveOffSet + 1;

            ix += 8;
        }
    }
}

//-----------------------------------------------------------------------------
void RestoreScreen(unsigned short xi, unsigned short yi, unsigned short pwidth, unsigned short pheight) {
    unsigned short xf, yf;
    unsigned int ix, iy;
    unsigned int offset, posX, posY, modY, saveOffSet, saveOffSetAnt;
    unsigned char pixel;
    unsigned char color;

    // Manter leitura rapida de 8 pixels (1 pixel por Byte)
    if ((xi & 0x0F) < 0x08)
        xi = xi - (xi & 0x0F);
    else
        xi = (xi - (xi & 0x0F)) + 0x08;

    // Define Final
    xf = (xi + pwidth);
    yf = (yi + pheight);

    if (xf > 255)
        xf = 255;

    if (yf > 191)
        yf = 191;

    saveOffSet = 0;

    for (iy = yi; iy <= yf; iy++)
    {
        ix = xi;
        while (ix <= xf)
        {
            posX = (int)(8 * (ix / 8));
            posY = (int)(256 * (iy / 8));
            modY = (int)(iy % 8);
            offset = posX + modY + posY;

            pixel = *(saverPat + saveOffSet);
            color = *(saverCor + saveOffSet);
            saveOffSet = saveOffSet + 1;

            setWriteAddress(*pattern_table + offset);
            *vvdgd = pixel;
            setWriteAddress(*color_table + offset);
            *vvdgd = color;

            ix += 8;
        }
    }

/*
    for (iy = yi; iy <= yf; iy++)
    {
        ix = xi;
        posX = (int)(8 * (ix / 8));
        posY = (int)(256 * (iy / 8));
        modY = (int)(iy % 8);
        offset = posX + modY + posY;

        setWriteAddress(*pattern_table + offset);
        saveOffSetAnt = saveOffSet;
        while (ix <= xf)
        {
            pixel = *(saverPat + saveOffSet++);
            *vvdgd = (pixel);
            ix += 8;
        }

        ix = xi;
        setWriteAddress(*color_table + offset);
        saveOffSet = saveOffSetAnt;
        while (ix <= xf)
        {
            color = *(saverCor + saveOffSet++);
            *vvdgd = (color);
            ix += 8;
        }
    }
*/
}

//-----------------------------------------------------------------------------
void SetDot(unsigned short x, unsigned short y, unsigned short color) {
    vdp_plot_hires(x, y, color, *bgcolor);
}

//-----------------------------------------------------------------------------
void SetByte(unsigned short ix, unsigned short iy, unsigned char pByte, unsigned short pfcolor, unsigned short pbcolor)
{
    unsigned int offset, offsetByte, posX, posY, modX, modY, xf, ixAnt;
    unsigned char pixel;
    unsigned char color;

    xf = ix + 8;
    if (xf > 255)
        xf = 255;

    ixAnt = ix;
    while (ix < xf)
    {
        posX = (int)(8 * (ix / 8));
        posY = (int)(256 * (iy / 8));
        modX = (int)(ix % 8);
        modY = (int)(iy % 8);

        offset = posX + modY + posY;

        if (modX > 0 || (modX == 0 && ((ix + 8) > xf)))
        {
            setReadAddress(*pattern_table + offset);
            setReadAddress(*pattern_table + offset);
            pixel = *vvdgd;
            setReadAddress(*color_table + offset);
            setReadAddress(*color_table + offset);
            color = *vvdgd;

            if (ix == ixAnt)
            {
                offsetByte = (8 - modX);
                pByte = pByte >> modX;
                pixel |= pByte;

                ix += (8 - modX);
            }
            else
            {
                pByte = pByte << offsetByte;
                pixel |= pByte;

                ix += (8 - offsetByte);
            }

            color = (color & 0x0F) | (pfcolor << 4);
        }
        else
        {
            pixel = pByte;
            color = (pbcolor & 0x0F) | (pfcolor << 4);

            ix += 8;
        }

        setWriteAddress(*pattern_table + offset);
        *vvdgd = (pixel);
        setWriteAddress(*color_table + offset);
        *vvdgd = (color);
    }
}

//-----------------------------------------------------------------------------
void FillRect(unsigned char xi, unsigned char yi, unsigned short pwidth, unsigned char pheight, unsigned char pcor) {
    unsigned short xf, yf;
    unsigned int ix, iy;
    unsigned int offset, posX, posY, modX, modY;
    unsigned char pixel;
    unsigned char color;
    unsigned char sqtdtam[10];

    xf = (xi + pwidth);
    yf = (yi + pheight);

    if (xf > 255)
        xf = 255;

    if (yf > 191)
        yf = 191;

    for (iy = yi; iy <= yf; iy++)
    {
        ix = xi;
        while (ix <= xf)
        {
            posX = (int)(8 * (ix / 8));
            posY = (int)(256 * (iy / 8));
            modX = (int)(ix % 8);
            modY = (int)(iy % 8);

            offset = posX + modY + posY;

            if (modX > 0 || (modX == 0 && ((ix + 8) > xf)))
            {
                setReadAddress(*pattern_table + offset);
                setReadAddress(*pattern_table + offset);
                pixel = *vvdgd;
                setReadAddress(*color_table + offset);
                setReadAddress(*color_table + offset);
                color = *vvdgd;

                if (pcor != 0x00)
                {
                    pixel |= 0x80 >> modX; //Set a "1"
                    color = (color & 0x0F) | (pcor << 4);
                }
                else
                {
                    pixel &= ~(0x80 >> modX); //Set bit as "0"
                    color = (color & 0xF0) | (*bgcolor & 0x0F);
                }

                ix++;
            }
            else
            {
                if (pcor != 0x00)
                {
                    pixel = 0xFF;
                    color = (*bgcolor & 0x0F) | (pcor << 4);
                }
                else
                {
                    pixel = 0x00;
                    color = (*bgcolor & 0x0F);
                }

                ix += 8;
            }

            setWriteAddress(*pattern_table + offset);
            *vvdgd = (pixel);
            setWriteAddress(*color_table + offset);
            *vvdgd = (color);
        }
    }
}

//-----------------------------------------------------------------------------
void DrawLine(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, unsigned short color) {
    int ix, iy;
    int zz,x,y,addx,addy,dx,dy;
    long P;

    if (y1 == y2)       // Horizontal
        FillRect(x1,y1,(x2 - x1),1,color);
    else if (x1 == x2)  // Vertical
    {
        for (iy = y1; iy <= y2; iy++)
            vdp_plot_hires(x1, iy, color, *bgcolor);
    }
    else    // Torta
    {
        dx = (x2 - x1);
        dy = (y2 - y1);

        if (dx < 0)
            dx = dx * (-1);

        if (dy < 0)
            dy = dy * (-1);

        x = x1;
        y = y1;

        if(x1 > x2)
            addx = -1;
        else
            addx = 1;

        if(y1 > y2)
            addy = -1;
        else
            addy = 1;

        if(dx >= dy)
        {
            P = (2 * dy) - dx;

            for(ix = 1; ix <= (dx + 1); ix++)
            {
                vdp_plot_hires(x, y, color, *bgcolor);

                if (P < 0)
                {
                    P = P + (2 * dy);
                    zz = x + addx;
                    x = zz;
                }
                else
                {
                    P = P + (2 * dy) - (2 * dx);
                    x = x + addx;
                    zz = y + addy;
                    y = zz;
                }
            }
        }
        else
        {
            P = (2 * dx) - dy;

            for(ix = 1; ix <= (dy +1); ix++)
            {
                vdp_plot_hires(x, y, color, *bgcolor);

                if (P < 0)
                {
                    P = P + (2 * dx);
                    y = y + addy;
                }
                else
                {
                    P = P + (2 * dx) - (2 * dy);
                    x = x + addx;
                    y = y + addy;
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------
void DrawRect(unsigned short xi, unsigned short yi, unsigned short pwidth, unsigned short pheight, unsigned short color) {
    unsigned short xf, yf;

    xf = (xi + pwidth);
    yf = (yi + pheight);

    DrawLine(xi,yi,xf,yi,color);
    DrawLine(xi,yf,xf,yf,color);
    DrawLine(xi,yi,xi,yf,color);
    DrawLine(xf,yi,xf,yf,color);
}

//-----------------------------------------------------------------------------
void DrawRoundRect(unsigned int xi, unsigned int yi, unsigned int pwidth, unsigned int pheight, unsigned char radius, unsigned char color) {
	unsigned short tSwitch, x1 = 0, y1, xt, yt, wt;

    y1 = radius;

	tSwitch = 3 - 2 * radius;

	while (x1 <= y1) {
	    xt = xi + radius - x1;
	    yt = yi + radius - y1;
	    vdp_plot_hires(xt, yt, color, 0);

	    xt = xi + radius - y1;
	    yt = yi + radius - x1;
	    vdp_plot_hires(xt, yt, color, 0);

        xt = xi + pwidth-radius + x1;
	    yt = yi + radius - y1;
	    vdp_plot_hires(xt, yt, color, 0);

        xt = xi + pwidth-radius + y1;
	    yt = yi + radius - x1;
	    vdp_plot_hires(xt, yt, color, 0);

        xt = xi + pwidth-radius + x1;
        yt = yi + pheight-radius + y1;
	    vdp_plot_hires(xt, yt, color, 0);

        xt = xi + pwidth-radius + y1;
        yt = yi + pheight-radius + x1;
	    vdp_plot_hires(xt, yt, color, 0);

	    xt = xi + radius - x1;
        yt = yi + pheight-radius + y1;
	    vdp_plot_hires(xt, yt, color, 0);

	    xt = xi + radius - y1;
        yt = yi + pheight-radius + x1;
	    vdp_plot_hires(xt, yt, color, 0);

	    if (tSwitch < 0) {
	    	tSwitch += (4 * x1 + 6);
	    } else {
	    	tSwitch += (4 * (x1 - y1) + 10);
	    	y1--;
	    }
	    x1++;
	}

    xt = xi + radius;
    yt = yi + pheight;
    wt = pwidth - (2 * radius);
	DrawHoriLine(xt, yi, wt, color);		// top
	DrawHoriLine(xt, yt, wt, color);	// bottom

    xt = xi + pwidth;
    yt = yi + radius;
    wt = pheight - (2 * radius);
	DrawVertLine(xi, yt, wt, color);		// left
	DrawVertLine(xt, yt, wt, color);	// right
}

//-----------------------------------------------------------------------------
void DrawCircle(unsigned short x0, unsigned short y0, unsigned char r, unsigned char pfil, unsigned short pcor) {
  int f = 1 - r;
  int ddF_x = 1;
  int ddF_y = -2 * r;
  int x = 0;
  int y = r;

  vdp_plot_hires(x0  , y0+r, pcor, 0);
  vdp_plot_hires(x0  , y0-r, pcor, 0);
  vdp_plot_hires(x0+r, y0  , pcor, 0);
  vdp_plot_hires(x0-r, y0  , pcor, 0);

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    vdp_plot_hires(x0 + x, y0 + y, pcor, 0);
    vdp_plot_hires(x0 - x, y0 + y, pcor, 0);
    vdp_plot_hires(x0 + x, y0 - y, pcor, 0);
    vdp_plot_hires(x0 - x, y0 - y, pcor, 0);
    vdp_plot_hires(x0 + y, y0 + x, pcor, 0);
    vdp_plot_hires(x0 - y, y0 + x, pcor, 0);
    vdp_plot_hires(x0 + y, y0 - x, pcor, 0);
    vdp_plot_hires(x0 - y, y0 - x, pcor, 0);
  }
}

//-----------------------------------------------------------------------------
void InvertRect(unsigned short xi, unsigned short yi, unsigned short pwidth, unsigned short pheight) {
    unsigned short xf, yf;
    unsigned int ix, iy;
    unsigned int offset, posX, posY, modX, modY;
    unsigned char pixel;
    unsigned char color, color1, color2, vprim = 0;
    unsigned char sqtdtam[10];

    xf = (xi + pwidth);
    yf = (yi + pheight);

    if (xf > 255)
        xf = 255;

    if (yf > 191)
        yf = 191;

    for (iy = yi; iy <= yf; iy++)
    {
        ix = xi;
        while (ix <= xf)
        {
            posX = (int)(8 * (ix / 8));
            posY = (int)(256 * (iy / 8));
            modX = (int)(ix % 8);
            modY = (int)(iy % 8);

            offset = posX + modY + posY;

            setReadAddress(*pattern_table + offset);
            setReadAddress(*pattern_table + offset);
            pixel = *vvdgd;
            setReadAddress(*color_table + offset);
            setReadAddress(*color_table + offset);
            color = *vvdgd;

            if (modX == 0)
                vprim = 0;

            if (modX > 0 || (modX == 0 && ((ix + 8) > xf)))
            {
                pixel &= ~(0x80 >> modX);

                if (!vprim)
                {
                    vprim = 1;
                    color1 = (color & 0xF0) >> 4;
                    color2 = (color & 0x0F) << 4;
                    color = (color1 | color2);
                }

                ix++;
            }
            else
            {
                pixel = ~pixel;
                color1 = (color & 0xF0) >> 4;
                color2 = (color & 0x0F) << 4;
                color = (color1 | color2);

                ix += 8;
            }

            setWriteAddress(*pattern_table + offset);
            *vvdgd = (pixel);
            setWriteAddress(*color_table + offset);
            *vvdgd = (color);
        }
    }
}

//-------------------------------------------------------------------------
void fillin(unsigned char* vvar, unsigned short x, unsigned short y, unsigned short pwidth, unsigned char vtipo)
{
    unsigned short cc = 0;
    unsigned char cchar, *vvarptr, vdisp = 0;

    if (vtipo == WINOPER)
    {
        vvarptr = vvar;

        while (*vvarptr)
        {
            cc += 6;
            *vvarptr++;
        }

        *vBufReceived = 0x00;

        readChar();

        if (*vBufReceived >= 0x20 && *vBufReceived < 0x7F && (x + cc + 6) < (x + pwidth))
        {
            *vvarptr++ = *vBufReceived;
            *vvarptr = 0x00;

            locatexy(x+cc,y+1);
            writecxy(6, *vBufReceived, *vcorwf, *vcorwb);

            vdisp = 1;
        }
        else
        {
            switch (*vBufReceived)
            {
                case 0x0D:  // Enter
                    break;
                case 0x08:  // BackSpace
                    if (*pposx > (x + 10))
                    {
                        *vvarptr = '\0';
                        vvarptr--;
                        if (vvarptr < vvar)
                            vvarptr = vvar;
                        *vvarptr = '\0';
                        *pposx = *pposx - 6;
                        FillRect(*pposx, (*pposy - 1), 6, 9, *vcorwb);
                        locatexy(*pposx,*pposy);
                        writecxy(6, 0xFF, *vcorwf, *vcorwb);
                        *pposx = *pposx - 6;
                    }
                    break;
            }
        }
    }

    if (vtipo == WINDISP || vdisp)
    {
        if (!vdisp)
        {
            FillRect(x-2,y-2,pwidth+4,13,*vcorwb);
            DrawRect(x-2,y-2,pwidth+4,13,*vcorwf);
        }

        vvarptr = vvar;
        locatexy(x,y+2);

        while (*vvarptr)
        {
            cchar = *vvarptr++;
            cc++;

            writecxy(6, cchar, *vcorwb, *vcorwf);

            if (*pposx >= x + pwidth)
                break;
        }
    }
}

//-----------------------------------------------------------------------------
void SelRect(unsigned short x, unsigned short y, unsigned short pwidth, unsigned short pheight)
{
    DrawRect((x - 1), (y - 1), (pwidth + 2), (pheight + 2), VDP_DARK_RED);
}

//-----------------------------------------------------------------------------
void PutIcone(unsigned int* vimage, unsigned short x, unsigned short y, unsigned char numSprite)
{
    // TBD
}

//-----------------------------------------------------------------------------
void PutImage(unsigned char* cimage, unsigned short x, unsigned short y)
{
    // TBD
}

//-----------------------------------------------------------------------------
void LoadIconLib(unsigned char* cfile)
{
    // TBD
}

void vdp_read_data_gui(unsigned int addr, unsigned int startaddr, unsigned int qtd)
{
    int ix;

    setReadAddress(addr);
    setReadAddress(addr);

    /**(tempDataBase + startaddr) = addr;

    for (ix = 0; ix < qtd; ix++)
        *(tempDataMgui2 + startaddr + ix) = *vvdgd;*/
}

//-----------------------------------------------------------------------------
unsigned int vdp_sprite_init_gui(unsigned char name, unsigned char priority, unsigned char color = 0)
{
    unsigned int addr = *sprite_attribute_table + 4*priority;
    unsigned char byteVdp;

    while (1)
    {
        setWriteAddress(addr);

        *vvdgd = (0);
        *vvdgd = (0);

        if(*sprite_size_sel)
            *vvdgd = (4*name);
        else
            *vvdgd = (4*name);
        *vvdgd = (0x80 | (color & 0xF));

        setReadAddress(addr);
        setReadAddress(addr);

        byteVdp = *vvdgd;
        if (byteVdp != 0)
            continue;

        byteVdp = *vvdgd;
        if (byteVdp != 0)
            continue;

        byteVdp = *vvdgd;
        if (byteVdp != (4*name))
            continue;

        byteVdp = *vvdgd;
        if (byteVdp != (0x80 | (color & 0xF)))
            continue;

        break;
    }

    return addr;
}

//-----------------------------------------------------------------------------
unsigned char read_status_reg_gui(void)
{
    unsigned char memByte;

    memByte = *vvdgc;

    return memByte;
}

//-----------------------------------------------------------------------------
unsigned char vdp_sprite_set_position_gui(unsigned int addr, unsigned int x, unsigned char y)
{
    unsigned char ec, xpos;
    unsigned char color;

    xpos = (unsigned char)(x & 0xFF);
    ec = 0;

    setReadAddress(addr + 3);
    setReadAddress(addr + 3);
    color = *vvdgd & 0x0f;

    setWriteAddress(addr);
    *vvdgd = y;
    setWriteAddress(addr + 1);
    *vvdgd = xpos;

    setWriteAddress(addr + 3);
    *vvdgd = ((ec << 7) | color);
    return read_status_reg_gui();
}

//-----------------------------------------------------------------------------
void startMGI(void) {
    unsigned char vnomefile[12];
    unsigned char lc, ll, *ptr_ico, *ptr_prg, *ptr_pos;
    unsigned char* vFinalOSPos;
    int percent;
    long ix;

    if (!*vUseMouse)
    {
        printText("Error, Mouse not Found !!!");
        return;
    }

    *vcorwf = VDP_WHITE;
    *vcorwb = VDP_TRANSPARENT;
    *vcorwb2 = VDP_DARK_BLUE;

    vdp_init_gui(VDP_MODE_G2, VDP_DARK_BLUE, 0, 0);
    vdp_set_bdcolor(VDP_DARK_BLUE);

    loadFile("/MGUI/IMAGES/UTILITY.PBM", (unsigned long*)0x00830000);
    putImagePbmP4((unsigned long*)0x00830000, 8, 1);

    writesxy(116,130,2,"MGUI",*vcorwf,*vcorwb);
    writesxy(71,140,1,"Graphical",*vcorwf,*vcorwb);
    writesxy(131,140,1,"Interface",*vcorwf,*vcorwb);
    writesxy(113,150,1,"v"versionMgui,*vcorwf,*vcorwb);

    writesxy(86,170,1,"Loading Config",*vcorwf,*vcorwb);
    loadFile("/MGUI/MGUI.CFG", memPosConfig);

    writesxy(53,170,1,"Loading Icons ",*vcorwf,*vcorwb);
    writesxy(137,170,1,"ICOFOLD.PBM",*vcorwf,*vcorwb);
    loadFile("/MGUI/IMAGES/ICOFOLD.PBM", imgsMenuSys);
    writesxy(137,170,1,"ICORUN.PBM ",*vcorwf,*vcorwb);
    loadFile("/MGUI/IMAGES/ICORUN.PBM", (imgsMenuSys + 64));
    writesxy(137,170,1,"ICOOS.PBM  ",*vcorwf,*vcorwb);
    loadFile("/MGUI/IMAGES/ICOOS.PBM", (imgsMenuSys + 128));
    writesxy(137,170,1,"ICOSET.PBM ",*vcorwf,*vcorwb);
    loadFile("/MGUI/IMAGES/ICOSET.PBM", (imgsMenuSys + 192));
    writesxy(137,170,1,"ICOOFF.PBM ",*vcorwf,*vcorwb);
    loadFile("/MGUI/IMAGES/ICOOFF.PBM", (imgsMenuSys + 256));
    writesxy(53,170,1,"      Please Wait...       ",*vcorwf,*vcorwb);

    for (ix = 0; ix < 99999; ix++);

    *vcorwf = VDP_WHITE;
    *vcorwb = VDP_TRANSPARENT;
    *vcorwb2 = VDP_BLACK;

    vdp_init_gui(VDP_MODE_G2, VDP_BLACK, 0, 0);
    vdp_set_bdcolor(VDP_BLACK);

    *mouseX = 128;
    *mouseY = 96;
    redrawMain();

    TrocaSpriteMouse(MOUSE_POINTER);

    *spthdlmouse = vdp_sprite_init_gui(0, 0, VDP_DARK_RED);
    *statusVdpSprite = vdp_sprite_set_position_gui(*spthdlmouse, *mouseX, *mouseY);

    /*vdp_read_data_gui(*sprite_attribute_table,0x0000,0x80);
    vdp_read_data_gui(*sprite_pattern_table,0x0100,0x800);
    vdp_read_data_gui(*spthdlmouse,0x0900,0x04);*/

    // Inicia Controles de Tela (Mouse e Teclado)
    *kbdKeyPntr = 0x00;
    *kbdKeyBuffer = 0x00;
    *vBufReceived = 0x00;

    *(vmfp + Reg_IERA) = 0x40;
    *(vmfp + Reg_IMRA) = 0x40;

    while(editortela());

    vdp_textcolor(VDP_WHITE, VDP_BLACK);
    vdp_init_gui(VDP_MODE_TEXT, (*fgcolor<<4) | (*bgcolor & 0x0f), 0, 0);

    clearScr();
}

//-------------------------------------------------------------------------
void showWindow(unsigned char* bstr, unsigned char x1, unsigned char y1, unsigned short pwidth, unsigned char pheight, unsigned char bbutton)
{
	unsigned short i, ii, xib, yib;
    unsigned char cc = 0, sqtdtam[10];

    // Desenha a Janela
    DrawRect(x1, y1, pwidth, pheight, *vcorwf);
	FillRect(x1 + 1, y1 + 1, pwidth - 2, pheight - 2, *vcorwb);

    if (*bstr) {
        DrawRect(x1, y1, pwidth, 12, *vcorwf);
        writesxy(x1 + 2, y1 + 3,1,bstr,*vcorwf,*vcorwb);
    }

    i = 1;
    for (ii = 0; ii <= 7; ii++)
        vbuttonwin[ii] = 0;

	// Desenha Botoes
    *vbbutton = bbutton;
	while (*vbbutton)
	{
		xib = x1 + 8 + (34 * (i - 1));
		yib = (y1 + pheight) - 12;
        vbuttonwiny = yib;
		i++;

        drawButtons(xib, yib);
	}
}

//-------------------------------------------------------------------------
void drawButtons(unsigned short xib, unsigned short yib) {
    // Desenha Bot?
	//FillRect(xib, yib, 42, 10, VDP_WHITE);
	DrawRoundRect(xib,yib,32,10,1,*vcorwf);  // rounded rectangle around text area

	// Escreve Texto do Bot?
	if (*vbbutton & BTOK)
	{
		writesxy(xib + 16 - 6, yib + 2,1,"OK",*vcorwf,*vcorwb);
        *vbbutton = *vbbutton & 0xFE;    // 0b11111110
        vbuttonwin[1] = xib;
	}
	else if (*vbbutton & BTSTART)
	{
		writesxy(xib + 16 - 15, yib + 2,1,"START",*vcorwf,*vcorwb);
        *vbbutton = *vbbutton & 0xDF;    // 0b11011111
        vbuttonwin[6] = xib;
	}
	else if (*vbbutton & BTCLOSE)
	{
		writesxy(xib + 16 - 15, yib + 2,1,"CLOSE",*vcorwf,*vcorwb);
        *vbbutton = *vbbutton & 0xBF;    // 0b10111111
        vbuttonwin[7] = xib;
	}
	else if (*vbbutton & BTCANCEL)
	{
		writesxy(xib + 16 - 12, yib + 2,1,"CANC",*vcorwf,*vcorwb);
        *vbbutton = *vbbutton & 0xFD;    // 0b11111101
        vbuttonwin[2] = xib;
	}
	else if (*vbbutton & BTYES)
	{
		writesxy(xib + 16 - 9, yib + 2,1,"YES",*vcorwf,*vcorwb);
        *vbbutton = *vbbutton & 0xFB;    // 0b11111011
        vbuttonwin[3] = xib;
	}
	else if (*vbbutton & BTNO)
	{
		writesxy(xib + 16 - 6, yib + 2,1,"NO",*vcorwf,*vcorwb);
        *vbbutton = *vbbutton & 0xF7;    // 0b11110111
        vbuttonwin[4] = xib;
	}
	else if (*vbbutton & BTHELP)
	{
		writesxy(xib + 16 - 12, yib + 2,1,"HELP",*vcorwf,*vcorwb);
        *vbbutton = *vbbutton & 0xEF;    // 0b11101111
        vbuttonwin[5] = xib;
	}
}

//-----------------------------------------------------------------------------
void redrawMain(void) {
    TrocaSpriteMouse(MOUSE_HOURGLASS);

    clearScrW(VDP_BLACK);

    // Desenhar Barra Menu Principal / Status
    desenhaMenu();

    TrocaSpriteMouse(MOUSE_POINTER);
}

//-----------------------------------------------------------------------------
void desenhaMenu(void)
{
    unsigned long lc, idx;
    unsigned int vx, vy;

    vx = COLMENU;
    vy = LINMENU;

    for (lc = 0; lc <= 4; lc++)
    {
        idx = lc * 64;
        putImagePbmP4((imgsMenuSys + idx), vx, vy);
        vx += 24;

        /*MostraIcone(vx, vy, lc,*vcorwf, *vcorwb);
        vx += 16;*/
    }

    DrawLine(0, 20 /*10*/, *vdpMaxCols, 20 /*10*/, *vcorwf);

/*    DrawCircle((*vdpMaxCols - 5), (*vdpMaxRows - 6), 3, 1, VDP_WHITE);
    DrawLine((*vdpMaxCols - 5), (*vdpMaxRows - 10), (*vdpMaxCols - 5), (*vdpMaxRows - 6), VDP_WHITE);*/
}

//--------------------------------------------------------------------------
unsigned char editortela(void)
{
    unsigned char vresp = 1, vwb;
    unsigned char vx, cc, vpos, vposiconx, vposicony, mpos;
    unsigned char *ptr_prg;

    // Monitora Mouse
    VerifyMouse();

    // Verifica se clicou no simbolo de sair
    if (*mouseBtnPres == 0x04) // Meio - Para reiniciar o sprite do mouse que as vezes nao aparece assim que roda o prog
    {
        *spthdlmouse = vdp_sprite_init_gui(0, 0, VDP_DARK_RED);
        *statusVdpSprite = vdp_sprite_set_position_gui(*spthdlmouse, *mouseX, *mouseY);
    }

    /**(vmfp + Reg_IERA) = 0x60;
    *(vmfp + Reg_IMRA) = 0x60;    */

    // Monitora Teclado
    //readChar();
    if (*kbdKeyPntr != 0x00)
    {
        if (*kbdKeyBuffer == 0x1B)  // ESC
            vresp = 0x00;
        else
        {
            /**(vmfp + Reg_IERA) = 0x40;
            *(vmfp + Reg_IMRA) = 0x40;*/
        }
    }

    if (*mouseBtnPres == 0x01)  // Esquerdo
    {
        if (*vposty <= 22)
            vresp = new_menu();
        /*else {
            vposiconx = COLINIICONS;
            vposicony = 40;
            vpos = 0;

            if (*vposty >= 136) {
                vpos = ICONSPERLINE * 3;
                vposicony = 136;
            }
            else if (*vposty >= 30) {
                vpos = ICONSPERLINE * 2;
                vposicony = 104;
            }
            else if (*vposty >= 30) {
                vpos = ICONSPERLINE;
                vposicony = 72;
            }

            if (*vpostx >= COLINIICONS && *vpostx <= (COLINIICONS + (24 + SPACEICONS) * ICONSPERLINE) && *vposty >= 40) {
                cc = 1;
                for(vx = (COLINIICONS + (24 + SPACEICONS) * (ICONSPERLINE - 1)); vx >= (COLINIICONS + (24 + SPACEICONS)); vx -= (24 + SPACEICONS)) {
                    if (*vpostx >= vx) {
                    vpos += ICONSPERLINE - cc;
                    vposiconx = vx;
                    break;
                    }

                    cc++;
                }

                ptr_prg = vFinalOS + (MEM_POS_MGICFG + 16) + 32 + 320;
                ptr_prg = ptr_prg + (vpos * 10);

                if (*ptr_prg != 0) {
                    InvertRect( vposiconx, vposicony, 24, 24);

                    strcpy(vbuf,ptr_prg);

                    TrocaSpriteMouse(MOUSE_HOURGLASS);  // Mostra Ampulheta

                    processCmd();

                    *vbuf = 0x00;

                    redrawMain();
                }
            }
        }*/
    }

    return vresp;
}

//-----------------------------------------------------------------------------
// read a byte from Mouse
//-----------------------------------------------------------------------------
unsigned long readMousePs2 (void)
{
    unsigned long bData = 0;
    unsigned long vTimeout = 0x0FFF;
    unsigned int ix;

    // shift in 11 bits from MSB to LSB
    for(ix = 0; ix < 11; ix++)
    {
        while((*mfpgpdr & 0x80) && vTimeout) vTimeout--; // wait for the clock to go LOW

        if (vTimeout)
            bData += (((*mfpgpdr & 0x02) >> 1) << ix);    // shift in a bit

        while(!(*mfpgpdr & 0x80) && vTimeout) vTimeout--;  // wait here while the clock is still low
    }

    return bData;
}

//-------------------------------------------------------------------------
void VerifyMouse(void)
{
    unsigned char mouseMoveRdy;
    int vTimeout;
    long rd[5];
    unsigned char stat = 0;
    char xx, yy;

    mouseMoveRdy = 0;
    vTimeout = 0x0FFF;
    *mouseBtnPres = 0x00;

    while((*mfpgpdr & 0x02) && vTimeout) vTimeout--; // Wait data = 0
    if (vTimeout)
    {
        rd[0] = readMousePs2();
        while((*mfpgpdr & 0x02) && vTimeout) vTimeout--; // Wait data = 0
        if (vTimeout)
        {
            rd[1] = readMousePs2();
            while((*mfpgpdr & 0x02) && vTimeout) vTimeout--; // Wait data = 0
            if (vTimeout)
            {
                rd[2] = readMousePs2();
                mouseMoveRdy = 1;
            }
        }
    }

    if (mouseMoveRdy)
    {
        rd[0] = (rd[0] >> 1) & 0x00FF; // shift out the start bit
        rd[1] = (rd[1] >> 1) & 0x00FF; // shift out the start bit
        rd[2] = (rd[2] >> 1) & 0x00FF; // shift out the start bit

        stat = (unsigned char)rd[0];
        xx = (char)rd[1];
        if (xx < -1) xx = -1;
        if (xx > 1) xx = 1;
        if ((xx == -1 && *mouseX > 0) || (xx == 1 && *mouseX < 255))
            *mouseX = *mouseX + xx;
        yy = (char)(rd[2] * (-1));
        if (yy < -1) yy = -1;
        if (yy > 1) yy = 1;
        if ((yy == -1 && *mouseY > 0) || (yy == 1 && *mouseY < 191))
            *mouseY = *mouseY + yy;

        *mouseBtnPres = stat & 0x07;

        *statusVdpSprite = vdp_sprite_set_position_gui(*spthdlmouse, *mouseX, *mouseY);

        if (*mouseBtnPres)
        {
            *vpostx = *mouseX;
            *vposty = *mouseY;
        }

        mouseMoveRdy = 0;
    }
}

//-------------------------------------------------------------------------
unsigned char waitButton(void)
{
  unsigned char i, ii, iii;
  ii = 0;
  VerifyMouse();

  if (*mouseBtnPres == 0x01)  // Esquerdo
  {
    for (i = 1; i <= 7; i++) {
        if (vbuttonwin[i] != 0 && *vpostx >= vbuttonwin[i] && *vpostx <= (vbuttonwin[i] + 32) && *vposty >= vbuttonwiny && *vposty <= (vbuttonwiny + 10)) {
        ii = 1;

        for (iii = 1; iii <= (i - 1); iii++)
            ii *= 2;

        break;
        }
    }
  }

  return ii;
}

//-------------------------------------------------------------------------
unsigned char message(char* bstr, unsigned char bbutton, unsigned short btime)
{
	unsigned short i, ii, iii, xi, yi, xf, xm, yf, ym, pwidth, pheight, xib, yib, xic, yic;
	unsigned char qtdnl, maxlenstr;
	unsigned char qtdcstr[8], poscstr[8], cc, dd, vbty = 0;
	unsigned char *bstrptr;

    TrocaSpriteMouse(MOUSE_HOURGLASS);

	qtdnl = 1;
	maxlenstr = 0;
	qtdcstr[1] = 0;
	poscstr[1] = 0;
	i = 0;

    for (ii = 0; ii <= 7; ii++)
        vbuttonwin[ii] = 0;

    bstrptr = bstr;
	while (*bstrptr)
	{
		qtdcstr[qtdnl]++;

		if (qtdcstr[qtdnl] > 26)
			qtdcstr[qtdnl] = 26;

		if (qtdcstr[qtdnl] > maxlenstr)
			maxlenstr = qtdcstr[qtdnl];

		if (*bstrptr == '\n')
		{
			qtdcstr[qtdnl]--;
			qtdnl++;

			if (qtdnl > 6)
				qtdnl = 6;

			qtdcstr[qtdnl] = 0;
			poscstr[qtdnl] = i + 1;
		}

        bstrptr++;
        i++;
	}

	if (maxlenstr > 26)
		maxlenstr = 26;

	if (qtdnl > 6)
		qtdnl = 6;

	pwidth = (maxlenstr + 1) * 6;
	pwidth = pwidth + 2;
	xm = pwidth / 2;
	xi = ((*vdpMaxCols) / 2) - xm + 1;
	xf = ((*vdpMaxCols) / 2) + xm - 1;

	pheight = 10 * qtdnl;
	pheight = pheight + 20;
	ym = pheight / 2;
	yi = ((*vdpMaxRows) / 2) - ym - 1;
	yf = ((*vdpMaxRows) / 2) + ym - 1;

	// Desenha Linha Fora
    SaveScreen(xi,yi,pwidth + 5,pheight + 5);

    FillRect(xi,yi,pwidth,pheight,*vcorwb);
	DrawRoundRect(xi,yi,pwidth,pheight,2,*vcorwf);  // rounded rectangle around text area

	// Escreve Texto Dentro da Caixa de Mensagem
	for (i = 1; i <= qtdnl; i++)
	{
		xib = xi + xm;
		xib = xib - ((qtdcstr[i] * 6) / 2);
		yib = yi + 2 + (10 * (i - 1));

        locatexy(xib, yib);
        bstrptr = bstr + poscstr[i];
		for (ii = poscstr[i]; ii <= (poscstr[i] + qtdcstr[i] - 1) ; ii++)
            writecxy(6, *bstrptr++, *vcorwf, *vcorwb);
	}

	// Desenha Botoes
    i = 1;
    *vbbutton = bbutton;
	while (*vbbutton)
	{
		xib = xi + 2 + (34 * (i - 1));
		yib = yf - 12;
        vbty = yib;
		i++;

        drawButtons(xib, yib);
	}

    ii = 0;

    if (!btime)
    {
        TrocaSpriteMouse(MOUSE_POINTER);

        while (!ii) {
            VerifyMouse();

            if (*mouseBtnPres == 0x01)  // Esquerdo
            {
                for (i = 1; i <= 7; i++) {
                    if (vbuttonwin[i] != 0 && *vpostx >= vbuttonwin[i] && *vpostx <= (vbuttonwin[i] + 32) && *vposty >= vbty && *vposty <= (vbty + 10))
                    {
                        ii = 1;

                        for (iii = 1; iii <= (i - 1); iii++)
                            ii *= 2;

                        break;
                    }
                }
            }
        }

        TrocaSpriteMouse(MOUSE_HOURGLASS);
    }
    else {
        for (dd = 0; dd <= 10; dd++)
        for (cc = 0; cc <= btime; cc++);
    }

    RestoreScreen(xi,yi,pwidth + 5,pheight + 5);

    TrocaSpriteMouse(MOUSE_POINTER);

    return ii;
}

//-----------------------------------------------------------------------------
void MostraIcone(unsigned short xi, unsigned short yi, unsigned char vicone, unsigned char colorfg, unsigned char colorbg)
{
    unsigned short yf;
    unsigned int ix, iy;
    unsigned int offset, posX, posY, modY, offsetIcon;
    unsigned char pixel, color = ((colorfg << 4) + colorbg);
    unsigned char* vTempIcones = *iconesMenuSys;

    // Define Final
    yf = (yi + 8);
    ix = 0;
    offsetIcon = (vicone * 8);

    for (iy = yi; iy <= yf; iy++)
    {
        posX = (int)(8 * (xi / 8));
        posY = (int)(256 * (iy / 8));
        modY = (int)(iy % 8);
        offset = posX + modY + posY;

        pixel = *(vTempIcones + offsetIcon + ix);
        setWriteAddress(*pattern_table + offset);
        *vvdgd = pixel;
        setWriteAddress(*color_table + offset);
        *vvdgd = color;

        ix++;
    }
}

//-----------------------------------------------------------------------------
//  vicone: 1 - Ponteiro, 2 - Ampulheta
//-----------------------------------------------------------------------------
void TrocaSpriteMouse(unsigned char vicone)
{
    long ix;
    unsigned char tempPtrMouse[8];
    unsigned char* vTempSpritePointer = *mousePointer;
    unsigned char* vTempSpriteHourGlass = *mouseHourGlass;

    // Inicializa ponteiro Mouse
    switch (vicone)
    {
        case 1:
            for (ix = 0; ix < 8; ix++)
                tempPtrMouse[ix] = *(vTempSpritePointer + ix);
            break;
        case 2:
            for (ix = 0; ix < 8; ix++)
                tempPtrMouse[ix] = *(vTempSpriteHourGlass + ix);
            break;
    }

    vdp_set_sprite_pattern(0, tempPtrMouse);
}

//-------------------------------------------------------------------------
unsigned char new_menu(void) {
    unsigned short vx, vy, lc, vposicony, mx, my, menyi[8], menyf[8];
    unsigned char vpos = 0, vresp, mpos;

    vresp = 1;

    if (*vpostx >= COLMENU && *vpostx <= (COLMENU + 16)) {
        mx = 0;
        my = LINHAMENU;
        mpos = 0;

        FillRect(mx,my,128,42,*vcorwb);
        DrawRect(mx,my,128,42,*vcorwf);

        mpos += 2;
        menyi[0] = my + mpos;
        writesxy(mx + 8,my + mpos,1,"Files",*vcorwf,*vcorwb);
        mpos += 12;
        menyf[0] = my + mpos;
        DrawLine(mx,my + mpos,mx+128,my + mpos,*vcorwf);

        mpos += 2;
        menyi[1] = my + mpos;
        writesxy(mx + 8,my + mpos,1,"Import File",*vcorwf,*vcorwb);
        mpos += 12;
        menyf[1] = my + mpos;
        mpos += 2;
        menyi[2] = my + mpos;
        writesxy(mx + 8,my + mpos,1,"About",*vcorwf,*vcorwb);
        mpos += 12;
        menyf[2] = my + mpos;
        DrawLine(mx,my + mpos,mx+128,my + mpos,*vcorwf);

        while (*mouseBtnPres)   // Aguarda liberar botoes do mouse
            VerifyMouse();

        while (*mouseBtnPres != 0x01)   // Loop enquanto botao esquerdo nao é pressionado
        {
            VerifyMouse();

            if (*mouseBtnPres == 0x01)  // Esquerdo
            {
                if ((*vposty >= my && *vposty <= my + 42) && (*vpostx >= mx && *vpostx <= mx + 128)) {
                    vpos = 0;
                    vposicony = 0;

                    for(vy = 0; vy <= 1; vy++) {
                        if (*vposty >= menyi[vy] && *vposty <= menyf[vy]) {
                            vposicony = menyi[vy];
                            break;
                        }

                        vpos++;
                    }

/*                    if (vposicony > 0)
                        InvertRect( mx + 4, vposicony, 120, 12);*/

                    switch (vpos) {
                        case 0: // Call "Files" Program from Disk
                            loadFile("/MGUIPROG/FILES.BIN", (unsigned long*)0x00830000);
                            if (!*verro)
                                runFromMguiCmd();
                            else
                                message("Loading Error...\0", BTCLOSE, 0);
                            break;
                        case 1: // Help
                            importFile();
                            break;
                        case 2: // About
                            message("MGUI v0.1\nGraphical User Interface\n \nwww.utilityinf.com.br\0", BTCLOSE, 0);
                            break;
                    }
                }
            }
        }

        redrawMain();
    }
    else {
        for (lc = 1; lc <= 4; lc++) {
            mx = COLMENU + (24 * lc);
            if (*vpostx >= mx && *vpostx <= (mx + 16)) {
/*                InvertRect( mx, 4, 8, 8);
                InvertRect( mx, 4, 8, 8);*/
                break;
            }
        }

        switch (lc) {
            case 1: // RUN
                break;
            case 2: // MMSJDOS
                break;
            case 3: // SETUP
                break;
            case 4: // EXIT
                mpos = message("Do you want to exit ?\0", BTYES | BTNO, 0);
                if (mpos == BTYES)
                    vresp = 0;
                else
                    redrawMain();

                break;
        }

        if (lc < 4)
            redrawMain();
    }

    return vresp;
}

//-----------------------------------------------------------------------------
void importFile(void)
{
    DWORD vStep, ix;
    unsigned char *xaddress = 0x00840000;
    BYTE vErro, vPerc;
    char vfilename[64], vstring[64];
    BYTE vwb, vresp, vBuffer[128];
    int iy;
    BYTE sqtdtam[10];

    SaveScreen(10,40,240,60);
    showWindow("Import File",10,40,240,50,BTOK | BTCANCEL);

    writesxy(12,57,8,"File Name:",*vcorwf,*vcorwb);
    fillin(vstring, 78, 57, 130, WINDISP);

    vErro = RETURN_OK;

    while (1)
    {
        fillin(vstring, 78, 57, 130, WINOPER);

        vwb = waitButton();

        if (vwb == BTOK || vwb == BTCANCEL)
            break;
    }

    RestoreScreen(10,40,240,60);

    if (vwb == BTOK)
    {
        if (vstring == 0)
        {
            message("Error, file name must be provided!!\0", BTCLOSE, 0);
            return;
        }

        for(ix = 0; ix < 12 && toupper(vstring[ix]) != 0x00; ix++)
            vfilename[ix] = toupper(vstring[ix]);

        vfilename[ix] = 0x00;

        vresp = message("Confirm serial Connected.\nImport File ?\0",(BTYES | BTNO), 0);
        if (vresp == BTYES)
        {
            TrocaSpriteMouse(MOUSE_HOURGLASS);

            SaveScreen(10,40,240,70);
            showWindow("Status Import File",10,40,240,70, BTCLOSE);

            // Verifica se o arquivo existe
            if (fsFindInDir(vfilename, TYPE_FILE) < ERRO_D_START)
            {
                writesxy(12,55,8,"Deleting File...",*vcorwf,*vcorwb);

                // Se existir, apaga
                fsDelFile(vfilename);
            }

            // Cria o Arquivo
            writesxy(12,55,8,"Creating File...",*vcorwf,*vcorwb);

            vErro = fsCreateFile(vfilename);
            if (vErro == RETURN_OK)
            {
                // Recebe os dados via Serial
                writesxy(12,55,8,"Reading Serial...",*vcorwf,*vcorwb);

                if (!loadSerialToMem("840000", 0))
                {
                    // Abre Arquivo
                    writesxy(12,55,8,"Opening File...",*vcorwf,*vcorwb);

                    fsOpenFile(vfilename);

                    // Grava no Arquivo
                    writesxy(12,55,8,"Writing File...",*vcorwf,*vcorwb);

                    DrawRect(18,68,203,14,*vcorwf);

                    vStep = *vSizeTotalRec / 20;
                    vPerc = 0;

                    for (ix = 0; ix < *vSizeTotalRec; ix += 128)
                    {
                        for (iy = 0; iy < 128; iy++)
                        {
                            if (ix > 0 && ((ix + iy) % vStep) == 0)
                            {
                                FillRect((21 + vPerc), 71, 8, 8, VDP_DARK_BLUE);
                                vPerc += 10;
                            }

                            vBuffer[iy] = *xaddress;
                            xaddress += 1;
                        }

                        vErro = fsWriteFile(vfilename, ix, vBuffer, 128);
                        if (vErro != RETURN_OK)
                            break;
                    }

                    // Fecha Arquivo
                    writesxy(12,55,8,"Closing File...",*vcorwf,*vcorwb);

                    fsCloseFile(vfilename, 0);

                    if (vErro == RETURN_OK)
                        writesxy(12,55,8,"Done !         ",*vcorwf,*vcorwb);
                    else
                    {
                        writesxy(12,55,8,"Writing File Error !",*vcorwf,*vcorwb);
                        itoa(vErro, sqtdtam, 16);
                        writesxy(12,65,8,sqtdtam,*vcorwf,*vcorwb);
                    }
                }
                else
                    writesxy(12,55,8,"Serial Load Error...",*vcorwf,*vcorwb);
            }
            else
            {
                writesxy(12,55,8,"Create File Error...",*vcorwf,*vcorwb);
                itoa(vErro, sqtdtam, 16);
                writesxy(12,65,8,sqtdtam,*vcorwf,*vcorwb);
                writesxy(12,75,8,vfilename,*vcorwf,*vcorwb);
            }

            TrocaSpriteMouse(MOUSE_POINTER);

            while (1)
            {
                vwb = waitButton();

                if (vwb == BTCLOSE)
                    break;
            }

            RestoreScreen(10,40,240,70);
        }
    }

    return;
}

//-----------------------------------------------------------------------------
void putImagePbmP4(unsigned char* cursor, unsigned short ix, unsigned short iy) 
{
    char tipo[3], cnum[5];
    int largura = 0, altura = 0;
    int bytes_por_linha,x,y,ixx;
    unsigned char* dados = cursor;
    unsigned char* linha = dados;

    // Ler o tipo do formato (P4)
    tipo[0] = cursor[0];
    tipo[1] = cursor[1];
    tipo[2] = '\0';
    cursor += 3;

    if (strcmp(tipo, "P4") != 0)
    {
        message("Invalid or unsupported PBM format\nexpected P4",BTCLOSE,0);
        return;
    }

    // Ignorar comentários
    while (*cursor == '#') {
        while (*cursor != '\n') cursor++; // Ignorar até o final da linha
        cursor++; // Pular o '\n'
    }

    // Ler largura e altura
    x = 0;
    y = 0;
    while(y < 8)
    {
        if (*cursor != ' ' && *cursor != '\n')
        {
            cnum[x] = *cursor;
            x++;
            cursor++;
            y++;
        }
        else
        {
            cnum[x] = '\0';
            x = 0;

            if (*cursor == ' ')
                largura = atoi(cnum);
            else
            {
                altura = atoi(cnum);
                cursor++;
                break;
            }

            cursor++;
        }
    }

    // Dados de pixels começam após o cabeçalho
    dados = cursor;

    // Calcular o número de bytes por linha (cada byte representa 8 pixels)
    bytes_por_linha = (largura + 7) / 8;

    // Processar os dados de pixels
    for (y = 0; y < altura; y++)
    {
        linha = dados + y * bytes_por_linha;

        // Enviar cada byte da linha para o vídeo
        ixx = ix;
        for (x = 0; x < bytes_por_linha; x++) 
        {
            SetByte(ixx, (iy + y), linha[x], *vcorwf, *vcorwb2);
            ixx += 8;
        }
    }
}

#ifndef __EM_OBRAS__
//-----------------------------------------------------------------------------
void desenhaIconesUsuario(void) {
  unsigned short vx, vy;
  unsigned char lc, lcok, *ptr_ico, *ptr_prg, *ptr_pos;

  // COLOCAR ICONSPERLINE = 10
  // COLOCAR SPACEICONS = 8

  *next_pos = 0;

  ptr_pos = vFinalOS + (MEM_POS_MGICFG + 16);
  ptr_ico = ptr_pos + 32;
  ptr_prg = ptr_ico + 320;

  for(lc = 0; lc <= (ICONSPERLINE * 4 - 1); lc++) {
    ptr_pos = ptr_pos + lc;
    ptr_ico = ptr_ico + (lc * 10);
    ptr_prg = ptr_prg + (lc * 10);

    if (*ptr_prg != 0 && *ptr_ico != 0) {
      if (*ptr_pos <= (ICONSPERLINE - 1)) {
        vx = COLINIICONS + (24 + SPACEICONS) * *ptr_pos;
        vy = 40;
      }
      else if (*ptr_pos <= (ICONSPERLINE * 2 - 1)) {
        vx = COLINIICONS + (24 + SPACEICONS) * (*ptr_pos - ICONSPERLINE);
        vy = 72;
      }
      else if (*ptr_pos <= (ICONSPERLINE * 3 - 1)) {
        vx = COLINIICONS + (24 + SPACEICONS) * (*ptr_pos - ICONSPERLINE);
        vy = 104;
      }
      else {
        vx = COLINIICONS + (24 + SPACEICONS) * (*ptr_pos - ICONSPERLINE * 2);
        vy = 136;
      }

      lcok = lc + 20;

      SendIcone(lcok);
      MostraIcone(vx, vy, lcok);

      *next_pos = *next_pos + 1;
    }
  }
}

//-----------------------------------------------------------------------------
void SendIcone_24x24(unsigned char vicone)
{
    unsigned char vnomefile[12];
    unsigned char *ptr_prg;
    unsigned long *ptr_viconef;
    unsigned short ix, iy, iz, pw, ph;
    unsigned char* pimage;
    unsigned char ic;

    ptr_prg = vFinalOS + (MEM_POS_MGICFG + 16) + 32 + 320;

    // Procura Icone no Disco se Nao for Padrao
    if (vicone >= 20)
    {
        vicone -= 20;
        ptr_prg = ptr_prg + (vicone * 10);
        _strcat(vnomefile,*ptr_prg,".ICO");
        loadFile(vnomefile, (unsigned long*)0x00FF9FF8);   // 12K espaco pra carregar arquivo. Colocar logica pra pegar tamanho e alocar espaco
        vicone += 20;
        if (*verro)
            vicone = 9;
        else
            ptr_viconef = viconef;
    }

    if (vicone < 20)
        ptr_viconef = vFinalOS + (MEM_POS_ICONES + (1152 * vicone));

    ic = 0;
    iz = 0;
    pw = 24;
    ph = 24;
    pimage = ptr_viconef;

    // Acumula dados, enviando em 9 vezes de 64 x 16 bits
    *vpicg = 0x04;
    *vpicg = 0xDE;
    *vpicg = pw;
    *vpicg = ph;
    *vpicg = vicone;

    *vpicg = 130;
    *vpicg = 0xDE;
    *vpicg = ic;

    for (ix = 0; ix < 576; ix++)
    {
        *vpicg = *pimage++ & 0x00FF;
        *vpicg = *pimage++ & 0x00FF;
        iz++;

        if (iz == 64 && ic < 8)
        {
            ic++;

            *vpicg = 130;
            *vpicg = 0xDE;
            *vpicg = ic;

            iz = 0;
        }
    }
}

//-----------------------------------------------------------------------------
void SendIcone(unsigned char vicone)
{
    unsigned char vnomefile[12];
    unsigned char *ptr_prg;
    unsigned long *ptr_viconef;
    unsigned short ix, iy, iz, pw, ph;
    unsigned char* pimage;
    unsigned char ic;

    ptr_prg = vFinalOS + (MEM_POS_MGICFG + 16) + 32 + 320;

    // Procura Icone no Disco se Nao for Padrao
    if (vicone >= 20)
    {
        vicone -= 20;
        ptr_prg = ptr_prg + (vicone * 10);
        _strcat(vnomefile,*ptr_prg,".ICO");
        loadFile(vnomefile, (unsigned long*)0x00FF9FF8);   // 12K espaco pra carregar arquivo. Colocar logica pra pegar tamanho e alocar espaco
        vicone += 20;
        if (*verro)
            vicone = 9;
        else
            ptr_viconef = viconef;
    }

    if (vicone < 20)
        ptr_viconef = vFinalOS + (MEM_POS_ICONES + (4608 * vicone));

    ic = 0;
    iz = 0;
    pw = 48;
    ph = 48;
    pimage = ptr_viconef;

    // Acumula dados, enviando em 36 vezes de 64 x 16 bits
    *vpicg = 0x04;
    *vpicg = 0xDE;
    *vpicg = pw;
    *vpicg = ph;
    *vpicg = vicone;

    *vpicg = 130;
    *vpicg = 0xDE;
    *vpicg = ic;

    for (ix = 0; ix < 2304; ix++)
    {
        *vpicg = *pimage++ & 0x00FF;
        *vpicg = *pimage++ & 0x00FF;
        iz++;

        if (iz == 64 && ic < 35)
        {
            ic++;

            *vpicg = 130;
            *vpicg = 0xDE;
            *vpicg = ic;

            iz = 0;
        }
    }
}

//------------------------------------------------------------------------
void VerifyMouse(unsigned char vtipo) {
}

//-------------------------------------------------------------------------
void new_icon(void) {
}

//-------------------------------------------------------------------------
void del_icon(void) {
}

//-------------------------------------------------------------------------
void mgi_setup(void) {
}

//-------------------------------------------------------------------------
void executeCmd(void) {
    unsigned char vstring[64], vwb;

    vstring[0] = '\0';

    strcpy(vparamstr,"Execute");
    vparam[0] = 10;
    vparam[1] = 40;
    vparam[2] = 280;
    vparam[3] = 50;
    vparam[4] = BTOK | BTCANCEL;
    showWindow();

    writesxy(12,55,1,"Execute:",*vcorwf,*vcorwb);
    fillin(vstring, 84, 55, 160, WINDISP);

    while (1) {
        fillin(vstring, 84, 55, 160, WINOPER);

        vwb = waitButton();

        if (vwb == BTOK || vwb == BTCANCEL)
            break;
    }

    if (vwb == BTOK) {
        strcpy(vbuf, vstring);

        MostraIcone(144, 104, ICON_HOURGLASS);  // Mostra Ampulheta

        // Chama processador de comandos
        processCmd();

        while (*vxmaxold != 0) {
            vwb = waitButton();

            if (vwb == BTCLOSE)
                break;
        }

        if (*vxmaxold != 0) {
            *vxmax = *vxmaxold;
            *vymax = *vymaxold;
            *vcol = 0;
            *vlin = 0;
            *voverx = 0;
            *vovery = 0;
            *vxmaxold = 0;
            *vymaxold = 0;
        }

        *vbuf = 0x00;  // Zera Buffer do teclado
    }
}

//-------------------------------------------------------------------------
void radioset(unsigned char* vopt, unsigned char *vvar, unsigned short x, unsigned short y, unsigned char vtipo) {
  unsigned char cc, xc;
  unsigned char cchar, vdisp = 0;

  xc = 0;
  cc = 0;
  cchar = ' ';

  while(vtipo == WINOPER && cchar != '\0') {
    cchar = vopt[cc];
    if (cchar == ',') {
      if (cchar == ',' && cc != 0)
        xc++;

      if (*vpostx >= x && *vpostx <= x + 8 && *vposty >= (y + (xc * 10)) && *vposty <= ((y + (xc * 10)) + 8)) {
        vvar[0] = xc;
        vdisp = 1;
      }
    }

    cc++;
  }

  xc = 0;
  cc = 0;

  while(vtipo == WINDISP || vdisp) {
    cchar = vopt[cc];

    if (cchar == ',') {
      if (cchar == ',' && cc != 0)
        xc++;

      FillRect(x, y + (xc * 10), 8, 8, White);
      DrawCircle(x + 4, y + (xc * 10) + 2, 4, 0, Black);

      if (vvar[0] == xc)
        DrawCircle(x + 4, y + (xc * 10) + 2, 3, 1, Black);
      else
        DrawCircle(x + 4, y + (xc * 10) + 2, 3, 0, Black);

      locatexy(x + 10, y + (xc * 10));
    }

    if (cchar != ',' && cchar != '\0')
      writecxy(6, cchar, Black, White);

    if (cchar == '\0')
      break;

    cc++;
  }
}

//-------------------------------------------------------------------------
void togglebox(unsigned char* bstr, unsigned char *vvar, unsigned short x, unsigned short y, unsigned char vtipo) {
  unsigned char cc = 0;
  unsigned char cchar, vdisp = 0;

  if (vtipo == WINOPER && *vpostx >= x && *vpostx <= x + 4 && *vposty >= y && *vposty <= y + 4) {
    if (vvar[0])
      vvar[0] = 0;
    else
      vvar[0] = 1;

    vdisp = 1;
  }

  if (vtipo == WINDISP || vdisp) {
    FillRect(x, y + 2, 4, 4, White);
    DrawRect(x, y + 2, 4, 4, Black);

    if (vvar[0]) {
      DrawLine(x, y + 2, x + 4, y + 6, Black);
      DrawLine(x, y + 6, x + 4, y + 2, Black);
    }

    if (vtipo == WINDISP) {
      x += 6;
      locatexy(x,y);
      while (bstr[cc] != 0) {
        cchar = bstr[cc];
        cc++;

        writecxy(6, cchar, Black, White);
        x += 6;
      }
    }
  }
}


//-------------------------------------------------------------------------
void combobox(unsigned char* vopt, unsigned char *vvar,unsigned char x, unsigned char y, unsigned char vtipo) {
}

//-------------------------------------------------------------------------
void editor(unsigned char* vtexto, unsigned char *vvar,unsigned char x, unsigned char y, unsigned char vtipo) {
}
#endif