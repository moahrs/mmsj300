/********************************************************************************
*    Programa    : modulos.cpp
*    Objetivo    : Comunica��o serial com o Modulo para grava��o via PIC
*    Criado em   : 20/12/2011
*    Programador : Moacir Jr.
*--------------------------------------------------------------------------------
*	  ------------------------------------------
*	  PROTOCOLO DE COMUNICACAO:
*	  ------------------------------------------
*	  SENTIDO  
*	   PC/PIC  COMANDO  FUN��O
*	      ->   DDDD     INICIA COMUNICACAO
*	   <-      DDDD     COMUNICACAO ESTABELECIDA
*	   <- ->   DDdd     RECEBE OU ENVIA DADO (dd)
*	      ->   EE00     DADO VERIFICADO COM SUCESSO
*	      ->   EE01     ERRO NA VERIFICA��O DO DADO
*	   <-      EE69     ENVIAR BYTE
*	   <-      EE70     INICIO DO PROCESSO DE GRAVA��O
*	   <-      EE71     GRAVA��O BYTE OK
*	   <-      EE72     GRAVA��O BYTE COM ERRO
*	   <-      EE73     TERMINO DO PROCESSO DE GRAVA��O
*	      ->   EEDD     ENCERRA COMUNICACAO
*	  ------------------------------------------
*--------------------------------------------------------------------------------
* Data        Responsavel  Motivo
* 20/12/2011  Moacir Jr.   cria��o
********************************************************************************/

#include <stdio.h> 
#include <stdlib.h> 
#include <conio.h> 
#include <dos.h> 
#include <iostream>
#include <string>
#include <windows.h>  //Necess�rio para: LoadLibrary(), GetProcAddress() e HINSTANCE.
using namespace std;

unsigned char endlsb, endlsb_bkp;
unsigned char endmsb, endmsb_bkp;
unsigned char endhighmsb;
unsigned char RetByte;       // Para armazenar o valor recebido da Porta Paralela.
unsigned char RetInput[2], RetErro; 
unsigned char showVerb = 0;

HANDLE hSerial = NULL;

void delay(unsigned int ttempo);
int IniciaSerial();
void TerminaSerial();
DWORD EnviaComando(unsigned char sbyte);
DWORD EnviaCtrl(unsigned char sbyte); 
DWORD RecebeComando();
DWORD RecebeByte();
void AcompanhaGravacao();

