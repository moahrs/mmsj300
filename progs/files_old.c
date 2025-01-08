/********************************************************************************
*    Programa    : files.c
*    Objetivo    : File Explorer for MMSJOS com MGUI
*    Criado em   : 25/12/2024
*    Programador : Moacir Jr.
*--------------------------------------------------------------------------------
* Data        Versao  Responsavel  Motivo
* 25/12/2024  0.1     Moacir Jr.   Criacao Versao Beta
*--------------------------------------------------------------------------------*/

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "../mmsj300api.h"
#include "../mmsjos.h"
#include "mgui.h"
#include "files.h"

//-----------------------------------------------------------------------------
// Principal
//-----------------------------------------------------------------------------
void main(void)
{
    BYTE vcont, ix, iy, cc, dd, ee, cnum[20], *cfileptr, *cfilepos;
    BYTE ikk, vnomefile[32], vnomefilenew[15], avdm2, avdm, avdl, vopc, vresp;
    unsigned long vtotbytes = 0;
    BYTE vstring[64], vwb;

    showWindow("File Explorer v0.1\0", 0, 0, 255, 191, BTNONE);

    vcont = 1;
    *vpos = 0;
    *vposold = 0xFF;
    vnomefile[0] = 0x00;
    FillRect(0,18,255,10,*vcorwb);
    DrawRect(0,18,255,10,*vcorwf);
    writesxy(2,20,8,"Name     Ext Modify      Size   Atrib\0", *vcorwf, *vcorwb);
//    DrawLine(1,31,315,31,*vcorwf);

    carregaDir();

    while (vcont)
    {
        linhastatus(1);

        listaDir();

        if (*vposold == *vpos)
            listaDir();

        linhastatus(0);

        *vposty = 0;

        while (1)
        {
            VerifyMouse();

            if (*mouseBtnPres == 0x02)  // Direito
            {
                if (*vposty >= 34 && *vposty <= 170)
                {
                    ee = 99;
                    dd = 0;
                    while (ee == 99)
                    {
                        if (*vposty >= clinha[dd] && *vposty <= (clinha[dd] + 10) && clinha[dd] != 0)
                            ee = dd;

                        dd++;
                    }

                    if (ee != 99 )
                    {
                        InvertRect(2,((ee + 1) * 10),253,10);

                        // Abre menu : Delete, Rename, Close
                        SaveScreen(30,10,45,30);

                        FillRect(30,10,45,30,*vcorwb);
                        DrawRect(30,10,45,30,*vcorwf);
                        writesxy(33,12,8,"Delete",*vcorwf,*vcorwb);
                        writesxy(33,20,8,"Rename",*vcorwf,*vcorwb);
                        DrawLine(30,28,75,28,*vcorwf);
                        writesxy(33,30,8,"Close",*vcorwf,*vcorwb);

                        vopc = 99;

                        while (1)
                        {
                            VerifyMouse();

                            if (*mouseBtnPres == 0x01)  // Esquerdo
                            {
                                if (*vpostx >= 36 && *vpostx <= 70)
                                {
                                    if (*vposty >= 12 && *vposty <= 37)
                                        RestoreScreen(30,10,45,30);

                                    if (*vposty >= 12 && *vposty <= 19 )
                                    {
                                        vopc = 0;
                                        InvertRect(31,12,43,8);
                                        break;
                                    }
                                    else if (*vposty >= 20 && *vposty <= 27)
                                    {
                                        vopc = 1;
                                        InvertRect(31,20,43,8);
                                        break;
                                    }
                                    else if (*vposty >= 30 && *vposty <= 37)
                                    {
                                        vopc = 2;
                                        InvertRect(31,30,43,8);
                                        break;
                                    }
                                }
                            }
                        }

                        // Executa op��o selecionada
                        if (vopc == 0) {
                            // Deleta Arquivo
                            *tempData = ee;
                            vresp = message("Confirm\nDelete File ?\0",(BTYES | BTNO), 0);
                            if (vresp == BTYES) {
                                linhastatus(4);
                                cfileptr = cfile + (40 * ee);
                                writesxy(2,2,8,cfileptr,*vcorwf,*vcorwb);
                                strcpy(vnomefile,cfileptr);
                                cfileptr += 9;
                                writesxy(2,10,8,cfileptr,*vcorwf,*vcorwb);
                                if (*cfileptr != 0x00) {
                                    _strcat(vnomefile,vnomefile,".");
                                    _strcat(vnomefile,vnomefile,cfileptr);
                                }

                                writesxy(2,20,8,vnomefile,*vcorwf,*vcorwb);

                                if (fsDelFile(vnomefile) >= ERRO_D_START)
                                    message("Delete File Error.\0",(BTCLOSE), 0);
                                else
                                    carregaDir();
                            }
                            break;
                        }
                        else if (vopc == 1) {
                            // Renomeia Arquivo
                            linhastatus(1);

                            // Abre janela para pedir novo nome
                            vstring[0] = '\0';

                            SaveScreen(10,40,240,50);
                            showWindow("Rename",10,40,240,50,BTOK | BTCANCEL);

                            writesxy(12,55,8,"New Name:",*vcorwf,*vcorwb);
                            fillin(vstring, 120, 55, 130, WINDISP);

                            while (1)
                            {
                                fillin(vstring, 120, 55, 130, WINOPER);

                                vwb = waitButton();

                                if (vwb == BTOK || vwb == BTCANCEL)
                                    break;
                            }

                            RestoreScreen(10,40,240,50);

                            if (vwb == BTOK) {
                                strcpy(vnomefilenew, vstring);

                                vresp = message("Confirm\nRename File ?\0",(BTYES | BTNO), 0);
                                if (vresp == BTYES) {
                                    linhastatus(5);
                                    cfileptr = cfile + (40 * ee);
                                    strcpy(vnomefile,cfileptr);
                                    cfileptr += 9;
                                    if (*cfileptr != 0x00) {
                                        _strcat(vnomefile,vnomefile,".");
                                        _strcat(vnomefile,vnomefile,cfileptr);
                                    }

                                    if (fsRenameFile(vnomefile,vnomefilenew) >= ERRO_D_START)
                                        message("Rename File Error.\0",(BTCLOSE), 0);
                                    else
                                        carregaDir();
                                }
                            }

                            break;
                        }
                        else if (vopc == 2) {
                            break;
                        }
                    }
                }
            }
            else if (*mouseBtnPres == 0x01)  // Esquerdo
            {
                if (*vposty > 170) {
                    // Ultima Linha
                    if (*vpostx > 10 && *vpostx <= 25) {               // Flecha Esquerda
                        *vposold = *vpos;
                        if (*vpos < 14)
                            *vpos = 0;
                        else
                            *vpos = *vpos - 14;

                        break;
                    }
                    else if (*vpostx >= 25 && *vpostx <= 45) {         // Flecha Direita
                        *vposold = *vpos;
                        *vpos = *vpos + 14;
                        break;
                    }
                    else if (*vpostx >= 100 && *vpostx <= 120) {       // Search
                        break;
                    }
                    else if (*vpostx >= 200 && *vpostx <= 220) {       // Sair
                        linhastatus(7);
                        vcont = 0;
                        break;
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------
void linhastatus(BYTE vtipomsgs)
{
    FillRect(0,175,255,15,*vcorwb);
    DrawRect(0,175,255,15,*vcorwf);

    switch (vtipomsgs) {
        case 0:
            MostraIcone(10, 180, 5,*vcorwf, *vcorwb);   // Icone <
            MostraIcone(30, 180, 6,*vcorwf, *vcorwb);   // Icone >
            MostraIcone(107, 180, 7,*vcorwf, *vcorwb);  // Icone Search
            MostraIcone(207, 180, 4,*vcorwf, *vcorwb);  // Icone Exit
            break;
        case 1:
            writesxy(7,180,8,"wait...\0",*vcorwf,*vcorwb);
            break;
        case 2:
            writesxy(7,180,8,"processing...\0",*vcorwf,*vcorwb);
            break;
        case 3:
            writesxy(7,180,8,"file not found...\0",*vcorwf,*vcorwb);
            break;
        case 4:
            writesxy(7,180,8,"Deleting file...\0",*vcorwf,*vcorwb);
            break;
        case 5:
            writesxy(7,180,8,"Renaming file...\0",*vcorwf,*vcorwb);
            break;
        case 6:
            writesxy(7,180,8,"New file name exist...\0",*vcorwf,*vcorwb);
            break;
        case 7:
            writesxy(7,180,8,"Exiting...\0",*vcorwf,*vcorwb);
            break;
    }
}

//--------------------------------------------------------------------------
void carregaDir(void)
{
    BYTE vcont, ikk, ix, iy, cc, dd, ee, cnum[20], *cfileptr;
    BYTE vnomefile[32];
    BYTE sqtdtam[10], cuntam;
    unsigned long vtotbytes = 0, vqtdtam;

    // Leitura dos Arquivos
    cfileptr = cfile;
    *cfileptr = 0x00;

    TrocaSpriteMouse(MOUSE_HOURGLASS);

    // Logica de leitura Diretorio FAT32
    if (fsFindInDir(NULL, TYPE_FIRST_ENTRY) < ERRO_D_START) {
        while (1) {
			if (vdir->Attr != ATTR_VOLUME) {
                // Nome
                for (cc = 0; cc <= 7; cc++) {
                    if (vdir->Name[cc] >= 32) {
                        *cfileptr++ = vdir->Name[cc];
                     }
                    else
                        *cfileptr++ = ' ';
                }

                *cfileptr++ = '\0';

                // Extens�o
                for (cc = 8; cc <= 10; cc++) {
                    ikk = cc - 8;
                    if (vdir->Ext[ikk] >= 32) {
                        *cfileptr++ = vdir->Ext[ikk];
                    }
                    else
                        *cfileptr++ = ' ';
                }

                *cfileptr++ = '\0';

                // Data Ultima Modificacao
                // Mes
                vqtdtam = (vdir->UpdateDate & 0x01E0) >> 5;
                if (vqtdtam < 1 || vqtdtam > 12)
                    vqtdtam = 1;

                vqtdtam--;

                *cfileptr++ = vmesc[vqtdtam][0];
                *cfileptr++ = vmesc[vqtdtam][1];
                *cfileptr++ = vmesc[vqtdtam][2];
                *cfileptr++ = '/';

                // Dia
                vqtdtam = vdir->UpdateDate & 0x001F;
			    memset(sqtdtam, 0x0, 10);
                itoa(vqtdtam, sqtdtam, 10);

                if (vqtdtam < 10) {
                    *cfileptr++ = '0';
                    *cfileptr++ = sqtdtam[0];
                }
                else {
                    *cfileptr++ = sqtdtam[0];
                    *cfileptr++ = sqtdtam[1];
                }
                *cfileptr++ = '/';

                // Ano
                vqtdtam = ((vdir->UpdateDate & 0xFE00) >> 9) + 1980;
				memset(sqtdtam, 0x0, 10);
                itoa(vqtdtam, sqtdtam, 10);

                *cfileptr++ = sqtdtam[0];
                *cfileptr++ = sqtdtam[1];
                *cfileptr++ = sqtdtam[2];
                *cfileptr++ = sqtdtam[3];

                *cfileptr++ = '\0';

                // Tamanho
                if (vdir->Attr != ATTR_DIRECTORY) {
                    // Reduz o tamanho a unidade (GB, MB ou KB)
                    vqtdtam = vdir->Size;

                    if ((vqtdtam & 0xC0000000) != 0) {
                        cuntam = 'G';
                        vqtdtam = ((vqtdtam & 0xC0000000) >> 30) + 1;
                    }
                    else if ((vqtdtam & 0x3FF00000) != 0) {
                        cuntam = 'M';
                        vqtdtam = ((vqtdtam & 0x3FF00000) >> 20) + 1;
                    }
                    else if ((vqtdtam & 0x000FFC00) != 0) {
                        cuntam = 'K';
                        vqtdtam = ((vqtdtam & 0x000FFC00) >> 10) + 1;
                    }
                    else
                        cuntam = ' ';

                    // Transforma para decimal
					memset(sqtdtam, 0x0, 10);
                    itoa(vqtdtam, sqtdtam, 10);

                    // Primeira Parte da Linha do dir, tamanho
                    for(ix = 0; ix <= 3; ix++) {
                        if (sqtdtam[ix] == 0)
                            break;
                    }

                    iy = (4 - ix);

                    for(ix = 0; ix <= 3; ix++) {
                        if (iy <= ix) {
                            ikk = ix - iy;
                            *cfileptr++ = sqtdtam[ikk];
                        }
                        else
                            *cfileptr++ = ' ';
                    }

                    *cfileptr++ = cuntam;
                }
                else {
                    *cfileptr++ = ' ';
                    *cfileptr++ = ' ';
                    *cfileptr++ = ' ';
                    *cfileptr++ = ' ';
                    *cfileptr++ = '0';
                }

                *cfileptr++ = '\0';

                // Atributos
                if (vdir->Attr == ATTR_DIRECTORY) {
                    *cfileptr++ = '<';
                    *cfileptr++ = 'D';
                    *cfileptr++ = 'I';
                    *cfileptr++ = 'R';
                    *cfileptr++ = '>';
                }
                else {
                    *cfileptr++ = ' ';
                    *cfileptr++ = ' ';
                    *cfileptr++ = ' ';
                    *cfileptr++ = ' ';
                    *cfileptr++ = ' ';
                }

                *cfileptr++ = '\0';

                cfileptr += 3;    // para fechar 40 pos

                *cfileptr = 0x00;
            }

            // Verifica se tem mais Arquivos
			for (ix = 0; ix <= 7; ix++) {
			    vnomefile[ix] = vdir->Name[ix];
				if (vnomefile[ix] == 0x20) {
					vnomefile[ix] = '\0';
					break;
			    }
			}

			vnomefile[ix] = '\0';

			if (vdir->Name[0] != '.') {
			    vnomefile[ix] = '.';
			    ix++;
				for (iy = 0; iy <= 2; iy++) {
				    vnomefile[ix] = vdir->Ext[iy];
					if (vnomefile[ix] == 0x20) {
						vnomefile[ix] = '\0';
						break;
				    }
				    ix++;
				}
				vnomefile[ix] = '\0';
			}

			if (fsFindInDir(vnomefile, TYPE_NEXT_ENTRY) >= ERRO_D_START)
				break;
        }
    }

    TrocaSpriteMouse(MOUSE_POINTER);
}

//--------------------------------------------------------------------------
void listaDir(void)
{
    WORD pposx, pposy, vretfs, dd, ww;
    BYTE *cfilepos, ee;

    TrocaSpriteMouse(MOUSE_HOURGLASS);

    for (dd = 0; dd <= 13; dd++)
        clinha[dd] = 0x00;

    pposy = 34;
    dd = *vpos;
    ee = 14;

    cfilepos = cfile + (40 * dd);

    while(*cfilepos)
    {
        // Nome
        pposx = 5;
        writesxy(pposx,pposy,6,cfilepos,*vcorwf,*vcorwb);

        // Ext
        pposx = 55;
        cfilepos += 9;
        writesxy(pposx,pposy,6,cfilepos,*vcorwf,*vcorwb);

        // Modif
        pposx = 75;
        cfilepos += 4;
        writesxy(pposx,pposy,6,cfilepos,*vcorwf,*vcorwb);

        // Tamanho
        pposx = 145;
        cfilepos += 12;
        writesxy(pposx,pposy,6,cfilepos,*vcorwf,*vcorwb);

        // Atrib
        pposx = 210;
        cfilepos += 6;
        writesxy(pposx,pposy,6,cfilepos,*vcorwf,*vcorwb);

        clinha[dd] = pposy;
        pposy += 10;
        dd++;
        ee--;

        if (ee == 0)
            break;

        cfilepos = cfile + (40 * dd);
    }

    if (dd == *vpos)
        *vpos = *vposold;

    if (ee > 0) {
        dd = 14 - ee;
        dd = dd * 10;
        dd = dd + 34;
        ww = ee * 10;
        FillRect(5,dd,249,ww,*vcorwb);
    }

    TrocaSpriteMouse(MOUSE_POINTER);
}

//--------------------------------------------------------------------------
void SearchFile(void)
{
}

//-----------------------------------------------------------------------------
BYTE * _strcat (BYTE * dst, BYTE * cp, BYTE * src) {
    while( *cp )
        *dst++ = *cp++;     /* copy to dst and find end of dst */

    while( *src )
        *dst++ = *src++;       /* Copy src to end of dst */

    *dst++ = 0x00;

    return( dst );                  /* return dst */
}