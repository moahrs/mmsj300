/********************************************************************************
*    Programa    : basic.c
*    Objetivo    : MMSJ-Basic para o MMSJ300
*    Criado em   : 10/10/2022
*    Programador : Moacir Jr.
*--------------------------------------------------------------------------------
* Data        Versao  Responsavel  Motivo
* 10/10/2022  0.1     Moacir Jr.   Criacao Versao Beta
* 26/06/2023  0.4     Moacir Jr.   Simplificacoes e ajustres
* 27/06/2023  0.4a    Moacir Jr.   Adaptar processos de for-next e if-then-else
* 01/07/2023  0.4b    Moacir Jr.   Ajuste de Bugs
* 03/07/2023  0.5     Moacir Jr.   Colocar Logica Ponto Flutuante
* 10/07/2023  0.5a    Moacir Jr.   Colocar Funcoes Graficas
* 11/07/2023  0.5b    Moacir Jr.   Colocar DATA-READ
* 20/07/2023  1.0     Moacir Jr.   Versao para publicacao
* 21/07/2023  1.0a    Moacir Jr.   Ajustes de memoria e bugs
* 23/07/2023  1.0b    Moacir Jr.   Ajustes bugs no for...next e if...then
* 24/07/2023  1.0c    Moacir Jr.   Retirada "BYE" message. Ajustes de bugs no gosub...return
* 25/07/2023  1.0d    Moacir Jr.   Ajuste no basInputGet, quando Get, mandar 1 pro inputLine e sem manipulacoa cursor
* 20/01/2024  1.0e    Moacir Jr.   Colocar para iniciar direto no Basic
*--------------------------------------------------------------------------------
* Variables Simples: start at 00800000
*   --------------------------------------------------------
*   Type ($ = String, # = Real, % = Integer)
*   Name (2 Bytes, 1st and 2nd letters of the name)
*   --------------- --------------- ------------------------
*   Integer         Real            String
*   --------------- --------------- ------------------------
*   0x00            0x00            Length
*   Value MSB       Value MSB       Pointer to String (High)
*   Value           Value           Pointer to String
*   Value           Value           Pointer to String
*   Value LSB       Value LSB       Pointer to String (Low)
*   --------------- --------------- ------------------------
*   Total: 8 Bytes
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
#include "mmsj300api.h"
#include "monitor.h"
#include "basic.h"

#define versionBasic "1.0e"
//#define __TESTE_TOKENIZE__ 1
//#define __DEBUG_ARRAYS__ 1

//-----------------------------------------------------------------------------
// Principal
//-----------------------------------------------------------------------------
void main(void)
{
    unsigned char vRetInput;

    // Timer para o Random
    *(vmfp + Reg_TADR) = 0xF5;  // 245
    *(vmfp + Reg_TACR) = 0x02;  // prescaler de 10. total 2,4576Mhz/10*245 = 1003KHz

    if (!*startBasic)
        clearScr();

    printText("MMSJ-BASIC v"versionBasic);
    printText("\r\n\0");

    if (!*startBasic)
        printText("Utility (c) 2022-2024\r\n\0");

    printText("OK\r\n\0");

    *vBufReceived = 0x00;
    *vbuf = '\0';
    *pProcess = 0x01;
    *pTypeLine = 0x00;
    *nextAddrLine = pStartProg;
    *firstLineNumber = 0;
    *addrFirstLineNumber = 0;
    *traceOn = 0;
    *lastHgrX = 0;
    *lastHgrY = 0;
    *fgcolorAnt = *fgcolor;
    *bgcolorAnt = *bgcolor;

    while (*pProcess)
    {
        vRetInput = inputLine(128,'$');

        if (*vbuf != 0x00 && (vRetInput == 0x0D || vRetInput == 0x0A))
        {
            printText("\r\n\0");

            processLine();

            if (!*pTypeLine && *pProcess)
                printText("\r\nOK\0");

            *vBufReceived = 0x00;
            *vbuf = '\0';

            if (!*pTypeLine && *pProcess)
                printText("\r\n\0");   // printText("\r\n>\0");
        }
        else if (vRetInput != 0x1B)
        {
            printText("\r\n\0");
        }
    }

    printText("\r\n\0");
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void processLine(void)
{
    unsigned char linhacomando[32], vloop, vToken;
    unsigned char *blin = vbuf;
    unsigned short varg = 0;
    unsigned short ix, iy, iz, ikk, kt;
    unsigned short vbytepic = 0, vrecfim;
    unsigned char cuntam, vLinhaArg[255], vparam2[16], vpicret;
    char vSpace = 0;
    int vReta;
    typeInf vRetInf;
    unsigned short vTam = 0;
    unsigned char *pSave = *nextAddrLine;
    unsigned long vNextAddr = 0;
    unsigned char vTimer;
    unsigned char vBuffer[20];
    unsigned char *vTempPointer;
    unsigned char sqtdtam[20];

    // Separar linha entre comando e argumento
    linhacomando[0] = '\0';
    vLinhaArg[0] = '\0';
    ix = 0;
    iy = 0;
    while (*blin)
    {
        if (!varg && *blin >= 0x20 && *blin <= 0x2F)
        {
            varg = 0x01;
            linhacomando[ix] = '\0';
            iy = ix;
            ix = 0;

            if (*blin != 0x20)
                vLinhaArg[ix++] = *blin;
            else
                vSpace = 1;
        }
        else
        {
            if (!varg)
                linhacomando[ix] = *blin;
            else
                vLinhaArg[ix] = *blin;
            ix++;
        }

        *blin++;
    }

    if (!varg)
    {
        linhacomando[ix] = '\0';
        iy = ix;
    }
    else
        vLinhaArg[ix] = '\0';

    vpicret = 0;

    // Processar e definir o que fazer
    if (linhacomando[0] != 0)
    {
        // Se for numero o inicio da linha, eh entrada de programa, senao eh comando direto
        if (linhacomando[0] >= 0x31 && linhacomando[0] <= 0x39) // 0 nao é um numero de linha valida
        {
            *pTypeLine = 0x01;

            // Entrada de programa
            tokenizeLine(vLinhaArg);
            saveLine(linhacomando, vLinhaArg);
        }
        else
        {
            *pTypeLine = 0x00;

            for (iz = 0; iz < iy; iz++)
                linhacomando[iz] = toupper(linhacomando[iz]);

            // Comando Direto
            if (!strcmp(linhacomando,"HOME") && iy == 4)
            {
                clearScr();
            }
            else if (!strcmp(linhacomando,"NEW") && iy == 3)
            {
                *pStartProg = 0x00;
                *(pStartProg + 1) = 0x00;
                *(pStartProg + 2) = 0x00;

                *nextAddrLine = pStartProg;
                *firstLineNumber = 0;
                *addrFirstLineNumber = 0;
            }
            else if (!strcmp(linhacomando,"EDIT") && iy == 4)
            {
                editLine(vLinhaArg);
            }
            else if (!strcmp(linhacomando,"LIST") && iy == 4)
            {
                listProg(vLinhaArg, 0);
            }
            else if (!strcmp(linhacomando,"LISTP") && iy == 5)
            {
                listProg(vLinhaArg, 1);
            }
            else if (!strcmp(linhacomando,"RUN") && iy == 3)
            {
                runProg(vLinhaArg);
            }
            else if (!strcmp(linhacomando,"DEL") && iy == 3)
            {
                delLine(vLinhaArg);
            }
            else if (!strcmp(linhacomando,"XLOAD") && iy == 5)
            {
                basXBasLoad();
            }
            else if (!strcmp(linhacomando,"TIMER") && iy == 5)
            {
                // Ler contador A do 68901
                vTimer = *(vmfp + Reg_TADR);

                // Devolve pra tela
                itoa(vTimer,vBuffer,10);
                printText("Timer: ");
                printText(vBuffer);
                printText("ms\r\n\0");
            }
            else if (!strcmp(linhacomando,"TRACE") && iy == 5)
            {
                *traceOn = 1;
            }
            else if (!strcmp(linhacomando,"NOTRACE") && iy == 7)
            {
                *traceOn = 0;
            }
            // *************************************************
            // ESSE COMANDO NAO VAI EXISTIR QUANDO FOR PRA BIOS
            // *************************************************
            else if (!strcmp(linhacomando,"QUIT") && iy == 4)
            {
                *pProcess = 0x00;
            }
            // *************************************************
            // *************************************************
            // *************************************************
            else
            {
                // Tokeniza a linha toda
                strcpy(vRetInf.tString, linhacomando);

                if (vSpace)
                    strcat(vRetInf.tString, " ");

                strcat(vRetInf.tString, vLinhaArg);

                tokenizeLine(vRetInf.tString);

                strcpy(vLinhaArg, vRetInf.tString);

                // Salva a linha pra ser interpretada
                vTam = strlen(vLinhaArg);
                vNextAddr = comandLineTokenized + (vTam + 6);

                *comandLineTokenized = ((vNextAddr & 0xFF0000) >> 16);
                *(comandLineTokenized + 1) = ((vNextAddr & 0xFF00) >> 8);
                *(comandLineTokenized + 2) =  (vNextAddr & 0xFF);

                // Grava numero da linha
                *(comandLineTokenized + 3) = 0xFF;
                *(comandLineTokenized + 4) = 0xFF;

                // Grava linha tokenizada
                for(kt = 0; kt < vTam; kt++)
                    *(comandLineTokenized + (kt + 5)) = vLinhaArg[kt];

                // Grava final linha 0x00
                *(comandLineTokenized + (vTam + 5)) = 0x00;
                *(comandLineTokenized + (vTam + 6)) = 0x00;
                *(comandLineTokenized + (vTam + 7)) = 0x00;
                *(comandLineTokenized + (vTam + 8)) = 0x00;

                *nextAddrSimpVar = pStartSimpVar;
                *nextAddrArrayVar = pStartArrayVar;
                *nextAddrString = pStartString;
                *vMaisTokens = 0;
                *vParenteses = 0x00;
                *vTemIf = 0x00;
                *vTemThen = 0;
                *vTemElse = 0;
                *vTemIfAndOr = 0x00;

                *pointerRunProg = comandLineTokenized + 5;

                vRetInf.tString[0] = 0x00;
                *ftos=0;
                *gtos=0;
                *vErroProc = 0;
                *randSeed = *(vmfp + Reg_TADR);
                do
                {
                    *doisPontos = 0;
                    *vInicioSentenca = 1;
                    vTempPointer = *pointerRunProg;
                    *pointerRunProg = *pointerRunProg + 1;
                    vReta = executeToken(*vTempPointer);
                } while (*doisPontos);

#ifndef __TESTE_TOKENIZE__
                if (*vdp_mode != VDP_MODE_TEXT)
                    basText();
#endif
                if (*vErroProc)
                {
                    showErrorMessage(*vErroProc, 0);
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------
// Transforma linha em tokens, se existirem
//-----------------------------------------------------------------------------
void tokenizeLine(unsigned char *pTokenized)
{
    unsigned char vLido[255], vLidoCaps[255], vAspas, vAchou = 0;
    unsigned char *blin = pTokenized;
    unsigned short ix, iy, kt, iz, iw;
    unsigned char vToken, vLinhaArg[255], vparam2[16], vpicret;
    char vbuffer [sizeof(long)*8+1];
    char vFirstComp = 0;
    char isToken;
    char vTemRem = 0;
//    unsigned char sqtdtam[20];

    // Separar linha entre comando e argumento
    vLinhaArg[0] = '\0';
    vLido[0]  = '\0';
    ix = 0;
    iy = 0;
    vAspas = 0;

    while (1)
    {
        vLido[ix] = '\0';

        if (*blin == 0x22)
            vAspas = !vAspas;

        // Se for quebrador sequencia, verifica se é um token
        if ((!vTemRem && !vAspas && strchr(" ;,+-<>()/*^=:",*blin)) || !*blin)
        {
            // Montar comparacoes "<>", ">=" e "<="
            if (((*blin == 0x3C || *blin == 0x3E) && (!vFirstComp && (*(blin + 1) == 0x3E || *(blin + 1) == 0x3D))) || (vFirstComp && *blin == 0x3D) || (vFirstComp && *blin == 0x3E))
            {
                if (!vFirstComp)
                {
                    for(kt = 0; kt < ix; kt++)
                        vLinhaArg[iy++] = vLido[kt];
                    vLido[0] = 0x00;
                    ix = 0;
                    vFirstComp = 1;
                }

                vLido[ix++] = *blin;

                if (ix < 2)
                {
                    blin++;

                    continue;
                }

                vFirstComp = 0;
            }

            if (vLido[0])
            {
                vToken = 0;
/*writeLongSerial("Aqui 332.666.2-[");
itoa(ix,sqtdtam,10);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(*blin,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");*/

                if (ix > 1)
                {
                    // Transforma em Caps pra comparar com os tokens
                    for (kt = 0; kt < ix; kt++)
                        vLidoCaps[kt] = toupper(vLido[kt]);

                    vLidoCaps[ix] = 0x00;

                    iz = strlen(vLidoCaps);

					// Compara pra ver se é um token
					for(kt = 0; kt < keywords_count; kt++)
					{
						iw = strlen(keywords[kt].keyword);

                        if (iw == 2 && iz == iw)
                        {
                            if (vLidoCaps[0] == keywords[kt].keyword[0] && vLidoCaps[1] == keywords[kt].keyword[1])
                            {
                                vToken = keywords[kt].token;
                                break;
                            }
                        }
                        else if (iz==iw)
                        {
                            if (strncmp(vLidoCaps, keywords[kt].keyword, iw) == 0)
                            {
                                vToken = keywords[kt].token;
                                break;
                            }
                        }
					}
                }

                if (vToken)
                {
                    if (vToken == 0x8C) // REM
                        vTemRem = 1;

                    vLinhaArg[iy++] = vToken;

                    //if (*blin == 0x28 || *blin == 0x29)
                    //    vLinhaArg[iy++] = *blin;

                    //if (*blin == 0x3A)  // :
                    if (*blin && *blin != 0x20 && vToken < 0xF0 && !vTemRem)
                        vLinhaArg[iy++] = toupper(*blin);
                }
                else
                {
                    for(kt = 0; kt < ix; kt++)
                        vLinhaArg[iy++] = vLido[kt];

                    if (*blin && *blin != 0x20)
                        vLinhaArg[iy++] = toupper(*blin);
                }
            }
            else
            {
                if (*blin && *blin != 0x20)
                    vLinhaArg[iy++] = toupper(*blin);
            }

            if (!*blin)
                break;

            vLido[0] = '\0';
            ix = 0;
        }
        else
        {
            if (!vAspas && !vTemRem)
                vLido[ix++] = toupper(*blin);
            else
                vLido[ix++] = *blin;
        }

        blin++;
    }

    vLinhaArg[iy] = 0x00;

    for(kt = 0; kt < iy; kt++)
        pTokenized[kt] = vLinhaArg[kt];

    pTokenized[iy] = 0x00;
}

//-----------------------------------------------------------------------------
// Salva a linha no formato:
// NN NN NN LL LL xxxxxxxxxxxx 00
// onde:
//      NN NN NN         = endereco da proxima linha
//      LL LL            = Numero da linha
//      xxxxxxxxxxxxxx   = Linha Tokenizada
//      00               = Indica fim da linha
//-----------------------------------------------------------------------------
void saveLine(unsigned char *pNumber, unsigned char *pTokenized)
{
    unsigned short vTam = 0, kt;
    unsigned char *pSave = *nextAddrLine;
    unsigned long vNextAddr = 0, vAntAddr = 0, vNextAddr2 = 0;
    unsigned short vNumLin = 0;
    unsigned char *pAtu = *nextAddrLine, *pLast = *nextAddrLine;

    vNumLin = atoi(pNumber);

    if (*firstLineNumber == 0)
    {
        *firstLineNumber = vNumLin;
        *addrFirstLineNumber = pStartProg;
    }
    else
    {
        vNextAddr = findNumberLine(vNumLin, 0, 0);

        if (vNextAddr > 0)
        {
            pAtu = vNextAddr;

            if (((*(pAtu + 3) << 8) | *(pAtu + 4)) == vNumLin)
            {
                printText("Line number already exists\r\n\0");
                return;
            }

            vAntAddr = findNumberLine(vNumLin, 1, 0);
        }
    }

    vTam = strlen(pTokenized);
    if (vTam)
    {
        // Calcula nova posicao da proxima linha
        if (vNextAddr == 0)
        {
            *nextAddrLine += (vTam + 6);
            vNextAddr = *nextAddrLine;

            *addrLastLineNumber = pSave;
        }
        else
        {
            if (*firstLineNumber > vNumLin)
            {
                *firstLineNumber = vNumLin;
                *addrFirstLineNumber = *nextAddrLine;
            }

            *nextAddrLine += (vTam + 6);
            vNextAddr2 = *nextAddrLine;

            if (vAntAddr != vNextAddr)
            {
                pLast = vAntAddr;
                vAntAddr = pSave;
                *pLast       = ((vAntAddr & 0xFF0000) >> 16);
                *(pLast + 1) = ((vAntAddr & 0xFF00) >> 8);
                *(pLast + 2) =  (vAntAddr & 0xFF);
            }

            pLast = *addrLastLineNumber;
            *pLast       = ((vNextAddr2 & 0xFF0000) >> 16);
            *(pLast + 1) = ((vNextAddr2 & 0xFF00) >> 8);
            *(pLast + 2) =  (vNextAddr2 & 0xFF);
        }

        pAtu = *nextAddrLine;
        *pAtu       = 0x00;
        *(pAtu + 1) = 0x00;
        *(pAtu + 2) = 0x00;
        *(pAtu + 3) = 0x00;
        *(pAtu + 4) = 0x00;

        // Grava endereco proxima linha
        *pSave++ = ((vNextAddr & 0xFF0000) >> 16);
        *pSave++ = ((vNextAddr & 0xFF00) >> 8);
        *pSave++ =  (vNextAddr & 0xFF);

        // Grava numero da linha
        *pSave++ = ((vNumLin & 0xFF00) >> 8);
        *pSave++ = (vNumLin & 0xFF);

        // Grava linha tokenizada
        for(kt = 0; kt < vTam; kt++)
            *pSave++ = *pTokenized++;

        // Grava final linha 0x00
        *pSave = 0x00;
    }
}

//-----------------------------------------------------------------------------
// Sintaxe:
//      LIST                : lista tudo
//      LIST <num>          : lista só a linha <num>
//      LIST <num>-         : lista a partir da linha <num>
//      LIST <numA>-<numB>  : lista o intervalo de <numA> até <numB>, inclusive
//
//      LISTP : mesmo que LIST, mas com pausa a cada scroll
//-----------------------------------------------------------------------------
void listProg(unsigned char *pArg, unsigned short pPause)
{
    // Default listar tudo
    unsigned short pIni = 0, pFim = 0xFFFF;
    unsigned char *vStartList = pStartProg;
    unsigned long vNextList;
    unsigned short vNumLin;
    char sNumLin [sizeof(short)*8+1], vFirstByte;
    unsigned char vtec;
    unsigned char vLinhaList[255], sNumPar[10], vToken;
    int iw, ix, iy, iz, vPauseRowCounter;
    unsigned char sqtdtam[20];

    if (pArg[0] != 0x00 && strchr(pArg,'-') != 0x00)
    {
        ix = 0;
        iy = 0;

        // listar intervalo
        while (pArg[ix] != '-')
            sNumPar[iy++] = pArg[ix++];

        sNumPar[iy] = 0x00;

        pIni = atoi(sNumPar);

        iy = 0;
        ix++;

        while (pArg[ix])
            sNumPar[iy++] = pArg[ix++];

        sNumPar[iy] = 0x00;

        if (sNumPar[0])
            pFim = atoi(sNumPar);
        else
            pFim = 0xFFFF;
    }
    else if (pArg[0] != 0x00)
    {
        // listar 1 linha
        pIni = atoi(pArg);
        pFim = pIni;
    }

    vStartList = findNumberLine(pIni, 0, 0);

    // Nao achou numero de linha inicial
    if (!vStartList)
    {
        printText("Non-existent line number\r\n\0");
        return;
    }

    vNextList = vStartList;
    vPauseRowCounter = 0;

    while (1)
    {
        // Guarda proxima posicao
        vNextList = (*(vStartList) << 16) | (*(vStartList + 1) << 8) | *(vStartList + 2);

        if (vNextList)
        {
            // Pega numero da linha
            vNumLin = (*(vStartList + 3) << 8) | *(vStartList + 4);

            if (vNumLin > pFim)
                break;

            vStartList += 5;
            ix = 0;

            // Coloca numero da linha na listagem
            itoa(vNumLin, sNumLin, 10);
            iz = 0;

            while (sNumLin[iz])
            {
                vLinhaList[ix++] = sNumLin[iz++];
            }

            vLinhaList[ix++] = 0x20;
            vFirstByte = 1;

            // Pega caracter a caracter da linha
            while (*vStartList)
            {
                vToken = *vStartList++;

                // Verifica se é token, se for, muda pra escrito
                if (vToken >= 0x80)
                {
                    // Procura token na lista
                    iy = findToken(vToken);
                    iz = 0;

                    if (!vFirstByte)
                    {
                        if (isalphas(*(vStartList - 2)) || isdigitus(*(vStartList - 2)) || *(vStartList - 2) == ')')
                            vLinhaList[ix++] = 0x20;
                    }
                    else
                        vFirstByte = 0;

                    while (keywords[iy].keyword[iz])
                    {
                        vLinhaList[ix++] = keywords[iy].keyword[iz++];
                    }

                    // Se nao for intervalo de funcao, coloca espaço depois do comando
                    if (*vStartList != '=' && (vToken < 0xC0 || vToken > 0xEF))
                        vLinhaList[ix++] = 0x20;

/*                    if (*vStartList != 0x28)
                        vLinhaList[ix++] = 0x20;*/
                }
                else
                {
                    // Apenas inclui na listagem
                    //if (strchr("+-*^/=;:><", *vTempPointer) || *vTempPointer >= 0xF0)
                    vLinhaList[ix++] = vToken;

                    // Se nao for aspas e o proximo for um token, inclui um espaço
                    if (vToken == 0x22 && *vStartList >=0x80)
                        vLinhaList[ix++] = 0x20;

                    /*if (isdigitus(vToken) && *vStartList!=')' && *vStartList!='.' && *vStartList!='"' && !isdigitus(*vStartList))
                        vLinhaList[ix++] = 0x20;*/
                }
            }

            vLinhaList[ix] = '\0';
            iw = strlen(vLinhaList) / 40;

            vLinhaList[ix++] = '\r';
            vLinhaList[ix++] = '\n';
            vLinhaList[ix++] = '\0';

            printText(vLinhaList);

            vPauseRowCounter = vPauseRowCounter + 1 + iw;

/*writeLongSerial("Aqui 332.666.0-[");
itoa(pPause,sqtdtam,10);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(vPauseRowCounter,sqtdtam,10);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(iw,sqtdtam,10);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(*videoCursorPosRowY,sqtdtam,10);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(*videoCursorPosRow,sqtdtam,10);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");*/

            if (pPause && vPauseRowCounter >= *vdpMaxRows)
            {
                printText("press any key to continue\0");
                vtec = inputLine(1,"@");
                vPauseRowCounter = 0;
                printText("\r\n\0");
                if (vtec == 0x1B)   // ESC
                    break;
            }

            vStartList = vNextList;
        }
        else
            break;
    }
}