// Programa Principal
int main(int argc, char *argv[])
{
  FILE *fp;
	char xfilebin[100] = "d:\\projetos\\mmsj300\\";
	int vnumFF = 0, verro, vnumerros, vsendlsb, vsendmsb, vsenddados;
	unsigned char dados, dadosf;
	unsigned char dadosrec;
  string argum;
  
	if (argc < 5)
	{
        if (argc < 3) 
        {
            printf("Erro. Nao foi passado o programa a ser enviado.\n");
            printf("Sintaxe: sendromword2 <nome do arquivo> <msb/lsb> [endinic]\n");
            printf("         <nome do arquivo> - nome do arquivo a importar com .bin\n");
            printf("         <lsb/msb/word> - lsb - grava dado LSB (grava no chip com 0xFE)\n");
            printf("                          msb - grava dado MSB (grava no chip com 0xFF)\n");
            printf("                          word - envia todos os dados na ordem que sao lidos (grava em ambos os chips)\n");
            printf("         [endinic] - em hex no formato 9999. Default 0000\n");
            printf("         [verb] -saida detalhado\n");
            return 0;
      	}
  	    else if (argc < 4)
        {
      	    argum = "0000";
        }
        else 
        {
            argum = argv[3];
        } 
    }
	else
    {
        if (strcmp(argv[4],"verb") == 0)
            showVerb = 1;
    }

	// Variaveis de Saida
	clrscr();
	
	// Inicio da Rotina
    if (showVerb)
    	printf(">Iniciando Programa��o do Modulo via porta Serial. Usando %s.\n", argv[1]);

    if (showVerb)
	    printf(">Inicializando Porta Serial (COM4, 19200, 8, N, 1).\n");
	IniciaSerial();

	if (showVerb)
	    printf(">Abrindo Comunica��o.\n");
	EnviaComando(0xDD);  // Envia Inicio de Comunica��o: 0xDD + 0xDD
	
	if (showVerb)
	    printf(">Aguardando Confirma��o.\n");
	RecebeComando(); // Recebe Confirma��o de Inicio: 0xDD + 0xDD
	
	if (RetInput[0] != 0xDD || RetInput[1] != 0xDD)
		vnumFF = 255;

	// Gravar Dados
	if (vnumFF == 0) 
    {
		if (showVerb)
	        printf(">Abrindo Arquivo.\n");
  	    strcat(xfilebin,argv[1]);
  	    fp = fopen(xfilebin,"rb");
    
        if (fp == NULL) {
  		    printf(">Erro ao Abrir Arquivo.\n");
        vnumFF = 255;
    }
  }
  else
	printf(">Comunica��o n�o Estabelecida. Recebido %02X:%02X. Verifique.\n", RetInput[0], RetInput[1]);

	// Enviando Dados (Adress LSB -> MSB -> Dados)
  if (vnumFF != 255) {
    sscanf(argum.substr(2,2).c_str(), "%x", &endlsb);
    sscanf(argum.substr(0,2).c_str(), "%x", &endmsb);
    
    vsendlsb = 0;
    vsendmsb = 0;
    vsenddados = 0;

    printf(">Enviando Dados.\n");
    
  	while (!feof(fp) && vnumFF <= 20)
  	{
  		dadosf = getc(fp);
  		
  		if (dados == 0xFF)
  			vnumFF += 1;
  
  		if (dados != 0xFF)
  			vnumFF = 0;

      if (!vsendlsb || !vsendmsb || (strcmp(argv[2],"word") == 0) || (strcmp(argv[2],"lsb") == 0 && (endlsb & 0x01) == 0) || (strcmp(argv[2],"msb") == 0 && (endlsb & 0x01) == 1)) 
      {        
        do 
        {
      		verro = 1;
            RetErro = 0;
      		while (verro)
      		{
              if (vsendlsb && vsendmsb && (strcmp(argv[2],"msb") == 0 && (endlsb & 0x01) == 0)) 
              {
                vsenddados = 1;            
                break;
              }
              
              dados = dadosf;
            
              if (!vsendlsb) 
                dados = endlsb;
              else if (!vsendmsb) 
                dados = endmsb;
               
      		  // Grava Dados
      		  printf(">%02X",endhighmsb);
      		  printf("%02X",endmsb);
      		  printf("%02X",endlsb);
      		  printf(" : %02X",dados);
      
      		  // Grava Dados
              printf(".");
      		  EnviaComando(dados); // Envia Byte: 0xDD + Byte
              printf(".");
      		  RetByte = RecebeComando(); // Recebe Byte: 0xDD + Byte
              printf(".");
    
      		  // Compara pra ver se Dado Enviado e Recebido est�o Iguais 
      		  if (RetInput[1] != dados && showVerb) 
              {
      		  	printf(" --> Erro Leitura. Leu %02X %02X\n", RetInput[1], RetByte);
                RetErro++;
                if (RetErro >= 0x03) 
                {
       			  RecebeByte(); // Recebe 1 Byte para limpar buffer
                  RetErro = 0;
                }

             	EnviaCtrl(0x01); // Compara��o Byte Com Erro: 0xEE + 0x01
      		  }
      		  else 
              {
                if (showVerb)
      			  printf(" --> OK.\r");
                else
                  printf("\r");

        		verro = 0;
        		EnviaCtrl(0x00); // Compara��o Byte OK: 0xEE + 0x00
        		RecebeComando(); // Rece Controle Final: 0xEE + 69 ou 70
              
            	if (RetInput[0] == 0xEE && RetInput[1] == 0x70)
                    AcompanhaGravacao();
        	  }
  
              if (!vsendlsb)
                vsendlsb = 1;
              else if (!vsendmsb)
                vsendmsb = 1;
              else if (!vsenddados)
                vsenddados = 1;            
      	    } 
        } while (!vsendlsb || !vsendmsb || !vsenddados); 
      }

      if (endlsb == 0xFF)
  	  {
	  	if (endmsb == 0xFF)
			endhighmsb += 1;

  		endmsb += 1;

  		endlsb = 0x00;
  	  }
  	  else
  		endlsb += 1;
  	}

  	printf(">\nFechando Arquivo.\n");

  	fclose(fp);
  
  	printf(">Finalizando Comunica��o.\n");
  
  	EnviaCtrl(0xDD); // Envia FIM: 0xEE + 0xDD

		RecebeComando(); // Recebe Controle Final: 0xEE + 69 ou 70

  	if (RetInput[0] == 0xEE && RetInput[1] == 0x70)
      AcompanhaGravacao();
    
  	TerminaSerial();
  
  	printf(">Dados Enviados.\n");
	printf(">Programa��o Concluida. Bom Teste.\n");
  }
  
	return(0);
}

