/********************************************************************************
*    Programa    : mmsjos.c
*    Objetivo    : MMSJOS - Versao vintage compatible
*    Criado em   : 11/03/2024
*    Programador : Moacir Jr.
*--------------------------------------------------------------------------------
* Data        Versao  Responsavel  Motivo
* 18/12/2024  0.1     Moacir Jr.   Criação Versão Beta
*                                  Adaptar para FAT32 com uno e SD CARD
* 25/12/2024  0.2     Moacir Jr.   Carregar Dados da Serial e Gravar no Arquivo
* 04/01/2025  0.3     Moacir Jr.   Receber pasta no nome do arquivo "<pasta>/<file>"
********************************************************************************/
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "mmsjos.h"
#include "mmsj300api.h"
#include "monitor.h"

#define versionMMSJOS "0.3"

//-----------------------------------------------------------------------------
// FAT16 Functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
DWORD fsInit(void)
{
    char verr = 0;

    if (fsMountDisk())
    {
        printDiskError(ERRO_B_OPEN_DISK);
        return ERRO_B_OPEN_DISK;
    }

    *vdiratuidx = 1;
    *vdiratu = '/';
    *(vdiratu + *vdiratuidx) = 0x00;

    return 0;
}

//-----------------------------------------------------------------------------
void fsVer(void)
{
    printText("\r\n\0");
    printText("MMSJ-OS v"versionMMSJOS);
}

//-----------------------------------------------------------------------------
void printDiskError(unsigned char pError)
{
    unsigned char sqtdtam[10];

    printText("Error: ");

    switch( pError )
    {
      case ERRO_B_FILE_NOT_FOUND    : printText("File not found"); break;
      case ERRO_B_READ_DISK         : printText("Reading disk"); break;
      case ERRO_B_WRITE_DISK        : printText("Writing disk"); break;
      case ERRO_B_OPEN_DISK         : printText("Opening disk"); break;
      case ERRO_B_INVALID_NAME      : printText("Invalid Folder or File Name"); break;
      case ERRO_B_DIR_NOT_FOUND     : printText("Directory not found"); break;
      case ERRO_B_CREATE_FILE       : printText("Creating file"); break;
      case ERRO_B_APAGAR_ARQUIVO    : printText("Deleting file"); break;
      case ERRO_B_FILE_FOUND        : printText("File already exist"); break;
      case ERRO_B_UPDATE_DIR        : printText("Updating directory"); break;
      case ERRO_B_OFFSET_READ       : printText("Offset read"); break;
      case ERRO_B_DISK_FULL         : printText("Disk full"); break;
      case ERRO_B_READ_FILE         : printText("Reading file"); break;
      case ERRO_B_WRITE_FILE        : printText("Writing file"); break;
      case ERRO_B_DIR_FOUND         : printText("Directory already exist"); break;
      case ERRO_B_CREATE_DIR        : printText("Creating directory"); break;
      case ERRO_B_NOT_FOUND         : printText("Not found"); break;
      default                       :
        itoa(pError, sqtdtam, 10);
        printText(sqtdtam);
        printText(" - Unknown Code");
        break;
    }

    printText("!\r\n\0");
}

//-----------------------------------------------------------------------------
// Main Function
//-----------------------------------------------------------------------------
void main(void)
{
    unsigned char vRetInput;
    int vRetProcCmd;

    printText("MMSJ-OS v"versionMMSJOS);
    printText("\r\n\0");

    fsInit();
    memInit();

    fsChangeDir("/");

    printText("Ok\r\n\0");
    printText("#>");

    while (1)
    {
        vRetInput = inputLine(128,'$');

        if (*vbuf != 0x00 && (vRetInput == 0x0D || vRetInput == 0x0A))
        {
            vRetProcCmd = 1;

            printText("\r\n\0");

            vRetProcCmd = fsOsCommand();

            *vBufReceived = 0x00;
            *vbuf = '\0';

            if (vRetProcCmd == 99)
                break;

            if (vRetProcCmd)
                printText("\r\n\0");

            printText("#>");
        }
        else if (vRetInput != 0x1B)
        {
            printText("\r\n\0");
            printText("#>");
        }
    }

    return;
}