//-----------------------------------------------------------------------------
// Sintaxe:
//      DEL <num>          : apaga só a linha <num>
//      DEL <num>-         : apaga a partir da linha <num> até o fim
//      DEL <numA>-<numB>  : apaga o intervalo de <numA> até <numB>, inclusive
//-----------------------------------------------------------------------------
void delLine(unsigned char *pArg)
{
    unsigned short pIni = 0, pFim = 0xFFFF;
    unsigned char *vStartList = pStartProg;
    unsigned long vDelAddr, vAntAddr, vNewAddr;
    unsigned short vNumLin;
    char sNumLin [sizeof(short)*8+1];
    unsigned char vLinhaList[255], sNumPar[10], vToken;
    int ix, iy, iz;

    if (pArg[0] != 0x00 && strchr(pArg,'-') != 0x00)
    {
        ix = 0;
        iy = 0;

        // listar intervalo
        while (pArg[ix] != '-')
            sNumPar[iy++] = pArg[ix++];

        sNumPar[iy] = 0x00;

        pIni = atoi(sNumPar);

        iy = 0;
        ix++;

        while (pArg[ix])
            sNumPar[iy++] = pArg[ix++];

        sNumPar[iy] = 0x00;

        if (sNumPar[0])
            pFim = atoi(sNumPar);
        else
            pFim = 0xFFFF;
    }
    else if (pArg[0] != 0x00)
    {
        pIni = atoi(pArg);
        pFim = pIni;
    }
    else
    {
        printText("Syntax Error !");
        return;
    }

    vDelAddr = findNumberLine(pIni, 0, 1);

    if (!vDelAddr)
    {
        printText("Non-existent line number\r\n\0");
        return;
    }

    while (1)
    {
        vStartList = vDelAddr;

        // Guarda proxima posicao
        vNewAddr = (*(vStartList) << 16) | (*(vStartList + 1) << 8) | *(vStartList + 2);

        if (!vNewAddr)
            break;

        // Pega numero da linha
        vNumLin = (*(vStartList + 3) << 8) | *(vStartList + 4);

        if (vNumLin > pFim)
            break;

        vAntAddr = findNumberLine(vNumLin, 1, 1);

        // Apaga a linha atual
        *vStartList       = 0x00;
        *(vStartList + 1) = 0x00;
        *(vStartList + 2) = 0x00;
        *(vStartList + 3) = 0x00;
        *(vStartList + 4) = 0x00;

        vStartList += 5;

        while (*vStartList)
            *vStartList++ = 0x00;

        vStartList = vAntAddr;
        *vStartList++ = ((vNewAddr & 0xFF0000) >> 16);
        *vStartList++ = ((vNewAddr & 0xFF00) >> 8);
        *vStartList++ =  (vNewAddr & 0xFF);

        // Se for a primeira linha, reposiciona na proxima
        if (*firstLineNumber == vNumLin)
        {
            if (vNewAddr)
            {
                vStartList = vNewAddr;

                // Pega numero da linha
                vNumLin = (*(vStartList + 3) << 8) | *(vStartList + 4);

                *firstLineNumber = vNumLin;
                *addrFirstLineNumber = vNewAddr;
            }
            else
            {
                *pStartProg = 0x00;
                *(pStartProg + 1) = 0x00;
                *(pStartProg + 2) = 0x00;

                *nextAddrLine = pStartProg;
                *firstLineNumber = 0;
                *addrFirstLineNumber = 0;
            }
        }

        if (!vNewAddr)
            break;

        vDelAddr = vNewAddr;
    }
}


//-----------------------------------------------------------------------------
// Sintaxe:
//      EDIT <num>          : Edita conteudo da linha <num>
//-----------------------------------------------------------------------------
void editLine(unsigned char *pNumber)
{
    int pIni = 0, ix, iy, iz, iw, ivv, vNumLin, pFim;
    unsigned char *vStartList = pStartProg, *vNextList;
    unsigned char vRetInput;
    char sNumLin [sizeof(short)*8+1], vFirstByte;
    unsigned char vLinhaList[255], sNumPar[10], vToken;

    if (pNumber[0] != 0x00)
    {
        // rodar desde uma linha especifica
        pIni = atoi(pNumber);
    }
    else
    {
        printText("Syntax Error !");
        return;
    }

    vStartList = findNumberLine(pIni, 0, 0);

    // Nao achou numero de linha inicial
    if (!vStartList)
    {
        printText("Non-existent line number\r\n\0");
        return;
    }

    // Carrega a linha no buffer
    // Guarda proxima posicao
    vNextList = (*(vStartList) << 16) | (*(vStartList + 1) << 8) | *(vStartList + 2);
	ix = 0;
    ivv = 0;

    if (vNextList)
    {
        // Pega numero da linha
        vNumLin = (*(vStartList + 3) << 8) | *(vStartList + 4);

        vStartList += 5;

        // Coloca numero da linha na listagem
        itoa(vNumLin, sNumLin, 10);
        iz = 0;

        while (sNumLin[iz++])
        {
            vLinhaList[ivv] = sNumLin[ivv];
            ivv++;
        }

        vLinhaList[ivv] = '\r';
        vLinhaList[ivv + 1] = '\n';
        vLinhaList[ivv + 2] = '\0';

        printText(vLinhaList);

        vFirstByte = 1;
        vbuf[ix] = 0x00;
        ix = 0;

        // Pega caracter a caracter da linha
        while (*vStartList)
        {
            vToken = *vStartList++;

            // Verifica se é token, se for, muda pra escrito
            if (vToken >= 0x80)
            {
                // Procura token na lista
                iy = findToken(vToken);
                iz = 0;

                if (!vFirstByte)
                {
                    if (isalphas(*(vStartList - 2)) || isdigitus(*(vStartList - 2)) || *(vStartList - 2) == ')')
                        vbuf[ix++] = 0x20;
                }
                else
                    vFirstByte = 0;

                while (keywords[iy].keyword[iz])
                {
                    vbuf[ix++] = keywords[iy].keyword[iz++];
                }

                // Se nao for intervalo de funcao, coloca espaço depois do comando
                if (*vStartList != '=' && (vToken < 0xC0 || vToken > 0xEF))
                    vbuf[ix++] = 0x20;
            }
            else
            {
                vbuf[ix++] = vToken;

                // Se nao for aspas e o proximo for um token, inclui um espaço
                if (vToken == 0x22 && *vStartList >=0x80)
                    vbuf[ix++] = 0x20;            }
        }
    }

    vbuf[ix] = '\0';

    // Edita a linha no buffer, usando o inputLine do monitor.c
    vRetInput = inputLine(128,'S'); // S - String Linha Editavel

    if (*vbuf != 0x00 && (vRetInput == 0x0D || vRetInput == 0x0A))
    {
        vLinhaList[ivv++] = 0x20;
        ix = strlen(vbuf);

        for(iz = 0; iz <= ix; iz++)
            vLinhaList[ivv++] = vbuf[iz];

        vLinhaList[ivv] = 0x00;

        for(iz = 0; iz <= ivv; iz++)
            vbuf[iz] = vLinhaList[iz];

        printText("\r\n\0");

        // Apaga a linha atual
        delLine(pNumber);

        // Reinsere a linha editada
        processLine();

        printText("\r\nOK\0");

        *vBufReceived = 0x00;
        *vbuf = '\0';

        printText("\r\n\0");
    }
    else if (vRetInput != 0x1B)
    {
        printText("\r\nAborted !!!\r\n\0");
    }
}