// adiciona atraso (delay))
void delay(unsigned int ttempo)
{
	for (unsigned int i = 0; i <= ttempo; i++);
}

// inicializa porta serial
int IniciaSerial(){ 
	char *NomePorta = "COM4"; //COM1, COM2... 

	hSerial = CreateFile(NomePorta,	 //Nome da porta. 
 						 GENERIC_READ|GENERIC_WRITE, //Para leitura e escrita. 
						 0,	 //(Zero) Nenhuma outra abertura ser� permitida. 
						 NULL,	 //Atributos de seguran�a. (NULL) padr�o. 
						 OPEN_EXISTING,	 //Cria��o ou abertura. 
						 0,	 //Entrada e sa�da sem overlapped. 
						 NULL	 //Atributos e Flags. Deve ser NULL para COM. 
						 ); 

	if(hSerial == INVALID_HANDLE_VALUE) 
		return false; //Erro ao tentar abrir a porta 

	DCB dcb; //Estrutura DCB � utilizada para definir todos os 
		 	 // par�metros da comunica��o. 

	if( !GetCommState(hSerial, &dcb)) 
		return false; //// Erro na leitura de DCB. 

	dcb.BaudRate = CBR_19200; 
	dcb.ByteSize = 8; 
	dcb.Parity = NOPARITY; 
	dcb.StopBits = ONESTOPBIT; 

	//Define novo estado. 
	if( SetCommState(hSerial, &dcb) == 0 ) 
		return false; //Erro. 

	return true;
} 

// fecha porta serial 
void TerminaSerial(){ 
	CloseHandle( hSerial );	 //Fecha a porta 
} 

//envia dados come�ando com 0xDDh 
DWORD EnviaComando(unsigned char sbyte){ 
	DWORD BytesEscritos = 0; 

	unsigned char cmd[2]; 

	cmd[0] = 0xDD;
	cmd[1] = sbyte; 

	WriteFile( hSerial, cmd, 2, &BytesEscritos, NULL ); 

	return BytesEscritos;
} 

// envia controle come�ando com 0xEEh
DWORD EnviaCtrl(unsigned char sbyte){ 
	DWORD BytesEscritos = 0; 

	unsigned char cmd[2]; 

	cmd[0] = 0xEE;
	cmd[1] = sbyte; 

	WriteFile( hSerial, cmd, 2, &BytesEscritos, NULL ); 

	return BytesEscritos;
} 

// aguarda recebimento de 2 bytes
DWORD RecebeComando(){ 
	DWORD BytesLidos = 0; 

	ReadFile( hSerial, RetInput, 2, &BytesLidos, NULL ); 

	return BytesLidos;
} 

DWORD RecebeByte(){ 
	DWORD BytesLidos = 0; 

	ReadFile( hSerial, RetInput, 1, &BytesLidos, NULL ); 

	return BytesLidos;
}

// acompanha grava��o dos 64 bytes enviados
void AcompanhaGravacao(){
  unsigned char vstatus;
  
  vstatus = 1;
  
  if (showVerb)
    printf("\n>PIC: Iniciando Grava��o 512 Bytes / 256 Words.\n");

  while (vstatus) {
  	RecebeComando(); // Recebe Controle: 0xEE + 71, 72 ou 73
    
    if (RetInput[0] == 0xEE && RetInput[1] == 0x72) {
  		printf(">PIC: Ocorreu(ram) Erro(s) de Grava��o(�es). Tentando Novamente.\n");
    	RecebeComando(); // Recebe Controle: 0xEE + endereco lsb com problema
        if (showVerb)
  		    printf(">PIC: LSB com erro %02X\n", RetInput[1]);
    	RecebeComando(); // Recebe Controle: 0xEE + dado que deveria ter sido  gravado
        if (showVerb)
            printf(">PIC: Gravado..: %02X\n", RetInput[1]);
    	RecebeComando(); // Recebe Controle: 0xEE + dado que foi lido
        if (showVerb)
            printf(">PIC: Lido.....: %02X\n", RetInput[1]);
    }
    else if (RetInput[0] == 0xEE && RetInput[1] == 0x71)
    {
        if (showVerb)
  		    printf(">PIC: Grava��o Executada com Sucesso.\n");
    }
    else if (RetInput[0] == 0xEE && RetInput[1] == 0x73)
    {
        if (showVerb)
  	        printf(">PIC: Fim de Grava��o.\n");
            
        vstatus = 0;
    }
  }
}

