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

    // Cria a Janela
    showWindow("File Explorer v0.1\0", 0, 0, 255, 191, BTNONE);

    vcont = 1;
    *vpos = 0;
    *vposold = 0xFF;
    vnomefile[0] = 0x00;

    // Prepara Cabeçalho
    FillRect(0,18,255,10,*vcorwb);
    DrawRect(0,18,255,10,*vcorwf);
    writesxy(16,20,8,"Name\0", *vcorwf, *vcorwb);
    writesxy(66,20,8,"Ext\0", *vcorwf, *vcorwb);
    writesxy(90,20,8,"Modify\0", *vcorwf, *vcorwb);
    writesxy(165,20,8,"Size\0", *vcorwf, *vcorwb);
    writesxy(200,20,8,"Atrib\0", *vcorwf, *vcorwb);

    // Carrega Diretorio
    carregaDir();
    
    // Lista Diretorio
    listaDir();

    // Loop Principal
    while (vcont)
    {
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

                        if (dd > 13)
                            break;
                    }

                    if (ee != 99 )
                    {
                        MostraIcone(8,clinha[ee],6,VDP_DARK_GREEN,*vcorwb);

                        // Abre menu : Delete, Rename, Close
                        SaveScreen(30,10,50,35);

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
                                        RestoreScreen(30,10,50,35);

                                    if (*vposty >= 12 && *vposty <= 19 )
                                    {
                                        vopc = 0;
                                        break;
                                    }
                                    else if (*vposty >= 20 && *vposty <= 27)
                                    {
                                        vopc = 1;
                                        break;
                                    }
                                    else if (*vposty >= 30 && *vposty <= 37)
                                    {
                                        vopc = 2;
                                        break;
                                    }
                                }
                            }
                        }

                        // Executa op��o selecionada
                        if (vopc == 0)
                        {
                            // Deleta Arquivo
                            vresp = message("Confirm\nDelete File ?\0",(BTYES | BTNO), 0);
                            if (vresp == BTYES)
                            {
                                strcpy(vnomefile,dfile->dir[ee].Name);
                                if (dfile->dir[ee].Ext[0] != 0x00)
                                {
                                    strcat(vnomefile,".");
                                    strcat(vnomefile,dfile->dir[ee].Ext);
                                }

                                linhastatus(4, vnomefile);

                                if (fsDelFile(vnomefile) >= ERRO_D_START)
                                    message("Delete File Error.\0",(BTCLOSE), 0);
                                else
                                {
                                    carregaDir();
                                    listaDir();
                                }
                            }

                            FillRect(8,clinha[ee],8,8,*vcorwb);

                            break;
                        }
                        else if (vopc == 1)
                        {
                            // Renomeia Arquivo
                            linhastatus(1, "\0");

                            // Abre janela para pedir novo nome
                            vstring[0] = '\0';

                            SaveScreen(10,40,240,60);
                            showWindow("Rename",10,40,240,50,BTOK | BTCANCEL);

                            writesxy(12,57,8,"New Name:",*vcorwf,*vcorwb);
                            fillin(vstring, 72, 57, 130, WINDISP);

                            while (1)
                            {
                                fillin(vstring, 72, 57, 130, WINOPER);

                                vwb = waitButton();

                                if (vwb == BTOK || vwb == BTCANCEL)
                                    break;
                            }

                            RestoreScreen(10,40,240,60);

                            if (vwb == BTOK) {
                                ix = 0;
                                while(vstring[ix])
                                {
                                    vnomefilenew[ix] = toupper(vstring[ix]);
                                    ix++;
                                }                                       
                                
                                vstring[ix] = 0x00;

                                vresp = message("Confirm\nRename File ?\0",(BTYES | BTNO), 0);
                                if (vresp == BTYES)
                                {
                                    strcpy(vnomefile,dfile->dir[ee].Name);
                                    if (dfile->dir[ee].Ext[0] != 0x00)
                                    {
                                        strcat(vnomefile,".");
                                        strcat(vnomefile,dfile->dir[ee].Ext);
                                    }

                                    linhastatus(5, vnomefile);

                                    if (fsRenameFile(vnomefile,vnomefilenew) >= ERRO_D_START)
                                        message("Rename File Error.\0",(BTCLOSE), 0);
                                    else
                                    {
                                        carregaDir();
                                        listaDir();
                                    }
                                }
                            }

                            linhastatus(0, "\0");

                            FillRect(8,clinha[ee],8,8,*vcorwb);

                            break;
                        }
                        else if (vopc == 2)
                        {
                            FillRect(8,clinha[ee],8,8,*vcorwb);

                            break;
                        }
                    }
                }
            }
            else if (*mouseBtnPres == 0x01)  // Esquerdo
            {
                if (*vposty > 170) {
                    // Ultima Linha
                    if (*vpostx > 5 && *vpostx <= 20) {               // Flecha Esquerda
                        *vposold = *vpos;
                        if (*vpos < 14)
                            *vpos = 0;
                        else
                            *vpos = *vpos - 14;

                        listaDir();

                        break;
                    }
                    else if (*vpostx >= 25 && *vpostx <= 40) {         // Flecha Direita
                        *vposold = *vpos;
                        *vpos = *vpos + 14;

                        listaDir();

                        break;
                    }
                    else if (*vpostx >= 100 && *vpostx <= 120) {       // Search
                        break;
                    }
                    else if (*vpostx >= 200 && *vpostx <= 220) {       // Sair
                        linhastatus(7,"\0");
                        vcont = 0;
                        break;
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------
void linhastatus(BYTE vtipomsgs, BYTE * vmsgs)
{
    FillRect(2,176,252,13,*vcorwb);
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

    if (*vmsgs)
        writesxy(151,180,8,vmsgs,*vcorwf,*vcorwb);
}

//--------------------------------------------------------------------------
void carregaDir(void)
{
    BYTE vcont, ikk, ix, iy, cc, dd, ee, cnum[20];
    BYTE vnomefile[32], dsize;
    BYTE sqtdtam[10], cuntam;
    unsigned long vtotbytes = 0, vqtdtam;
    FILES_DIR ddir;

    // Leitura dos Arquivos
    *dFileCursor = 0;
    dsize = sizeof(FILES_DIR);
    dfile->pos = 0;

    TrocaSpriteMouse(MOUSE_HOURGLASS);

    // Logica de leitura Diretorio FAT32
    if (fsFindInDir(NULL, TYPE_FIRST_ENTRY) < ERRO_D_START)
    {
        while (1)
        {
			if (vdir->Attr != ATTR_VOLUME)
            {
                // Nome
                for (cc = 0; cc <= 7; cc++)
                {
                    if (vdir->Name[cc] > 32)
                        ddir.Name[cc] = vdir->Name[cc];
                    else
                        ddir.Name[cc] = '\0';
                }

                ddir.Name[8] = '\0';

                // Extens�o
                for (cc = 0; cc <= 2; cc++)
                {
                    if (vdir->Ext[cc] > 32)
                        ddir.Ext[cc] = vdir->Ext[cc];
                    else
                        ddir.Ext[cc] = '\0';
                }

                ddir.Ext[3] = '\0';

                // Data Ultima Modificacao
                // Mes
                vqtdtam = (vdir->UpdateDate & 0x01E0) >> 5;
                if (vqtdtam < 1 || vqtdtam > 12)
                    vqtdtam = 1;

                vqtdtam--;

                ddir.Modify[0] = vmesc[vqtdtam][0];
                ddir.Modify[1] = vmesc[vqtdtam][1];
                ddir.Modify[2] = vmesc[vqtdtam][2];
                ddir.Modify[3] = '/';

                // Dia
                vqtdtam = vdir->UpdateDate & 0x001F;
			    memset(sqtdtam, 0x0, 10);
                itoa(vqtdtam, sqtdtam, 10);

                if (vqtdtam < 10) {
                    ddir.Modify[4] = '0';
                    ddir.Modify[5] = sqtdtam[0];
                }
                else {
                    ddir.Modify[4] = sqtdtam[0];
                    ddir.Modify[5] = sqtdtam[1];
                }
                ddir.Modify[6] = '/';

                // Ano
                vqtdtam = ((vdir->UpdateDate & 0xFE00) >> 9) + 1980;
				memset(sqtdtam, 0x0, 10);
                itoa(vqtdtam, sqtdtam, 10);

                ddir.Modify[7] = sqtdtam[0];
                ddir.Modify[8] = sqtdtam[1];
                ddir.Modify[9] = sqtdtam[2];
                ddir.Modify[10] = sqtdtam[3];

                ddir.Modify[11] = '\0';

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
                            ddir.Size[ix] = sqtdtam[ikk];
                        }
                        else
                            ddir.Size[ix] = ' ';
                    }

                    ddir.Size[ix] = cuntam;
                }
                else {
                    ddir.Size[0] = ' ';
                    ddir.Size[1] = ' ';
                    ddir.Size[2] = ' ';
                    ddir.Size[3] = ' ';
                    ddir.Size[4] = '0';
                }

                ddir.Size[5] = '\0';

                // Atributos
                if (vdir->Attr == ATTR_DIRECTORY) {
                    ddir.Attr[0] = '<';
                    ddir.Attr[1] = 'D';
                    ddir.Attr[2] = 'I';
                    ddir.Attr[3] = 'R';
                    ddir.Attr[4] = '>';
                }
                else {
                    ddir.Attr[0] = ' ';
                    ddir.Attr[1] = ' ';
                    ddir.Attr[2] = ' ';
                    ddir.Attr[3] = ' ';
                    ddir.Attr[4] = ' ';
                }

                ddir.Attr[5] = '\0';

                dfile->dir[*dFileCursor] = ddir;
                dfile->pos = *dFileCursor;
                *dFileCursor = *dFileCursor + 1;
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
    WORD pposy, vretfs, dd, ww;
    BYTE ee, cc,ix, cstring[10];

    linhastatus(1, "\0");

    TrocaSpriteMouse(MOUSE_HOURGLASS);

    for (dd = 0; dd <= 13; dd++)
        clinha[dd] = 0x00;

    pposy = 34;
    dd = *vpos;

    if (dd < 0)
        dd = 0;

    if (dd >= *dFileCursor)
        dd = (*dFileCursor - 1);

    ee = 14;
    cc = 0;

    while(1)
    {
        for (ix = 0; ix < 8; ix++)
        {
            if (dfile->dir[dd].Name[ix] == 0x00)
                cstring[ix] = 0x20;
            else
                cstring[ix] = dfile->dir[dd].Name[ix];
        }
        cstring[8] = '\0';

        // Nome
        writesxy(16,pposy,6,cstring,*vcorwf,*vcorwb2);

        for (ix = 0; ix < 3; ix++)
        {
            if (dfile->dir[dd].Ext[ix] == 0x00)
                cstring[ix] = 0x20;
            else
                cstring[ix] = dfile->dir[dd].Ext[ix];
        }
        cstring[3] = '\0';

        // Ext
        writesxy(66,pposy,6,cstring,*vcorwf,*vcorwb2);

        // Modif
        writesxy(90,pposy,6,dfile->dir[dd].Modify,*vcorwf,*vcorwb2);

        // Tamanho
        writesxy(165,pposy,6,dfile->dir[dd].Size,*vcorwf,*vcorwb2);

        // Atrib
        writesxy(200,pposy,6,dfile->dir[dd].Attr,*vcorwf,*vcorwb2);

        clinha[cc] = pposy;
        pposy += 10;
        dd++;
        cc++;
        ee--;

        if (dd == *dFileCursor)
            break;

        if (ee == 0)
            break;
    }

    if (ee > 0) {
        dd = 14 - ee;
        dd = dd * 10;
        dd = dd + 34;
        ww = ee * 10;
        FillRect(5,dd,249,ww,*vcorwb);
    }

    TrocaSpriteMouse(MOUSE_POINTER);

    linhastatus(0, "\0");
}

//--------------------------------------------------------------------------
void SearchFile(void)
{
}