//-----------------------------------------------------------------------------
char fsOsCommand(void)
{
    BYTE linhacomando[64], linhaarg[64], vloop;
    BYTE *blin = vbuf, vbuffer[128], vlinha[40];
    WORD varg = 0, vcrc, vcrcpic;
    WORD ix, iy, iz, ikk;
    WORD vbytepic = 0, vrecfim;
    BYTE *vdirptr = (BYTE*)&vdir;
    BYTE sqtdtam[10], cuntam, vparam[32], vparam2[32], vpicret;
    DWORD vretfat, vclusterdiratu, vclusterdirsrc, vclusterdirdst;
    long vqtdtam;
    BYTE izzzz, logpath = 0;
    char cTemp[128];
    BYTE vposTemp = 0, vrettype, logwildcard = 0;

    // Separar linha entre comando e argumento
    linhacomando[0] = '\0';
    linhaarg[0] = '\0';
    vparam[0] = '\0';
    vparam2[0] = '\0';
    ix = 0;
    iy = 0;
    while (*blin != 0)
    {
        if (!varg && *blin == 0x20)
        {
            varg = 0x01;
            linhacomando[ix] = '\0';
            iy = ix;
            ix = 0;
        }
        else
        {
            if (!varg)
                linhacomando[ix] = toupper(*blin);
            else
                linhaarg[ix] = toupper(*blin);
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
    {
        linhaarg[ix] = '\0';

        ikk = 0;
        iz = 0;
        varg = 0;
        while (ikk < ix)
        {
            if (linhaarg[ikk] == 0x20)
                varg = 1;
            else
            {
                if (!varg)
                    vparam[ikk] = linhaarg[ikk];
                else
                {
                    vparam2[iz] = linhaarg[ikk];
                    iz++;
                }
            }

            ikk++;
        }
    }

    vparam[ikk] = '\0';
    vparam2[iz] = '\0';

    if (linhaarg[0] == 0x00)
    {
        vparam[0] = '\0';
        vparam2[0] = '\0';
    }

    vpicret = 0;

    // Processar e definir o que fazer
    if (linhacomando[0] != 0)
    {
        if (!strcmp(linhacomando,"CLS") && iy == 3)
        {
            clearScr();
        }
        else if (!strcmp(linhacomando,"CLEAR") && iy == 5)
        {
            clearScr();
        }
        else if (!strcmp(linhacomando,"QUIT") && iy == 4)
        {
            return 99;
        }
        else if (!strcmp(linhacomando,"VER") && iy == 3)
        {
            fsVer();
        }
        else if (!strcmp(linhacomando,"PWD") && iy == 3)
        {
            printText(vdiratu);
            printText("\r\n\0");
        }
        else if ((!strcmp(linhacomando,"LS") && iy == 2) ||
                 (!strcmp(linhacomando,"RM") && iy == 2) ||
                 (!strcmp(linhacomando,"CP") && iy == 2))
        {
            vclusterdiratu = *vclusterdir;

            memcpy(vparam2, vparam, 32);

            if (vparam2[0] > 0x20)
            {
                // Acha o caminho final
                vretpath = fsFindDirPath(vparam2, FIND_PATH_LAST);

                // Verifica se tem wildcard
                logwildcard = contains_wildcards(vretpath->Name);

                // Verifica Erro
                if (vretpath == FIND_PATH_RET_ERROR && !logwildcard)
                {
                    printText("File not found.\r\n\0");
                    return 0;
                }

                *vclusterdir = vretpath->ClusterDir;

                logpath = 1;
            }

            if (fsFindInDir(NULL, TYPE_FIRST_ENTRY) >= ERRO_D_START)
            {
                printText("File not found..\r\n\0");
            }
            else
            {
                vclusterdirsrc = vretpath->ClusterDir;

                while (1)
                {
                    // Pega nome do arquivo atual
                    for (ix = 0; ix <= 7; ix++)
                    {
                        vparam[ix] = vdir->Name[ix];
                        if (vparam[ix] == 0x20)
                        {
                            vparam[ix] = '\0';
                            break;
                        }
                    }

                    vparam[ix] = '\0';

                    if (vdir->Name[0] != '.')
                    {
                        vparam[ix] = '.';
                        ix++;
                        for (iy = 0; iy <= 2; iy++)
                        {
                            vparam[ix] = vdir->Ext[iy];
                            if (vparam[ix] == 0x20)
                            {
                                vparam[ix] = '\0';
                                break;
                            }
                            ix++;
                        }
                        vparam[ix] = '\0';
                    }

                    if (!strcmp(linhacomando,"LS") && iy == 2)
                    {
                        if (vretpath == FIND_PATH_RET_FOLDER || (vretpath != FIND_PATH_RET_FOLDER && matches_wildcard(vretpath->Name, vparam)))
                        {
                            if (vdir->Attr != ATTR_VOLUME)
                            {
                                memset(vbuffer, 0x0, 128);
                                vdirptr = (BYTE*)&vdir;

                                for(ix = 40; ix <= 79; ix++)
                                    vbuffer[ix] = *vdirptr++;

                                if (vdir->Attr != ATTR_DIRECTORY)
                                {
                                    // Reduz o tamanho a unidade (GB, MB ou KB)
                                    vqtdtam = vdir->Size;

                                    if ((vqtdtam & 0xC0000000) != 0)
                                    {
                                        cuntam = 'G';
                                        vqtdtam = ((vqtdtam & 0xC0000000) >> 30) + 1;
                                    }
                                    else if ((vqtdtam & 0x3FF00000) != 0)
                                    {
                                        cuntam = 'M';
                                        vqtdtam = ((vqtdtam & 0x3FF00000) >> 20) + 1;
                                    }
                                    else if ((vqtdtam & 0x000FFC00) != 0)
                                    {
                                        cuntam = 'K';
                                        vqtdtam = ((vqtdtam & 0x000FFC00) >> 10) + 1;
                                    }
                                    else
                                        cuntam = ' ';

                                    // Transforma para decimal
                                    memset(sqtdtam, 0x0, 10);
                                    itoa(vqtdtam, sqtdtam, 10);

                                    // Primeira Parte da Linha do dir, tamanho
                                    for(ix = 0; ix <= 3; ix++)
                                    {
                                        if (sqtdtam[ix] == 0)
                                            break;
                                    }

                                    iy = (4 - ix);

                                    for(ix = 0; ix <= 3; ix++)
                                    {
                                        if (iy <= ix)
                                        {
                                            ikk = ix - iy;
                                            vbuffer[ix] = sqtdtam[ix - iy];
                                        }
                                        else
                                            vbuffer[ix] = ' ';
                                    }

                                    vbuffer[4] = cuntam;
                                }
                                else
                                {
                                    vbuffer[0] = ' ';
                                    vbuffer[1] = ' ';
                                    vbuffer[2] = ' ';
                                    vbuffer[3] = ' ';
                                    vbuffer[4] = '0';
                                }

                                vbuffer[5] = ' ';

                                // Segunda parte da linha do dir, data ult modif
                                // Mes
                                vqtdtam = (vdir->UpdateDate & 0x01E0) >> 5;
                                if (vqtdtam < 1 || vqtdtam > 12)
                                    vqtdtam = 1;

                                vqtdtam--;

                                vbuffer[6] = vmesc[vqtdtam][0];
                                vbuffer[7] = vmesc[vqtdtam][1];
                                vbuffer[8] = vmesc[vqtdtam][2];
                                vbuffer[9] = ' ';

                                // Dia
                                vqtdtam = vdir->UpdateDate & 0x001F;
                                memset(sqtdtam, 0x0, 10);
                                itoa(vqtdtam, sqtdtam, 10);

                                if (vqtdtam < 10)
                                {
                                    vbuffer[10] = '0';
                                    vbuffer[11] = sqtdtam[0];
                                }
                                else
                                {
                                    vbuffer[10] = sqtdtam[0];
                                    vbuffer[11] = sqtdtam[1];
                                }
                                vbuffer[12] = ' ';

                                // Ano
                                vqtdtam = ((vdir->UpdateDate & 0xFE00) >> 9) + 1980;
                                memset(sqtdtam, 0x0, 10);
                                itoa(vqtdtam, sqtdtam, 10);

                                vbuffer[13] = sqtdtam[0];
                                vbuffer[14] = sqtdtam[1];
                                vbuffer[15] = sqtdtam[2];
                                vbuffer[16] = sqtdtam[3];
                                vbuffer[17] = ' ';

                                // Terceira parte da linha do dir, nome.ext
                                ix = 18;
                                varg = 0;
                                while (vdir->Name[varg] != 0x20 && vdir->Name[varg] != 0x00 && varg <= 7)
                                {
                                    vbuffer[ix] = vdir->Name[varg];
                                    ix++;
                                    varg++;
                                }

                                vbuffer[ix] = '.';
                                ix++;

                                varg = 0;
                                while (vdir->Ext[varg] != 0x20 && vdir->Ext[varg] != 0x00 && varg <= 2)
                                {
                                    vbuffer[ix] = vdir->Ext[varg];
                                    ix++;
                                    varg++;
                                }

                                if (varg == 0)
                                {
                                    ix--;
                                    vbuffer[ix] = ' ';
                                    ix++;
                                }

                                // Quarta parte da linha do dir, "/" para diretorio
                                if (vdir->Attr == ATTR_DIRECTORY)
                                {
                                    ix--;
                                    vbuffer[ix] = '/';
                                    ix++;
                                }

                                vbuffer[ix] = '\0';

                                for(ix = 0; ix <= 39; ix++)
                                    vlinha[ix] = vbuffer[ix];
                            }
                            else
                            {
                                memset(vlinha, 0x20, 40);
                                vlinha[5]  = 'D';
                                vlinha[6]  = 'i';
                                vlinha[7]  = 's';
                                vlinha[8]  = 'k';
                                vlinha[9]  = ' ';
                                vlinha[10] = 'N';
                                vlinha[11] = 'a';
                                vlinha[12] = 'm';
                                vlinha[13] = 'e';
                                vlinha[14] = ' ';
                                vlinha[15] = 'i';
                                vlinha[16] = 's';
                                vlinha[17] = ' ';
                                ix = 18;
                                varg = 0;
                                while (vdir->Name[varg] != 0x00 && varg <= 7)
                                {
                                    vlinha[ix] = vdir->Name[varg];
                                    ix++;
                                    varg++;
                                }

                                varg = 0;
                                while (vdir->Ext[varg] != 0x00 && varg <= 2)
                                {
                                    vlinha[ix] = vdir->Ext[varg];
                                    ix++;
                                    varg++;
                                }

                                vlinha[ix] = '\0';
                            }

                            // Mostra linha
                            printText("\r\n\0");
                            printText(vlinha);
                        }

                        vretfat = RETURN_OK;

                        // Verifica se Tem mais arquivos no diretorio
                        if (fsFindInDir(vparam, TYPE_NEXT_ENTRY) >= ERRO_D_START)
                        {
                            printText("\r\n\0");
                            break;
                        }
                    }
                    else if (!strcmp(linhacomando,"RM") && iy == 2)
                    {
                        if (logwildcard)
                        {
                            if (matches_wildcard(vretpath->Name, vparam))
                                vretfat = fsDelFile(vparam);

                            if (vretfat != RETURN_OK)
                                break;
                        }
                        else
                        {
                            vretfat = fsDelFile(vretpath->Name);
                            break;
                        }
                    }
                    else if (!strcmp(linhacomando,"CP") && iy == 2)
                    {
                        ikk = 0;
                        vretfat = RETURN_OK;

                        if (logwildcard)
                        {
                            if (!matches_wildcard(vretpath->Name, vparam))
                                break;
                        }
                        else
                            strcpy(vparam, vretpath->Name);

                        *vclusterdir = vclusterdirsrc;

                        if (fsOpenFile(vparam) != RETURN_OK)
                        {
                            vretfat = ERRO_B_NOT_FOUND;
                        }
                        else
                        {
                            *vclusterdir = vclusterdiratu;
                            vrettype = fsFindDirPath(vparam2, FIND_PATH_LAST); // nao tem comparacao com erro pois o arquivo destino pode nao existir
                            if (!isValidFilename(vretpath->Name))
                                vretfat = ERRO_B_INVALID_NAME;
                            else
                            {
                                if (vrettype == FIND_PATH_RET_FOLDER)
                                    strcpy(vparam2, vparam);
                                else
                                    strcpy(vparam2, vretpath->Name);

                                vclusterdirdst = vretpath->ClusterDir;
                                *vclusterdir = vclusterdirdst;

                                if (fsOpenFile(vparam2) != RETURN_OK)
                                {
                                    if (fsCreateFile(vparam2) != RETURN_OK)
                                        vretfat = ERRO_B_CREATE_FILE;
                                }

                                //memcpy(vdirdst, vdir, sizeof(FAT32_DIR));
                            }
                        }

                        while (vretfat == RETURN_OK)
                        {
                            *vclusterdir = vclusterdirsrc;
                            if (fsReadFile(vparam, ikk, vbuffer, 128) > 0)
                            {
                                *vclusterdir = vclusterdirdst;
                                //memcpy(tempData2, vbuffer, 128);
                                if (fsWriteFile(vparam2, ikk, vbuffer, 128) != RETURN_OK)
                                {
                                    vretfat = ERRO_B_WRITE_FILE;
                                    break;
                                }

                                ikk += 128;
                            }
                            else
                                break;
                        }

                        if (!logwildcard)
                            break;

                        *vclusterdir = vclusterdirsrc;
                    }
                }
            }

            *vclusterdir = vclusterdiratu;

            if (vretfat != RETURN_OK)
            {
                printDiskError(vretfat);
                printText("\r\n\0");
            }
        }
        else
        {
            if (!strcmp(linhacomando,"REN") && iy == 3) // Arquivo (somente 1, nao uar wildcard)
            {
                if (fsFindDirPath(vparam, FIND_PATH_PART) == FIND_PATH_RET_ERROR)
                {
                    printText("file not found.\r\n\0");
                    vretfat = ERRO_B_NOT_FOUND;
                }
                else
                {
                    if (!isValidFilename(vparam2))
                        vretfat = ERRO_B_INVALID_NAME;
                    else
                    {
                        *vclusterdir = vretpath->ClusterDir;
                        vretfat = fsRenameFile(vretpath->Name, vparam2);
                    }
                }

                *vclusterdir = vretpath->ClusterDirAtu;
            }
            else if (!strcmp(linhacomando,"MD") && iy == 2)
            {
                vretfat = fsMakeDir(linhaarg);
            }
            else if (!strcmp(linhacomando,"CD") && iy == 2)
            {
                vretfat = fsChangeDir(linhaarg);
            }
            else if (!strcmp(linhacomando,"RD") && iy == 2)
            {
                vretfat = fsRemoveDir(linhaarg);
            }
            else if (!strcmp(linhacomando,"SERTOFILE") && iy == 9) // Arquivo (usa 1 soh)
            {
                vretfat = fsLoadSerialToFile(linhaarg, "810000");  // Carrega da Serial para o Arquivo
            }
            else if (!strcmp(linhacomando,"DATE") && iy == 4)
            {
                // TBD
            }
            else if (!strcmp(linhacomando,"TIME") && iy == 4)
            {
                // TBD
            }
            else if (!strcmp(linhacomando,"FORMAT") && iy == 6)
            {
                vretfat = fsFormat(0x5678, linhaarg);
            }
            else if (!strcmp(linhacomando,"MODE") && iy == 4)
            {
                // A definir
                ix = 255;
            }
            else if (!strcmp(linhacomando,"CAT") && iy == 3) // Arquivo (usa 1 soh)
            {
                catFile(linhaarg);
                ix = 255;
            }
            else
            {
                // Verifica se tem Arquivo com esse nome na pasta atual no disco
                ix = iy;
                linhacomando[ix] = '.';
                ix++;
                linhacomando[ix] = 'B';
                ix++;
                linhacomando[ix] = 'I';
                ix++;
                linhacomando[ix] = 'N';
                ix++;
                linhacomando[ix] = '\0';

                vretfat = fsFindInDir(linhacomando, TYPE_FILE);
                if (vretfat <= ERRO_D_START)
                {
                    // Se tiver, carrega em 0x00810000 e executa
                    loadFile(linhacomando, (unsigned long*)0x00810000);
                    if (!*verro)
                    {
                        runFromOsCmd();
                    }
                    else
                    {
                        printText("Loading File Error...\r\n\0");
                    }

                    ix = 255;
                }
                else
                {
                    // Se nao tiver, mostra erro
                    printText("Invalid Command or File Name\r\n\0");

                    ix = 255;
                }
            }

            if (ix != 255)
            {
                if (vpicret)
                {
                    for (varg = 0; varg < ix; varg++)
                        fsSendByte(linhaarg[varg], FS_DATA);

                    vbytepic = fsRecByte(FS_DATA);
                }

                if (((vpicret) && (vbytepic != RETURN_OK)) || ((!vpicret) && (vretfat != RETURN_OK)))
                {
                    printDiskError(vretfat);
                    printText("\r\n\0");
                }
                else
                {
                    if (!strcmp(linhacomando,"CD"))
                    {
                        if (linhaarg[0] == '.' && linhaarg[1] == '.')
                        {
                            while (*vdiratup != '/')
                            {
                                *vdiratup-- = '\0';
                            }

                            if (vdiratup > vdiratu)
                                *vdiratup = '\0';
                            else
                                *vdiratup++;
                        }
                        else if(linhaarg[0] == '/')
                        {
                            vdiratup = vdiratu;
                            *vdiratup++ = '/';
                            *vdiratup = '\0';
                       }
                        else if(linhaarg[0] != '.')
                        {
                            *vdiratup--;
                            if (*vdiratup++ != '/')
                                *vdiratup++ = '/';
                            for (varg = 0; varg < ix; varg++)
                                *vdiratup++ = linhaarg[varg];
                            *vdiratup = '\0';
                        }
                    }
                    else if (!strcmp(linhacomando,"DATE"))
                    {
                        /*for(ix = 0; ix <= 9; ix++)
                        {
                            recPic();
                            vlinha[ix] = vbytepic;
                        }*/

                        vlinha[ix] = '\0';
                        printText("  Date is \0");
                        printText(vlinha);
                        printText("\r\n\0");
                    }
                    else if (!strcmp(linhacomando,"TIME"))
                    {
                        /*for(ix = 0; ix <= 7; ix++)
                        {
                            recPic();
                            vlinha[ix] = vbytepic;
                        }*/

                        vlinha[ix] = '\0';
                        printText("  Time is \0");
                        printText(vlinha);
                        printText("\r\n\0");
                    }
                    else if (!strcmp(linhacomando,"FORMAT"))
                    {
                        printText("Format disk was successfully\r\n\0");
                    }
                }
            }
        }
    }

    return 0;
}

//-----------------------------------------------------------------------------
// Delay Function
//-----------------------------------------------------------------------------
void delayus(int pTimeUS)
{
    unsigned int ix;

    for(ix = 0; ix <= pTimeUS; ix++);    // +/- 1us * pTimeMs parada
}

//-----------------------------------------------------------------------------
// Memory Allocation Functions
//-----------------------------------------------------------------------------
void memInit(void)
{
    // Alloc all memmory available minus reserved
    vMemAloc->prev = NULL;
    vMemAloc->name[0] = 0x00;
    vMemAloc->address = 0x00810000;
    vMemAloc->size = (*vtotmem * 1024) - 0x00010000 - 0x00040000; // 0x00600000 - 0x0063FFFF (256KB) = Reserved and 0x00800000 - 0x0080FFFF (64KB) = Reserved SO
    vMemAloc->status = 0;
    vMemAloc->next = NULL;
}

//-----------------------------------------------------------------------------
// Disk Functions
//-----------------------------------------------------------------------------
int fsSendByte(unsigned char vByte, unsigned char pType)
{
    if (pType == 0)
        *vdskc = vByte;
    else if (pType == 1)
        *vdskd = vByte;
    else if (pType == 2)
        *vdskp = vByte;

    return 1;
}

//-----------------------------------------------------------------------------
unsigned char fsRecByte(unsigned char pType)
{
    unsigned char vByte;

    if (pType == 0)
        vByte = *vdskc;
    else if (pType == 1)
        vByte = *vdskd;

    return vByte;
}

//-----------------------------------------------------------------------------
// FAT32 Functions
//-----------------------------------------------------------------------------
BYTE fsMountDisk(void)
{
	// LER MBR
    if (!fsSectorRead((DWORD)0x0000,gDataBuffer))
		return ERRO_B_READ_DISK;

    vdisk->firsts  = (((DWORD)gDataBuffer[457] << 24) & 0xFF000000);
    vdisk->firsts |= (((DWORD)gDataBuffer[456] << 16) & 0x00FF0000);
    vdisk->firsts |= (((DWORD)gDataBuffer[455] << 8) & 0x0000FF00);
    vdisk->firsts |= ((DWORD)gDataBuffer[454] & 0x000000FF);

    // LER FIRST CLUSTER
	if (!fsSectorRead(vdisk->firsts,gDataBuffer))
		return ERRO_B_READ_DISK;

    vdisk->reserv  = (WORD)gDataBuffer[15] << 8;
    vdisk->reserv |= (WORD)gDataBuffer[14];

	vdisk->fat = vdisk->reserv + vdisk->firsts;

    vdisk->sectorSize  = (DWORD)gDataBuffer[12] << 8;
    vdisk->sectorSize |= (DWORD)gDataBuffer[11];
	vdisk->SecPerClus = gDataBuffer[13];

    vdisk->fatsize  = (DWORD)gDataBuffer[39] << 24;
    vdisk->fatsize |= (DWORD)gDataBuffer[38] << 16;
    vdisk->fatsize |= (DWORD)gDataBuffer[37] << 8;
    vdisk->fatsize |= (DWORD)gDataBuffer[36];

    vdisk->root  = (DWORD)gDataBuffer[47] << 24;
    vdisk->root |= (DWORD)gDataBuffer[46] << 16;
    vdisk->root |= (DWORD)gDataBuffer[45] << 8;
    vdisk->root |= (DWORD)gDataBuffer[44];

	vdisk->type = FAT32;

	vdisk->data = vdisk->reserv + (2 * vdisk->fatsize);

	*vclusterdir = vdisk->root;

	return RETURN_OK;
}

//-------------------------------------------------------------------------
void fsSetClusterDir (DWORD vclusdiratu) {
    *vclusterdir = vclusdiratu;
}

//-------------------------------------------------------------------------
DWORD fsGetClusterDir (void) {
    return *vclusterdir;
}

//-------------------------------------------------------------------------
BYTE fsCreateFile(char * vfilename)
{
	// Verifica ja existe arquivo com esse nome
	if (fsFindInDir(vfilename, TYPE_ALL) < ERRO_D_START)
		return ERRO_B_FILE_FOUND;

	// Cria o arquivo com o nome especificado
	if (fsFindInDir(vfilename, TYPE_CREATE_FILE) >= ERRO_D_START)
		return ERRO_B_CREATE_FILE;

	return RETURN_OK;
}

//-------------------------------------------------------------------------
BYTE fsOpenFile(char * vfilename)
{
	WORD vdirdate, vbytepic;
	BYTE ds1307[7], ix, vlinha[12], vtemp[5];

	// Abre o arquivo especificado
	if (fsFindInDir(vfilename, TYPE_FILE) >= ERRO_D_START)
		return ERRO_B_FILE_NOT_FOUND;

	// Ler Data/Hora do PIC
    // TBD
    ds1307[3] = 01;
    ds1307[4] = 01;
    ds1307[5] = 2024;

	// Converte para a Data/Hora da FAT32
	vdirdate = datetimetodir(ds1307[3], ds1307[4], ds1307[5], CONV_DATA);

	// Grava nova data no lastaccess
	vdir->LastAccessDate  = vdirdate;

 	if (fsUpdateDir() != RETURN_OK)
		return ERRO_B_UPDATE_DIR;

	return RETURN_OK;
}


//-------------------------------------------------------------------------
BYTE fsCloseFile(char * vfilename, BYTE vupdated)
{
	WORD vdirdate, vdirtime, vbytepic;
	BYTE ds1307[7], vtemp[5], ix, vlinha[12];

	if (fsFindInDir(vfilename, TYPE_FILE) < ERRO_D_START) {
		if (vupdated) {
			// Ler Data/Hora do DS1307 - I2C
            // TBD
            ds1307[3] = 01;
            ds1307[4] = 01;
            ds1307[5] = 2024;
            ds1307[0] = 00;
            ds1307[1] = 00;
            ds1307[2] = 00;

			// Converte para a Data/Hora da FAT32
			vdirtime = datetimetodir(ds1307[0], ds1307[1], ds1307[2], CONV_HORA);
			vdirdate = datetimetodir(ds1307[3], ds1307[4], ds1307[5], CONV_DATA);

			// Grava nova data no lastaccess e nova data/hora no update date/time
			vdir->LastAccessDate  = vdirdate;
			vdir->UpdateTime = vdirtime;
			vdir->UpdateDate = vdirdate;

			if (fsUpdateDir() != RETURN_OK)
				return ERRO_B_UPDATE_DIR;
		}
	}
	else
		return ERRO_B_NOT_FOUND;

	return RETURN_OK;
}

//-------------------------------------------------------------------------
DWORD fsInfoFile(char * vfilename, BYTE vtype)
{
	DWORD vinfo = ERRO_D_NOT_FOUND, vtemp;

	// retornar as informa?es conforme solicitado.
	if (fsFindInDir(vfilename, TYPE_FILE) < ERRO_D_START) {
		switch (vtype) {
			case INFO_SIZE:
				vinfo = vdir->Size;
				break;
			case INFO_CREATE:
			    vtemp = (vdir->CreateDate << 16) | vdir->CreateTime;
				vinfo = (vtemp);
				break;
			case INFO_UPDATE:
			    vtemp = (vdir->UpdateDate << 16) | vdir->UpdateTime;
				vinfo = (vtemp);
				break;
			case INFO_LAST:
				vinfo = vdir->LastAccessDate;
				break;
		}
	}
	else
		return ERRO_D_NOT_FOUND;

	return vinfo;
}

//-------------------------------------------------------------------------
BYTE fsDelFile(char * vfilename)
{
	// Apaga o arquivo solicitado
	if (fsFindInDir(vfilename, TYPE_DEL_FILE) >= ERRO_D_START)
		return ERRO_B_APAGAR_ARQUIVO;

	return RETURN_OK;
}

//-------------------------------------------------------------------------
BYTE fsRenameFile(char * vfilename, char * vnewname)
{
	DWORD vclusterfile;
	WORD ikk;
	BYTE ixx, iyy;

	// Verificar se nome j?nao existe
	vclusterfile = fsFindInDir(vnewname, TYPE_ALL);

	if (vclusterfile < ERRO_D_START)
		return ERRO_B_FILE_FOUND;

	// Procura arquivo a ser renomeado
	vclusterfile = fsFindInDir(vfilename, TYPE_FILE);

	if (vclusterfile >= ERRO_D_START)
		return ERRO_B_FILE_NOT_FOUND;

	// Altera nome na estrutura vdir
	memset(vdir->Name, 0x20, 8);
	memset(vdir->Ext, 0x20, 3);

	iyy = 0;
	for (ixx = 0; ixx <= strlen(vnewname); ixx++) {
		if (vnewname[ixx] == '\0')
			break;
		else if (vnewname[ixx] == '.')
			iyy = 8;
		else {
			if (iyy <= 7)
				vdir->Name[iyy] = vnewname[ixx];
			else {
			    ikk = iyy - 8;
				vdir->Ext[ikk] = vnewname[ixx];
			}

			iyy++;
		}
	}

	// Altera o nome, as demais informacoes nao alteram
	if (fsUpdateDir() != RETURN_OK)
		return ERRO_B_UPDATE_DIR;

	return RETURN_OK;
}

//-------------------------------------------------------------------------
BYTE fsLoadSerialToFile(char * vfilename, char * vPosMem)
{
    DWORD vSize, ix, vStep;
    unsigned char *xaddress = hexToLong(vPosMem);
    unsigned char vBuffer[128];
    int iy;
    BYTE vmovposyatu = 0;

    if (vfilename == 0)
    {
        printText("Error, file name must be provided!!\r\n\0");
        return ERRO_B_WRITE_FILE;;
    }

    // Verifica se o arquivo existe
	if (fsFindInDir(vfilename, TYPE_FILE) < ERRO_D_START)
    {
        // Se existir, apaga
        fsDelFile(vfilename);
    }

    // Cria o Arquivo
    fsCreateFile(vfilename);

    // Recebe os dados via Serial
    if (!loadSerialToMem(vPosMem, 1))
    {
        // Abre Arquivo
        printText("Opening File...\r\n\0");

        fsOpenFile(vfilename);

        // Grava no Arquivo
        printText("Writing File...\r\n\0");

        printChar(218,1);
        for (ix = 0; ix < 20; ix++)
            printChar(196,1);
        printChar(191,1);

        printText("\r\n\0");

        printChar(179,1);
        for (ix = 0; ix < 20; ix++)
            printChar(' ',1);
        printChar(179,1);

        printText("\r\n\0");

        printChar(192,1);
        for (ix = 0; ix < 20; ix++)
            printChar(196,1);
        printChar(217,1);

        printText("\r\n\0");

        vmovposyatu = *videoCursorPosRowY;

        vStep = *vSizeTotalRec / 20;

        vdp_set_cursor(1, (*videoCursorPosRowY - 2));

        for (ix = 0; ix < *vSizeTotalRec; ix += 128)
        {
            for (iy = 0; iy < 128; iy++)
            {
                if (ix > 0 && ((ix + iy) % vStep) == 0)
                    printChar(254, 1);

                vBuffer[iy] = *xaddress;
                xaddress += 1;
            }

            if (fsWriteFile(vfilename, ix, vBuffer, 128) != RETURN_OK)
                return ERRO_B_WRITE_FILE;
        }

        vdp_set_cursor(0, vmovposyatu);

        // Fecha Arquivo
        printText("\r\nClosing File...\r\n\0");

        fsCloseFile(vfilename, 0);
    }
    else
    {
        printText("Serial Load Error...");

        return ERRO_B_WRITE_FILE;
    }

    return RETURN_OK;
}

//-------------------------------------------------------------------------
// Rotina para escrever/ler no disco
//-------------------------------------------------------------------------
BYTE fsRWFile(DWORD vclusterini, DWORD voffset, BYTE *buffer, BYTE vtype)
{
	DWORD vdata, vclusternew, vfat;
	WORD vpos, vsecfat, voffsec, voffclus, vtemp1, vtemp2, ikk, ikj;

	// Calcula offset de setor e cluster
	voffsec = voffset / vdisk->sectorSize;
	voffclus = voffsec / vdisk->SecPerClus;
	vclusternew = vclusterini;

	// Procura o cluster onde esta o setor a ser lido
	for (vpos = 0; vpos < voffclus; vpos++) {
		// Em operacao de escrita, como vai mexer com disco, salva buffer no setor de swap
		if (vtype == OPER_WRITE) {
		    ikk = vdisk->fat - 1;
			if (!fsSectorWrite(ikk, buffer, FALSE))
				return ERRO_B_READ_DISK;
		}

		vclusternew = fsFindNextCluster(vclusterini, NEXT_FIND);

		// Se for leitura e o offset der dentro do ultimo cluster, sai
		if (vtype == OPER_READ && vclusternew == LAST_CLUSTER_FAT32)
			return RETURN_OK;

		// Se for gravacao e o offset der dentro do ultimo cluster, cria novo cluster
		if ((vtype == OPER_WRITE || vtype == OPER_READWRITE) && vclusternew == LAST_CLUSTER_FAT32) {
			// Calcula novo cluster livre
			vclusternew = fsFindClusterFree(FREE_USE);

			if (vclusternew == ERRO_D_DISK_FULL)
				return ERRO_B_DISK_FULL;

			// Procura Cluster atual para altera?o
			vsecfat = vclusterini / 128;
			vfat = vdisk->fat + vsecfat;

			if (!fsSectorRead(vfat, gDataBuffer))
				return ERRO_B_READ_DISK;

			// Grava novo cluster no cluster atual
			vpos = (vclusterini - ( 128 * vsecfat)) * 4;
			gDataBuffer[vpos] = (BYTE)(vclusternew & 0xFF);
			ikk = vpos + 1;
			gDataBuffer[ikk] = (BYTE)((vclusternew / 0x100) & 0xFF);
			ikk = vpos + 2;
			gDataBuffer[ikk] = (BYTE)((vclusternew / 0x10000) & 0xFF);
			ikk = vpos + 3;
			gDataBuffer[ikk] = (BYTE)((vclusternew / 0x1000000) & 0xFF);

			if (!fsSectorWrite(vfat, gDataBuffer, FALSE))
				return ERRO_B_WRITE_DISK;
		}

		vclusterini = vclusternew;

		// Em operacao de escrita, como mexeu com disco, le o buffer salvo no setor swap
		if (vtype == OPER_WRITE) {
		    ikk = vdisk->fat - 1;
			if (!fsSectorRead(ikk, buffer))
				return ERRO_B_READ_DISK;
		}
	}

	// Posiciona no setor dentro do cluster para ler/gravar
	vtemp1 = ((vclusternew - 2) * vdisk->SecPerClus);
	vtemp2 = (vdisk->reserv + vdisk->firsts + (2 * vdisk->fatsize));
	vdata = vtemp1 + vtemp2;
	vtemp1 = (voffclus * vdisk->SecPerClus);
	vdata += voffsec - vtemp1;

	if (vtype == OPER_READ || vtype == OPER_READWRITE) {
		// Le o setor e coloca no buffer
		if (!fsSectorRead(vdata, buffer))
			return ERRO_B_READ_DISK;
	}
	else {
		// Grava o buffer no setor
		if (!fsSectorWrite(vdata, buffer, FALSE))
			return ERRO_B_WRITE_DISK;
	}

	return RETURN_OK;
}

//-------------------------------------------------------------------------
// Retorna um buffer de "vsize" (max 255) Bytes, a partir do "voffset".
//-------------------------------------------------------------------------
WORD fsReadFile(char * vfilename, DWORD voffset, BYTE *buffer, WORD vsizebuffer)
{
	WORD ix, iy, vsizebf = 0;
	WORD vsize, vsetor = 0, vsizeant = 0;
	WORD voffsec, vtemp, ikk, ikj;
	DWORD vclusterini;
    BYTE sqtdtam[10];

	vclusterini = fsFindInDir(vfilename, TYPE_FILE);

	if (vclusterini >= ERRO_D_START)
		return 0;	// Erro na abertura/Arquivo nao existe

	// Verifica se o offset eh maior que o tamanho do arquivo
	if (voffset > vdir->Size)
		return 0;

	// Verifica se offset vai precisar gravar mais de 1 setor (entre 2 setores)
	vtemp = voffset / vdisk->sectorSize;
	voffsec = (voffset - (vdisk->sectorSize * (vtemp)));

	if ((voffsec + vsizebuffer) > vdisk->sectorSize)
		vsetor = 1;

/*itoa(vsetor, sqtdtam, 10);
printText(sqtdtam, *vcorf, *vcorb);
printText(".\n\0");*/

/*itoa(voffsec, sqtdtam, 10);
printText(sqtdtam, *vcorf, *vcorb);
printText(".\n\0");*/

/*itoa(vdisk->sectorSize, sqtdtam, 10);
printText(sqtdtam, *vcorf, *vcorb);
printText(".\n\0");*/

/*itoa(voffset, sqtdtam, 10);
printText(sqtdtam, *vcorf, *vcorb);
printText(".\n\0");*/

/*itoa(vsizebuffer, sqtdtam, 10);
printText(sqtdtam, *vcorf, *vcorb);
printText(".\n\0");*/

	for (ix = 0; ix <= vsetor; ix++) {
    	vtemp = voffset / vdisk->sectorSize;
    	voffsec = (voffset - (vdisk->sectorSize * (vtemp)));

		// Ler setor do offset
		if (fsRWFile(vclusterini, voffset, gDataBuffer, OPER_READ) != RETURN_OK)
			return vsizebf;

		// Verifica tamanho a ser gravado
		if ((voffsec + vsizebuffer) <= vdisk->sectorSize)
			vsize = vsizebuffer - vsizeant;
		else
			vsize = vdisk->sectorSize - voffsec;

		vsizebf += vsize;

		if (vsizebf > (vdir->Size - voffset))
			vsizebf = vdir->Size - voffset;

/*itoa(vsize, sqtdtam, 10);
printText(sqtdtam, *vcorf, *vcorb);
printText(".\n\0");*/

        if (vsetor == 0)
            vsize = vsizebuffer;

		// Retorna os dados no buffer
		for (iy = 0; iy < vsize; iy++) {
		    ikk = vsizeant + iy;
		    ikj = voffsec + iy;
			buffer[ikk] = gDataBuffer[ikj];
        }

		vsizeant = vsize;
		voffset += vsize;
	}

	return vsizebf;
}

//-------------------------------------------------------------------------
// buffer a ser gravado nao pode ter mais que 512 bytes
//-------------------------------------------------------------------------
BYTE fsWriteFile(char * vfilename, DWORD voffset, BYTE *buffer, BYTE vsizebuffer)
{
	BYTE vsetor = 0, ix, iy;
	WORD vsize, vsizeant = 0;
	WORD voffsec, vtemp, ikk, ikj;
	DWORD vclusterini;

	vclusterini = fsFindInDir(vfilename, TYPE_FILE);

	if (vclusterini >= ERRO_D_START)
		return ERRO_B_FILE_NOT_FOUND;	// Erro na abertura/Arquivo nao existe

	// Verifica se offset vai precisar gravar mais de 1 setor (entre 2 setores)
	vtemp = voffset / vdisk->sectorSize;
	voffsec = (voffset - (vdisk->sectorSize * (vtemp)));

	if ((voffsec + vsizebuffer) > vdisk->sectorSize)
		vsetor = 1;

	for (ix = 0; ix <= vsetor; ix++) {
    	vtemp = voffset / vdisk->sectorSize;
    	voffsec = (voffset - (vdisk->sectorSize * (vtemp)));

*tempData = vclusterini;

		// Ler setor do offset
		if (fsRWFile(vclusterini, voffset, gDataBuffer, OPER_READWRITE) != RETURN_OK)
			return ERRO_B_READ_FILE;
memcpy(tempData2,gDataBuffer,512);
		// Verifica tamanho a ser gravado
		if ((voffsec + vsizebuffer) <= vdisk->sectorSize)
			vsize = vsizebuffer - vsizeant;
		else
			vsize = vdisk->sectorSize - voffsec;

		// Prepara buffer para grava?o
		for (iy = 0; iy < vsize; iy++) {
		    ikk = iy + voffsec;
		    ikj = vsizeant + iy;
			gDataBuffer[ikk] = buffer[ikj];
		}
*(tempData + 1) = vclusterini;
memcpy(tempData3,gDataBuffer,512);

		// Grava setor
		if (fsRWFile(vclusterini, voffset, gDataBuffer, OPER_WRITE) != RETURN_OK)
			return ERRO_B_WRITE_FILE;

		vsizeant = vsize;

		if (vsetor == 1)
			voffset += vsize;
	}

	if ((voffset + vsizebuffer) > vdir->Size) {
		vdir->Size = voffset + vsizebuffer;

		if (fsUpdateDir() != RETURN_OK)
			return ERRO_B_UPDATE_DIR;
	}

	return RETURN_OK;
}

//-------------------------------------------------------------------------
BYTE fsMakeDir(char * vdirname)
{
    if (fsFindDirPath(vdirname, FIND_PATH_PART) == FIND_PATH_RET_ERROR)
        return ERRO_B_CREATE_DIR;

    if (!isValidFilename(vretpath->Name))
		return ERRO_B_INVALID_NAME;

    *vclusterdir = vretpath->ClusterDir;

	// Verifica ja existe arquivo/dir com esse nome
	if (fsFindInDir(vretpath->Name, TYPE_ALL) < ERRO_D_START)
    {
        *vclusterdir = vretpath->ClusterDirAtu;
		return ERRO_B_DIR_FOUND;
    }

	// Cria o dir solicitado
	if (fsFindInDir(vretpath->Name, TYPE_CREATE_DIR) >= ERRO_D_START)
    {
        *vclusterdir = vretpath->ClusterDirAtu;
		return ERRO_B_CREATE_DIR;
    }

    *vclusterdir = vretpath->ClusterDirAtu;

	return RETURN_OK;
}

//-------------------------------------------------------------------------
// [<folder>/<folder>/<folder>/]<file>
//-------------------------------------------------------------------------
BYTE fsFindDirPath(char * vpath, char vtype)
{
    DWORD vclusterdirnew, vclusterdirant;
    int ix, iy;
    BYTE vret = FIND_PATH_RET_FOLDER;

    vclusterdirant = *vclusterdir;
    vclusterdirnew = *vclusterdir;
    vretpath->ClusterDirAtu = *vclusterdir;

    ix = 0;

    // Verifica se eh diretorio raiz
	if (vpath[0] == '/')
    {
		vclusterdirnew = vdisk->root;
        *vclusterdir = vclusterdirnew;
        ix++;
    }

    // Loop ateh a ultima pasta
    if (vpath[1] != 0x00)
    {
        while(1)
        {
            iy = 0;

            while(vpath[ix] != 0x00 && vpath[ix] != '/')
            {
                vretpath->Name[iy] = vpath[ix];
                ix++;
                iy++;
            }

            vretpath->Name[iy] = '\0';

            if (vpath[ix] == 0x00 && vtype == FIND_PATH_PART)
                break;

            vclusterdirnew = fsFindInDir(vretpath->Name, TYPE_DIRECTORY);

            if (vclusterdirnew >= ERRO_D_START)
            {
                if (fsFindInDir(vretpath->Name, TYPE_FILE) >= ERRO_D_START)
                {
                    vret = FIND_PATH_RET_ERROR;
//                    vretpath->ClusterDir = ERRO_D_START;
                    vretpath->ClusterDir = *vclusterdir;
                }
                else
                {
                    vret = FIND_PATH_RET_FILE;
                    vretpath->ClusterDir = *vclusterdir;
                }

                *vclusterdir = vclusterdirant;
                return vret;
            }

            *vclusterdir = vclusterdirnew;
            vretpath->ClusterDir = vclusterdirnew;

            if (vpath[ix] == 0x00)
                break;

            ix++;
        }
    }

    *vclusterdir = vclusterdirant;

    vretpath->ClusterDir = vclusterdirnew;

    return vret;
}

//-------------------------------------------------------------------------
BYTE fsChangeDir(char * vdirname)
{
	DWORD vclusterdirnew;

	// Troca o diretorio conforme especificado
	/*if (vdirname[0] == '/' && vdirname[1] == '\0')
		vclusterdirnew = vdisk->root;
	else
    {*/
        if (fsFindDirPath(vdirname, FIND_PATH_LAST) == FIND_PATH_RET_ERROR)
    		return ERRO_B_DIR_NOT_FOUND;
        vclusterdirnew = vretpath->ClusterDir;
    //}

	// Coloca o novo diretorio como atual
	*vclusterdir = vclusterdirnew;
    vretpath->ClusterDirAtu = vclusterdirnew;

	return RETURN_OK;
}

//-------------------------------------------------------------------------
BYTE fsRemoveDir(char * vdirname)
{
    if (fsFindDirPath(vdirname, FIND_PATH_PART) == FIND_PATH_RET_ERROR)
        return ERRO_B_DIR_NOT_FOUND;

    *vclusterdir = vretpath->ClusterDir;

    /*printText("Aqui 2 - ");
    printText(vretpath->Name);
    printText("\r\n");*/

	// Apaga o diretorio conforme especificado
	if (fsFindInDir(vretpath->Name, TYPE_DEL_DIR) >= ERRO_D_START)
    {
        *vclusterdir = vretpath->ClusterDirAtu;
		return ERRO_B_DIR_NOT_FOUND;
    }

    *vclusterdir = vretpath->ClusterDirAtu;

	return RETURN_OK;
}

//-------------------------------------------------------------------------
BYTE fsPwdDir(BYTE *vdirpath) {
    if (*vclusterdir == vdisk->root) {
        vdirpath[0] = '/';
        vdirpath[1] = '\0';
    }
    else {
        vdirpath[0] = 'o';
        vdirpath[1] = '\0';
    }

	return RETURN_OK;
}

//-------------------------------------------------------------------------
DWORD fsFindInDir(char * vname, BYTE vtype)
{
	DWORD vfat, vdata, vclusterfile, vclusterdirnew, vclusteratual, vtemp1, vtemp2;
	BYTE fnameName[9], fnameExt[4];
	WORD im, ix, iy, iz, vpos, vsecfat, ventrydir, ixold;
	WORD vdirdate, vdirtime, ikk, ikj, vtemp, vbytepic;
	BYTE vcomp, iw, ds1307[7], iww, vtempt[5], vlinha[5];
    BYTE sqtdtam[10];

	memset(fnameName, 0x20, 8);
	memset(fnameExt, 0x20, 3);

	if (vname != NULL) {
		if (vname[0] == '.' && vname[1] == '.') {
			fnameName[0] = vname[0];
			fnameName[1] = vname[1];
		}
		else if (vname[0] == '.') {
			fnameName[0] = vname[0];
		}
		else {
			iy = 0;
			for (ix = 0; ix <= strlen(vname); ix++) {
				if (vname[ix] == '\0')
					break;
				else if (vname[ix] == '.')
					iy = 8;
				else {
					for (iww = 0; iww <= 56; iww++) {
						if (strValidChars[iww] == vname[ix])
							break;
					}

					if (iww > 56)
						return ERRO_D_INVALID_NAME;

					if (iy <= 7)
						fnameName[iy] = vname[ix];
					else {
					    ikk = iy - 8;
						fnameExt[ikk] = vname[ix];
					}

					iy++;
				}
			}
		}
	}

	vfat = vdisk->fat;
	vtemp1 = ((*vclusterdir - 2) * vdisk->SecPerClus);
	vtemp2 = (vdisk->reserv + vdisk->firsts + (2 * vdisk->fatsize));
	vdata = vtemp1 + vtemp2;

	vclusterfile = ERRO_D_NOT_FOUND;
	vclusterdirnew = *vclusterdir;
	ventrydir = 0;

	while (vdata != LAST_CLUSTER_FAT32) {
		for (iw = 0; iw < vdisk->SecPerClus; iw++) {

      		if (!fsSectorRead(vdata, gDataBuffer))
				return ERRO_D_READ_DISK;

			for (ix = 0; ix < vdisk->sectorSize; ix += 32) {
				for (iy = 0; iy < 8; iy++) {
				    ikk = ix + iy;
					vdir->Name[iy] = gDataBuffer[ikk];
				}

				for (iy = 0; iy < 3; iy++) {
				    ikk = ix + 8 + iy;
					vdir->Ext[iy] = gDataBuffer[ikk];
                }

                ikk = ix + 11;
				vdir->Attr = gDataBuffer[ikk];

                ikk = ix + 15;
				vdir->CreateTime  = (WORD)gDataBuffer[ikk] << 8;
                ikk = ix + 14;
				vdir->CreateTime |= (WORD)gDataBuffer[ikk];

                ikk = ix + 17;
				vdir->CreateDate  = (WORD)gDataBuffer[ikk] << 8;
                ikk = ix + 16;
				vdir->CreateDate |= (WORD)gDataBuffer[ikk];

                ikk = ix + 19;
				vdir->LastAccessDate  = (WORD)gDataBuffer[ikk] << 8;
                ikk = ix + 18;
				vdir->LastAccessDate |= (WORD)gDataBuffer[ikk];

                ikk = ix + 23;
				vdir->UpdateTime  = (WORD)gDataBuffer[ikk] << 8;
                ikk = ix + 22;
				vdir->UpdateTime |= (WORD)gDataBuffer[ikk];

                ikk = ix + 25;
				vdir->UpdateDate  = (WORD)gDataBuffer[ikk] << 8;
                ikk = ix + 24;
				vdir->UpdateDate |= (WORD)gDataBuffer[ikk];

                ikk = ix + 21;
				vdir->FirstCluster  = (DWORD)gDataBuffer[ikk] << 24;
                ikk = ix + 20;
				vdir->FirstCluster |= (DWORD)gDataBuffer[ikk] << 16;
                ikk = ix + 27;
				vdir->FirstCluster |= (DWORD)gDataBuffer[ikk] << 8;
                ikk = ix + 26;
				vdir->FirstCluster |= (DWORD)gDataBuffer[ikk];

                ikk = ix + 31;
				vdir->Size  = (DWORD)gDataBuffer[ikk] << 24;
                ikk = ix + 30;
				vdir->Size |= (DWORD)gDataBuffer[ikk] << 16;
                ikk = ix + 29;
				vdir->Size |= (DWORD)gDataBuffer[ikk] << 8;
                ikk = ix + 28;
				vdir->Size |= (DWORD)gDataBuffer[ikk];

				vdir->DirClusSec = vdata;
				vdir->DirEntry = ix;

				if (vtype == TYPE_FIRST_ENTRY && vdir->Attr != 0x0F) {
					if (vdir->Name[0] != DIR_DEL) {
			 			if (vdir->Name[0] != DIR_EMPTY) {
							vclusterfile = vdir->FirstCluster;
    						vdata = LAST_CLUSTER_FAT32;
    						break;
    					}
					}
				}

				if (vtype == TYPE_EMPTY_ENTRY || vtype == TYPE_CREATE_FILE || vtype == TYPE_CREATE_DIR) {
					if (vdir->Name[0] == DIR_EMPTY || vdir->Name[0] == DIR_DEL) {
						vclusterfile = ventrydir;

						if (vtype != TYPE_EMPTY_ENTRY) {
							vclusterfile = fsFindClusterFree(FREE_USE);

							if (vclusterfile >= ERRO_D_START)
								return ERRO_D_NOT_FOUND;

						    if (!fsSectorRead(vdata, gDataBuffer))
								return ERRO_D_READ_DISK;

							for (iz = 0; iz <= 10; iz++) {
								if (iz <= 7) {
								    ikk = ix + iz;
									gDataBuffer[ikk] = fnameName[iz];
								}
								else {
								    ikk = ix + iz;
								    ikj = iz - 8;
									gDataBuffer[ikk] = fnameExt[ikj];
								}
							}

							if (vtype == TYPE_CREATE_FILE)
								gDataBuffer[ix + 11] = 0x00;
							else
								gDataBuffer[ix + 11] = ATTR_DIRECTORY;

							// Ler Data/Hora do DS1307 - I2C
                            ds1307[3] = 01;
                            ds1307[4] = 01;
                            ds1307[5] = 2024;
                            ds1307[0] = 00;
                            ds1307[1] = 00;
                            ds1307[2] = 00;

						    // Converte para a Data/Hora da FAT32
							vdirtime = datetimetodir(ds1307[0], ds1307[1], ds1307[2], CONV_HORA);
							vdirdate = datetimetodir(ds1307[3], ds1307[4], ds1307[5], CONV_DATA);

							// Coloca dados no buffer para gravacao
							ikk = ix + 12;
							gDataBuffer[ikk] = 0x00;	// case
							ikk = ix + 13;
							gDataBuffer[ikk] = 0x00;	// creation time in ms
							ikk = ix + 14;
							gDataBuffer[ikk] = (BYTE)(vdirtime & 0xFF);	// creation time (ds1307)
							ikk = ix + 15;
							gDataBuffer[ikk] = (BYTE)((vdirtime >> 8) & 0xFF);
							ikk = ix + 16;
							gDataBuffer[ikk] = (BYTE)(vdirdate & 0xFF);	// creation date (ds1307)
							ikk = ix + 17;
							gDataBuffer[ikk] = (BYTE)((vdirdate >> 8) & 0xFF);
							ikk = ix + 18;
							gDataBuffer[ikk] = (BYTE)(vdirdate & 0xFF);	// last access	(ds1307)
							ikk = ix + 19;
							gDataBuffer[ikk] = (BYTE)((vdirdate >> 8) & 0xFF);

							ikk = ix + 22;
							gDataBuffer[ikk] = (BYTE)(vdirtime & 0xFF);	// time update (ds1307)
							ikk = ix + 23;
							gDataBuffer[ikk] = (BYTE)((vdirtime >> 8) & 0xFF);
							ikk = ix + 24;
							gDataBuffer[ikk] = (BYTE)(vdirdate & 0xFF);	// date update (ds1307)
							ikk = ix + 25;
							gDataBuffer[ikk] = (BYTE)((vdirdate >> 8) & 0xFF);

							ikk = ix + 26;
						    gDataBuffer[ikk] = (BYTE)(vclusterfile & 0xFF);
							ikk = ix + 27;
						    gDataBuffer[ikk] = (BYTE)((vclusterfile / 0x100) & 0xFF);
							ikk = ix + 20;
						    gDataBuffer[ikk] = (BYTE)((vclusterfile / 0x10000) & 0xFF);
							ikk = ix + 21;
						    gDataBuffer[ikk] = (BYTE)((vclusterfile / 0x1000000) & 0xFF);

							ikk = ix + 28;
							gDataBuffer[ikk] = 0x00;
							ikk = ix + 29;
							gDataBuffer[ikk] = 0x00;
							ikk = ix + 30;
							gDataBuffer[ikk] = 0x00;
							ikk = ix + 31;
							gDataBuffer[ikk] = 0x00;

							if (!fsSectorWrite(vdata, gDataBuffer, FALSE))
								return ERRO_D_WRITE_DISK;

							if (vtype == TYPE_CREATE_DIR) {
	  							// Posicionar na nova posicao do diretorio
                            	vtemp1 = ((vclusterfile - 2) * vdisk->SecPerClus);
                            	vtemp2 = (vdisk->reserv + vdisk->firsts + (2 * vdisk->fatsize));
                            	vdata = vtemp1 + vtemp2;

								// Limpar novo cluster do diretorio (Zerar)
								memset(gDataBuffer, 0x00, vdisk->sectorSize);

								for (iz = 0; iz < vdisk->SecPerClus; iz++) {
								    if (!fsSectorWrite(vdata, gDataBuffer, FALSE))
										return ERRO_D_WRITE_DISK;
									vdata++;
								}

                            	vtemp1 = ((vclusterfile - 2) * vdisk->SecPerClus);
                            	vtemp2 = (vdisk->reserv + vdisk->firsts + (2 * vdisk->fatsize));
                            	vdata = vtemp1 + vtemp2;

	  							// Criar diretorio . (atual)
	  							memset(gDataBuffer, 0x00, vdisk->sectorSize);

	  							ix = 0;
	  							gDataBuffer[0] = '.';
	  							gDataBuffer[1] = 0x20;
	  							gDataBuffer[2] = 0x20;
	  							gDataBuffer[3] = 0x20;
	  							gDataBuffer[4] = 0x20;
	  							gDataBuffer[5] = 0x20;
	  							gDataBuffer[6] = 0x20;
	  							gDataBuffer[7] = 0x20;
	  							gDataBuffer[8] = 0x20;
	  							gDataBuffer[9] = 0x20;
	  							gDataBuffer[10] = 0x20;

	  							gDataBuffer[11] = 0x10;

								gDataBuffer[12] = 0x00;	// case
								gDataBuffer[13] = 0x00;	// creation time in ms
								gDataBuffer[14] = (BYTE)(vdirtime & 0xFF);	// creation time (ds1307)
								gDataBuffer[15] = (BYTE)((vdirtime >> 8) & 0xFF);
								gDataBuffer[16] = (BYTE)(vdirdate & 0xFF);	// creation date (ds1307)
								gDataBuffer[17] = (BYTE)((vdirdate >> 8) & 0xFF);
								gDataBuffer[18] = (BYTE)(vdirdate & 0xFF);	// last access	(ds1307)
								gDataBuffer[19] = (BYTE)((vdirdate >> 8) & 0xFF);

								gDataBuffer[22] = (BYTE)(vdirtime & 0xFF);	// time update (ds1307)
								gDataBuffer[23] = (BYTE)((vdirtime >> 8) & 0xFF);
								gDataBuffer[24] = (BYTE)(vdirdate & 0xFF);	// date update (ds1307)
								gDataBuffer[25] = (BYTE)((vdirdate >> 8) & 0xFF);

	  						    gDataBuffer[26] = (BYTE)(vclusterfile & 0xFF);
	  						    gDataBuffer[27] = (BYTE)((vclusterfile / 0x100) & 0xFF);
	  						    gDataBuffer[20] = (BYTE)((vclusterfile / 0x10000) & 0xFF);
	  						    gDataBuffer[21] = (BYTE)((vclusterfile / 0x1000000) & 0xFF);

	  							gDataBuffer[28] = 0x00;
	  							gDataBuffer[29] = 0x00;
	  							gDataBuffer[30] = 0x00;
	  							gDataBuffer[31] = 0x00;

	  							// Criar diretorio .. (anterior)
	  							ix = 32;
	  							gDataBuffer[32] = '.';
	  							gDataBuffer[33] = '.';
	  							gDataBuffer[34] = 0x20;
	  							gDataBuffer[35] = 0x20;
	  							gDataBuffer[36] = 0x20;
	  							gDataBuffer[37] = 0x20;
	  							gDataBuffer[38] = 0x20;
	  							gDataBuffer[39] = 0x20;
	  							gDataBuffer[40] = 0x20;
	  							gDataBuffer[41] = 0x20;
	  							gDataBuffer[42] = 0x20;

	  							gDataBuffer[43] = 0x10;

								gDataBuffer[44] = 0x00;	// case
								gDataBuffer[45] = 0x00;	// creation time in ms
								gDataBuffer[46] = (BYTE)(vdirtime & 0xFF);	// creation time (ds1307)
								gDataBuffer[47] = (BYTE)((vdirtime >> 8) & 0xFF);
								gDataBuffer[48] = (BYTE)(vdirdate & 0xFF);	// creation date (ds1307)
								gDataBuffer[49] = (BYTE)((vdirdate >> 8) & 0xFF);
								gDataBuffer[50] = (BYTE)(vdirdate & 0xFF);	// last access	(ds1307)
								gDataBuffer[51] = (BYTE)((vdirdate >> 8) & 0xFF);

								gDataBuffer[54] = (BYTE)(vdirtime & 0xFF);	// time update (ds1307)
								gDataBuffer[55] = (BYTE)((vdirtime >> 8) & 0xFF);
								gDataBuffer[56] = (BYTE)(vdirdate & 0xFF);	// date update (ds1307)
								gDataBuffer[57] = (BYTE)((vdirdate >> 8) & 0xFF);

	  						    gDataBuffer[58] = (BYTE)(*vclusterdir & 0xFF);
	  						    gDataBuffer[59] = (BYTE)((*vclusterdir / 0x100) & 0xFF);
	  						    gDataBuffer[52] = (BYTE)((*vclusterdir / 0x10000) & 0xFF);
	  						    gDataBuffer[53] = (BYTE)((*vclusterdir / 0x1000000) & 0xFF);

	  							gDataBuffer[60] = 0x00;
	  							gDataBuffer[61] = 0x00;
	  							gDataBuffer[62] = 0x00;
	  							gDataBuffer[63] = 0x00;

	  						    if (!fsSectorWrite(vdata, gDataBuffer, FALSE))
	  								return ERRO_D_WRITE_DISK;
	              			}

							vdata = LAST_CLUSTER_FAT32;
							break;
						}

						vdata = LAST_CLUSTER_FAT32;
						break;
					}
				}
				else if (vtype != TYPE_FIRST_ENTRY) {
					if (vdir->Name[0] != DIR_EMPTY && vdir->Name[0] != DIR_DEL) {
						vcomp = 1;
						for (iz = 0; iz <= 10; iz++) {
							if (iz <= 7) {
								if (fnameName[iz] != vdir->Name[iz]) {
									vcomp = 0;
									break;
								}
							}
							else {
							    ikk = iz - 8;
								if (fnameExt[ikk] != vdir->Ext[ikk]) {
									vcomp = 0;
									break;
								}
							}
						}

						if (vcomp) {
							if (vtype == TYPE_ALL || (vtype == TYPE_FILE && vdir->Attr != ATTR_DIRECTORY) || (vtype == TYPE_DIRECTORY && vdir->Attr == ATTR_DIRECTORY)) {
		  						vclusterfile = vdir->FirstCluster;
		  						break;
	  						}
	  						else if (vtype == TYPE_NEXT_ENTRY) {
		  						vtype = TYPE_FIRST_ENTRY;
		  					}
	  						else if (vtype == TYPE_DEL_FILE || vtype == TYPE_DEL_DIR) {
								// Guardando Cluster Atual
								vclusteratual = vdir->FirstCluster;

		  						// Apagando no Diretorio
		                		gDataBuffer[ix] = DIR_DEL;
		                		ikk = ix + 26;
								gDataBuffer[ikk] = 0x00;
		                		ikk = ix + 27;
								gDataBuffer[ikk] = 0x00;
		                		ikk = ix + 20;
								gDataBuffer[ikk] = 0x00;
		                		ikk = ix + 21;
								gDataBuffer[ikk] = 0x00;

								if (!fsSectorWrite(vdata, gDataBuffer, FALSE))
		          			  		return ERRO_D_WRITE_DISK;

				                // Apagando vestigios na FAT
	          					while (1) {
				                    // Procura Proximo Cluster e ja zera
			           			    vclusterdirnew = fsFindNextCluster(vclusteratual, NEXT_FREE);

					                if (vclusterdirnew >= ERRO_D_START)
					                    return ERRO_D_NOT_FOUND;

					                if (vclusterdirnew == LAST_CLUSTER_FAT32) {
						                vclusterfile = LAST_CLUSTER_FAT32;
						          		vdata = LAST_CLUSTER_FAT32;
						          		break;
					                }

			            			// Tornar cluster atual o proximo
			            			vclusteratual = vclusterdirnew;
	          					}
	  						}
						}
					}
				}

				if (vdir->Name[0] == DIR_EMPTY) {
					vdata = LAST_CLUSTER_FAT32;
					break;
				}
			}

			if (vclusterfile < ERRO_D_START || vdata == LAST_CLUSTER_FAT32)
				break;

			ventrydir++;
			vdata++;
		}

		// Se conseguiu concluir a operacao solicitada, sai do loop
		if (vclusterfile < ERRO_D_START || vdata == LAST_CLUSTER_FAT32)
			break;
		else {
			// Posiciona na FAT, o endereco da pasta atual
			vsecfat = vclusterdirnew / 128;
			vfat = vdisk->fat + vsecfat;

		    if (!fsSectorRead(vfat, gDataBuffer))
				return ERRO_D_READ_DISK;

            vtemp = vclusterdirnew - ( 128 * vsecfat);
			vpos = vtemp * 4;
            ikk = vpos + 3;
			vclusterdirnew  = (DWORD)gDataBuffer[ikk] << 24;
            ikk = vpos + 2;
			vclusterdirnew |= (DWORD)gDataBuffer[ikk] << 16;
            ikk = vpos + 1;
			vclusterdirnew |= (DWORD)gDataBuffer[ikk] << 8;
            ikk = vpos;
			vclusterdirnew |= (DWORD)gDataBuffer[ikk];

			if (vclusterdirnew != LAST_CLUSTER_FAT32) {
				// Devolve a proxima posicao para procura/uso
            	vtemp1 = ((vclusterdirnew - 2) * vdisk->SecPerClus);
            	vtemp2 = (vdisk->reserv + vdisk->firsts + (2 * vdisk->fatsize));
            	vdata = vtemp1 + vtemp2;
			}
			else {
				// Se for para criar uma nova entrada no diretorio e nao tem mais espaco
				// Cria uma nova entrada na Fat
				if (vtype == TYPE_EMPTY_ENTRY || vtype == TYPE_CREATE_FILE || vtype == TYPE_CREATE_DIR) {
					vclusterdirnew = fsFindClusterFree(FREE_USE);

					if (vclusterdirnew < ERRO_D_START) {
					    if (!fsSectorRead(vfat, gDataBuffer))
							return ERRO_D_READ_DISK;

					    gDataBuffer[vpos] = (BYTE)(vclusterdirnew & 0xFF);
					    ikk = vpos + 1;
					    gDataBuffer[ikk] = (BYTE)((vclusterdirnew / 0x100) & 0xFF);
					    ikk = vpos + 2;
					    gDataBuffer[ikk] = (BYTE)((vclusterdirnew / 0x10000) & 0xFF);
					    ikk = vpos + 3;
					    gDataBuffer[ikk] = (BYTE)((vclusterdirnew / 0x1000000) & 0xFF);

					    if (!fsSectorWrite(vfat, gDataBuffer, FALSE))
							return ERRO_D_WRITE_DISK;

						// Posicionar na nova posicao do diretorio
                    	vtemp1 = ((vclusterdirnew - 2) * vdisk->SecPerClus);
                    	vtemp2 = (vdisk->reserv + vdisk->firsts + (2 * vdisk->fatsize));
                    	vdata = vtemp1 + vtemp2;

						// Limpar novo cluster do diretorio (Zerar)
						memset(gDataBuffer, 0x00, vdisk->sectorSize);

						for (iz = 0; iz < vdisk->SecPerClus; iz++) {
						    if (!fsSectorWrite(vdata, gDataBuffer, FALSE))
								return ERRO_D_WRITE_DISK;
							vdata++;
						}

                    	vtemp1 = ((vclusterdirnew - 2) * vdisk->SecPerClus);
                    	vtemp2 = (vdisk->reserv + vdisk->firsts + (2 * vdisk->fatsize));
                    	vdata = vtemp1 + vtemp2;
					}
					else {
						vclusterdirnew = LAST_CLUSTER_FAT32;
						vclusterfile = ERRO_D_NOT_FOUND;
						vdata = vclusterdirnew;
					}
				}
				else {
					vdata = vclusterdirnew;
				}
			}
		}
	}

	return vclusterfile;
}

//-------------------------------------------------------------------------
BYTE fsUpdateDir()
{
	BYTE iy;
	WORD ventry, ikk;

	if (!fsSectorRead(vdir->DirClusSec, gDataBuffer))
		return ERRO_B_READ_DISK;

    ventry = vdir->DirEntry;

	for (iy = 0; iy < 8; iy++) {
	    ikk = ventry + iy;
		gDataBuffer[ikk] = vdir->Name[iy];
	}

	for (iy = 0; iy < 3; iy++) {
	    ikk = ventry + 8 + iy;
		gDataBuffer[ikk] = vdir->Ext[iy];
	}

    ikk = ventry + 18;
	gDataBuffer[ikk] = (BYTE)(vdir->LastAccessDate & 0xFF);	// last access	(ds1307)
    ikk = ventry + 19;
	gDataBuffer[ikk] = (BYTE)((vdir->LastAccessDate / 0x100) & 0xFF);

    ikk = ventry + 22;
	gDataBuffer[ikk] = (BYTE)(vdir->UpdateTime & 0xFF);	// time update (ds1307)
    ikk = ventry + 23;
	gDataBuffer[ikk] = (BYTE)((vdir->UpdateTime / 0x100) & 0xFF);

    ikk = ventry + 24;
	gDataBuffer[ikk] = (BYTE)(vdir->UpdateDate & 0xFF);	// date update (ds1307)
    ikk = ventry + 25;
	gDataBuffer[ikk] = (BYTE)((vdir->UpdateDate / 0x100) & 0xFF);

    ikk = ventry + 28;
    gDataBuffer[ikk] = (BYTE)(vdir->Size & 0xFF);
    ikk = ventry + 29;
    gDataBuffer[ikk] = (BYTE)((vdir->Size / 0x100) & 0xFF);
    ikk = ventry + 30;
    gDataBuffer[ikk] = (BYTE)((vdir->Size / 0x10000) & 0xFF);
    ikk = ventry + 31;
    gDataBuffer[ikk] = (BYTE)((vdir->Size / 0x1000000) & 0xFF);

   if (!fsSectorWrite(vdir->DirClusSec, gDataBuffer, FALSE))
		return ERRO_B_WRITE_DISK;

	return RETURN_OK;
}

//-------------------------------------------------------------------------
DWORD fsFindNextCluster(DWORD vclusteratual, BYTE vtype)
{
	DWORD vfat, vclusternew;
	WORD vpos, vsecfat, ikk;

	vsecfat = vclusteratual / 128;
	vfat = vdisk->fat + vsecfat;

	if (!fsSectorRead(vfat, gDataBuffer))
		return ERRO_D_READ_DISK;

	vpos = (vclusteratual - ( 128 * vsecfat)) * 4;
	ikk = vpos + 3;
	vclusternew  = (DWORD)gDataBuffer[ikk] << 24;
	ikk = vpos + 2;
	vclusternew |= (DWORD)gDataBuffer[ikk] << 16;
	ikk = vpos + 1;
	vclusternew |= (DWORD)gDataBuffer[ikk] << 8;
	vclusternew |= (DWORD)gDataBuffer[vpos];

	if (vtype != NEXT_FIND) {
		if (vtype == NEXT_FREE) {
			gDataBuffer[vpos] = 0x00;
        	ikk = vpos + 1;
			gDataBuffer[ikk] = 0x00;
        	ikk = vpos + 2;
			gDataBuffer[ikk] = 0x00;
        	ikk = vpos + 3;
			gDataBuffer[ikk] = 0x00;
		}
		else if (vtype == NEXT_FULL) {
			gDataBuffer[vpos] = 0xFF;
        	ikk = vpos + 1;
			gDataBuffer[ikk] = 0xFF;
        	ikk = vpos + 2;
			gDataBuffer[ikk] = 0xFF;
        	ikk = vpos + 3;
			gDataBuffer[ikk] = 0x0F;
		}

		if (!fsSectorWrite(vfat, gDataBuffer, FALSE))
			return ERRO_D_WRITE_DISK;
	}

  return vclusternew;
}

//-------------------------------------------------------------------------
DWORD fsFindClusterFree(BYTE vtype)
{
  	DWORD vclusterfree = 0x00, cc, vfat;
	WORD jj, ikk, ikk2, ikk3;

	vfat = vdisk->fat;

	for (cc = 0; cc <= vdisk->fatsize; cc++) {
	    // LER FAT SECTOR
		if (!fsSectorRead(vfat, gDataBuffer))
			return ERRO_D_READ_DISK;

		// Procura Cluster Livre dentro desse setor
		for (jj = 0; jj < vdisk->sectorSize; jj += 4) {
		    ikk = jj + 1;
		    ikk2 = jj + 2;
		    ikk3 = jj + 3;
			if (gDataBuffer[jj] == 0x00 && gDataBuffer[ikk] == 0x00 && gDataBuffer[ikk2] == 0x00 && gDataBuffer[ikk3] == 0x00)
			    break;

			vclusterfree++;
		}

		// Se achou algum setor livre, sai do loop
		if (jj < vdisk->sectorSize)
			break;

		// Soma mais 1 para procurar proximo cluster
		vfat++;
	}

	if (cc > vdisk->fatsize)
		vclusterfree = ERRO_D_DISK_FULL;
	else {
		if (vtype == FREE_USE) {
		    gDataBuffer[jj] = 0xFF;
		    ikk = jj + 1;
		    gDataBuffer[ikk] = 0xFF;
		    ikk = jj + 2;
		    gDataBuffer[ikk] = 0xFF;
		    ikk = jj + 3;
		    gDataBuffer[ikk] = 0x0F;

		    if (!fsSectorWrite(vfat, gDataBuffer, FALSE))
				return ERRO_D_WRITE_DISK;
		}
	}

	return (vclusterfree);
}

//-------------------------------------------------------------------------
BYTE fsFormat (long int serialNumber, char * volumeID)
{
    WORD    j;
    DWORD   secCount, RootDirSectors;
    DWORD   root, fat, firsts, fatsize, test;
    DWORD   Index;
	BYTE    SecPerClus;

    BYTE *  dataBufferPointer = gDataBuffer;

	// Ler MBR
    if (!fsSectorRead(0x00, gDataBuffer))
		return ERRO_B_READ_DISK;

    secCount  = (DWORD)gDataBuffer[461] << 24;
    secCount |= (DWORD)gDataBuffer[460] << 16;
    secCount |= (DWORD)gDataBuffer[459] << 8;
    secCount |= (DWORD)gDataBuffer[458];

    firsts  = (DWORD)gDataBuffer[457] << 24;
    firsts |= (DWORD)gDataBuffer[456] << 16;
    firsts |= (DWORD)gDataBuffer[455] << 8;
    firsts |= (DWORD)gDataBuffer[454];

    *(dataBufferPointer + 450) = 0x0B;

    if (!fsSectorWrite (0x00, gDataBuffer, TRUE))
		return ERRO_B_WRITE_DISK;

	//-------------------

	if (secCount >= 0x000EEB7F && secCount <= 0x01000000)	// 512 MB to 8 GB, 8 sectors per cluster
		SecPerClus = 8;
	else if (secCount > 0x01000000 && secCount <= 0x02000000) // 8 GB to 16 GB, 16 sectors per cluster
		SecPerClus = 16;
	else if (secCount > 0x02000000 && secCount <= 0x04000000) // 16 GB to 32 GB, 32 sectors per cluster
		SecPerClus = 32;
	else if (secCount > 0x04000000) // More than 32 GB, 64 sectors per cluster
		SecPerClus = 64;
	//-------------------

	//-------------------
    fatsize = (secCount - 0x26);
    fatsize = (fatsize / ((256 * SecPerClus + 2) / 2));
    fat = 0x26 + firsts;
    root = fat + (2 * fatsize);
	//-------------------

	// Formata MicroSD
    memset (gDataBuffer, 0x00, MEDIA_SECTOR_SIZE);

    // Non-file system specific values
    gDataBuffer[0] = 0xEB;         //Jump instruction
    gDataBuffer[1] = 0x3C;
    gDataBuffer[2] = 0x90;
    gDataBuffer[3] =  'M';         //OEM Name
    gDataBuffer[4] =  'M';
    gDataBuffer[5] =  'S';
    gDataBuffer[6] =  'J';
    gDataBuffer[7] =  ' ';
    gDataBuffer[8] =  'F';
    gDataBuffer[9] =  'A';
    gDataBuffer[10] = 'T';

    gDataBuffer[11] = 0x00;             //Sector size
    gDataBuffer[12] = 0x02;

    gDataBuffer[13] = SecPerClus;   //Sectors per cluster

    gDataBuffer[14] = 0x26;         //Reserved sector count
    gDataBuffer[15] = 0x00;

	fat = 0x26 + firsts;

    gDataBuffer[16] = 0x02;         //number of FATs

    gDataBuffer[17] = 0x00;          //Max number of root directory entries - 512 files allowed
    gDataBuffer[18] = 0x00;

    gDataBuffer[19] = 0x00;         //total sectors
    gDataBuffer[20] = 0x00;

    gDataBuffer[21] = 0xF8;         //Media Descriptor

    gDataBuffer[22] = 0x00;         //Sectors per FAT
    gDataBuffer[23] = 0x00;

    gDataBuffer[24] = 0x3F;         //Sectors per track
    gDataBuffer[25] = 0x00;

    gDataBuffer[26] = 0xFF;         //Number of heads
    gDataBuffer[27] = 0x00;

    // Hidden sectors = sectors between the MBR and the boot sector
    gDataBuffer[28] = (BYTE)(firsts & 0xFF);
    gDataBuffer[29] = (BYTE)((firsts / 0x100) & 0xFF);
    gDataBuffer[30] = (BYTE)((firsts / 0x10000) & 0xFF);
    gDataBuffer[31] = (BYTE)((firsts / 0x1000000) & 0xFF);

    // Total Sectors = same as sectors in the partition from MBR
    gDataBuffer[32] = (BYTE)(secCount & 0xFF);
    gDataBuffer[33] = (BYTE)((secCount / 0x100) & 0xFF);
    gDataBuffer[34] = (BYTE)((secCount / 0x10000) & 0xFF);
    gDataBuffer[35] = (BYTE)((secCount / 0x1000000) & 0xFF);

	// Sectors per FAT
	gDataBuffer[36] = (BYTE)(fatsize & 0xFF);
    gDataBuffer[37] = (BYTE)((fatsize / 0x100) & 0xFF);
    gDataBuffer[38] = (BYTE)((fatsize / 0x10000) & 0xFF);
    gDataBuffer[39] = (BYTE)((fatsize / 0x1000000) & 0xFF);

    gDataBuffer[40] = 0x00;         //Active FAT
    gDataBuffer[41] = 0x00;

    gDataBuffer[42] = 0x00;         //File System version
    gDataBuffer[43] = 0x00;

    gDataBuffer[44] = 0x02;         //First cluster of the root directory
    gDataBuffer[45] = 0x00;
    gDataBuffer[46] = 0x00;
    gDataBuffer[47] = 0x00;

    gDataBuffer[48] = 0x01;         //FSInfo
    gDataBuffer[49] = 0x00;

    gDataBuffer[50] = 0x00;         //Backup Boot Sector
    gDataBuffer[51] = 0x00;

    gDataBuffer[52] = 0x00;         //Reserved for future expansion
    gDataBuffer[53] = 0x00;
    gDataBuffer[54] = 0x00;
    gDataBuffer[55] = 0x00;
    gDataBuffer[56] = 0x00;
    gDataBuffer[57] = 0x00;
    gDataBuffer[58] = 0x00;
    gDataBuffer[59] = 0x00;
    gDataBuffer[60] = 0x00;
    gDataBuffer[61] = 0x00;
    gDataBuffer[62] = 0x00;
    gDataBuffer[63] = 0x00;

    gDataBuffer[64] = 0x00;         // Physical drive number

    gDataBuffer[65] = 0x00;         // Reserved (current head)

    gDataBuffer[66] = 0x29;         // Signature code

    gDataBuffer[67] = (BYTE)(serialNumber & 0xFF);
    gDataBuffer[68] = (BYTE)((serialNumber / 0x100) & 0xFF);
    gDataBuffer[69] = (BYTE)((serialNumber / 0x10000) & 0xFF);
    gDataBuffer[70] = (BYTE)((serialNumber / 0x1000000) & 0xFF);

    // Volume ID
    if (volumeID != NULL)
    {
        for (Index = 0; (*(volumeID + Index) != 0) && (Index < 11); Index++)
        {
            gDataBuffer[Index + 71] = *(volumeID + Index);
        }
        while (Index < 11)
        {
            gDataBuffer[71 + Index++] = 0x20;
        }
    }
    else
    {
        for (Index = 0; Index < 11; Index++)
        {
            gDataBuffer[Index+71] = 0;
        }
    }

    gDataBuffer[82] = 'F';
    gDataBuffer[83] = 'A';
    gDataBuffer[84] = 'T';
    gDataBuffer[85] = '3';
    gDataBuffer[86] = '2';
    gDataBuffer[87] = ' ';
    gDataBuffer[88] = ' ';
    gDataBuffer[89] = ' ';

    gDataBuffer[510] = 0x55;
    gDataBuffer[511] = 0xAA;

	if (!fsSectorWrite(firsts, gDataBuffer, FALSE))
		return ERRO_B_WRITE_DISK;

    // Erase the FAT
    memset (gDataBuffer, 0x00, MEDIA_SECTOR_SIZE);

    gDataBuffer[0] = 0xF8;          //BPB_Media byte value in its low 8 bits, and all other bits are set to 1
    gDataBuffer[1] = 0xFF;
    gDataBuffer[2] = 0xFF;
    gDataBuffer[3] = 0x0F;

    gDataBuffer[4] = 0xFF;          //Disk is clean and no read/write errors were encountered
    gDataBuffer[5] = 0xFF;
    gDataBuffer[6] = 0xFF;
    gDataBuffer[7] = 0xFF;

    gDataBuffer[8]  = 0xFF;         //Root Directory EOF
    gDataBuffer[9]  = 0xFF;
    gDataBuffer[10] = 0xFF;
    gDataBuffer[11] = 0x0F;

    for (j = 1; j != 0xFFFF; j--)
    {
        if (!fsSectorWrite (fat + (j * fatsize), gDataBuffer, FALSE))
			return ERRO_B_WRITE_DISK;
    }

    memset (gDataBuffer, 0x00, 12);

    for (Index = fat + 1; Index < (fat + fatsize); Index++)
    {
        for (j = 1; j != 0xFFFF; j--)
        {
            if (!fsSectorWrite (Index + (j * fatsize), gDataBuffer, FALSE))
				return ERRO_B_WRITE_DISK;
        }
    }

    // Erase the root directory
    for (Index = 1; Index < SecPerClus; Index++)
    {
        if (!fsSectorWrite (root + Index, gDataBuffer, FALSE))
			return ERRO_B_WRITE_DISK;
    }

    // Create a drive name entry in the root dir
    Index = 0;
    while ((*(volumeID + Index) != 0) && (Index < 11))
    {
        gDataBuffer[Index] = *(volumeID + Index);
        Index++;
    }
    while (Index < 11)
    {
        gDataBuffer[Index++] = ' ';
    }
    gDataBuffer[11] = 0x08;
    gDataBuffer[17] = 0x11;
    gDataBuffer[19] = 0x11;
    gDataBuffer[23] = 0x11;

    if (!fsSectorWrite (root, gDataBuffer, FALSE))
		return ERRO_B_WRITE_DISK;

	return RETURN_OK;
}

//-------------------------------------------------------------------------
BYTE fsSectorRead(DWORD vsector, BYTE* vbuffer){
    BYTE vbytes[4], dd, vByte = 0;
    unsigned int ix, cc;
    DWORD vsectorok;
    unsigned char sqtdtam[11];

    vsectorok = (vsector & 0xFF000000) >> 24;
    vbytes[0] = (BYTE)vsectorok;
    vsectorok = (vsector & 0x00FF0000) >> 16;
    vbytes[1] = (BYTE)vsectorok;
    vsectorok = (vsector & 0x0000FF00) >> 8;
    vbytes[2] = (BYTE)vsectorok;
    vsectorok = vsector & 0x000000FF;
    vbytes[3] = (BYTE)vsectorok;

    // Envia comando resetar e abortar tudo
    fsSendByte('a', FS_CMD);

    // Comando recebido ok ?
    vByte = fsRecByte(FS_CMD);

    if (vByte != ALL_OK)
        return 0;

    // Envia Cluster
    fsSendByte(vbytes[0], FS_PAR);
    fsSendByte(vbytes[1], FS_PAR);
    fsSendByte(vbytes[2], FS_PAR);
    fsSendByte(vbytes[3], FS_PAR);
    // Envia offset
    fsSendByte(0x00, FS_PAR);
    fsSendByte(0x00, FS_PAR);
    // Envia Qtd (512)
    fsSendByte(0x02, FS_PAR);
    fsSendByte(0x00, FS_PAR);

    // Envia comando
    fsSendByte('r', FS_CMD);

    // Comando recebido ok ?
    vByte = fsRecByte(FS_CMD);

    if (vByte != ALL_OK)
        return 0;

    // Comando Executado ok ?
    vByte = fsRecByte(FS_CMD);

    if (vByte != ALL_OK)
        return 0;

    // Carrega Dados Recebidos
    for (cc = 0; cc < 512 ; cc++)
    {
        vByte = fsRecByte(FS_DATA);
        *(vbuffer + cc) = vByte;
    }

    return 1;
}

//-------------------------------------------------------------------------
BYTE fsSectorWrite(DWORD vsector, BYTE* vbuffer, BYTE vtipo){
    BYTE vbytes[4], dd, vByte = 0;
    unsigned int ix, cc;
    DWORD vsectorok;
    unsigned char sqtdtam[11];

    vsectorok = (vsector & 0xFF000000) >> 24;
    vbytes[0] = (BYTE)vsectorok;
    vsectorok = (vsector & 0x00FF0000) >> 16;
    vbytes[1] = (BYTE)vsectorok;
    vsectorok = (vsector & 0x0000FF00) >> 8;
    vbytes[2] = (BYTE)vsectorok;
    vsectorok = vsector & 0x000000FF;
    vbytes[3] = (BYTE)vsectorok;

    // Envia comando resetar e abortar tudo
    fsSendByte('a', FS_CMD);

    // Comando recebido ok ?
    vByte = fsRecByte(FS_CMD);

    if (vByte != ALL_OK)
        return 0;

    // Envia Buffer
    for (cc = 0; cc < 512 ; cc++)
    {
        vByte = *(vbuffer + cc);
        fsSendByte(vByte, FS_DATA);
    }

    // Envia Cluster
    fsSendByte(vbytes[0], FS_PAR);
    fsSendByte(vbytes[1], FS_PAR);
    fsSendByte(vbytes[2], FS_PAR);
    fsSendByte(vbytes[3], FS_PAR);

    // Envia comando
    fsSendByte('w', FS_CMD);

    // Comando recebido ok ?
    vByte = fsRecByte(FS_CMD);

    if (vByte != ALL_OK)
        return 0;

    // Comando Executado ok ?
    vByte = fsRecByte(FS_CMD);

    if (vByte != ALL_OK)
        return 0;

    return 1;
}

//-----------------------------------------------------------------------------
void catFile(BYTE *parquivo) {
    WORD vbytepic;
    BYTE *mcfgfileptr = mcfgfile, vqtd = 1;
    BYTE *parqptr = parquivo;
    DWORD vsizefile;
    BYTE sqtdtam[10];

    while (*parqptr++)
        vqtd++;

    vsizefile = loadFile(parquivo, (unsigned long*)0x00611F04);   // 12K espaco pra carregar arquivo. Colocar logica pra pegar tamanho e alocar espaco

    if (!*verro) {
        /*itoa(vsizefile, sqtdtam, 10);
        printText(sqtdtam);
        printText("\r\n\0");*/

        while (vsizefile > 0) {
            /*itoa(vsizefile, sqtdtam, 10);
            printText(sqtdtam);
            printText("\r\n\0");*/

            if (*mcfgfileptr == 0x0D) {
                printText("\r\0");
            }
            else if (*mcfgfileptr == 0x0A) {
                printText("\r\n\0");
            }
            else if (*mcfgfileptr == 0x1A || *mcfgfileptr == 0x00) {
                    break;
            }
            else {
                if (*mcfgfileptr >= 0x20 && *mcfgfileptr < 0xFF)
                    printChar(*mcfgfileptr, 1);
                else
                    printChar(0x20, 1);
            }

            mcfgfileptr++;
            vsizefile--;
        }
    }
    else {
        printText("Loading file error...\r\n\0");
    }
}

//-----------------------------------------------------------------------------
DWORD loadFile(BYTE *parquivo, unsigned short* xaddress)
{
    unsigned short cc, dd;
    BYTE vbuffer[512];
    unsigned int vbytegrava = 0;
    unsigned short xdado = 0, xcounter = 0;
    unsigned short vcrc, vcrcpic, vloop;
    DWORD vsizeR, vsizefile = 0;

	vsizefile = 0;
    *verro = 0;

    if (fsFindDirPath(parquivo, FIND_PATH_PART) == FIND_PATH_RET_ERROR)
    {
        *verro = 1;
        return vsizefile;
    }

    *vclusterdir = vretpath->ClusterDir;

    if (fsOpenFile(vretpath->Name) == RETURN_OK)
    {
		while (1)
        {
			vsizeR = fsReadFile(vretpath->Name, vsizefile, vbuffer, 512);

			if (vsizeR != 0)
            {
                for (dd = 0; dd < 512; dd += 2)
                {
                	vbytegrava = (WORD)vbuffer[dd] << 8;
                	vbytegrava = vbytegrava | (vbuffer[dd + 1] & 0x00FF);

                    // Grava Dados na Posição Especificada
                    *xaddress = vbytegrava;
                    xaddress += 1;
                }

                vsizefile += 512;
			}
			else
				break;
		}

        // Fecha o Arquivo
    	fsCloseFile(vretpath->Name, 0);
    }
    else
        *verro = 1;

    *vclusterdir = vretpath->ClusterDirAtu;

    return vsizefile;
}

//-------------------------------------------------------------------------
WORD datetimetodir(BYTE hr_day, BYTE min_month, BYTE sec_year, BYTE vtype)
{
	WORD vconv = 0, vtemp;

	if (vtype == CONV_DATA) {
	    vtemp = sec_year - 1980;
		vconv  = (WORD)(vtemp & 0x7F) << 9;
		vconv |= (WORD)(min_month & 0x0F) << 5;
		vconv |= (WORD)(hr_day & 0x1F);
	}
	else {
		vconv  = (WORD)(hr_day & 0x1F) << 11;
		vconv |= (WORD)(min_month & 0x3F) << 5;
		vtemp = sec_year / 2;
		vconv |= (WORD)(vtemp & 0x1F);
	}

	return vconv;
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
void strncpy2( char* _dst, const char* _src, int _n )
{
    int i = 0;
    while(i != _n)
    {
        *_dst = *_src;
        _dst++;
        _src++;
        i++;
    }
}

//-----------------------------------------------------------------------------
int isValidFilename(char *filename)
{
    char valid_chars[60];
    int len, i;
    char name_part[9];
    char ext_part[4];
    char *dot;
    int name_len = 0, ext_len = 0;

    strcpy(valid_chars,"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!#$&'()@^_`{}~");

    len = strlen(filename);

    // Verificar comprimento total
    if (len == 0 || len > 12) {
        return 0;
    }

    // Dividir o nome e a extensão (se existir)
    name_part[0] = '\0';
    ext_part[0] = '\0';
    dot = strchr(filename, '.');

    if (dot) {
        // Nome e extensão devem ser separados pelo ponto
        name_len = dot - filename;
        ext_len = len - name_len - 1;

        if (name_len == 0 || name_len > 8 || ext_len > 3) {
            return 0; // Nome ou extensão inválidos
        }

        strncpy2(name_part, filename, name_len);
        name_part[name_len] = 0x00;
        strncpy2(ext_part, dot + 1, ext_len);
        ext_part[ext_len] = 0x00;
    } else {
        // Sem ponto, apenas o nome principal
        if (len > 8) {
            return 0;
        }
        strncpy2(name_part, filename, len);
        name_part[len] = 0x00;
    }

    // Validar o nome
    for (i = 0; name_part[i] != '\0'; i++) {
        if (!strchr(valid_chars, toupper(name_part[i]))) {
            return 0;
        }
    }

    // Validar a extensão (se houver)
    for (i = 0; ext_part[i] != '\0'; i++) {
        if (!strchr(valid_chars, toupper(ext_part[i]))) {
            return 0;
        }
    }

    return 1; // Tudo está correto
}

// Função para verificar se um nome de arquivo corresponde ao padrão
BYTE matches_wildcard(const char *pattern, const char *filename)
{
    while (*pattern && *filename) 
    {
        if (*pattern == '*') 
        {
            // Avança no padrão e tenta corresponder com todos os sufixos possíveis
            pattern++;

            if (!*pattern) 
                return 1; // '*' no final combina com qualquer coisa

            while (*filename) 
            {
                if (matches_wildcard(pattern, filename)) 
                    return 1;

                filename++;
            }
            return 0;
        } 
        else if (*pattern == '?' || *pattern == *filename) 
        {
            // '?' combina com qualquer caractere ou caracteres iguais
            pattern++;
            filename++;
        } 
        else 
        {
            return 0;
        }
    }
    // Retorna true se ambos terminarem juntos
    return (!*pattern && !*filename);
}

//-----------------------------------------------------------------------------
// Função principal para filtrar arquivos
//-----------------------------------------------------------------------------
void filter_files(const char *pattern, const char **file_list, int file_count, char **result_list, int *result_count)
{
    int count = 0, i;
    for (i = 0; i < file_count; i++) 
    {
        if (matches_wildcard(pattern, file_list[i])) 
        {
            result_list[count++] = file_list[i];
        }
    }

    *result_count = count;
}

//-----------------------------------------------------------------------------
BYTE contains_wildcards(const char *pattern) 
{
    while (*pattern) 
    {
        if (*pattern == '*' || *pattern == '?') 
        {
            return 1;
        }
        pattern++;
    }
    return 0;
}