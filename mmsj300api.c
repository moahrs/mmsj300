//-----------------------------------------------------------------------------//-----------------------------------------------------------------------------
void clearScr(WORD pcolor) {
    *vpicg = 2;
    *vpicg = 0xD0;
    *vpicg = pcolor;

    locateScr(0, 0, REPOS_CURSOR);
}

//-----------------------------------------------------------------------------
void printPromptScr(WORD plinadd) {
    if (plinadd)
        *vlin = *vlin + 1;

    locateScr(0,*vlin, NOREPOS_CURSOR);

    printStrScr("#\0", White, Black);
    printStrScr(vdiratu, White, Black);
    printStrScr(">\0", White, Black);

    *vinip = *vcol;
}

//-----------------------------------------------------------------------------
void printStrScr(BYTE *msgs, WORD pcolor, WORD pbcolor) {
    BYTE ix = 10, iy, ichange = 0;
    BYTE *ss = msgs;

    while (*ss) {
      if (*ss >= 0x20)
          ix++;
      else
          ichange = 1;

      if ((*vcol + (ix - 10)) > *vxmax)
          ichange = 2;

      *ss++;

      if (!*ss && !ichange)
         ichange = 3;

      if (ichange) {
         // Manda Sequencia de Controle
         if (ix > 10) {
            *vpicg = ix;
            *vpicg = 0xD1;
            *vpicg = (*vcol * 8) >> 8;
            *vpicg = *vcol * 8;
            *vpicg = (*vlin * 10) >> 8;
            *vpicg = *vlin * 10;
            *vpicg = 8;
            *vpicg = pcolor >> 8;
            *vpicg = pcolor;
            *vpicg = pbcolor >> 8;
            *vpicg = pbcolor;
         }

         if (ichange == 1)
            ix++;

         iy = 11;
         while (*msgs && iy <= ix) {
            if (*msgs >= 0x20) {
                *vpicg = *msgs;
                *vcol = *vcol + 1;
            }
            else {
                if (*msgs == 0x0D) {
                    *vcol = 0;
                }
                else if (*msgs == 0x0A) {
                    *vcol = 0;  // So para teste, despois tiro e coloco '\r' junto com '\n'
                    *vlin = *vlin + 1;
                }

                locateScr(*vcol, *vlin, NOREPOS_CURSOR);
            }

            *msgs++;
            iy++;
        }

        if (ichange == 2) {
            *vcol = 0;
            *vlin = *vlin + 1;
            locateScr(*vcol, *vlin, NOREPOS_CURSOR);
        }

        ichange = 0;
        ix = 10;
      }
    }
}

//-----------------------------------------------------------------------------
void printByteScr(BYTE pbyte, WORD pcolor, WORD pbcolor) {
    *vpicg = 0x0B;
    *vpicg = 0xD2;
    *vpicg = (*vcol * 8) >> 8;
    *vpicg = *vcol * 8;
    *vpicg = (*vlin * 10) >> 8;
    *vpicg = *vlin * 10;
    *vpicg = 8;
    *vpicg = pcolor >> 8;
    *vpicg = pcolor;
    *vpicg = pbcolor >> 8;
    *vpicg = pbcolor;
    *vpicg = pbyte;

    *vcol = *vcol + 1;

    locateScr(*vcol, *vlin, REPOS_CURSOR_ON_CHANGE);
}

//-----------------------------------------------------------------------------
void locateScr(BYTE pcol, BYTE plin, BYTE pcur) {
    WORD vend, ix, iy, ichange = 0;
    WORD vlcdf[16];

    if (pcol > *vxmax) {
        pcol = 0;
        plin++;
        ichange = 1;
    }

    if (plin > *vymax) {
        *vpicg = 2;
        *vpicg = 0xD9;
        *vpicg = 10;
        pcol = 0;
        plin = *vymax;
        ichange = 1;
    }

    *vcol = pcol;
    *vlin = plin;

    if (pcur == 1 || (pcur == 2 && ichange)) {
        printByteScr(0x08, White, Black);
        *vcol = *vcol - 1;
    }
}

//-----------------------------------------------------------------------------
void loadFile(unsigned short* xaddress)
{
  unsigned short cc, dd;
  unsigned short vrecfim, vbytepic, vbyteprog[128];
  unsigned int vbytegrava = 0;
  unsigned short xdado = 0, xcounter = 0;
  unsigned short vcrc, vcrcpic, vloop;

  vrecfim = 1;
  *verro = 0;

  while (vrecfim) {
    vloop = 1;
    while (vloop) {
        // Processa Retorno do PIC
      	recPic();

        if (vbytepic == picCommData) {
            // Carrega Dados Recebidos
            vcrc = 0;
    		for (cc = 0; cc <= 127 ; cc++)
      		{
          		recPic(); // Ler dados do PIC
      			vbyteprog[cc] = vbytepic;
      			vcrc += vbytepic;
      		}

            // Recebe 2 Bytes CRC
      		recPic();
      		vcrcpic = vbytepic;
      		recPic();
      		vcrcpic |= ((vbytepic << 8) & 0xFF00);

            if (vcrc == vcrcpic) {
                sendPic(0x01);
                sendPic(0xC5);
                vloop = 0;
            }
            else {
                sendPic(0x01);
                sendPic(0xFF);
            }
        }
        else if (vbytepic == picCommStop) {
            // Finaliza Comunicação Serial
            vloop = 0;
      		vrecfim = 0;
        }
        else {
            vloop = 0;
            vrecfim = 0;
            *verro = 1;
        }
    }

    if (vrecfim) {
        for (dd = 00; dd <= 127; dd += 2){
        	vbytegrava = vbyteprog[dd] << 8;
        	vbytegrava = vbytegrava | (vbyteprog[dd + 1] & 0x00FF);

            // Grava Dados na Posição Especificada
            *xaddress = vbytegrava;
            xaddress += 1;
        }
    }
  }
}