//-----------------------------------------------------------------------------
// Sintaxe:
//      RUN                : Executa o programa a partir da primeira linha do prog
//      RUN <num>          : Executa a partir da linha <num>
//-----------------------------------------------------------------------------
void runProg(unsigned char *pNumber)
{
    // Default rodar desde a primeira linha
    int pIni = 0, ix;
    unsigned char *vStartList = pStartProg;
    unsigned long vNextList;
    unsigned short vNumLin;
    unsigned int vInt;
    unsigned char vString[255], vTipoRet;
    unsigned long vReal;
    typeInf vRetInf;
    unsigned int vReta;
    char sNumLin [sizeof(short)*8+1];
    char vbuffer [sizeof(long)*8+1];
    unsigned char *vPointerChangedPointer;
    unsigned char *pForStack = forStack;
    unsigned char sqtdtam[20];
    unsigned char *vTempPointer;

    *nextAddrSimpVar = pStartSimpVar;
    *nextAddrArrayVar = pStartArrayVar;
    *nextAddrString = pStartString;

    for (ix = 0; ix < 0x2000; ix++)
        *(pStartSimpVar + ix) = 0x00;

    for (ix = 0; ix < 0x6000; ix++)
        *(pStartArrayVar + ix) = 0x00;

    for (ix = 0; ix < 0x800; ix++)
        *(pForStack + ix) = 0x00;

    if (pNumber[0] != 0x00)
    {
        // rodar desde uma linha especifica
        pIni = atoi(pNumber);
    }

    vStartList = findNumberLine(pIni, 0, 0);

    // Nao achou numero de linha inicial
    if (!vStartList)
    {
        printText("Non-existent line number\r\n\0");
        return;
    }

    vNextList = vStartList;

    *ftos=0;
    *gtos=0;
    *changedPointer = 0;
    *vDataPointer = 0;
    *randSeed = *(vmfp + Reg_TADR);
    *onErrGoto = 0;

    while (1)
    {
        if (*changedPointer!=0)
            vStartList = *changedPointer;

        // Guarda proxima posicao
        vNextList = (*(vStartList) << 16) | (*(vStartList + 1) << 8) | *(vStartList + 2);
        *nextAddr = vNextList;

        if (vNextList)
        {
            // Pega numero da linha
            vNumLin = (*(vStartList + 3) << 8) | *(vStartList + 4);

            vStartList += 5;

            // Pega caracter a caracter da linha
            *changedPointer = 0;
            *vMaisTokens = 0;
            *vParenteses = 0x00;
            *vTemIf = 0x00;
            *vTemThen = 0;
            *vTemElse = 0;
            *vTemIfAndOr = 0x00;
            vRetInf.tString[0] = 0x00;

            *pointerRunProg = vStartList;

            *vErroProc = 0;

            do
            {
                readChar();
                if (*vBufReceived==27)
                {
                    // volta para modo texto
#ifndef __TESTE_TOKENIZE__
                    if (*vdp_mode != VDP_MODE_TEXT)
                        basText();
#endif
                    // mostra mensagem de para subita
                    printText("\r\nStopped at ");
                    itoa(vNumLin, sNumLin, 10);
                    printText(sNumLin);
                    printText("\r\n");

                    // sai do laço
                    *nextAddr = 0;
                    break;
                }

                *doisPontos = 0;
                *vParenteses = 0x00;
                *vInicioSentenca = 1;

                if (*traceOn)
                {
                    printText("\r\nExecuting at ");
                    itoa(vNumLin, sNumLin, 10);
                    printText(sNumLin);
                    printText("\r\n");
                }

                vTempPointer = *pointerRunProg;
                *pointerRunProg = *pointerRunProg + 1;
                vReta = executeToken(*vTempPointer);

                if (*vErroProc)
                {
                    if (*onErrGoto == 0)
                        break;

                    *vErroProc = 0;
                    *changedPointer = *onErrGoto;
                }

                if (*changedPointer!=0)
                {
                    vPointerChangedPointer = *changedPointer;

                    if (*vPointerChangedPointer == 0x3A)
                    {
                        *pointerRunProg = *changedPointer;
                        *changedPointer = 0;
                    }
                }

                vTempPointer = *pointerRunProg;
                if (*vTempPointer != 0x00)
                {
                    if (*vTempPointer == 0x3A)
                    {
                        *doisPontos = 1;
                        *pointerRunProg = *pointerRunProg + 1;
                    }
                    else
                    {
                        if (*doisPontos && *vTempPointer <= 0x80)
                        {
                            // nao faz nada
                        }
                        else
                        {
                            nextToken();
                            if (*vErroProc) break;
                        }
                    }
                }
            } while (*doisPontos);

            if (*vErroProc)
            {
#ifndef __TESTE_TOKENIZE__
                if (*vdp_mode != VDP_MODE_TEXT)
                    basText();
#endif
                showErrorMessage(*vErroProc, vNumLin);
                break;
            }

            if (*nextAddr == 0)
                break;

            vNextList = *nextAddr;

            vStartList = vNextList;
        }
        else
            break;
    }

#ifndef __TESTE_TOKENIZE__
    if (*vdp_mode != VDP_MODE_TEXT)
        basText();
#endif
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void showErrorMessage(unsigned int pError, unsigned int pNumLine)
{
    char sNumLin [sizeof(short)*8+1];

    printText("\r\n");
    printText(listError[pError]);

    if (pNumLine > 0)
    {
        itoa(pNumLine, sNumLin, 10);

        printText(" at ");
        printText(sNumLin);
    }

    printText(" !\r\n\0");

    *vErroProc = 0;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int executeToken(unsigned char pToken)
{
    char vReta = 0;
#ifndef __TESTE_TOKENIZE__
    unsigned char *pForStack = forStack;
    int ix;
    unsigned char sqtdtam[20];

    switch (pToken)
    {
        case 0x00:  // End of Line
            vReta = 0;
            break;
        case 0x80:  // Let
            vReta = basLet();
            break;
        case 0x81:  // Print
            vReta = basPrint();
            break;
        case 0x82:  // IF
            vReta = basIf();
            break;
        case 0x83:  // THEN - nao faz nada
            vReta = 0;
            break;
        case 0x85:  // FOR
            vReta = basFor();
            break;
        case 0x86:  // TO - nao faz nada
            vReta = 0;
            break;
        case 0x87:  // NEXT
            vReta = basNext();
            break;
        case 0x88:  // STEP - nao faz nada
            vReta = 0;
            break;
        case 0x89:  // GOTO
            vReta = basGoto();
            break;
        case 0x8A:  // GOSUB
            vReta = basGosub();
            break;
        case 0x8B:  // RETURN
            vReta = basReturn();
            break;
        case 0x8C:  // REM - Ignora todas a linha depois dele
            vReta = 0;
            break;
        case 0x8D:  // INVERSE
            vReta = basInverse();
            break;
        case 0x8E:  // NORMAL
            vReta = basNormal();
            break;
        case 0x8F:  // DIM
            vReta = basDim();
            break;
        case 0x90:  // Nao fax nada, soh teste, pode ser retirado
            vReta = 0;
            break;
        case 0x91:  // DIM
            vReta = basOnVar();
            break;
        case 0x92:  // Input
            vReta = basInputGet(250);
            break;
        case 0x93:  // Get
            vReta = basInputGet(1);
            break;
        case 0x94:  // vTAB
            vReta = basVtab();
            break;
        case 0x95:  // HTAB
            vReta = basHtab();
            break;
        case 0x96:  // Home
            clearScr();
            break;
        case 0x97:  // CLEAR - Clear all variables
            for (ix = 0; ix < 0x2000; ix++)
                *(pStartSimpVar + ix) = 0x00;

            for (ix = 0; ix < 0x6000; ix++)
                *(pStartArrayVar + ix) = 0x00;

            for (ix = 0; ix < 0x800; ix++)
                *(pForStack + ix) = 0x00;

            vReta = 0;
            break;
        case 0x98:  // DATA - Ignora toda a linha depois dele, READ vai ler essa linha
            vReta = 0;
            break;
        case 0x99:  // Read
            vReta = basRead();
            break;
        case 0x9A:  // Restore
            vReta = basRestore();
            break;
        case 0x9E:  // END
            vReta = basEnd();
            break;
        case 0x9F:  // STOP
            vReta = basStop();
            break;
        case 0xB0:  // TEXT
            vReta = basText();
            break;
        case 0xB1:  // GR
            vReta = basGr();
            break;
        case 0xB2:  // HGR
            vReta = basHgr();
            break;
        case 0xB3:  // COLOR
            vReta = basColor();
            break;
        case 0xB4:  // PLOT
            vReta = basPlot();
            break;
        case 0xB5:  // HLIN
            vReta = basHVlin(1);
            break;
        case 0xB6:  // VLIN
            vReta = basHVlin(2);
            break;
        case 0xB8:  // HCOLOR
            vReta = basHcolor();
            break;
        case 0xB9:  // HPLOT
            vReta = basHplot();
            break;
        case 0xBA:  // AT - Nao faz nada
            vReta = 0;
            break;
        case 0xBB:  // ONERR
            vReta = basOnErr();
            break;
        case 0xC4:  // ASC
            vReta = basAsc();
            break;
        case 0xCD:  // PEEK
            vReta = basPeekPoke('R');
            break;
        case 0xCE:  // POKE
            vReta = basPeekPoke('W');
            break;
        case 0xD1:  // RND
            vReta = basRnd();
            break;
        case 0xDB:  // Len
            vReta = basLen();
            break;
        case 0xDC:  // Val
            vReta = basVal();
            break;
        case 0xDD:  // Str$
            vReta = basStr();
            break;
        case 0xE0:  // SCRN
            vReta = basScrn();
            break;
        case 0xE1:  // Chr$
            vReta = basChr();
            break;
        case 0xE2:  // Fre(0)
            vReta = basFre();
            break;
        case 0xE3:  // Sqrt
            vReta = basTrig(6);
            break;
        case 0xE4:  // Sin
            vReta = basTrig(1);
            break;
        case 0xE5:  // Cos
            vReta = basTrig(2);
            break;
        case 0xE6:  // Tan
            vReta = basTrig(3);
            break;
        case 0xE7:  // Log
            vReta = basTrig(4);
            break;
        case 0xE8:  // Exp
            vReta = basTrig(5);
            break;
        case 0xE9:  // SPC
            vReta = basSpc();
            break;
        case 0xEA:  // Tab
            vReta = basTab();
            break;
        case 0xEB:  // Mid$
            vReta = basLeftRightMid('M');
            break;
        case 0xEC:  // Right$
            vReta = basLeftRightMid('R');
            break;
        case 0xED:  // Left$
            vReta = basLeftRightMid('L');
            break;
        case 0xEE:  // INT
            vReta = basInt();
            break;
        case 0xEF:  // ABS
            vReta = basAbs();
            break;
        default:
            if (pToken < 0x80)  // variavel sem LET
            {
                *pointerRunProg = *pointerRunProg - 1;
                vReta = basLet();
            }
            else // Nao forem operadores logicos
            {
                *vErroProc = 14;
                vReta = 14;
            }
    }
#endif
    return vReta;
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
int nextToken(void)
{
    unsigned char *temp;
    int vRet, ccc;
    unsigned char sqtdtam[20];
    unsigned char *vTempPointer;

    *token_type = 0;
    *tok = 0;
    temp = token;

    vTempPointer = *pointerRunProg;
    if (*vTempPointer >= 0x80 && *vTempPointer < 0xF0)   // is a command
    {
        *tok = *vTempPointer;
        *token_type = COMMAND;
        *token = *vTempPointer;
        *(token + 1) = 0x00;

        return *token_type;
    }

    if (*vTempPointer == '\0') { // end of file
        *token = 0;
        *tok = FINISHED;
        *token_type = DELIMITER;

        return *token_type;
    }

    while(iswhite(*vTempPointer)) // skip over white space
    {
        *pointerRunProg = *pointerRunProg + 1;
        vTempPointer = *pointerRunProg;
    }

    if (*vTempPointer == '\r') { // crlf
        *pointerRunProg = *pointerRunProg + 2;
        *tok = EOL;
        *token = '\r';
        *(token + 1) = '\n';
        *(token + 2) = 0;
        *token_type = DELIMITER;

        return *token_type;
    }

    if (strchr("+-*^/=;:,><", *vTempPointer) || *vTempPointer >= 0xF0) { // delimiter
        *temp = *vTempPointer;
        *pointerRunProg = *pointerRunProg + 1; // advance to next position
        temp++;
        *temp = 0;
        *token_type = DELIMITER;

        return *token_type;
    }

    if (*vTempPointer == 0x28 || *vTempPointer == 0x29)
    {
        if (*vTempPointer == 0x28)
            *token_type = OPENPARENT;
        else
            *token_type = CLOSEPARENT;

        *token = *vTempPointer;
        *(token + 1) = 0x00;

        *pointerRunProg = *pointerRunProg + 1;

        return *token_type;
    }

    if (*vTempPointer == ":")
    {
        *doisPontos = 1;
        *token_type = DOISPONTOS;

        return *token_type;
    }

    if (*vTempPointer == '"') { // quoted string
        *pointerRunProg = *pointerRunProg + 1;
        vTempPointer = *pointerRunProg;

        while(*vTempPointer != '"'&& *vTempPointer != '\r')
        {
            *temp++ = *vTempPointer;
            *pointerRunProg = *pointerRunProg + 1;
            vTempPointer = *pointerRunProg;
        }

        if (*vTempPointer == '\r')
        {
            *vErroProc = 14;
            return 0;
        }

        *pointerRunProg = *pointerRunProg + 1;
        *temp = 0;
        *token_type = QUOTE;

        return *token_type;
    }

    if (isdigitus(*vTempPointer)) { // number
        while(!isdelim(*vTempPointer) && (*vTempPointer < 0x80 || *vTempPointer >= 0xF0))
        {
            *temp++ = *vTempPointer;
            *pointerRunProg = *pointerRunProg + 1;
            vTempPointer = *pointerRunProg;
        }
        *temp = '\0';
        *token_type = NUMBER;

        return *token_type;
    }

    if (isalphas(*vTempPointer)) { // var or command
        while(!isdelim(*vTempPointer) && (*vTempPointer < 0x80 || *vTempPointer >= 0xF0))
        {
            *temp++ = *vTempPointer;
            *pointerRunProg = *pointerRunProg + 1;
            vTempPointer = *pointerRunProg;
        }

        *temp = '\0';
        *token_type = VARIABLE;

        return *token_type;
    }

    *temp = '\0';

    // see if a string is a command or a variable
    if (*token_type == STRING) {
        *token_type = VARIABLE;
    }

    return *token_type;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int findToken(unsigned char pToken)
{
    unsigned char kt;

    // Procura o Token na lista e devolve a posicao
    for(kt = 0; kt < keywords_count; kt++)
    {
        if (keywords[kt].token == pToken)
            return kt;
    }

    // Procura o Token nas operacões de 1 char
    /*for(kt = 0; kt < keywordsUnique_count; kt++)
    {
        if (keywordsUnique[kt].token == pToken)
            return (kt + 0x80);
    }*/

    return 14;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
unsigned long findNumberLine(unsigned short pNumber, unsigned char pTipoRet, unsigned char pTipoFind)
{
    unsigned char *vStartList = *addrFirstLineNumber;
    unsigned char *vLastList = *addrFirstLineNumber;
    unsigned short vNumber = 0;
    char vbuffer [sizeof(long)*8+1];

    if (pNumber)
    {
        while(vStartList)
        {
            vNumber = ((*(vStartList + 3) << 8) | *(vStartList + 4));

            if ((!pTipoFind && vNumber < pNumber) || (pTipoFind && vNumber != pNumber))
            {
                vLastList = vStartList;
                vStartList = (*(vStartList) << 16) | (*(vStartList + 1) << 8) | *(vStartList + 2);
            }
            else
                break;
        }
    }

    if (!pTipoRet)
        return vStartList;
    else
        return vLastList;
}

//--------------------------------------------------------------------------------------
// Return true if c is a alphabetical (A-Z or a-z).
//--------------------------------------------------------------------------------------
int isalphas(unsigned char c)
{
    if ((c>0x40 && c<0x5B) || (c>0x60 && c<0x7B))
       return 1;

    return 0;
}

//--------------------------------------------------------------------------------------
// Return true if c is a number (0-9).
//--------------------------------------------------------------------------------------
int isdigitus(unsigned char c)
{
    if (c>0x2F && c<0x3A)
       return 1;

    return 0;
}

//--------------------------------------------------------------------------------------
// Return true if c is a delimiter.
//--------------------------------------------------------------------------------------
int isdelim(unsigned char c)
{
    if (strchr(" ;,+-<>()/*^=:", c) || c==9 || c=='\r' || c==0 || c>=0xF0)
       return 1;

    return 0;
}

//--------------------------------------------------------------------------------------
// Return 1 if c is space or tab.
//--------------------------------------------------------------------------------------
int iswhite(unsigned char c)
{
    if (c==' ' || c=='\t')
       return 1;

    return 0;
}

//--------------------------------------------------------------------------------------
// Load basic program in memory, throught xmodem protocol
// Syntaxe:
//          XBASLOAD
//--------------------------------------------------------------------------------------
int basXBasLoad(void)
{
    unsigned char vRet = 0;
    unsigned char vByte = 0;
    unsigned char *vTemp = pStartXBasLoad;
    unsigned char *vBufptr = vbuf;

    printText("Loading Basic Program...\r\n");

    // Carrega programa em outro ponto da memoria
    vRet = loadSerialToMem("890000",0);

    // Se tudo OK, tokeniza como se estivesse sendo digitado
    if (!vRet)
    {
        printText("Done.\r\n");
        printText("Processing...\r\n");

        while (1)
        {
            vByte = *vTemp++;

            if (vByte != 0x1A)
            {
                if (vByte != 0xD && vByte != 0x0A)
                    *vBufptr++ = vByte;
                else
                {
                    vTemp++;
                    *vBufptr = 0x00;
                    vBufptr = vbuf;
                    processLine();
                }
            }
            else
                break;
        }

        printText("Done.\r\n");
    }
    else
    {
        if (vRet == 0xFE)
            *vErroProc = 19;
        else
            *vErroProc = 20;
    }

    return 0;
}

#ifndef __TESTE_TOKENIZE__
//-----------------------------------------------------------------------------
// Retornos: -1 - Erro, 0 - Nao Existe, 1 - eh um valor numeral
//           [endereco > 1] - Endereco da variavel
//
//           se retorno > 1: pVariable vai conter o valor numeral (qdo 1) ou
//                           o conteudo da variavel (qdo endereco)
//-----------------------------------------------------------------------------
long findVariable(unsigned char* pVariable)
{
    unsigned char* vLista = pStartSimpVar;
    unsigned char* vTemp = pStartSimpVar;
    unsigned char* vListaAtu;
    long vEnder = 0, vVal = 0, vVal1 = 0, vVal2 = 0, vVal3 = 0, vVal4 = 0;
    int ix = 0, iy = 0, iz = 0, iw = 0;
    unsigned char sqtdtam[10];
    unsigned char vDim[88];
    unsigned int vTempDim = 0;
    unsigned long vOffSet;
    unsigned char ixDim = 0;
    unsigned char vArray = 0;
    unsigned long vPosNextVar = 0;
    unsigned char* vPosValueVar = 0;
    unsigned char vTamValue = 4;
    unsigned char *vTempPointer;
    unsigned short iDim = 0;
    unsigned int xxxttt;

    // Verifica se eh array (tem parenteses logo depois do nome da variavel)
#ifdef __DEBUG_ARRAYS__
writeLongSerial("Aqui 444.666.0-[");
writeLongSerial(pVariable);
writeLongSerial("]\r\n");
#endif

    vTempPointer = *pointerRunProg;
    if (*vTempPointer == 0x28)
    {
        // Define que eh array
        vArray = 1;

        // Procura as dimensoes
        nextToken();
        if (*vErroProc) return 0;

        // Erro, primeiro caracter depois da variavel, deve ser abre parenteses
        if (*tok == EOL || *tok == FINISHED || *token_type != OPENPARENT)
        {
            *vErroProc = 15;
            return 0;
        }

        do
        {
#ifdef __DEBUG_ARRAYS__
writeLongSerial("Aqui 444.666.0.0.1-[");
itoa(vDim[0],sqtdtam,10);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");
#endif
            nextToken();
            if (*vErroProc) return 0;
#ifdef __DEBUG_ARRAYS__
writeLongSerial("Aqui 444.666.0.0.2-[");
itoa(vDim[0],sqtdtam,10);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");
#endif
            if (*token_type == QUOTE) { // is string, error
                *vErroProc = 16;
                return 0;
            }
            else { // is expression
#ifdef __DEBUG_ARRAYS__
writeLongSerial("Aqui 444.666.0.0.3-[");
itoa(vDim[0],sqtdtam,10);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");
#endif
                putback();
#ifdef __DEBUG_ARRAYS__
writeLongSerial("Aqui 444.666.0.0.4-[");
itoa(vDim[0],sqtdtam,10);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");
#endif

                getExp(&vTempDim);
#ifdef __DEBUG_ARRAYS__
writeLongSerial("Aqui 444.666.0.0.5-[");
itoa(vDim[0],sqtdtam,10);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");
#endif
                if (*vErroProc) return 0;
#ifdef __DEBUG_ARRAYS__
writeLongSerial("Aqui 444.666.0.0.6-[");
itoa(vDim[0],sqtdtam,10);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
xxxttt = &vDim[0];
itoa(xxxttt,sqtdtam,10);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");
#endif
                if (*value_type == '$')
                {
                    *vErroProc = 16;
                    return 0;
                }
#ifdef __DEBUG_ARRAYS__
writeLongSerial("Aqui 444.666.0.1-[");
itoa(vTempDim,sqtdtam,10);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(vDim[0],sqtdtam,10);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
xxxttt = &vDim[0];
itoa(xxxttt,sqtdtam,10);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");
#endif

                if (*value_type == '#')
                {
                    vTempDim = fppInt(vTempDim);
                    *value_type = '%';
                }
#ifdef __DEBUG_ARRAYS__
writeLongSerial("Aqui 444.666.0.0.7-[");
itoa(vDim[0],sqtdtam,10);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
xxxttt = &vDim[0];
itoa(xxxttt,sqtdtam,10);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");
#endif

                vDim[ixDim] = vTempDim + 1;

#ifdef __DEBUG_ARRAYS__
writeLongSerial("Aqui 444.666.0.2-[");
itoa(vTempDim,sqtdtam,10);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(ixDim,sqtdtam,10);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(vDim[ixDim],sqtdtam,10);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(vDim[0],sqtdtam,10);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");
#endif
                ixDim++;
#ifdef __DEBUG_ARRAYS__
writeLongSerial("Aqui 444.666.0.0.8-[");
itoa(vDim[0],sqtdtam,10);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");
#endif
            }

#ifdef __DEBUG_ARRAYS__
writeLongSerial("Aqui 444.666.0.0.9-[");
itoa(vDim[0],sqtdtam,10);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");
#endif
            if (*token == ',')
            {
#ifdef __DEBUG_ARRAYS__
writeLongSerial("Aqui 444.666.0.0.A-[");
itoa(vDim[0],sqtdtam,10);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");
#endif
                *pointerRunProg = *pointerRunProg + 1;
#ifdef __DEBUG_ARRAYS__
writeLongSerial("Aqui 444.666.0.0.B-[");
itoa(vDim[0],sqtdtam,10);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");
#endif
                vTempPointer = *pointerRunProg;
            }
            else
                break;
#ifdef __DEBUG_ARRAYS__
writeLongSerial("Aqui 444.666.0.0.C-[");
itoa(vDim[0],sqtdtam,10);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");
#endif
        } while(1);
#ifdef __DEBUG_ARRAYS__
writeLongSerial("Aqui 444.666.0.0.D-[");
itoa(vDim[0],sqtdtam,10);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");
#endif

        // Deve ter pelo menos 1 elemento
        if (ixDim < 1)
        {
            *vErroProc = 21;
            return 0;
        }

        nextToken();
        if (*vErroProc) return 0;

        // Ultimo caracter deve ser fecha parenteses
        if (*token_type!=CLOSEPARENT)
        {
            *vErroProc = 15;
            return 0;
        }
    }

    // Procura na lista geral de variaveis simples / array
    if (vArray)
        vLista = pStartArrayVar;
    else
        vLista = pStartSimpVar;

    while(1)
    {
#ifdef __DEBUG_ARRAYS__
writeLongSerial("Aqui 444.666.1-[");
itoa(*(vLista + 3),sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(*(vLista + 4),sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(*(vLista + 5),sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(*(vLista + 6),sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");
#endif
        vPosNextVar  = (((unsigned long)*(vLista + 3) << 24) & 0xFF000000);
        vPosNextVar |= (((unsigned long)*(vLista + 4) << 16) & 0x00FF0000);
        vPosNextVar |= (((unsigned long)*(vLista + 5) << 8) & 0x0000FF00);
        vPosNextVar |= ((unsigned long)*(vLista + 6) & 0x000000FF);
        *value_type = *vLista;

#ifdef __DEBUG_ARRAYS__
writeLongSerial("Aqui 444.666.2-[");
itoa(vLista,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(vPosNextVar,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(pVariable[0],sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(pVariable[1],sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");
#endif
        if (*(vLista + 1) == pVariable[0] && *(vLista + 2) ==  pVariable[1])
        {
#ifdef __DEBUG_ARRAYS__
writeLongSerial("Aqui 444.666.3-[");
itoa(*(vLista + 1),sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(*(vLista + 2),sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(ixDim,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(vDim[0],sqtdtam,10);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(vDim[1],sqtdtam,10);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");
#endif
            // Pega endereco da variavel pra delvover
            if (vArray)
            {
                if (*vLista == '$')
                    vTamValue = 5;

                // Verifica se os tamanhos da dimensao informada e da variavel sao iguais
                if (ixDim != vLista[7])
                {
                    *vErroProc = 21;
                    return 0;
                }

                // Verifica se as posicoes informadas sao iguais ou menores que as da variavel, e ja calcula a nova posicao
                iw = (ixDim - 1);
                iz = 0;
                for (ix = ((ixDim - 1) * 2 ); ix >= 0; ix -= 2)
                {
                    // Verifica tamanho posicao
                    iDim = ((vLista[ix + 8] << 8) | vLista[ix + 9]);

#ifdef __DEBUG_ARRAYS__
writeLongSerial("Aqui 444.666.4-[");
itoa(ix,sqtdtam,10);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(vLista,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(vLista[ix + 8],sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(vLista[ix + 9],sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(vDim[iw],sqtdtam,10);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(iDim,sqtdtam,10);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(iw,sqtdtam,10);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");
#endif
                    if (vDim[iw] > iDim)
                    {
                        *vErroProc = 21;
                        return 0;
                    }

                    // Calcular a posicao do conteudo da variavel
                    vPosValueVar = vPosValueVar + (powNum(iDim, iz) * (vDim[iw] - 1 ) * vTamValue);

                    iw--;
                }

#ifdef __DEBUG_ARRAYS__
writeLongSerial("Aqui 444.666.5-[");
itoa(vLista,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa((ixDim * 2),sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(vPosValueVar,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");
#endif
                vOffSet = vLista;
                vPosValueVar = vPosValueVar + (vOffSet + 8 + (ixDim * 2));
                vEnder = vPosValueVar;
            }
            else
            {
                vPosValueVar = vLista + 3;
                vEnder = vLista;
            }

#ifdef __DEBUG_ARRAYS__
writeLongSerial("Aqui 444.666.6-[");
itoa(vLista,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa((ixDim * 2),sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(vPosValueVar,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");
#endif
            // Pelo tipo da variavel, ja retorna na variavel de nome o conteudo da variavel
            if (*vLista == '$')
            {
                vOffSet  = (((unsigned long)*(vPosValueVar + 1) << 24) & 0xFF000000);
                vOffSet |= (((unsigned long)*(vPosValueVar + 2) << 16) & 0x00FF0000);
                vOffSet |= (((unsigned long)*(vPosValueVar + 3) << 8) & 0x0000FF00);
                vOffSet |= ((unsigned long)*(vPosValueVar + 4) & 0x000000FF);
                vTemp = vOffSet;

#ifdef __DEBUG_ARRAYS__
writeLongSerial("Aqui 444.666.7-[");
itoa(vTemp,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa((ixDim * 2),sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(vPosValueVar,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");
#endif

                iy = *vPosValueVar;
                iz = 0;

                for (ix = 0; ix < iy; ix++)
                {
                    pVariable[iz++] = *(vTemp + ix); // Numero gerado
                    pVariable[iz] = 0x00;
                }

                pVariable[iz++] = 0x00;
            }
            else
            {
                if (!vArray)
                    vPosValueVar++;

                pVariable[0] = *(vPosValueVar);
                pVariable[1] = *(vPosValueVar + 1);
                pVariable[2] = *(vPosValueVar + 2);
                pVariable[3] = *(vPosValueVar + 3);
                pVariable[4] = 0x00;
            }

            return vEnder;
        }

        if (vArray)
            vLista = vPosNextVar;
        else
            vLista += 8;

        if ((!vArray && vLista >= pStartArrayVar) || (vArray && vLista >= pStartProg) || *vLista == 0x00)
            break;
    }

    return 0;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
char createVariable(unsigned char* pVariable, unsigned char* pValor, char pType)
{
    char vRet = 0;
    long vTemp = 0;
    char vbuffer [sizeof(long)*8+1];
    unsigned char* vNextSimpVar;
    char vLenVar = 0;

    vTemp = *nextAddrSimpVar;
    vNextSimpVar = *nextAddrSimpVar;

    vLenVar = strlen(pVariable);

    *vNextSimpVar++ = pType;
    *vNextSimpVar++ = pVariable[0];
    *vNextSimpVar++ = pVariable[1];

    vRet = updateVariable(vNextSimpVar, pValor, pType, 0);
    *nextAddrSimpVar += 8;

    return vRet;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
char updateVariable(unsigned long* pVariable, unsigned char* pValor, char pType, char pOper)
{
    long vNumVal = 0;
    int ix, iz = 0;
    char vbuffer [sizeof(long)*8+1];
    unsigned char* vNextSimpVar;
    unsigned char* vNextString;
    char pNewStr = 0;
    unsigned long vOffSet;
//    unsigned char* sqtdtam[20];

    vNextSimpVar = pVariable;
    *atuVarAddr = pVariable;

/*writeLongSerial("Aqui 333.666.0-[");
itoa(pVariable,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(pValor,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(pType,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");*/

    if (pType == '#' || pType == '%')   // Real ou Inteiro
    {
        if (vNextSimpVar < pStartArrayVar)
            *vNextSimpVar++ = 0x00;

        *vNextSimpVar++ = pValor[0];
        *vNextSimpVar++ = pValor[1];
        *vNextSimpVar++ = pValor[2];
        *vNextSimpVar++ = pValor[3];
    }
    else // String
    {
        iz = strlen(pValor);    // Tamanho da strings

/*writeLongSerial("Aqui 333.666.1-[");
itoa(*vNextSimpVar,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(iz,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(pOper,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");*/
        // Se for o mesmo tamanho ou menor, usa a mesma posicao
        if (*vNextSimpVar <= iz && pOper)
        {
            vOffSet  = (((unsigned long)*(vNextSimpVar + 1) << 24) & 0xFF000000);
            vOffSet |= (((unsigned long)*(vNextSimpVar + 2) << 16) & 0x00FF0000);
            vOffSet |= (((unsigned long)*(vNextSimpVar + 3) << 8) & 0x0000FF00);
            vOffSet |= ((unsigned long)*(vNextSimpVar + 4) & 0x000000FF);
            vNextString = vOffSet;

            if (pOper == 2 && vNextString == 0)
            {
                vNextString = *nextAddrString;
                pNewStr = 1;
            }
        }
        else
            vNextString = *nextAddrString;

        vNumVal = vNextString;
/*writeLongSerial("Aqui 333.666.2-[");
itoa(nextAddrString,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(vNextString,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(vNumVal,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");*/

        for (ix = 0; ix < iz; ix++)
        {
            *vNextString++ = pValor[ix];
        }

/*writeLongSerial("Aqui 333.666.3-[");
itoa(nextAddrString,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(vNextString,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(vNumVal,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");*/

        if (*vNextSimpVar > iz || !pOper || pNewStr)
            *nextAddrString = vNextString;

/*writeLongSerial("Aqui 333.666.4-[");
itoa(vNextString,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(vNumVal,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");*/

        *vNextSimpVar++ = iz;

/*writeLongSerial("Aqui 333.666.5-[");
itoa(vNextString,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");*/

        *vNextSimpVar++ = ((vNumVal & 0xFF000000) >>24);
        *vNextSimpVar++ = ((vNumVal & 0x00FF0000) >>16);
        *vNextSimpVar++ = ((vNumVal & 0x0000FF00) >>8);
        *vNextSimpVar++ = (vNumVal & 0x000000FF);
/*writeLongSerial("Aqui 333.666.6-[");
itoa(vNextString,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");*/
    }

/*    *(vNextSimpVar + 1) = 0x00;
    *(vNextSimpVar + 2) = 0x00;
    *(vNextSimpVar + 3) = 0x00;
    *(vNextSimpVar + 4) = 0x00;*/

    return 0;
}

//--------------------------------------------------------------------------------------
char createVariableArray(unsigned char* pVariable, char pType, unsigned int pNumDim, unsigned int *pDim)
{
    char vRet = 0;
    long vTemp = 0;
    unsigned char* vTempC = &vTemp;
    char vbuffer [sizeof(long)*8+1];
    unsigned char* vNextArrayVar;
    char vLenVar = 0;
    int ix, vTam;
    long vAreaFree = (pStartString - *nextAddrArrayVar);
    long vSizeTotal = 0;
//    unsigned char sqtdtam[20];

    vTemp = *nextAddrArrayVar;
    vNextArrayVar = *nextAddrArrayVar;

    vLenVar = strlen(pVariable);

    *vNextArrayVar++ = pType;
    *vNextArrayVar++ = pVariable[0];
    *vNextArrayVar++ = pVariable[1];
    vTam = 0;

    for (ix = 0; ix < pNumDim; ix++)
    {
        // Somando mais 1, porque 0 = 1 em quantidade e e em posicao (igual ao c)
        pDim[ix] = pDim[ix] /*+ 1*/ ;

        // Definir o tamanho do campo de dados do array
        if (vTam == 0)
            vTam = pDim[ix] /*- 1*/ ;
        else
            vTam = vTam * (pDim[ix] /*- 1*/ );

/*writeLongSerial("Aqui 333.666.0-[");
itoa(vTam,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(ix,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(pDim[ix],sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");*/
    }

/*writeLongSerial("Aqui 333.666.1-[");
itoa(vTam,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(pNumDim,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");*/

    if (pType == '$')
        vTam = vTam * 5;
    else
        vTam = vTam * 4;

/*writeLongSerial("Aqui 333.666.2-[");
itoa(pType,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(vTam,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");*/

    vSizeTotal = vTam + 8;
    vSizeTotal = vSizeTotal + (pNumDim *2);

/*writeLongSerial("Aqui 333.666.3-[");
itoa(pStartString,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(*nextAddrArrayVar,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(vAreaFree,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(vSizeTotal,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");*/

    if (vSizeTotal > vAreaFree)
    {
        *vErroProc = 22;
        return 0;
    }

    // Coloca setup do array
    vTemp = vTemp + vTam + 8 + (pNumDim * 2);
    *vNextArrayVar++ = vTempC[0];
    *vNextArrayVar++ = vTempC[1];
    *vNextArrayVar++ = vTempC[2];
    *vNextArrayVar++ = vTempC[3];
    *vNextArrayVar++ = pNumDim;

    for (ix = 0; ix < pNumDim; ix++)
    {
        *vNextArrayVar++ = (pDim[ix] >> 8);
        *vNextArrayVar++ = (pDim[ix] & 0xFF);
    }

    // Limpa area de dados (zera)
    for (ix = 0; ix < vTam; ix++)
        *(vNextArrayVar + ix) = 0x00;

    *nextAddrArrayVar = vTemp;

    return 0;
}

//--------------------------------------------------------------------------------------
// Return a token to input stream.
//--------------------------------------------------------------------------------------
void putback(void)
{
    unsigned char *t;

    if (*token_type==COMMAND)    // comando nao faz isso
        return;

    t = token;
    while (*t++)
        *pointerRunProg = *pointerRunProg - 1;
}

//--------------------------------------------------------------------------------------
// Return compara 2 strings
//--------------------------------------------------------------------------------------
int ustrcmp(char *X, char *Y)
{
    while (*X)
    {
        // if characters differ, or end of the second string is reached
        if (*X != *Y) {
            break;
        }

        // move to the next pair of characters
        X++;
        Y++;
    }

    // return the ASCII difference after converting `char*` to `unsigned char*`
    return *(unsigned char*)X - *(unsigned char*)Y;
}

//--------------------------------------------------------------------------------------
// Entry point into parser.
//--------------------------------------------------------------------------------------
void getExp(unsigned char *result)
{
    unsigned char sqtdtam[10];

    nextToken();
    if (*vErroProc) return;

    if (!*token) {
        *vErroProc = 2;
        return;
    }

    level2(result);
    if (*vErroProc) return;

    putback(); // return last token read to input stream

    return;
}

//--------------------------------------------------------------------------------------
//  Add or subtract two terms real/int or string.
//--------------------------------------------------------------------------------------
void level2(unsigned char *result)
{
    char  op;
    unsigned char hold[50];
    unsigned char valueTypeAnt;
    unsigned int *lresult = result;
    unsigned int *lhold = hold;
    unsigned char* sqtdtam[10];

    level3(result);
    if (*vErroProc) return;

    op = *token;

    while(op == '+' || op == '-') {
        nextToken();
        if (*vErroProc) return;

        valueTypeAnt = *value_type;

        level3(&hold);
        if (*vErroProc) return;

        if (*value_type != valueTypeAnt)
        {
            if (*value_type == '$' || valueTypeAnt == '$')
            {
                *vErroProc = 16;
                return;
            }
        }

        // Se forem diferentes os 2, se for um deles string, da erro, se nao, passa o inteiro para real
        if (*value_type == '$' && valueTypeAnt == '$' && op == '+')
            strcat(result,&hold);
        else if ((*value_type == '$' || valueTypeAnt == '$') && op == '-')
        {
            *vErroProc = 16;
            return;
        }
        else
        {
            if (*value_type != valueTypeAnt)
            {
                if (*value_type == '$' || valueTypeAnt == '$')
                {
                    *vErroProc = 16;
                    return;
                }
                else if (*value_type == '#')
                {
                    *lresult = fppReal(*lresult);
                }
                else
                {
                    *lhold = fppReal(*lhold);
                    *value_type = '#';
                }
            }

            if (*value_type == '#')
                arithReal(op, result, &hold);
            else
                arithInt(op, result, &hold);
        }

        op = *token;
    }

    return;
}

//--------------------------------------------------------------------------------------
// Multiply or divide two factors real/int.
//--------------------------------------------------------------------------------------
void level3(unsigned char *result)
{
    char  op;
    unsigned char hold[50];
    unsigned int *lresult = result;
    unsigned int *lhold = hold;
    char value_type_ant=0;
    unsigned char* sqtdtam[10];

    do
    {
        level30(result);
        if (*vErroProc) return;
        if (*token==0xF3||*token==0xF4)
        {
            nextToken();
            if (*vErroProc) return;
        }
        else
            break;
    }
    while (1);

    op = *token;
    while(op == '*' || op == '/' || op == '%') {
        if (*value_type == '$')
        {
            *vErroProc = 16;
            return;
        }

        nextToken();
        if (*vErroProc) return;

        value_type_ant = *value_type;

        level4(&hold);
        if (*vErroProc) return;

        // Se forem diferentes os 2, se for um deles string, da erro, se nao, passa o inteiro para real
        if (*value_type == '$' || value_type_ant == '$')
        {
            *vErroProc = 16;
            return;
        }

        if (*value_type != value_type_ant)
        {
            if (*value_type == '#')
            {
                *lresult = fppReal(*lresult);
            }
            else
            {
                *lhold = fppReal(*lhold);
                *value_type = '#';
            }
        }

        // se valor inteiro e for divisao, obrigatoriamente devolve valor real
        if (*value_type == '%' && op == '/')
        {
            *lresult = fppReal(*lresult);
            *lhold = fppReal(*lhold);
            *value_type = '#';
        }

        if (*value_type == '#')
            arithReal(op, result, &hold);
        else
            arithInt(op, result, &hold);

        op = *token;
    }

    return;
}

//--------------------------------------------------------------------------------------
// Is a NOT
//--------------------------------------------------------------------------------------
void level30(unsigned char *result)
{
    char  op;
    int *iLog = result;

    op = 0;
    if (*token == 0xF8) // NOT
    {
        op = *token;
        nextToken();
        if (*vErroProc) return;
    }

    level31(result);
    if (*vErroProc) return;

    if (op)
    {
        if (*value_type == '$' || *value_type == '#')
        {
            *vErroProc = 16;
            return;
        }

        *iLog = !*iLog;
    }

    return;
}

//--------------------------------------------------------------------------------------
// Process logic conditions
//--------------------------------------------------------------------------------------
void level31(unsigned char *result)
{
    unsigned char  op;
    unsigned char hold[50];
    char value_type_ant=0;
    int *rVal = result;
    int *hVal = hold;
    unsigned char* sqtdtam[10];

    level32(result);
    if (*vErroProc) return;

    op = *token;
    if (op==0xF3 /* AND */|| op==0xF4 /* OR */) {
        nextToken();
        if (*vErroProc) return;

        level32(&hold);
        if (*vErroProc) return;

/*writeLongSerial("Aqui 333.666.0-[");
itoa(op,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(*rVal,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(*hVal,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");*/

        if (op==0xF3)
            *rVal = (*rVal && *hVal);
        else
            *rVal = (*rVal || *hVal);

/*riteLongSerial("Aqui 333.666.1-[");
itoa(op,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(*rVal,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");*/
    }

    return;
}

//--------------------------------------------------------------------------------------
// Process logic conditions
//--------------------------------------------------------------------------------------
void level32(unsigned char *result)
{
    unsigned char  op;
    unsigned char hold[50];
    unsigned char value_type_ant=0;
    unsigned int *lresult = result;
    unsigned int *lhold = hold;
    unsigned char sqtdtam[20];

    level4(result);
    if (*vErroProc) return;

    op = *token;
    if (op=='=' || op=='<' || op=='>' || op==0xF5 /* >= */ || op==0xF6 /* <= */|| op==0xF7 /* <> */) {
//        if (op==0xF5 /* >= */ || op==0xF6 /* <= */|| op==0xF7)
//            pointerRunProg++;

        nextToken();
        if (*vErroProc) return;

        value_type_ant = *value_type;

        level4(&hold);
        if (*vErroProc) return;

        if ((value_type_ant=='$' && *value_type!='$') || (value_type_ant != '$' && *value_type == '$'))
        {
            *vErroProc = 16;
            return;
        }

        // Se forem diferentes os 2, se for um deles string, da erro, se nao, passa o inteiro para real
        if (*value_type != value_type_ant)
        {
            if (*value_type == '#')
            {
                *lresult = fppReal(*lresult);
            }
            else
            {
                *lhold = fppReal(*lhold);
                *value_type = '#';
            }
        }

        if (*value_type == '$')
            logicalString(op, result, &hold);
        else if (*value_type == '#')
            logicalNumericFloat(op, result, &hold);
        else
            logicalNumericInt(op, result, &hold);
    }

    return;
}

//--------------------------------------------------------------------------------------
// Process integer exponent real/int.
//--------------------------------------------------------------------------------------
void level4(unsigned char *result)
{
    unsigned char hold[50];
    unsigned int *lresult = result;
    unsigned int *lhold = hold;
    char value_type_ant=0;

    level5(result);
    if (*vErroProc) return;

    if (*token== '^') {
        if (*value_type == '$')
        {
            *vErroProc = 16;
            return;
        }

        nextToken();
        if (*vErroProc) return;

        value_type_ant = *value_type;

        level4(&hold);
        if (*vErroProc) return;

        if (*value_type == '$')
        {
            *vErroProc = 16;
            return;
        }

        // Se forem diferentes os 2, se for um deles string, da erro, se nao, passa o inteiro para real
        if (*value_type != value_type_ant)
        {
            if (*value_type == '#')
            {
                *lresult = fppReal(*lresult);
            }
            else
            {
                *lhold = fppReal(*lhold);
                *value_type = '#';
            }
        }

        if (*value_type == '#')
            arithReal('^', result, &hold);
        else
            arithInt('^', result, &hold);
    }

    return;
}

//--------------------------------------------------------------------------------------
// Is a unary + or -.
//--------------------------------------------------------------------------------------
void level5(unsigned char *result)
{
    char  op;

    op = 0;
    if (*token_type==DELIMITER && (*token=='+' || *token=='-')) {
        op = *token;
        nextToken();
        if (*vErroProc) return;
    }

    level6(result);
    if (*vErroProc) return;

    if (op)
    {
        if (*value_type == '$')
        {
            *vErroProc = 16;
            return;
        }

        if (*value_type == '#')
            unaryReal(op, result);
        else
            unaryInt(op, result);
    }

    return;
}

//--------------------------------------------------------------------------------------
// Process parenthesized expression real/int/string or function.
//--------------------------------------------------------------------------------------
void level6(unsigned char *result)
{
    if ((*token == '(') && (*token_type == OPENPARENT)) {
        nextToken();
        if (*vErroProc) return;

        level2(result);
        if (*token != ')')
        {
            *vErroProc = 15;
            return;
        }

        nextToken();
        if (*vErroProc) return;
    }
    else
    {
        primitive(result);
        return;
    }

    return;
}

//--------------------------------------------------------------------------------------
// Find value of number or variable.
//--------------------------------------------------------------------------------------
void primitive(unsigned char *result)
{
    unsigned long ix;
    unsigned char* vix = &ix;
    unsigned char* vRet;
    unsigned char sqtdtam[10];
    unsigned char *vTempPointer;

    switch(*token_type) {
        case VARIABLE:
            if (strlen(token) < 3)
            {
                *value_type=VARTYPEDEFAULT;

                if (strlen(token) == 2 && *(token + 1) < 0x30)
                    *value_type = *(token + 1);
            }
            else
            {
                *value_type = *(token + 2);
            }

            vRet = find_var(token);
            if (*vErroProc) return;
            if (*value_type == '$')  // Tipo da variavel
                strcpy(result,vRet);
            else
            {
                for (ix = 0;ix < 5;ix++)
                    result[ix] = vRet[ix];
            }
            nextToken();
            if (*vErroProc) return;
            return;
        case QUOTE:
            *value_type='$';
            strcpy(result,token);
            nextToken();
            if (*vErroProc) return;
            return;
        case NUMBER:
            if (strchr(token,'.'))  // verifica se eh numero inteiro ou real
            {
                *value_type='#'; // Real
                ix=floatStringToFpp(token);
                if (*vErroProc) return;
            }
            else
            {
                *value_type='%'; // Inteiro
                ix=atoi(token);
            }

            vix = &ix;

            result[0] = vix[0];
            result[1] = vix[1];
            result[2] = vix[2];
            result[3] = vix[3];

            nextToken();
            if (*vErroProc) return;
            return;
        case COMMAND:
            vTempPointer = *pointerRunProg;
            *token = *vTempPointer;
            *pointerRunProg = *pointerRunProg + 1;
            executeToken(*vTempPointer);  // Retorno do resultado da funcao deve voltar pela variavel token. *value_type tera o tipo de retorno
            if (*vErroProc) return;

            if (*value_type == '$')  // Tipo do retorno
                strcpy(result,token);
            else
            {
                for (ix = 0; ix < 4; ix++)
                {
                    result[ix] = *(token + ix);
                }
            }

            nextToken();
            if (*vErroProc) return;
            return;
        default:
            *vErroProc = 14;
            return;
    }

    return;
}

//--------------------------------------------------------------------------------------
// Perform the specified arithmetic inteiro.
//--------------------------------------------------------------------------------------
void arithInt(char o, char *r, char *h)
{
    int t, ex;
    int *rVal = r; //(int)((int)(r[0] << 24) | (int)(r[1] << 16) | (int)(r[2] << 8) | (int)(r[3]));
    int *hVal = h; //(int)((int)(h[0] << 24) | (int)(h[1] << 16) | (int)(h[2] << 8) | (int)(h[3]));
    char* vRval = rVal;

    switch(o) {
        case '-':
            *rVal = *rVal - *hVal;
            break;
        case '+':
            *rVal = *rVal + *hVal;
            break;
        case '*':
            *rVal = *rVal * *hVal;
            break;
        case '/':
            *rVal = (*rVal)/(*hVal);
            break;
        case '^':
            ex = *rVal;
            if (*hVal==0) {
                *rVal = 1;
                break;
            }
            ex = powNum(*rVal,*hVal);
            *rVal = ex;
            break;
    }

    r[0] = vRval[0];
    r[1] = vRval[1];
    r[2] = vRval[2];
    r[3] = vRval[3];
}


//--------------------------------------------------------------------------------------
// Perform the specified arithmetic real.
//--------------------------------------------------------------------------------------
void arithReal(char o, char *r, char *h)
{
    int t, ex;
    unsigned long *rVal = r; //(int)((int)(r[0] << 24) | (int)(r[1] << 16) | (int)(r[2] << 8) | (int)(r[3]));
    unsigned long *hVal = h; //(int)((int)(h[0] << 24) | (int)(h[1] << 16) | (int)(h[2] << 8) | (int)(h[3]));
    char* vRval = rVal;

    switch(o) {
        case '-':
            *rVal = fppSub(*rVal, *hVal);
            break;
        case '+':
            *rVal = fppSum(*rVal, *hVal);
            break;
        case '*':
            *rVal = fppMul(*rVal, *hVal);
            break;
        case '/':
            *rVal = fppDiv(*rVal, *hVal);
            break;
        case '^':
            *rVal = fppPwr(*rVal, *hVal);
            break;
    }
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void logicalNumericFloat(unsigned char o, char *r, char *h)
{
    int t, ex;
    unsigned long *rVal = r; //(int)((int)(r[0] << 24) | (int)(r[1] << 16) | (int)(r[2] << 8) | (int)(r[3]));
    unsigned long *hVal = h; //(int)((int)(h[0] << 24) | (int)(h[1] << 16) | (int)(h[2] << 8) | (int)(h[3]));
    unsigned long oCCR = 0;

    oCCR = fppComp(*rVal, *hVal);

    *rVal = 0;
    *value_type = '%';

    switch(o) {
        case '=':
            if (oCCR & 0x04)    // Z=1
                *rVal = 1;
            break;
        case '>':
            if (!(oCCR & 0x08) && !(oCCR & 0x04))   // N=0 e Z=0
                *rVal = 1;
            break;
        case '<':
            if ((oCCR & 0x08) && !(oCCR & 0x04))   // N=1 e Z=0
                *rVal = 1;
            break;
        case 0xF5:  // >=
            if (!(oCCR & 0x08) || (oCCR & 0x04))   // N=0 ou Z=1
                *rVal = 1;
            break;
        case 0xF6:  // <=
            if ((oCCR & 0x08) || (oCCR & 0x04))   // N=1 ou Z=1
                *rVal = 1;
            break;
        case 0xF7:  // <>
            if (!(oCCR & 0x04)) // z=0
                *rVal = 1;
            break;
    }
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
char logicalNumericFloatLong(unsigned char o, long r, long h)
{
    char ex = 0;
    unsigned long oCCR = 0;

    oCCR = fppComp(r, h);

    *value_type = '%';

    switch(o) {
        case '=':
            if (oCCR & 0x04)    // Z=1
                ex = 1;
            break;
        case '>':
            if (!(oCCR & 0x08) && !(oCCR & 0x04))   // N=0 e Z=0
                ex = 1;
            break;
        case '<':
            if ((oCCR & 0x08) && !(oCCR & 0x04))   // N=1 e Z=0
                ex = 1;
            break;
        case 0xF5:  // >=
            if (!(oCCR & 0x08) || (oCCR & 0x04))   // N=0 ou Z=1
                ex = 1;
            break;
        case 0xF6:  // <=
            if ((oCCR & 0x08) || (oCCR & 0x04))   // N=1 ou Z=1
                ex = 1;
            break;
        case 0xF7:  // <>
            if (!(oCCR & 0x04)) // z=0
                ex = 1;
            break;
    }

    return ex;
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void logicalNumericInt(unsigned char o, char *r, char *h)
{
    int t, ex;
    int *rVal = r; //(int)((int)(r[0] << 24) | (int)(r[1] << 16) | (int)(r[2] << 8) | (int)(r[3]));
    int *hVal = h; //(int)((int)(h[0] << 24) | (int)(h[1] << 16) | (int)(h[2] << 8) | (int)(h[3]));

    switch(o) {
        case '=':
            *rVal = (*rVal == *hVal);
            break;
        case '>':
            *rVal = (*rVal > *hVal);
            break;
        case '<':
            *rVal = (*rVal < *hVal);
            break;
        case 0xF5:
            *rVal = (*rVal >= *hVal);
            break;
        case 0xF6:
            *rVal = (*rVal <= *hVal);
            break;
        case 0xF7:
            *rVal = (*rVal != *hVal);
            break;
    }
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void logicalString(unsigned char o, char *r, char *h)
{
    int t, ex;
    int *rVal = r;

    ex = ustrcmp(r,h);
    *value_type = '%';

    switch(o) {
        case '=':
            *rVal = (ex == 0);
            break;
        case '>':
            *rVal = (ex > 0);
            break;
        case '<':
            *rVal = (ex < 0);
            break;
        case 0xF5:
            *rVal = (ex >= 0);
            break;
        case 0xF6:
            *rVal = (ex <= 0);
            break;
        case 0xF7:
            *rVal = (ex != 0);
            break;
    }
}

//--------------------------------------------------------------------------------------
// Reverse the sign.
//--------------------------------------------------------------------------------------
void unaryInt(char o, int *r)
{
    if (o=='-')
        *r = -(*r);
}

//--------------------------------------------------------------------------------------
// Reverse the sign.
//--------------------------------------------------------------------------------------
void unaryReal(char o, int *r)
{
    if (o=='-')
    {
        *r = fppNeg(*r);
    }
}

//--------------------------------------------------------------------------------------
// Find the value of a variable.
//--------------------------------------------------------------------------------------
unsigned char* find_var(char *s)
{
    unsigned char vTemp[250];

    *vErroProc = 0x00;

    if (!isalphas(*s)){
        *vErroProc = 4; // not a variable
        return 0;
    }

    if (strlen(s) < 3)
    {
        vTemp[0] = *s;
        vTemp[2] = VARTYPEDEFAULT;

        if (strlen(s) == 2 && *(s + 1) < 0x30)
            vTemp[2] = *(s + 1);

        if (strlen(s) == 2 && isalphas(*(s + 1)))
            vTemp[1] = *(s + 1);
        else
            vTemp[1] = 0x00;
    }
    else
    {
        vTemp[0] = *s++;
        vTemp[1] = *s++;
        vTemp[2] = *s;
    }

    if (!findVariable(&vTemp))
    {
        *vErroProc = 4; // not a variable
        return 0;
    }

    return vTemp;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void forPush(for_stack i)
{
    if (*ftos>FOR_NEST)
    {
        *vErroProc = 10;
        return;
    }

    *(forStack + *ftos) = i;
    *ftos = *ftos + 1;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
for_stack forPop(void)
{
    for_stack i;

    *ftos = *ftos - 1;

    if (*ftos<0)
    {
        *vErroProc = 11;
        return(*forStack);
    }

    i=*(forStack + *ftos);

    return(i);
}

//-----------------------------------------------------------------------------
// GOSUB stack push function.
//-----------------------------------------------------------------------------
void gosubPush(unsigned long i)
{
    if (*gtos>SUB_NEST)
    {
        *vErroProc = 12;
        return;
    }

    *(gosubStack + *gtos)=i;

    *gtos = *gtos + 1;
}

//-----------------------------------------------------------------------------
// GOSUB stack pop function.
//-----------------------------------------------------------------------------
unsigned long gosubPop(void)
{
    unsigned long i;

    *gtos = *gtos - 1;

    if (*gtos<0)
    {
        *vErroProc = 13;
        return 0;
    }

    i=*(gosubStack + *gtos);

    return i;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
unsigned int powNum(unsigned int pbase, unsigned char pexp)
{
    unsigned int iz, vRes = pbase;

    if (pexp > 0)
    {
        pexp--;

        for(iz = 0; iz < pexp; iz++)
        {
            vRes = vRes * pbase;
        }
    }
    else
        vRes = 1;

    return vRes;
}

//-----------------------------------------------------------------------------
// FUNCOES PONTO FLUTUANTE
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Convert from String to Float Single-Precision
//-----------------------------------------------------------------------------
unsigned long floatStringToFpp(unsigned char* pFloat)
{
    unsigned long vFpp;

    *floatBufferStr = pFloat;
    STR_TO_FP();
    vFpp = *floatNumD7;

    return vFpp;
}

//-----------------------------------------------------------------------------
// Convert from Float Single-Precision to String
//-----------------------------------------------------------------------------
int fppTofloatString(unsigned long pFpp, unsigned char *buf)
{
    *floatBufferStr = buf;
    *floatNumD7 = pFpp;
    FP_TO_STR();

    return 0;
}

//-----------------------------------------------------------------------------
// Float Function to SUM D7+D6
//-----------------------------------------------------------------------------
unsigned long fppSum(unsigned long pFppD7, unsigned long pFppD6)
{
    *floatNumD7 = pFppD7;
    *floatNumD6 = pFppD6;
    FPP_SUM();

    return *floatNumD7;
}

//-----------------------------------------------------------------------------
// Float Function to Subtraction D7-D6
//-----------------------------------------------------------------------------
unsigned long fppSub(unsigned long pFppD7, unsigned long pFppD6)
{
    *floatNumD7 = pFppD7;
    *floatNumD6 = pFppD6;
    FPP_SUB();

    return *floatNumD7;
}

//-----------------------------------------------------------------------------
// Float Function to Mul D7*D6
//-----------------------------------------------------------------------------
unsigned long fppMul(unsigned long pFppD7, unsigned long pFppD6)
{
    *floatNumD7 = pFppD7;
    *floatNumD6 = pFppD6;
    FPP_MUL();

    return *floatNumD7;
}

//-----------------------------------------------------------------------------
// Float Function to Division D7/D6
//-----------------------------------------------------------------------------
unsigned long fppDiv(unsigned long pFppD7, unsigned long pFppD6)
{
    *floatNumD7 = pFppD7;
    *floatNumD6 = pFppD6;
    FPP_DIV();

    return *floatNumD7;
}

//-----------------------------------------------------------------------------
// Float Function to Power D7^D6
//-----------------------------------------------------------------------------
unsigned long fppPwr(unsigned long pFppD7, unsigned long pFppD6)
{
    *floatNumD7 = pFppD7;
    *floatNumD6 = pFppD6;
    FPP_PWR();

    return *floatNumD7;
}

//-----------------------------------------------------------------------------
// Float Function Convert Float to Int
//-----------------------------------------------------------------------------
long fppInt(unsigned long pFppD7)
{
    *floatNumD7 = pFppD7;
    FPP_INT();

    return *floatNumD7;
}

//-----------------------------------------------------------------------------
// Float Function Convert Int to Float
//-----------------------------------------------------------------------------
unsigned long fppReal(long pFppD7)
{
    *floatNumD7 = pFppD7;
    FPP_FPP();

    return *floatNumD7;
}

//-----------------------------------------------------------------------------
// Float Function Return SIN
//-----------------------------------------------------------------------------
unsigned long fppSin(long pFppD7)
{
    *floatNumD7 = pFppD7;
    FPP_SIN();

    return *floatNumD7;
}

//-----------------------------------------------------------------------------
// Float Function Return COS
//-----------------------------------------------------------------------------
unsigned long fppCos(long pFppD7)
{
    *floatNumD7 = pFppD7;
    FPP_COS();

    return *floatNumD7;
}

//-----------------------------------------------------------------------------
// Float Function Return TAN
//-----------------------------------------------------------------------------
unsigned long fppTan(long pFppD7)
{
    *floatNumD7 = pFppD7;
    FPP_TAN();

    return *floatNumD7;
}

//-----------------------------------------------------------------------------
// Float Function Return SIN Hiperb
//-----------------------------------------------------------------------------
unsigned long fppSinH(long pFppD7)
{
    *floatNumD7 = pFppD7;
    FPP_SINH();

    return *floatNumD7;
}

//-----------------------------------------------------------------------------
// Float Function Return COS Hiperb
//-----------------------------------------------------------------------------
unsigned long fppCosH(long pFppD7)
{
    *floatNumD7 = pFppD7;
    FPP_COSH();

    return *floatNumD7;
}

//-----------------------------------------------------------------------------
// Float Function Return TAN Hiperb
//-----------------------------------------------------------------------------
unsigned long fppTanH(long pFppD7)
{
    *floatNumD7 = pFppD7;
    FPP_TANH();

    return *floatNumD7;
}

//-----------------------------------------------------------------------------
// Float Function Return Sqrt
//-----------------------------------------------------------------------------
unsigned long fppSqrt(long pFppD7)
{
    *floatNumD7 = pFppD7;
    FPP_SQRT();

    return *floatNumD7;
}

//-----------------------------------------------------------------------------
// Float Function Return TAN Hiperb
//-----------------------------------------------------------------------------
unsigned long fppLn(long pFppD7)
{
    *floatNumD7 = pFppD7;
    FPP_LN();

    return *floatNumD7;
}

//-----------------------------------------------------------------------------
// Float Function Return Exp
//-----------------------------------------------------------------------------
unsigned long fppExp(long pFppD7)
{
    *floatNumD7 = pFppD7;
    FPP_EXP();

    return *floatNumD7;
}

//-----------------------------------------------------------------------------
// Float Function Return ABS
//-----------------------------------------------------------------------------
unsigned long fppAbs(long pFppD7)
{
    *floatNumD7 = pFppD7;
    FPP_ABS();

    return *floatNumD7;
}

//-----------------------------------------------------------------------------
// Float Function Return Neg
//-----------------------------------------------------------------------------
unsigned long fppNeg(long pFppD7)
{
    *floatNumD7 = pFppD7;
    FPP_NEG();

    return *floatNumD7;
}

//-----------------------------------------------------------------------------
// Float Function to Comp 2 float values D7-D6
//-----------------------------------------------------------------------------
unsigned long fppComp(unsigned long pFppD7, unsigned long pFppD6)
{
    *floatNumD7 = pFppD7;
    *floatNumD6 = pFppD6;

    FPP_CMP();

    return *floatNumD7;
}

//-----------------------------------------------------------------------------
// Processa Parametros do comando/funcao Basic
// Parametros:
//      tipoRetorno: 0 - Valor Final, 1 - Nome Variavel
//      temParenteses: 1 - tem, 0 - nao
//      qtdParam: Quanto parametros tem 1 a 255
//      tipoParams: Array com o tipo de cada param ($, % e #)ex: 3 params = [$,%,%]
//      retParams: Pointer para o retorno dos parametros para a função Utilizar
//-----------------------------------------------------------------------------
int procParam(unsigned char tipoRetorno, unsigned char temParenteses, unsigned char tipoSeparador, unsigned char qtdParam, unsigned char *tipoParams,  unsigned char *retParams)
{
    int ix, iy;
    unsigned char answer[200], varTipo, vTipoParam;
    char last_delim, last_token_type = 0;
    unsigned char sqtdtam[10];
    long *vConvVal;
    long *vValor = answer;
    unsigned char *vTempRetParam = retParams;

    nextToken();
    if (*vErroProc) return 0;

    // Se obriga parenteses, primeiro caracter deve ser abre parenteses
    if (temParenteses)
    {
        if (*tok == EOL || *tok == FINISHED || *token_type != OPENPARENT)
        {
            *vErroProc = 15;
            return 0;
        }

        nextToken();
        if (*vErroProc) return 0;
    }

    if (qtdParam == 255)
        *retParams++ = 0x00;

    for (ix = 0; ix < qtdParam; ix++)
    {
        if (qtdParam < 255)
            vTipoParam = tipoParams[ix];
        else
            vTipoParam = tipoParams[0];

        if (tipoRetorno == 0)
        {
            // Valor Final
            if (*token_type == QUOTE)  /* se o parametro nao pedir string, error */
            {
                if (vTipoParam != '$')
                {
                    *vErroProc = 16;
                    return 0;
                }

                // Transfere a String pro retorno do parametro
                iy = 0;
                while (token[iy])
                    *retParams++ = token[iy++];

                *retParams++ = 0x00;
            }
            else
            {
                /* is expression */
                last_token_type = *token_type;

                putback();

                getExp(&answer);
                if (*vErroProc) return 0;

                if (*value_type == '$')
                {
                    if (vTipoParam != '$')   /* se o parametro nao pedir string, error */
                    {
                        *vErroProc = 16;
                        return 0;
                    }

                    // Transfere a String pro retorno do parametro
                    iy = 0;
                    while (answer[iy])
                        *retParams++ = answer[iy++];

                    *retParams++ = 0x00;
                }
                else
                {
                    if (vTipoParam == '$')   /* se nao é uma string, mas o parametro pedir string, error */
                    {
                        *vErroProc = 16;
                        return 0;
                    }

                    // Converter aqui pro valor solicitado (de int pra dec e dec pra int). @ = nao converte
                    if (vTipoParam != '@' && vTipoParam != *value_type)
                    {
                        if (vTipoParam == '%')
                            vConvVal = fppInt(*vValor);
                        else
                            vConvVal = fppReal(*vValor);

                        *vValor = vConvVal;
                    }

                    // Transfere o numero gerado para o retorno do parametro
                    *retParams++ = answer[0];
                    *retParams++ = answer[1];
                    *retParams++ = answer[2];
                    *retParams++ = answer[3];

                    // Se for @, o proximo byte desse valor é o tipo
                    if (vTipoParam == '@')
                        *retParams++ = *value_type;
                }
            }
        }
        else
        {
            // Nome Variavel
            if (!isalphas(*token)) {
                *vErroProc = 4;
                return 0;
            }

            if (strlen(token) < 3)
            {
                *varName = *token;
                varTipo = VARTYPEDEFAULT;

                if (strlen(token) == 2 && *(token + 1) < 0x30)
                    varTipo = *(token + 1);

                if (strlen(token) == 2 && isalphas(*(token + 1)))
                    *(varName + 1) = *(token + 1);
                else
                    *(varName + 1) = 0x00;

                *(varName + 2) = varTipo;
            }
            else
            {
                *varName = *token;
                *(varName + 1) = *(token + 1);
                *(varName + 2) = *(token + 2);
                varTipo = *(varName + 2);
            }

            answer[0] = varTipo;
        }

        if ((ix + 1) != qtdParam)
        {
            // Verifica se tem separador
            if (tipoSeparador == 0 && qtdParam != 255)
            {
                *vErroProc = 27;
                return 0;
            }

            nextToken();
            if (*vErroProc) return 0;

            // Se for um separador diferente do definido
            if (*token != tipoSeparador)
            {
                // Se for qtd definida, erro
                if (qtdParam != 255)
                {
                    *vErroProc = 18;
                    return 0;
                }
                else
                {
                    *vTempRetParam = (ix + 1);
                    break;
                }
            }

            nextToken();
            if (*vErroProc) return 0;
        }
    }

    last_delim = *token;

    if (temParenteses)
    {
        if (qtdParam == 1)
        {
            nextToken();
            if (*vErroProc) return 0;
        }

        // Ultimo caracter deve ser fecha parenteses
        if (*token_type != CLOSEPARENT)
        {
            *vErroProc = 15;
            return 0;
        }
    }

    if (qtdParam != 1 && tipoRetorno == 0)
    {
        if (*token != 0xBA && *token != 0x86)   // AT and TO token's
        {
            nextToken();
            if (*vErroProc) return 0;

            if (*token == ':' || *token == tipoSeparador)
                putback();
        }
    }

    return 0;
}

//-----------------------------------------------------------------------------
// FUNCOES BASIC
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Joga pra tela Texto.
// Syntaxe:
//      Print "<Texto>"/<value>[, "<Texto>"/<value>][; "<Texto>"/<value>]
//-----------------------------------------------------------------------------
int basPrint(void)
{
    unsigned char vAspas = 0, vVirgula = 0, vTemp[250];
    char sNumLin [sizeof(short)*8+1];
    int ix = 0, iy = 0, iz = 0, iw = 0, vToken;
    unsigned char answer[200];
    long *lVal = answer;
    int  *iVal = answer;
    int len=0, spaces;
    char last_delim, last_token_type = 0;
    unsigned char sqtdtam[10];

    do {
        nextToken();
        if (*vErroProc) return 0;

        if (*tok == EOL || *tok == FINISHED)
            break;

        if (*token_type == QUOTE) { // is string
            printText(token);

            nextToken();
            if (*vErroProc) return 0;
        }
        else if (*token!=':') { // is expression
            last_token_type = *token_type;

            putback();

            getExp(&answer);
            if (*vErroProc) return 0;

            if (*value_type != '$')
            {
                if (*value_type == '#')
                {
                    // Real
                    fppTofloatString(*lVal, answer);
                    if (*vErroProc) return 0;
                }
                else
                {
                    // Inteiro
                    itoa(*iVal, answer, 10);
                }
            }

            printText(answer);

            nextToken();
            if (*vErroProc) return 0;
        }

        last_delim = *token;

        if (*token==',') {
            // compute number of spaces to move to next tab
            spaces = 8 - (len % 8);
            while(spaces) {
                printChar(' ',1);
                spaces--;
            }
        }
        else if (*token==';' || *token=='+')
            /* do nothing */;
        else if (*token==':')
        {
            *pointerRunProg = *pointerRunProg - 1;
        }
        else if (*tok!=EOL && *tok!=FINISHED && *token!=':')
        {
            *vErroProc = 14;
            return 0;
        }
    } while (*token==';' || *token==',' || *token=='+');

    if (*tok == EOL || *tok == FINISHED || *token==':') {
        if (last_delim != ';' && last_delim!=',')
            printText("\r\n");
    }

    return 0;
}

//-----------------------------------------------------------------------------
// Devolve o caracter ligado ao codigo ascii passado
// Syntaxe:
//      CHR$(<codigo ascii>)
//-----------------------------------------------------------------------------
int basChr(void)
{
    unsigned char vAspas = 0, vVirgula = 0, vTemp[250];
    char sNumLin [sizeof(short)*8+1];
    int ix = 0, iy = 0, iz = 0, iw = 0, vToken;
    unsigned char answer[10];
    long *lVal = answer;
    int  *iVal = answer;
    int len=0, spaces;
    char last_delim, last_token_type = 0;
    unsigned char sqtdtam[10];

    nextToken();
    if (*vErroProc) return 0;

    // Erro, primeiro caracter deve ser abre parenteses
    if (*tok == EOL || *tok == FINISHED || *token_type != OPENPARENT)
    {
        *vErroProc = 15;
        return 0;
    }

    nextToken();
    if (*vErroProc) return 0;

    if (*token_type == QUOTE) { /* is string, error */
        *vErroProc = 16;
        return 0;
    }
    else { /* is expression */
        last_token_type = *token_type;

        putback();

        getExp(&answer);
        if (*vErroProc) return 0;

        if (*value_type == '$')
        {
            *vErroProc = 16;
            return 0;
        }

        if (*value_type == '#')
        {
            *iVal = fppInt(*iVal);
            *value_type = '%';
        }

        // Inteiro
        if (*iVal<0 || *iVal>255)
        {
            *vErroProc = 5;
            return 0;
        }
    }

    last_delim = *token;

    nextToken();
    if (*vErroProc) return 0;

    // Ultimo caracter deve ser fecha parenteses
    if (*token_type!=CLOSEPARENT)
    {
        *vErroProc = 15;
        return 0;
    }

    *token=(char)*iVal;
    *(token + 1)=0x00;
    *value_type='$';

    return 0;
}

//-----------------------------------------------------------------------------
// Devolve o numerico da string
// Syntaxe:
//      VAL(<string>)
//-----------------------------------------------------------------------------
int basVal(void)
{
    unsigned char vAspas = 0, vVirgula = 0, vTemp[250];
    char sNumLin [sizeof(short)*8+1];
    int ix = 0, iy = 0, iz = 0, iw = 0, vToken;
    unsigned char answer[20];
    int  iVal = answer;
    int vValue = 0;
    int len=0, spaces;
    char last_delim, last_value_type=' ', last_token_type = 0;
    unsigned char sqtdtam[10];

    nextToken();
    if (*vErroProc) return 0;

    // Erro, primeiro caracter deve ser abre parenteses
    if (*tok == EOL || *tok == FINISHED || *token_type != OPENPARENT)
    {
        *vErroProc = 15;
        return 0;
    }

    nextToken();
    if (*vErroProc) return 0;

    if (*token_type == QUOTE) { /* is string, error */
        if (strchr(token,'.'))  // verifica se eh numero inteiro ou real
        {
            last_value_type='#'; // Real
            iVal=floatStringToFpp(token);
            if (*vErroProc) return 0;
        }
        else
        {
            last_value_type='%'; // Inteiro
            iVal=atoi(token);
        }
    }
    else { /* is expression */
        last_token_type = *token_type;

        putback();

        getExp(&answer);
        if (*vErroProc) return 0;

        if (*value_type != '$')
        {
            *vErroProc = 16;
            return 0;
        }

        if (strchr(answer,'.'))  // verifica se eh numero inteiro ou real
        {
            last_value_type='#'; // Real
            iVal=floatStringToFpp(answer);
            if (*vErroProc) return 0;
        }
        else
        {
            last_value_type='%'; // Inteiro
            iVal=atoi(answer);
        }
    }

    last_delim = *token;

    nextToken();
    if (*vErroProc) return 0;

    // Ultimo caracter deve ser fecha parenteses
    if (*token_type!=CLOSEPARENT)
    {
        *vErroProc = 15;
        return 0;
    }

    *token=((int)(iVal & 0xFF000000) >> 24);
    *(token + 1)=((int)(iVal & 0x00FF0000) >> 16);
    *(token + 2)=((int)(iVal & 0x0000FF00) >> 8);
    *(token + 3)=(iVal & 0x000000FF);

    *value_type = last_value_type;

    return 0;
}

//-----------------------------------------------------------------------------
// Devolve a string do numero
// Syntaxe:
//      STR$(<Numero>)
//-----------------------------------------------------------------------------
int basStr(void)
{
    unsigned char vAspas = 0, vVirgula = 0, vTemp[250];
    char sNumLin [sizeof(short)*8+1];
    int ix = 0, iy = 0, iz = 0, iw = 0, vToken;
    unsigned char answer[50];
    long *lVal = answer;
    int  *iVal = answer;
    int len=0, spaces;
    char last_delim, last_token_type = 0;
    unsigned char sqtdtam[10];

    nextToken();
    if (*vErroProc) return 0;

    // Erro, primeiro caracter deve ser abre parenteses
    if (*tok == EOL || *tok == FINISHED || *token_type != OPENPARENT)
    {
        *vErroProc = 15;
        return 0;
    }

    nextToken();
    if (*vErroProc) return 0;

    if (*token_type == QUOTE) { /* is string, error */
        *vErroProc = 16;
        return 0;
    }
    else { /* is expression */
        last_token_type = *token_type;

        putback();

        getExp(&answer);
        if (*vErroProc) return 0;

        if (*value_type == '$')
        {
            *vErroProc = 16;
            return 0;
        }
    }

    last_delim = *token;

    nextToken();
    if (*vErroProc) return 0;

    // Ultimo caracter deve ser fecha parenteses
    if (*token_type!=CLOSEPARENT)
    {
        *vErroProc = 15;
        return 0;
    }

    if (*value_type=='#')    // real
    {
        fppTofloatString(*iVal,token);
        if (*vErroProc) return 0;
    }
    else    // Inteiro
    {
        itoa(*iVal,token,10);
    }

    *value_type='$';

    return 0;
}

//-----------------------------------------------------------------------------
// Devolve o tamanho da string
// Syntaxe:
//      LEN(<string>)
//-----------------------------------------------------------------------------
int basLen(void)
{
    unsigned char vAspas = 0, vVirgula = 0, vTemp[250];
    char sNumLin [sizeof(short)*8+1];
    int ix = 0, iy = 0, iz = 0, iw = 0, vToken;
    unsigned char answer[200];
    int iVal = 0;
    int vValue = 0;
    int len=0, spaces;
    char last_delim, last_token_type = 0;
    unsigned char sqtdtam[10];

    nextToken();
    if (*vErroProc) return 0;

    // Erro, primeiro caracter deve ser abre parenteses
    if (*tok == EOL || *tok == FINISHED || *token_type != OPENPARENT)
    {
        *vErroProc = 15;
        return 0;
    }

    nextToken();
    if (*vErroProc) return 0;

    if (*token_type == QUOTE) { /* is string, error */
        iVal=strlen(token);
    }
    else { /* is expression */
        last_token_type = *token_type;

        putback();

        getExp(&answer);
        if (*vErroProc) return 0;

        if (*value_type != '$')
        {
            *vErroProc = 16;
            return 0;
        }

        iVal=strlen(answer);
    }

    last_delim = *token;

    nextToken();
    if (*vErroProc) return 0;

    // Ultimo caracter deve ser fecha parenteses
    if (*token_type!=CLOSEPARENT)
    {
        *vErroProc = 15;
        return 0;
    }

    *token=((int)(iVal & 0xFF000000) >> 24);
    *(token + 1)=((int)(iVal & 0x00FF0000) >> 16);
    *(token + 2)=((int)(iVal & 0x0000FF00) >> 8);
    *(token + 3)=(iVal & 0x000000FF);

    *value_type='%';

    return 0;
}

//-----------------------------------------------------------------------------
// Devolve qtd memoria usuario disponivel
// Syntaxe:
//      FRE(0)
//-----------------------------------------------------------------------------
int basFre(void)
{
    unsigned char vAspas = 0, vVirgula = 0, vTemp[250];
    char sNumLin [sizeof(short)*8+1];
    int ix = 0, iy = 0, iz = 0, iw = 0, vToken;
    unsigned char answer[50];
    long *lVal = answer;
    int  *iVal = answer;
    long vTotal = 0;
    char vbuffer [sizeof(long)*8+1];
    int len=0, spaces;
    char last_delim;
    unsigned char sqtdtam[10];

    nextToken();
    if (*vErroProc) return 0;

    // Erro, primeiro caracter deve ser abre parenteses
    if (*tok == EOL || *tok == FINISHED || *token_type != OPENPARENT)
    {
        *vErroProc = 15;
        return 0;
    }

    nextToken();
    if (*vErroProc) return 0;

    if (*token_type == QUOTE) { /* is string, error */
        *vErroProc = 16;
        return 0;
    }
    else { /* is expression */
        putback();

        getExp(&answer);
        if (*vErroProc) return 0;

        if (*iVal!=0)
        {
            *vErroProc = 5;
            return 0;
        }
    }

    last_delim = *token;

    nextToken();
    if (*vErroProc) return 0;

    // Ultimo caracter deve ser fecha parenteses
    if (*token_type!=CLOSEPARENT)
    {
        *vErroProc = 15;
        return 0;
    }

    // Calcula Quantidade de Memoria e printa na tela
    vTotal = (pStartArrayVar - pStartSimpVar) + (pStartString - pStartArrayVar);
/*    printText("Memory Free for: \r\n\0");
     ltoa(vTotal, vbuffer, 10);
    printText("     Variables: \0");
    printText(vbuffer);
    printText("Bytes\r\n\0");

    vTotal = pStartProg - *nextAddrArrayVar;
    ltoa(vTotal, vbuffer, 10);
    printText("        Arrays: \0");
    printText(vbuffer);
    printText("Bytes\r\n\0");

    vTotal = pStartXBasLoad - *nextAddrLine;
    ltoa(vTotal, vbuffer, 10);
    printText("       Program: \0");
    printText(vbuffer);
    printText("Bytes\r\n\0");*/

    *token=((int)(vTotal & 0xFF000000) >> 24);
    *(token + 1)=((int)(vTotal & 0x00FF0000) >> 16);
    *(token + 2)=((int)(vTotal & 0x0000FF00) >> 8);
    *(token + 3)=(vTotal & 0x000000FF);

    *value_type='%';

    return 0;
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
int basTrig(unsigned char pFunc)
{
    unsigned long vReal = 0, vResult = 0;

    nextToken();
    if (*vErroProc) return 0;

    // Erro, primeiro caracter deve ser abre parenteses
    if (*tok == EOL || *tok == FINISHED || *token_type != OPENPARENT)
    {
        *vErroProc = 15;
        return 0;
    }

    nextToken();
    if (*vErroProc) return 0;

    putback();

    getExp(&vReal); //

    if (*value_type == '$')
    {
        *vErroProc = 16;
        return 0;
    }
    else if (*value_type != '#')
    {
        *value_type='#'; // Real
        vReal=fppReal(vReal);
    }

    nextToken();
    if (*vErroProc) return 0;

    // Ultimo caracter deve ser fecha parenteses
    if (*token_type!=CLOSEPARENT)
    {
        *vErroProc = 15;
        return 0;
    }

    switch (pFunc)
    {
        case 1: // sin
            vResult = fppSin(vReal);
            break;
        case 2: // cos
            vResult = fppCos(vReal);
            break;
        case 3: // tan
            vResult = fppTan(vReal);
            break;
        case 4: // log (ln)
            vResult = fppLn(vReal);
            break;
        case 5: // exp
            vResult = fppExp(vReal);
            break;
        case 6: // sqrt
            vResult = fppSqrt(vReal);
            break;
        default:
            *vErroProc = 14;
            return 0;
    }


    *token=((int)(vResult & 0xFF000000) >> 24);
    *(token + 1)=((int)(vResult & 0x00FF0000) >> 16);
    *(token + 2)=((int)(vResult & 0x0000FF00) >> 8);
    *(token + 3)=(vResult & 0x000000FF);

    *value_type = '#';

    return 0;
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
int basAsc(void)
{
    unsigned char answer[20];
    int  iVal = answer;
    char last_delim;

    nextToken();
    if (*vErroProc) return 0;

    // Erro, primeiro caracter deve ser abre parenteses
    if (*tok == EOL || *tok == FINISHED || *token_type != OPENPARENT)
    {
        *vErroProc = 15;
        return 0;
    }

    nextToken();
    if (*vErroProc) return 0;

    if (*token_type == QUOTE) { /* is string, error */
        if (strlen(token)>1)
        {
            *vErroProc = 6;
            return 0;
        }

        iVal = *token;
    }
    else { /* is expression */
        putback();

        getExp(&answer);
        if (*vErroProc) return 0;

        if (*value_type != '$')
        {
            *vErroProc = 16;
            return 0;
        }

        iVal = *answer;
    }

    last_delim = *token;

    nextToken();
    if (*vErroProc) return 0;

    // Ultimo caracter deve ser fecha parenteses
    if (*token_type!=CLOSEPARENT)
    {
        *vErroProc = 15;
        return 0;
    }

    *token=((int)(iVal & 0xFF000000) >> 24);
    *(token + 1)=((int)(iVal & 0x00FF0000) >> 16);
    *(token + 2)=((int)(iVal & 0x0000FF00) >> 8);
    *(token + 3)=(iVal & 0x000000FF);

    *value_type = '%';

    return 0;
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
int basLeftRightMid(char pTipo)
{
    int ix = 0, iy = 0, iz = 0, iw = 0, vToken;
    unsigned char answer[200], vTemp[200];
    int vqtd = 0, vstart = 0;
    unsigned char sqtdtam[10];

    nextToken();
    if (*vErroProc) return 0;

    // Erro, primeiro caracter deve ser abre parenteses
    if (*tok == EOL || *tok == FINISHED || *token_type != OPENPARENT)
    {
        *vErroProc = 15;
        return 0;
    }

    nextToken();
    if (*vErroProc) return 0;

    if (*token_type == QUOTE) { /* is string, error */
        strcpy(vTemp, token);
    }
    else { /* is expression */
        putback();

        getExp(&answer);
        if (*vErroProc) return 0;

        if (*value_type != '$')
        {
            *vErroProc = 16;
            return 0;
        }

        strcpy(vTemp, answer);
    }

    nextToken();
    if (*vErroProc) return 0;

    // Deve ser uma virgula para Receber a qtd, e se for mid = a posiao incial
    if (*token!=',')
    {
        *vErroProc = 18;
        return 0;
    }

    nextToken();
    if (*vErroProc) return 0;

    if (*token_type == QUOTE) { /* is string, error */
        *vErroProc = 16;
        return 0;
    }
    else { /* is expression */
        putback();

        if (pTipo=='M')
        {
            getExp(&vstart);
            vqtd=strlen(vTemp);
        }
        else
            getExp(&vqtd);

        if (*vErroProc) return 0;

        if (*value_type == '$')
        {
            *vErroProc = 16;
            return 0;
        }
    }

    if (pTipo == 'M')
    {
        // Deve ser uma virgula para Receber a qtd
        if (*token==',')
        {
            nextToken();
            if (*vErroProc) return 0;

            if (*token_type == QUOTE) { /* is string, error */
                *vErroProc = 16;
                return 0;
            }
            else { /* is expression */
                //putback();

                getExp(&vqtd);

                if (*vErroProc) return 0;

                if (*value_type == '$')
                {
                    *vErroProc = 16;
                    return 0;
                }
            }
        }
    }

    nextToken();
    if (*vErroProc) return 0;

    // Ultimo caracter deve ser fecha parenteses
    if (*token_type!=CLOSEPARENT)
    {
        *vErroProc = 15;
        return 0;
    }

    if (vqtd > strlen(vTemp))
    {
        if (pTipo=='M')
            vqtd = (strlen(vTemp) - vstart) + 1;
        else
            vqtd = strlen(vTemp);
    }

    if (pTipo == 'L') // Left$
    {
        for (ix = 0; ix < vqtd; ix++)
            *(token + ix) = vTemp[ix];
        *(token + ix) = 0x00;
    }
    else if (pTipo == 'R') // Right$
    {
        iy = strlen(vTemp);
        iz = (iy - vqtd);
        iw = 0;
        for (ix = iz; ix < iy; ix++)
            *(token + iw++) = vTemp[ix];
        *(token + iw)=0x00;
    }
    else  // Mid$
    {
        iy = strlen(vTemp);
        iw=0;
        vstart--;

        for (ix = vstart; ix < iy; ix++)
        {
            if (iw <= iy && vqtd-- > 0)
                *(token + iw++) = vTemp[ix];
            else
                break;
        }

        *(token + iw) = 0x00;
    }

    *value_type = '$';

    return 0;
}

//--------------------------------------------------------------------------------------
//  Comandos de memoria
//      Leitura de Memoria:   peek(<endereco>)
//      Gravacao em endereco: poke(<endereco>,<byte>)
//--------------------------------------------------------------------------------------
int basPeekPoke(char pTipo)
{
    int ix = 0, iy = 0, iz = 0, iw = 0, vToken;
    unsigned char answer[30], vTemp[30];
    unsigned char *vEnd = 0;
    unsigned int vByte = 0;
    unsigned char sqtdtam[10];

    nextToken();
    if (*vErroProc) return 0;

    // Erro, primeiro caracter deve ser abre parenteses
    if (*tok == EOL || *tok == FINISHED || *token_type != OPENPARENT)
    {
        *vErroProc = 15;
        return 0;
    }

    nextToken();
    if (*vErroProc) return 0;

    if (*token_type == QUOTE) { /* is string, error */
        *vErroProc = 16;
        return 0;
    }
    else { /* is expression */
        putback();

        getExp(&vEnd);

        if (*vErroProc) return 0;

        if (*value_type == '$')
        {
            *vErroProc = 16;
            return 0;
        }
    }

    // Deve ser uma virgula para Receber a qtd
    if (pTipo == 'W')
    {
        if (*token==',')
        {
            nextToken();
            if (*vErroProc) return 0;

            if (*token_type == QUOTE) { /* is string, error */
                *vErroProc = 16;
                return 0;
            }
            else { /* is expression */
                //putback();

                getExp(&vByte);

                if (*vErroProc) return 0;

                if (*value_type == '$')
                {
                    *vErroProc = 16;
                    return 0;
                }
            }
        }
    }

    nextToken();
    if (*vErroProc) return 0;

    // Ultimo caracter deve ser fecha parenteses
    if (*token_type!=CLOSEPARENT)
    {
        *vErroProc = 15;
        return 0;
    }

    if (pTipo == 'R')
    {
        *token = 0;
        *(token + 1) = 0;
        *(token + 2) = 0;
        *(token + 3) = *vEnd;
    }
    else
    {
        *vEnd = (char)vByte;
    }

    *value_type = '%';

    return 0;
}


//--------------------------------------------------------------------------------------
//  Array (min 1 dimensoes)
//      Sintaxe:
//              DIM (<dim 1>[,<dim 2>[,<dim 3>,<dim 4>,...,<dim n>])
//--------------------------------------------------------------------------------------
int basDim(void)
{
    int ix = 0, iy = 0, iz = 0, iw = 0, vToken;
    unsigned char answer[30], vTemp[30];
    unsigned char sqtdtam[10];
    unsigned int vDim[88], ixDim = 0, vTempDim = 0;
    unsigned char varTipo;
    long vRetFV;

    nextToken();
    if (*vErroProc) return 0;

    // Pega o nome da variavel
    if (!isalphas(*token)) {
        *vErroProc = 4;
        return 0;
    }

    if (strlen(token) < 3)
    {
        *varName = *token;
        varTipo = VARTYPEDEFAULT;

        if (strlen(token) == 2 && *(token + 1) < 0x30)
            varTipo = *(token + 1);

        if (strlen(token) == 2 && isalphas(*(token + 1)))
            *(varName + 1) = *(token + 1);
        else
            *(varName + 1) = 0x00;

        *(varName + 2) = varTipo;
    }
    else
    {
        *varName = *token;
        *(varName + 1) = *(token + 1);
        *(varName + 2) = *(token + 2);
        iz = strlen(token) - 1;
        varTipo = *(varName + 2);
    }

    nextToken();
    if (*vErroProc) return 0;

    // Erro, primeiro caracter depois da variavel, deve ser abre parenteses
    if (*tok == EOL || *tok == FINISHED || *token_type != OPENPARENT)
    {
        *vErroProc = 15;
        return 0;
    }

    do
    {
        nextToken();
        if (*vErroProc) return 0;

        if (*token_type == QUOTE) { /* is string, error */
            *vErroProc = 16;
            return 0;
        }
        else { /* is expression */
            putback();

            getExp(&vTempDim);
            if (*vErroProc) return 0;

            if (*value_type == '$')
            {
                *vErroProc = 16;
                return 0;
            }

            if (*value_type == '#')
            {
                vTempDim = fppInt(vTempDim);
                *value_type = '%';
            }

            vTempDim += 1; // porque nao é de 1 a x, é de 0 a x, entao é x + 1
            vDim[ixDim] = vTempDim;

            ixDim++;
        }

        if (*token == ',')
        {
            *pointerRunProg = *pointerRunProg + 1;
        }
        else
            break;
    } while(1);

    // Deve ter pelo menos 1 elemento
    if (ixDim < 1)
    {
        *vErroProc = 21;
        return 0;
    }

    nextToken();
    if (*vErroProc) return 0;

    // Ultimo caracter deve ser fecha parenteses
    if (*token_type!=CLOSEPARENT)
    {
        *vErroProc = 15;
        return 0;
    }

    // assign the value
    vRetFV = findVariable(varName);
    // Se nao existe a variavel, cria variavel e atribui o valor
    if (!vRetFV)
        createVariableArray(varName, varTipo, ixDim, vDim);
    else
    {
        *vErroProc = 23;
        return 0;
    }

    return 0;
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
int basIf(void)
{
    unsigned int vCond = 0;
    unsigned char *vTempPointer;

    getExp(&vCond); // get target value

    if (*value_type == '$' || *value_type == '#') {
        *vErroProc = 16;
        return 0;
    }

    nextToken();
    if (*vErroProc) return 0;

    if (*token!=0x83)
    {
        *vErroProc = 8;
        return 0;
    }

    if (vCond)
    {
        // Vai pro proximo comando apos o Then e continua
        *pointerRunProg = *pointerRunProg + 1;

        // simula ":" para continuar a execucao
        *doisPontos = 1;
    }
    else
    {
        // Ignora toda a linha
        vTempPointer = *pointerRunProg;
        while (*vTempPointer)
        {
            *pointerRunProg = *pointerRunProg + 1;
            vTempPointer = *pointerRunProg;
        }
    }

    return 0;
}

//--------------------------------------------------------------------------------------
// Atribuir valor a uma variavel/array - comando opcional.
// Syntaxe:
//            [LET] <variavel/array(x[,y])> = <string/valor>
//--------------------------------------------------------------------------------------
int basLet(void)
{
    long vRetFV, iz;
    unsigned char varTipo;
    unsigned char value[200];
    unsigned long *lValue = &value;
    unsigned char sqtdtam[10];
    unsigned char vArray = 0;
    unsigned char *vTempPointer;

    /* get the variable name */
    nextToken();
    if (*vErroProc) return 0;

    if (!isalphas(*token)) {
        *vErroProc = 4;
        return 0;
    }

    if (strlen(token) < 3)
    {
        *varName = *token;
        varTipo = VARTYPEDEFAULT;

        if (strlen(token) == 2 && *(token + 1) < 0x30)
            varTipo = *(token + 1);

        if (strlen(token) == 2 && isalphas(*(token + 1)))
            *(varName + 1) = *(token + 1);
        else
            *(varName + 1) = 0x00;

        *(varName + 2) = varTipo;
    }
    else
    {
        *varName = *token;
        *(varName + 1) = *(token + 1);
        *(varName + 2) = *(token + 2);
        iz = strlen(token) - 1;
        varTipo = *(varName + 2);
    }

    // verifica se é array (abre parenteses no inicio)
    vTempPointer = *pointerRunProg;
    if (*vTempPointer == 0x28)
    {
        vRetFV = findVariable(varName);
        if (*vErroProc) return 0;

        if (!vRetFV)
        {
            *vErroProc = 4;
            return 0;
        }

        vArray = 1;
    }

    // get the equals sign
    nextToken();
    if (*vErroProc) return 0;

    if (*token!='=') {
        *vErroProc = 3;
        return 0;
    }
    /* get the value to assign to varName */
    getExp(&value);

    if (varTipo == '#' && *value_type != '#')
        *lValue = fppReal(*lValue);

    // assign the value
    if (!vArray)
    {
        vRetFV = findVariable(varName);
        // Se nao existe a variavel, cria variavel e atribui o valor
        if (!vRetFV)
            createVariable(varName, value, varTipo);
        else // se ja existe, altera
            updateVariable((vRetFV + 3), value, varTipo, 1);
    }
    else
    {
        updateVariable(vRetFV, value, varTipo, 2);
    }

    return 0;
}

//--------------------------------------------------------------------------------------
// Entrada pelo teclado de numeros/caracteres ateh teclar ENTER (INPUT)
// Entrada pelo teclado de um unico caracter ou numero (GET)
// Entrada dos dados de acordo com o tipo de variavel $(qquer), %(Nums), #(Nums & '.')
// Syntaxe:
//          INPUT ["texto",]<variavel> : A variavel sera criada se nao existir
//          GET <variavel> : A variavel sera criada se nao existir
//--------------------------------------------------------------------------------------
int basInputGet(unsigned char pSize)
{
    unsigned char vAspas = 0, vVirgula = 0, vTemp[250];
    char sNumLin [sizeof(short)*8+1];
    int ix = 0, iy = 0, iz = 0, iw = 0, vToken;
    unsigned char answer[200], vtec;
    long *lVal = answer;
    int  *iVal = answer;
    char vTemTexto = 0;
    int len=0, spaces;
    char last_delim;
    unsigned char *buffptr = vbuf;
    long vRetFV;
    unsigned char varTipo;
    char vArray = 0;
    unsigned char *vTempPointer;

    do {
        nextToken();
        if (*vErroProc) return 0;

        if (*tok == EOL || *tok == FINISHED)
            break;

        if (*token_type == QUOTE) /* is string */
        {
            if (vTemTexto)
            {
                *vErroProc = 14;
                return 0;
            }

            printText(token);

            nextToken();
            if (*vErroProc) return 0;

            vTemTexto = 1;
        }
        else /* is expression */
        {
            // Verifica se comeca com letra, pois tem que ser uma variavel agora
            if (!isalphas(*token))
            {
                *vErroProc = 4;
                return 0;
            }

            if (strlen(token) < 3)
            {
                *varName = *token;
                varTipo = VARTYPEDEFAULT;

                if (strlen(token) == 2 && *(token + 1) < 0x30)
                    varTipo = *(token + 1);

                if (strlen(token) == 2 && isalphas(*(token + 1)))
                    *(varName + 1) = *(token + 1);
                else
                    *(varName + 1) = 0x00;

                *(varName + 2) = varTipo;
            }
            else
            {
                *varName = *token;
                *(varName + 1) = *(token + 1);
                *(varName + 2) = *(token + 2);
                iz = strlen(token) - 1;
                varTipo = *(varName + 2);
            }

            answer[0] = 0x00;
            *vbuf = 0x00;

            if (pSize == 1)
            {
                // GET
                for (ix = 0; ix < 15000; ix++)
                {
                    vtec = *vBufReceived;
                    if (vtec)
                        break;
                }
                *vBufReceived = 0x00;
//                vtec = inputLine(1,'@');    // Qualquer coisa

                if (varTipo != '$' && vtec)
                {
                    if (!isdigitus(vtec))
                        vtec = 0;
                }

                answer[0] = vtec;
                answer[1] = 0x00;
            }
            else
            {
                // INPUT
                vtec = inputLine(255,varTipo);

                if (*vbuf != 0x00 && (vtec == 0x0D || vtec == 0x0A))
                {
                    ix = 0;

                    while (*buffptr)
                    {
                        answer[ix++] = *buffptr++;
                        answer[ix] = 0x00;
                    }
                }

                printText("\r\n");
            }

            if (varTipo!='$')
            {
                if (varTipo=='#')  // verifica se eh numero inteiro ou real
                {
                    iVal=floatStringToFpp(answer);
                    if (*vErroProc) return 0;
                }
                else
                {
                    iVal=atoi(answer);
                }

                answer[0]=((int)(*iVal & 0xFF000000) >> 24);
                answer[1]=((int)(*iVal & 0x00FF0000) >> 16);
                answer[2]=((int)(*iVal & 0x0000FF00) >> 8);
                answer[3]=(char)(*iVal & 0x000000FF);
            }

            vTempPointer = *pointerRunProg;
            if (*vTempPointer == 0x28)
            {
                vRetFV = findVariable(varName);
                if (*vErroProc) return 0;

                if (!vRetFV)
                {
                    *vErroProc = 4;
                    return 0;
                }

                vArray = 1;
            }

            if (!vArray)
            {
                // assign the value
                vRetFV = findVariable(varName);

                // Se nao existe variavel e inicio sentenca, cria variavel e atribui o valor
                if (!vRetFV)
                    createVariable(varName, answer, varTipo);
                else // se ja existe, altera
                    updateVariable((vRetFV + 3), answer, varTipo, 1);
            }
            else
            {
                updateVariable(vRetFV, answer, varTipo, 2);
            }

            vTemTexto=2;
            nextToken();
            if (*vErroProc) return 0;
        }

        last_delim = *token;

        if (vTemTexto==1 && *token==';')
            /* do nothing */;
        else if (vTemTexto==1 && *token!=';')
        {
            *vErroProc = 14;
            return 0;
        }
        else if (vTemTexto!=1 && *token==';')
        {
            *vErroProc = 14;
            return 0;
        }
        else if (*tok!=EOL && *tok!=FINISHED && *token!=':')
        {
            *vErroProc = 14;
            return 0;
        }
    } while (*token==';');

    return 0;
}

//--------------------------------------------------------------------------------------
char forFind(for_stack *i, unsigned char* endLastVar)
{
    int ix;
    unsigned char sqtdtam[10];
    for_stack *j;

    j = forStack;

    for(ix = 0; ix < *ftos; ix++)
    {
        if (j[ix].nameVar[0] == endLastVar[1] && j[ix].nameVar[1] == endLastVar[2])
        {
            *i = j[ix];

            return ix;
        }
        else if (!j[ix].nameVar[0])
            return -1;
    }

    return -1;
}

//--------------------------------------------------------------------------------------
// Inicio do laco de repeticao
// Syntaxe:
//          FOR <variavel> = <inicio> TO <final> [STEP <passo>] : A variavel sera criada se nao existir
//--------------------------------------------------------------------------------------
int basFor(void)
{
    for_stack i, *j;
    int value=0;
    long *endVarCont;
    long iStep = 1;
    long iTarget = 0;
    unsigned char* endLastVar;
    unsigned char sqtdtam[10];
    char vRetVar = -1;
    unsigned char *vTempPointer;
    char vResLog1 = 0, vResLog2 = 0;
    char vResLog3 = 0, vResLog4 = 0;

    basLet();
    if (*vErroProc) return 0;

    endLastVar = *atuVarAddr - 3;
    endVarCont = *atuVarAddr + 1;

    vRetVar = forFind(&i, endLastVar);

    if (vRetVar < 0)
    {
        i.nameVar[0]=endLastVar[1];
        i.nameVar[1]=endLastVar[2];
        i.nameVar[2]=endLastVar[0];
    }

    if (i.nameVar[2] == '#')
        i.step = fppReal(iStep);
    else
        i.step = iStep;

    i.endVar = endVarCont;

    nextToken();
    if (*vErroProc) return 0;

    if (*tok!=0x86) /* read and discard the TO */
    {
        *vErroProc = 9;
        return 0;
    }

    *pointerRunProg = *pointerRunProg + 1;

    getExp(&iTarget); /* get target value */

    if (i.nameVar[2] == '#' && *value_type == '%')
        i.target = fppReal(iTarget);
    else
        i.target = iTarget;

    if (*tok==0x88) /* read STEP */
    {
        *pointerRunProg = *pointerRunProg + 1;

        getExp(&iStep); /* get target value */

        if (i.nameVar[2] == '#' && *value_type == '%')
            i.step = fppReal(iStep);
        else
            i.step = iStep;
    }

    endVarCont=i.endVar;

    // if loop can execute at least once, push info on stack     //    if ((i.step > 0 && *endVarCont <= i.target) || (i.step < 0 && *endVarCont >= i.target))
    if (i.nameVar[2] == '#')
    {
        vResLog1 = logicalNumericFloatLong(0xF6 /* <= */, *endVarCont, i.target);
        vResLog2 = logicalNumericFloatLong(0xF5 /* >= */, *endVarCont, i.target);
        vResLog3 = logicalNumericFloatLong('>', i.step, 0);
        vResLog4 = logicalNumericFloatLong('<', i.step, 0);
    }
    else
    {
        vResLog1 = (*endVarCont <= i.target);
        vResLog2 = (*endVarCont >= i.target);
        vResLog3 = (i.step > 0);
        vResLog4 = (i.step < 0);
    }

    if (vResLog3 && vResLog1 || (vResLog4 && vResLog2))
    {
        vTempPointer = *pointerRunProg;
        if (*vTempPointer==0x3A) // ":"
        {
            i.progPosPointerRet = *pointerRunProg;
        }
        else
            i.progPosPointerRet = *nextAddr;

        if (vRetVar < 0)
            forPush(i);
        else
        {
            j = (forStack + vRetVar);
            j->target = i.target;
            j->step = i.step;
            j->endVar = i.endVar;
            j->progPosPointerRet = i.progPosPointerRet;
        }
    }
    else  /* otherwise, skip loop code alltogether */
    {
        vTempPointer = *pointerRunProg;
        while(*vTempPointer != 0x87) // Search NEXT
        {
            *pointerRunProg = *pointerRunProg + 1;
            vTempPointer = *pointerRunProg;

            // Verifica se chegou no next
            if (*vTempPointer == 0x87)
            {
                // Verifica se tem letra, se nao tiver, usa ele
                if (*(vTempPointer + 1)!=0x00)
                {
                    // verifica se é a mesma variavel que ele tem
                    if (*(vTempPointer + 1) != i.nameVar[0])
                    {
                        *pointerRunProg = *pointerRunProg + 1;
                        vTempPointer = *pointerRunProg;
                    }
                    else
                    {
                        if (*(vTempPointer + 2) != i.nameVar[1] && *(vTempPointer + 2) != i.nameVar[2])
                        {
                            *pointerRunProg = *pointerRunProg + 1;
                            vTempPointer = *pointerRunProg;
                        }
                    }
                }
            }
        }
    }

    return 0;
}

//--------------------------------------------------------------------------------------
// Final/Incremento do Laco de repeticao, voltando para o commando/linha após o FOR
// Syntaxe:
//          NEXT [<variavel>]
//--------------------------------------------------------------------------------------
int basNext(void)
{
    unsigned char sqtdtam[10];
    for_stack i;
    int *endVarCont;
    unsigned char answer[3];
    char vRetVar = -1;
    unsigned char *vTempPointer;
    char vResLog1 = 0, vResLog2 = 0;
    char vResLog3 = 0, vResLog4 = 0;

/*writeLongSerial("Aqui 777.666.0-[");
itoa(*pointerRunProg,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]-[");
itoa(*pointerRunProg,sqtdtam,16);
writeLongSerial(sqtdtam);
writeLongSerial("]\r\n");*/
    vTempPointer = *pointerRunProg;
    if (isalphas(*vTempPointer))
    {
        // procura pela variavel no forStack
        nextToken();
        if (*vErroProc) return 0;

        if (*token_type != VARIABLE)
        {
            *vErroProc = 4;
            return 0;
        }

        answer[1] = *token;

        if (strlen(token) == 1)
        {
            answer[0] = 0x00;
            answer[2] = 0x00;
        }
        else if (strlen(token) == 2)
        {
            if (*(token + 1) < 0x30)
            {
                answer[0] = *(token + 1);
                answer[2] = 0x00;
            }
            else
            {
                answer[0] = 0x00;
                answer[2] = *(token + 1);
            }
        }
        else
        {
            answer[0] = *(token + 2);
            answer[2] = *(token + 1);
        }

        vRetVar = forFind(&i,answer);

        if (vRetVar < 0)
        {
            *vErroProc = 11;
            return 0;
        }
    }
    else // faz o pop da pilha
        i = forPop(); // read the loop info

    endVarCont = i.endVar;

    if (i.nameVar[2] == '#')
    {
        *endVarCont = fppSum(*endVarCont,i.step); // inc/dec, using step, control variable
    }
    else
        *endVarCont = *endVarCont + i.step; // inc/dec, using step, control variable

    if (i.nameVar[2] == '#')
    {
        vResLog1 = logicalNumericFloatLong('>', *endVarCont, i.target);
        vResLog2 = logicalNumericFloatLong('<', *endVarCont, i.target);
        vResLog3 = logicalNumericFloatLong('>', i.step, 0);
        vResLog4 = logicalNumericFloatLong('<', i.step, 0);
    }
    else
    {
        vResLog1 = (*endVarCont > i.target);
        vResLog2 = (*endVarCont < i.target);
        vResLog3 = (i.step > 0);
        vResLog4 = (i.step < 0);
    }


    // compara se ja chegou no final  //     if ((i.step > 0 && *endVarCont>i.target) || (i.step < 0 && *endVarCont<i.target))
    if ((vResLog3 && vResLog1) || (vResLog4 && vResLog2))
        return 0 ;  // all done

    *changedPointer = i.progPosPointerRet;  // loop

    if (vRetVar < 0)
        forPush(i);  // otherwise, restore the info

    return 0;
}

//--------------------------------------------------------------------------------------
// Salta para uma linha se erro
// Syntaxe:
//          ON <VAR> GOSUB <num.linha 1>,<num.linha 2>,...,,<num.linha n>
//          ON <VAR> GOTO <num.linha 1>,<num.linha 2>,...,<num.linha n>
//--------------------------------------------------------------------------------------
int basOnVar(void)
{
    unsigned char* vNextAddrGoto;
    unsigned int vNumLin = 0;
    unsigned char *vTempPointer;
    unsigned int vSalto;
    unsigned int iSalto = 0;
    unsigned int ix;

    vTempPointer = *pointerRunProg;
    if (isalphas(*vTempPointer))
    {
        // procura pela variavel no forStack
        nextToken();
        if (*vErroProc) return 0;

        if (*token_type != VARIABLE)
        {
            *vErroProc = 4;
            return 0;
        }

        putback();

        getExp(&iSalto);
        if (*vErroProc) return 0;

        if (*value_type != '%')
        {
            *vErroProc = 16;
            return 0;
        }

        if (iSalto == 0 || iSalto > 255)
        {
            *vErroProc = 5;
            return 0;
        }
    }
    else
    {
        *vErroProc = 4;
        return 0;
    }

    vTempPointer = *pointerRunProg;

    // Se nao for goto ou gosub, erro
    if (*vTempPointer != 0x89 && *vTempPointer != 0x8A)
    {
        *vErroProc = 14;
        return 0;
    }

    vSalto = *vTempPointer;
    ix = 0;
    *pointerRunProg = *pointerRunProg + 1;

    while (1)
    {
        getExp(&vNumLin); // get target value

        if (*value_type == '$' || *value_type == '#') {
            *vErroProc = 16;
            return 0;
        }

        ix++;

        if (ix == iSalto)
            break;

        nextToken();
        if (*vErroProc) return 0;

        // Deve ser uma virgula
        if (*token!=',')
        {
            *vErroProc = 18;
            return 0;
        }

        nextToken();
        if (*vErroProc) return 0;

        putback();
    }

    if (ix == 0 || ix > iSalto)
    {
        *vErroProc = 14;
        return 0;
    }

    vNextAddrGoto = findNumberLine(vNumLin, 0, 0);

    if (vSalto == 0x89)
    {
        // GOTO
        if (vNextAddrGoto > 0)
        {
            if ((unsigned int)(((unsigned int)*(vNextAddrGoto + 3) << 8) | *(vNextAddrGoto + 4)) == vNumLin)
            {
                *changedPointer = vNextAddrGoto;
                return 0;
            }
            else
            {
                *vErroProc = 7;
                return 0;
            }
        }
        else
        {
            *vErroProc = 7;
            return 0;
        }
    }
    else
    {
        // GOSUB
        if (vNextAddrGoto > 0)
        {
            if ((unsigned int)(((unsigned int)*(vNextAddrGoto + 3) << 8) | *(vNextAddrGoto + 4)) == vNumLin)
            {
                gosubPush(*nextAddr);
                *changedPointer = vNextAddrGoto;
                return 0;
            }
            else
            {
                *vErroProc = 7;
                return 0;
            }
        }
        else
        {
            *vErroProc = 7;
            return 0;
        }
    }

    return 0;
}

//--------------------------------------------------------------------------------------
// Salta para uma linha se erro
// Syntaxe:
//          ONERR GOTO <num.linha>
//--------------------------------------------------------------------------------------
int basOnErr(void)
{
    unsigned char* vNextAddrGoto;
    unsigned int vNumLin = 0;
    unsigned char sqtdtam[10];
    unsigned char *vTempPointer;

    vTempPointer = *pointerRunProg;

    // Se nao for goto, erro
    if (*vTempPointer != 0x89)
    {

        *vErroProc = 14;
        return 0;
    }

    // soma mais um pra ir pro numero da linha
    *pointerRunProg = *pointerRunProg + 1;

    getExp(&vNumLin); // get target value

    if (*value_type == '$' || *value_type == '#') {
        *vErroProc = 17;
        return 0;
    }

    vNextAddrGoto = findNumberLine(vNumLin, 0, 0);

    if (vNextAddrGoto > 0)
    {
        if ((unsigned int)(((unsigned int)*(vNextAddrGoto + 3) << 8) | *(vNextAddrGoto + 4)) == vNumLin)
        {
            *onErrGoto = vNextAddrGoto;
            return 0;
        }
        else
        {
            *vErroProc = 7;
            return 0;
        }
    }
    else
    {
        *vErroProc = 7;
        return 0;
    }

    return 0;
}

//--------------------------------------------------------------------------------------
// Salta para uma linha, sem retorno
// Syntaxe:
//          GOTO <num.linha>
//--------------------------------------------------------------------------------------
int basGoto(void)
{
    unsigned char* vNextAddrGoto;
    unsigned int vNumLin = 0;
    unsigned char sqtdtam[10];

    nextToken();
    if (*vErroProc) return 0;

    // Erro, primeiro caracter deve ser abre parenteses
    if (*tok == EOL || *tok == FINISHED)
    {
        *vErroProc = 14;
        return 0;
    }

    putback();

    getExp(&vNumLin); // get target value

    if (*value_type == '$' || *value_type == '#') {
        *vErroProc = 17;
        return 0;
    }

    vNextAddrGoto = findNumberLine(vNumLin, 0, 0);

    if (vNextAddrGoto > 0)
    {
        if ((unsigned int)(((unsigned int)*(vNextAddrGoto + 3) << 8) | *(vNextAddrGoto + 4)) == vNumLin)
        {
            *changedPointer = vNextAddrGoto;
            return 0;
        }
        else
        {
            *vErroProc = 7;
            return 0;
        }
    }
    else
    {
        *vErroProc = 7;
        return 0;
    }

    return 0;
}

//--------------------------------------------------------------------------------------
// Salta para uma linha e guarda a posicao atual para voltar
// Syntaxe:
//          GOSUB <num.linha>
//--------------------------------------------------------------------------------------
int basGosub(void)
{
    unsigned char* vNextAddrGoto;
    unsigned int vNumLin = 0;

    nextToken();
    if (*vErroProc) return 0;

    // Erro, primeiro caracter deve ser abre parenteses
    if (*tok == EOL || *tok == FINISHED)
    {
        *vErroProc = 14;
        return 0;
    }

    putback();

    getExp(&vNumLin); // get target valuedel 20

    if (*value_type == '$' || *value_type == '#') {
        *vErroProc = 17;
        return 0;
    }

    vNextAddrGoto = findNumberLine(vNumLin, 0, 0);

    if (vNextAddrGoto > 0)
    {
        if ((unsigned int)(((unsigned int)*(vNextAddrGoto + 3) << 8) | *(vNextAddrGoto + 4)) == vNumLin)
        {
            gosubPush(*nextAddr);
            *changedPointer = vNextAddrGoto;
            return 0;
        }
        else
        {
            *vErroProc = 7;
            return 0;
        }
    }
    else
    {
        *vErroProc = 7;
        return 0;
    }

    return 0;
}

//--------------------------------------------------------------------------------------
// Retorna de um Gosub
// Syntaxe:
//          RETURN
//--------------------------------------------------------------------------------------
int basReturn(void)
{
    unsigned long i;

    i = gosubPop();

    *changedPointer = i;

    return 0;
}

//--------------------------------------------------------------------------------------
// Retorna um numero real como inteiro
// Syntaxe:
//          INT(<number real>)
//--------------------------------------------------------------------------------------
int basInt(void)
{
    int vReal = 0, vResult = 0;

    nextToken();
    if (*vErroProc) return 0;

    // Erro, primeiro caracter deve ser abre parenteses
    if (*tok == EOL || *tok == FINISHED || *token_type != OPENPARENT)
    {
        *vErroProc = 15;
        return 0;
    }

    nextToken();
    if (*vErroProc) return 0;

    putback();

    getExp(&vReal); //

    if (*value_type == '$')
    {
        *vErroProc = 16;
        return 0;
    }

    nextToken();
    if (*vErroProc) return 0;

    // Ultimo caracter deve ser fecha parenteses
    if (*token_type!=CLOSEPARENT)
    {
        *vErroProc = 15;
        return 0;
    }

    if (*value_type == '#')
        vResult = fppInt(vReal);
    else
        vResult = vReal;

    *value_type='%';

    *token=((int)(vResult & 0xFF000000) >> 24);
    *(token + 1)=((int)(vResult & 0x00FF0000) >> 16);
    *(token + 2)=((int)(vResult & 0x0000FF00) >> 8);
    *(token + 3)=(vResult & 0x000000FF);

    return 0;
}

//--------------------------------------------------------------------------------------
// Retorna um numero absoluto como inteiro
// Syntaxe:
//          ABS(<number real>)
//--------------------------------------------------------------------------------------
int basAbs(void)
{
    int vReal = 0, vResult = 0;

    nextToken();
    if (*vErroProc) return 0;

    // Erro, primeiro caracter deve ser abre parenteses
    if (*tok == EOL || *tok == FINISHED || *token_type != OPENPARENT)
    {
        *vErroProc = 15;
        return 0;
    }

    nextToken();
    if (*vErroProc) return 0;

    putback();

    getExp(&vReal); //

    if (*value_type == '$')
    {
        *vErroProc = 16;
        return 0;
    }

    nextToken();
    if (*vErroProc) return 0;

    // Ultimo caracter deve ser fecha parenteses
    if (*token_type!=CLOSEPARENT)
    {
        *vErroProc = 15;
        return 0;
    }

    if (*value_type == '#')
        vResult = fppAbs(vReal);
    else
    {
        vResult = vReal;

        if (vResult < 1)
            vResult = vResult * (-1);
    }

    *value_type='%';

    *token=((int)(vResult & 0xFF000000) >> 24);
    *(token + 1)=((int)(vResult & 0x00FF0000) >> 16);
    *(token + 2)=((int)(vResult & 0x0000FF00) >> 8);
    *(token + 3)=(vResult & 0x000000FF);

    return 0;
}

//--------------------------------------------------------------------------------------
// Retorna um numero randomicamente
// Syntaxe:
//          RND(<number>)
//--------------------------------------------------------------------------------------
int basRnd(void)
{
    unsigned long vRand;
    int vReal = 0, vResult = 0;
    unsigned char vTRand[20];
    unsigned char vSRand[20];

    nextToken();
    if (*vErroProc) return 0;

    // Erro, primeiro caracter deve ser abre parenteses
    if (*tok == EOL || *tok == FINISHED || *token_type != OPENPARENT)
    {
        *vErroProc = 15;
        return 0;
    }

    nextToken();
    if (*vErroProc) return 0;

    putback();

    getExp(&vReal); //

    if (*value_type == '$')
    {
        *vErroProc = 16;
        return 0;
    }

    nextToken();
    if (*vErroProc) return 0;

    // Ultimo caracter deve ser fecha parenteses
    if (*token_type != CLOSEPARENT)
    {
        *vErroProc = 15;
        return 0;
    }

    if (vReal == 0)
    {
        vRand = *randSeed;
    }
    else if (vReal >= -1 && vReal < 0)
    {
        vRand = *(vmfp + Reg_TADR);
        vRand = (vRand << 3);
        vRand += 0x466;
        vRand -= ((*(vmfp + Reg_TADR)) * 3);
        *randSeed = vRand;
    }
    else if (vReal > 0 && vReal <= 1)
    {
        vRand = *randSeed;
        vRand = (vRand << 3);
        vRand += 0x466;
        vRand -= ((*(vmfp + Reg_TADR)) * 3);
        *randSeed = vRand;
    }
    else
    {
        *vErroProc = 5;
        return 0;
    }

    itoa(vRand, vTRand, 10);
    vSRand[0] = '0';
    vSRand[1] = '.';
    vSRand[2] = 0x00;

    strcat(vSRand, vTRand);

    vRand = floatStringToFpp(vSRand);

    *value_type='#';

    *token=((int)(vRand & 0xFF000000) >> 24);
    *(token + 1)=((int)(vRand & 0x00FF0000) >> 16);
    *(token + 2)=((int)(vRand & 0x0000FF00) >> 8);
    *(token + 3)=(vRand & 0x000000FF);

    return 0;
}

//--------------------------------------------------------------------------------------
// Seta posicao vertical (linha em texto e y em grafico)
// Syntaxe:
//          VTAB <numero>
//--------------------------------------------------------------------------------------
int basVtab(void)
{
    unsigned int vRow = 0;

    getExp(&vRow);

    if (*value_type == '$') {
        *vErroProc = 16;
        return 0;
    }

    if (*value_type == '#')
    {
        vRow = fppInt(vRow);
        *value_type = '%';
    }

    vdp_set_cursor(*videoCursorPosColX, vRow);

    return 0;
}

//--------------------------------------------------------------------------------------
// Seta posicao horizontal (coluna em texto e x em grafico)
// Syntaxe:
//          HTAB <numero>
//--------------------------------------------------------------------------------------
int basHtab(void)
{
    unsigned int vColumn = 0;

    getExp(&vColumn);

    if (*value_type == '$') {
        *vErroProc = 16;
        return 0;
    }

    if (*value_type == '#')
    {
        vColumn = fppInt(vColumn);
        *value_type = '%';
    }

    vdp_set_cursor(vColumn, *videoCursorPosRowY);

    return 0;
}

//--------------------------------------------------------------------------------------
// Finaliza o programa sem erro
// Syntaxe:
//          END
//--------------------------------------------------------------------------------------
int basEnd(void)
{
    *nextAddr = 0;

    return 0;
}

//--------------------------------------------------------------------------------------
// Finaliza o programa com erro
// Syntaxe:
//          STOP
//--------------------------------------------------------------------------------------
int basStop(void)
{
    *vErroProc = 1;

    return 0;
}

//--------------------------------------------------------------------------------------
// Retorna 'n' Espaços
// Syntaxe:
//          SPC <numero>
//--------------------------------------------------------------------------------------
int basSpc(void)
{
    unsigned int vSpc = 0;
    int ix = 0, iy = 0, iz = 0, iw = 0, vToken;
    unsigned char answer[20];
    int  *iVal = answer;
    unsigned char vTab, vColumn;
    unsigned char sqtdtam[10];

    nextToken();
    if (*vErroProc) return 0;

    // Erro, primeiro caracter deve ser abre parenteses
    if (*tok == EOL || *tok == FINISHED || *token_type != OPENPARENT)
    {
        *vErroProc = 15;
        return 0;
    }

    nextToken();
    if (*vErroProc) return 0;

    if (*token_type == QUOTE) { /* is string, error */
        *vErroProc = 16;
        return 0;
    }
    else { /* is expression */
        putback();

        getExp(&answer);
        if (*vErroProc) return 0;

        if (*value_type == '$')
        {
            *vErroProc = 16;
            return 0;
        }

        if (*value_type == '#')
        {
            *iVal = fppInt(*iVal);
            *value_type = '%';
        }
    }

    nextToken();
    if (*vErroProc) return 0;

    // Ultimo caracter deve ser fecha parenteses
    if (*token_type!=CLOSEPARENT)
    {
        *vErroProc = 15;
        return 0;
    }

    vSpc=(char)*iVal;

    for (ix = 0; ix < vSpc; ix++)
        *(token + ix) = ' ';

    *(token + ix) = 0;
    *value_type = '$';

    return 0;
}

//--------------------------------------------------------------------------------------
// Advance 'n' columns
// Syntaxe:
//          TAB <numero>
//--------------------------------------------------------------------------------------
int basTab(void)
{
    int ix = 0, iy = 0, iz = 0, iw = 0, vToken;
    unsigned char answer[20];
    int  *iVal = answer;
    unsigned char vTab, vColumn;
    unsigned char sqtdtam[10];

    nextToken();
    if (*vErroProc) return 0;

    // Erro, primeiro caracter deve ser abre parenteses
    if (*tok == EOL || *tok == FINISHED || *token_type != OPENPARENT)
    {
        *vErroProc = 15;
        return 0;
    }

    nextToken();
    if (*vErroProc) return 0;

    if (*token_type == QUOTE) { /* is string, error */
        *vErroProc = 16;
        return 0;
    }
    else { /* is expression */
        putback();

        getExp(&answer);
        if (*vErroProc) return 0;

        if (*value_type == '$')
        {
            *vErroProc = 16;
            return 0;
        }

        if (*value_type == '#')
        {
            *iVal = fppInt(*iVal);
            *value_type = '%';
        }
    }

    nextToken();
    if (*vErroProc) return 0;

    // Ultimo caracter deve ser fecha parenteses
    if (*token_type!=CLOSEPARENT)
    {
        *vErroProc = 15;
        return 0;
    }

    vTab=(char)*iVal;

    vColumn = *videoCursorPosColX;

    if (vTab>vColumn)
    {
        vColumn = vColumn + vTab;

        while (vColumn>*vdpMaxCols)
        {
            vColumn = vColumn - *vdpMaxCols;
            if (*videoCursorPosRowY < *vdpMaxRows)
                *videoCursorPosRowY += 1;
        }

        vdp_set_cursor(vColumn, *videoCursorPosRowY);
    }

    *token = ' ';
    *value_type='$';

    return 0;
}

//--------------------------------------------------------------------------------------
// Text Screen Mode (40 cols x 24 rows)
// Syntaxe:
//          TEXT
//--------------------------------------------------------------------------------------
int basText(void)
{
    *fgcolor = VDP_WHITE;
    *bgcolor = VDP_BLACK;
    vdp_init(VDP_MODE_TEXT, (*fgcolor<<4) | (*bgcolor & 0x0f), 0, 0);
    clearScr();
    return 0;
}

//--------------------------------------------------------------------------------------
// Low Resolution Screen Mode (64x48)
// Syntaxe:
//          GR
//--------------------------------------------------------------------------------------
int basGr(void)
{
    vdp_init(VDP_MODE_MULTICOLOR, 0, 0, 0);
    return 0;
}

//--------------------------------------------------------------------------------------
// High Resolution Screen Mode (256x192)
// Syntaxe:
//          HGR
//--------------------------------------------------------------------------------------
int basHgr(void)
{
    vdp_init(VDP_MODE_G2, 0x0, 1, 0);
    vdp_set_bdcolor(VDP_BLACK);
    return 0;
}

//--------------------------------------------------------------------------------------
// Inverte as Cores de tela (COR FRENTE <> COR NORMAL)
// Syntaxe:
//          INVERSE
//
//    **********************************************************************************
//    ** SOMENTE PARA COMPATIBILIDADE, NO TMS91xx E TMS99xx NAO FUNCIONA COR POR CHAR **
//    **********************************************************************************
//--------------------------------------------------------------------------------------
int basInverse(void)
{
/*    unsigned char vTempCor;

    *fgcolorAnt = *fgcolor;
    *bgcolorAnt = *bgcolor;
    vTempCor = *fgcolor;
    *fgcolor = *bgcolor;
    *bgcolor = vTempCor;
    vdp_textcolor(*fgcolor,*bgcolor);*/

    return 0;
}

//--------------------------------------------------------------------------------------
// Volta as cores de tela as cores iniciais
// Syntaxe:
//          NORMAL
//
//    **********************************************************************************
//    ** SOMENTE PARA COMPATIBILIDADE, NO TMS91xx E TMS99xx NAO FUNCIONA COR POR CHAR **
//    **********************************************************************************
//--------------------------------------------------------------------------------------
int basNormal(void)
{
/*    *fgcolor = *fgcolorAnt;
    *bgcolor = *bgcolorAnt;
    vdp_textcolor(*fgcolor,*bgcolor);*/

    return 0;
}

//--------------------------------------------------------------------------------------
// Muda a cor do plot em baixa/alta resolucao (GR or HGR from basHcolor)
// Syntaxe:
//          COLOR=<color>
//--------------------------------------------------------------------------------------
int basColor(void)
{
    int ix = 0, iy = 0, iz = 0, iw = 0, vToken;
    unsigned char answer[20];
    int  *iVal = answer;
    unsigned char vTab, vColumn;
    unsigned char sqtdtam[10];
    unsigned char *vTempPointer;

    if (*vdp_mode != VDP_MODE_MULTICOLOR && *vdp_mode != VDP_MODE_G2)
    {
        *vErroProc = 24;
        return 0;
    }

    vTempPointer = *pointerRunProg;
    if (*vTempPointer != '=')
    {
        *vErroProc = 3;
        return 0;
    }

    *pointerRunProg = *pointerRunProg + 1;
    vTempPointer = *pointerRunProg;

    nextToken();
    if (*vErroProc) return 0;

    if (*token_type == QUOTE) { /* is string, error */
        *vErroProc = 16;
        return 0;
    }
    else { /* is expression */
        putback();

        getExp(&answer);
        if (*vErroProc) return 0;

        if (*value_type == '$')
        {
            *vErroProc = 16;
            return 0;
        }

        if (*value_type == '#')
        {
            *iVal = fppInt(*iVal);
            *value_type = '%';
        }
    }

    *fgcolor=(char)*iVal;

    *value_type='%';

    return 0;
}

//--------------------------------------------------------------------------------------
// Coloca um dot ou preenche uma area com a color previamente definida
// Syntaxe:
//          PLOT <x entre 0 e 63>, <y entre 0 e 47>
//--------------------------------------------------------------------------------------
int basPlot(void)
{
    int ix = 0, iy = 0, iz = 0, iw = 0, vToken;
    unsigned char answer[20];
    int  *iVal = answer;
    unsigned char vx, vy;
    unsigned char sqtdtam[10];

    if (*vdp_mode != VDP_MODE_MULTICOLOR)
    {
        *vErroProc = 24;
        return 0;
    }

    nextToken();
    if (*vErroProc) return 0;

    if (*token_type == QUOTE) { /* is string, error */
        *vErroProc = 16;
        return 0;
    }
    else { /* is expression */
        putback();

        getExp(&answer);
        if (*vErroProc) return 0;

        if (*value_type == '$')
        {
            *vErroProc = 16;
            return 0;
        }

        if (*value_type == '#')
        {
            *iVal = fppInt(*iVal);
            *value_type = '%';
        }
    }

    vx=(char)*iVal;

    if (*token != ',')
    {
        *vErroProc = 18;
        return 0;
    }

    nextToken();
    if (*vErroProc) return 0;

    if (*token_type == QUOTE) { /* is string, error */
        *vErroProc = 16;
        return 0;
    }
    else { /* is expression */
        //putback();

        getExp(&answer);
        if (*vErroProc) return 0;

        if (*value_type == '$')
        {
            *vErroProc = 16;
            return 0;
        }

        if (*value_type == '#')
        {
            *iVal = fppInt(*iVal);
            *value_type = '%';
        }
    }

    vy=(char)*iVal;

    vdp_plot_color(vx, vy, *fgcolor);

    *value_type='%';

    return 0;
}

//--------------------------------------------------------------------------------------
// Desenha uma linha horizontal de x1, y1 até x2, y1
// Syntaxe:
//          HLIN <x1>, <x2> at <y1>
//               x1 e x2 : de 0 a 63
//                    y1 : de 0 a 47
//
// Desenha uma linha vertical de x1, y1 até x1, y2
// Syntaxe:
//          VLIN <y1>, <y2> at <x1>
//                    x1 : de 0 a 63
//               y1 e y2 : de 0 a 47
//--------------------------------------------------------------------------------------
int basHVlin(unsigned char vTipo)   // 1 - HLIN, 2 - VLIN
{
    int ix = 0, iy = 0, iz = 0, iw = 0, vToken;
    unsigned char answer[20];
    int  *iVal = answer;
    unsigned char vx1, vx2, vy;
    unsigned char sqtdtam[10];

    if (*vdp_mode != VDP_MODE_MULTICOLOR)
    {
        *vErroProc = 24;
        return 0;
    }

    nextToken();
    if (*vErroProc) return 0;

    if (*token_type == QUOTE) { /* is string, error */
        *vErroProc = 16;
        return 0;
    }
    else { /* is expression */
        putback();

        getExp(&answer);
        if (*vErroProc) return 0;

        if (*value_type == '$')
        {
            *vErroProc = 16;
            return 0;
        }

        if (*value_type == '#')
        {
            *iVal = fppInt(*iVal);
            *value_type = '%';
        }
    }

    vx1=(char)*iVal;

    if (*token != ',')
    {
        *vErroProc = 18;
        return 0;
    }

    nextToken();
    if (*vErroProc) return 0;

    if (*token_type == QUOTE) { /* is string, error */
        *vErroProc = 16;
        return 0;
    }
    else { /* is expression */
        //putback();

        getExp(&answer);
        if (*vErroProc) return 0;

        if (*value_type == '$')
        {
            *vErroProc = 16;
            return 0;
        }

        if (*value_type == '#')
        {
            *iVal = fppInt(*iVal);
            *value_type = '%';
        }
    }

    vx2=(char)*iVal;

    if (*token != 0xBA) // AT Token
    {
        *vErroProc = 18;
        return 0;
    }

    *pointerRunProg = *pointerRunProg + 1;

    nextToken();
    if (*vErroProc) return 0;

    if (*token_type == QUOTE) { /* is string, error */
        *vErroProc = 16;
        return 0;
    }
    else { /* is expression */
        putback();

        getExp(&answer);
        if (*vErroProc) return 0;

        if (*value_type == '$')
        {
            *vErroProc = 16;
            return 0;
        }

        if (*value_type == '#')
        {
            *iVal = fppInt(*iVal);
            *value_type = '%';
        }
    }

    vy=(char)*iVal;

    if (vx2 < vx1)
    {
        ix = vx1;
        vx1 = vx2;
        vx2 = ix;
    }

    if (vTipo == 1)   // HLIN
    {
        for(ix = vx1; ix <= vx2; ix++)
            vdp_plot_color(ix, vy, *fgcolor);
    }
    else   // VLIN
    {
        for(ix = vx1; ix <= vx2; ix++)
            vdp_plot_color(vy, ix, *fgcolor);
    }
    *value_type='%';

    return 0;
}

//--------------------------------------------------------------------------------------
//
// Syntaxe:
//
//--------------------------------------------------------------------------------------
int basScrn(void)
{
    int ix = 0, iy = 0, iz = 0, iw = 0, vToken;
    unsigned char answer[20];
    int *iVal = answer;
    int *tval = token;
    unsigned char vx, vy;
    unsigned char sqtdtam[10];

    if (*vdp_mode != VDP_MODE_MULTICOLOR)
    {
        *vErroProc = 24;
        return 0;
    }

    nextToken();
    if (*vErroProc) return 0;

    // Erro, primeiro caracter deve ser abre parenteses
    if (*tok == EOL || *tok == FINISHED || *token_type != OPENPARENT)
    {
        *vErroProc = 15;
        return 0;
    }

    nextToken();
    if (*vErroProc) return 0;

    if (*token_type == QUOTE) { /* is string, error */
        *vErroProc = 16;
        return 0;
    }
    else { /* is expression */
        putback();

        getExp(&answer);
        if (*vErroProc) return 0;

        if (*value_type != '%')
        {
            *vErroProc = 16;
            return 0;
        }
    }

    vx=(char)*iVal;

    nextToken();
    if (*vErroProc) return 0;

    if (*token!=',')
    {
        *vErroProc = 18;
        return 0;
    }

    nextToken();
    if (*vErroProc) return 0;

    if (*token_type == QUOTE) { /* is string, error */
        *vErroProc = 16;
        return 0;
    }
    else { /* is expression */
        putback();

        getExp(&answer);
        if (*vErroProc) return 0;

        if (*value_type != '%')
        {
            *vErroProc = 16;
            return 0;
        }
    }

    vy=(char)*iVal;

    nextToken();
    if (*vErroProc) return 0;

    // Ultimo caracter deve ser fecha parenteses
    if (*token_type!=CLOSEPARENT)
    {
        *vErroProc = 15;
        return 0;
    }

    // Ler Aqui.. a cor e devolver em *tval
    *tval = vdp_read_color_pixel(vx,vy);

    *value_type='%';

    return 0;
}

//--------------------------------------------------------------------------------------
//
// Syntaxe:
//
//--------------------------------------------------------------------------------------
int basHcolor(void)
{
    if (*vdp_mode != VDP_MODE_G2)
    {
        *vErroProc = 24;
        return 0;
    }

    basColor();
    if (*vErroProc) return 0;

    return 0;
}

//--------------------------------------------------------------------------------------
//
// Syntaxe:
//
//--------------------------------------------------------------------------------------
int basHplot(void)
{
    int ix = 0, iy = 0, iz = 0, iw = 0, vToken;
    unsigned char answer[20];
    int  *iVal = answer;
    int rivx, rivy;
    unsigned long riy, rlvx, rlvy, vDiag;
    unsigned char vx, vy, vtemp;
    unsigned char sqtdtam[10];
    unsigned char vOper = 0;
    int x,y,addx,addy,dx,dy;
    long P;

    if (*vdp_mode != VDP_MODE_G2)
    {
        *vErroProc = 24;
        return 0;
    }

    do
    {
        nextToken();
        if (*vErroProc) return 0;

        if (*token != 0x86)
        {
            if (*token_type == QUOTE) { // is string, error
                *vErroProc = 16;
                return 0;
            }
            else { // is expression
                putback();

                getExp(&answer);
                if (*vErroProc) return 0;

                if (*value_type == '$')
                {
                    *vErroProc = 16;
                    return 0;
                }

                if (*value_type == '#')
                {
                    *iVal = fppInt(*iVal);
                    *value_type = '%';
                }
            }

            vx = (unsigned char)*iVal;

            if (*token != ',')
            {
                *vErroProc = 18;
                return 0;
            }

            nextToken();
            if (*vErroProc) return 0;

            if (*token_type == QUOTE) { // is string, error
                *vErroProc = 16;
                return 0;
            }
            else { // is expression
                //putback();

                getExp(&answer);
                if (*vErroProc) return 0;

                if (*value_type == '$')
                {
                    *vErroProc = 16;
                    return 0;
                }

                if (*value_type == '#')
                {
                    *iVal = fppInt(*iVal);
                    *value_type = '%';
                }
            }

            vy = (unsigned char)*iVal;

            if (!vOper)
                vOper = 1;
        }
        else
        {
           // *pointerRunProg = *pointerRunProg + 1;
        }

        if (*tok == EOL || *tok == FINISHED || *token == 0x86)    // Fim de linha, programa ou token
        {
            if (!vOper)
            {
                vOper = 2;
            }
            else if (vOper == 1)
            {
                *lastHgrX = vx;
                *lastHgrY = vy;

                if (*token != 0x86)
                    vdp_plot_hires(vx, vy, *fgcolor, *bgcolor);
            }
            else if (vOper == 2)
            {
                if (vx == *lastHgrX && vy == *lastHgrY)
                    vdp_plot_hires(vx, vy, *fgcolor, *bgcolor);
                else
                {
                    dx = (vx - *lastHgrX);
                    dy = (vy - *lastHgrY);

                    if (dx < 0)
                        dx = dx * (-1);

                    if (dy < 0)
                        dy = dy * (-1);

                    x = *lastHgrX;
                    y = *lastHgrY;

                    if(*lastHgrX > vx)
                       addx = -1;
                    else
                       addx = 1;

                    if(*lastHgrY > vy)
                      addy = -1;
                    else
                      addy = 1;

                    if(dx >= dy)
                    {
                        P = (2 * dy) - dx;

                        for(ix = 1; ix <= (dx + 1); ix++)
                        {
                            vdp_plot_hires(x, y, *fgcolor, 0);

                            if (P < 0)
                            {
                                P = P + (2 * dy);
                                x = (x + addx);
                            }
                            else
                            {
                                P = P + (2 * dy) - (2 * dx);
                                x = x + addx;
                                y = y + addy;
                            }
                        }
                    }
                    else
                    {
                        P = (2 * dx) - dy;

                        for(ix = 1; ix <= (dy +1); ix++)
                        {
                            vdp_plot_hires(x, y, *fgcolor, 0);

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

                *lastHgrX = vx;
                *lastHgrY = vy;
            }

            if (*token == 0x86)
            {
                *pointerRunProg = *pointerRunProg + 1;
            }
        }

        vOper = 2;
   } while (*token == 0x86); // TO Token

    *value_type='%';

    return 0;
}

//--------------------------------------------------------------------------------------
// Ler dados no comando DATA
// Syntaxe:
//          READ <variavel>
//--------------------------------------------------------------------------------------
int basRead(void)
{
    int ix = 0, iy = 0, iz = 0;
    unsigned char answer[100];
    int  *iVal = answer;
    unsigned char varTipo, vArray = 0;
    unsigned char sqtdtam[10];
    unsigned long vTemp;
    unsigned char *vTempLine;
    long vRetFV;
    unsigned char *vTempPointer;

    // Pega a variavel
    nextToken();
    if (*vErroProc) return 0;

    if (*tok == EOL || *tok == FINISHED)
    {
        *vErroProc = 4;
        return 0;
    }

    if (*token_type == QUOTE) { /* is string */
        *vErroProc = 4;
        return 0;
    }
    else { /* is expression */
        // Verifica se comeca com letra, pois tem que ser uma variavel
        if (!isalphas(*token))
        {
            *vErroProc = 4;
            return 0;
        }

        if (strlen(token) < 3)
        {
            *varName = *token;
            varTipo = VARTYPEDEFAULT;

            if (strlen(token) == 2 && *(token + 1) < 0x30)
                varTipo = *(token + 1);

            if (strlen(token) == 2 && isalphas(*(token + 1)))
                *(varName + 1) = *(token + 1);
            else
                *(varName + 1) = 0x00;

            *(varName + 2) = varTipo;
        }
        else
        {
            *varName = *token;
            *(varName + 1) = *(token + 1);
            *(varName + 2) = *(token + 2);
            iz = strlen(token) - 1;
            varTipo = *(varName + 2);
        }
    }

    // Procurar Data
    if (*vDataPointer == 0)
    {
        // Primeira Leitura, procura primeira ocorrencia
        *vDataLineAtu = *addrFirstLineNumber;

        do
        {
            *vDataPointer = *vDataLineAtu;

            vTempLine = *vDataPointer;
            if (*(vTempLine + 5) == 0x98)    // Token do comando DATA é o primeiro comando da linha
            {
                *vDataPointer = (*vDataLineAtu + 6);
                *vDataFirst = *vDataLineAtu;
                break;
            }

            vTempLine = *vDataLineAtu;
            vTemp  = ((*vTempLine & 0xFF) << 16);
            vTemp |= ((*(vTempLine + 1) & 0xFF) << 8);
            vTemp |= (*(vTempLine + 2) & 0xFF);

            *vDataLineAtu = vTemp;
            vTempLine = *vDataLineAtu;

        } while (*vTempLine);
    }

    if (*vDataPointer == 0xFFFFFFFF)
    {
        *vErroProc = 26;
        return 0;
    }

    *vDataBkpPointerProg = *pointerRunProg;

    *pointerRunProg = *vDataPointer;

    nextToken();
    if (*vErroProc) return 0;

    if (*token_type == QUOTE) {
        strcpy(answer,token);
        *value_type = '$';
    }
    else { /* is expression */
        putback();

        getExp(&answer);
        if (*vErroProc) return 0;
    }

    // Pega ponteiro atual (proximo numero/char)
    *vDataPointer = *pointerRunProg + 1;

    // Devolve ponteiro anterior
    *pointerRunProg = *vDataBkpPointerProg;

    // Se nao foi virgula, é final de linha, procura proximo comando data
    if (*token != ',')
    {
        do
        {
            vTempLine = *vDataLineAtu;
            vTemp  = ((*(vTempLine) & 0xFF) << 16);
            vTemp |= ((*(vTempLine + 1) & 0xFF) << 8);
            vTemp |= (*(vTempLine + 2) & 0xFF);

            *vDataLineAtu = vTemp;
            vTempLine = *vDataLineAtu;
            if (!*vDataLineAtu)
            {
                *vDataPointer = 0xFFFFFFFF;
                break;
            }

            *vDataPointer = *vDataLineAtu;

            vTempLine = *vDataPointer;
            if (*(vTempLine + 5) == 0x98)    // Token do comando DATA é o primeiro comando da linha
            {
                *vDataPointer = (*vDataLineAtu + 6);
                break;
            }

            vTempLine = *vDataLineAtu;
        } while (*vTempLine);
    }

    if (varTipo != *value_type)
    {
        if (*value_type == '$' || varTipo == '$')
        {
            *vErroProc = 16;
            return 0;
        }

        if (*value_type == '%')
            *iVal = fppReal(*iVal);
        else
            *iVal = fppInt(*iVal);

        *value_type = varTipo;
    }

    vTempPointer = *pointerRunProg;
    if (*vTempPointer == 0x28)
    {
        vRetFV = findVariable(varName);
        if (*vErroProc) return 0;

        if (!vRetFV)
        {
            *vErroProc = 4;
            return 0;
        }

        vArray = 1;
    }

    if (!vArray)
    {
        // assign the value
        vRetFV = findVariable(varName);

        // Se nao existe variavel e inicio sentenca, cria variavel e atribui o valor
        if (!vRetFV)
            createVariable(varName, answer, varTipo);
        else // se ja existe, altera
            updateVariable((vRetFV + 3), answer, varTipo, 1);
    }
    else
    {
        updateVariable(vRetFV, answer, varTipo, 2);
    }

    return 0;
}

//--------------------------------------------------------------------------------------
// Volta ponteiro do READ para o primeiro item dos comandos DATA
// Syntaxe:
//          RESTORE
//--------------------------------------------------------------------------------------
int basRestore(void)
{
    *vDataLineAtu = *vDataFirst;
    *vDataPointer = (*vDataLineAtu + 6);

    return 0;
}
#endif