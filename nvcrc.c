// 
//  Расчет и проверка CRC-массива, защищающего образ NVRAM
// 
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "nvfile.h"
#include "nvio.h"
#include "nvid.h"

// Смещение до поля CRC в файле
extern uint32_t crcoff;

// тип CRC, используемый в файле
// 0 - нет crc
// 1 - первый тип CRC, для V7R11, с массивом контрольных сумм
// 2 - второй тип CRC, для V7R22, с индивидуальной КС
int32_t crcmode;

// таблица констант CRC-32

uint32_t crctab[256] = {
0x00000000,0x4C11DB7, 0x09823B6E,0x0D4326D9,0x130476DC,0x17C56B6B,0x1A864DB2,0x1E475005,
0x2608EDB8,0x22C9F00F,0x2F8AD6D6,0x2B4BCB61,0x350C9B64,0x31CD86D3,0x3C8EA00A,0x384FBDBD,
0x4C11DB70,0x48D0C6C7,0x4593E01E,0x4152FDA9,0x5F15ADAC,0x5BD4B01B,0x569796C2,0x52568B75,
0x6A1936C8,0x6ED82B7F,0x639B0DA6,0x675A1011,0x791D4014,0x7DDC5DA3,0x709F7B7A,0x745E66CD,
0x9823B6E0,0x9CE2AB57,0x91A18D8E,0x95609039,0x8B27C03C,0x8FE6DD8B,0x82A5FB52,0x8664E6E5,
0xBE2B5B58,0xBAEA46EF,0xB7A96036,0xB3687D81,0xAD2F2D84,0xA9EE3033,0xA4AD16EA,0xA06C0B5D,
0xD4326D90,0xD0F37027,0xDDB056FE,0xD9714B49,0xC7361B4C,0xC3F706FB,0xCEB42022,0xCA753D95,
0xF23A8028,0xF6FB9D9F,0xFBB8BB46,0xFF79A6F1,0xE13EF6F4,0xE5FFEB43,0xE8BCCD9A,0xEC7DD02D,
0x34867077,0x30476DC0,0x3D044B19,0x39C556AE,0x278206AB,0x23431B1C,0x2E003DC5,0x2AC12072,
0x128E9DCF,0x164F8078,0x1B0CA6A1,0x1FCDBB16, 0x18AEB13, 0x54BF6A4, 0x808D07D, 0xCC9CDCA,
0x7897AB07,0x7C56B6B0,0x71159069,0x75D48DDE,0x6B93DDDB,0x6F52C06C,0x6211E6B5,0x66D0FB02,
0x5E9F46BF,0x5A5E5B08,0x571D7DD1,0x53DC6066,0x4D9B3063,0x495A2DD4,0x44190B0D,0x40D816BA,
0xACA5C697,0xA864DB20,0xA527FDF9,0xA1E6E04E,0xBFA1B04B,0xBB60ADFC,0xB6238B25,0xB2E29692,
0x8AAD2B2F,0x8E6C3698,0x832F1041,0x87EE0DF6,0x99A95DF3,0x9D684044,0x902B669D,0x94EA7B2A,
0xE0B41DE7,0xE4750050,0xE9362689,0xEDF73B3E,0xF3B06B3B,0xF771768C,0xFA325055,0xFEF34DE2,
0xC6BCF05F,0xC27DEDE8,0xCF3ECB31,0xCBFFD686,0xD5B88683,0xD1799B34,0xDC3ABDED,0xD8FBA05A,
0x690CE0EE,0x6DCDFD59,0x608EDB80,0x644FC637,0x7A089632,0x7EC98B85,0x738AAD5C,0x774BB0EB,
0x4F040D56,0x4BC510E1,0x46863638,0x42472B8F,0x5C007B8A,0x58C1663D,0x558240E4,0x51435D53,
0x251D3B9E,0x21DC2629,0x2C9F00F0,0x285E1D47,0x36194D42,0x32D850F5,0x3F9B762C,0x3B5A6B9B,
 0x315D626, 0x7D4CB91, 0xA97ED48, 0xE56F0FF,0x1011A0FA,0x14D0BD4D,0x19939B94,0x1D528623,
0xF12F560E,0xF5EE4BB9,0xF8AD6D60,0xFC6C70D7,0xE22B20D2,0xE6EA3D65,0xEBA91BBC,0xEF68060B,
0xD727BBB6,0xD3E6A601,0xDEA580D8,0xDA649D6F,0xC423CD6A,0xC0E2D0DD,0xCDA1F604,0xC960EBB3,
0xBD3E8D7E,0xB9FF90C9,0xB4BCB610,0xB07DABA7,0xAE3AFBA2,0xAAFBE615,0xA7B8C0CC,0xA379DD7B,
0x9B3660C6,0x9FF77D71,0x92B45BA8,0x9675461F,0x8832161A,0x8CF30BAD,0x81B02D74,0x857130C3,
0x5D8A9099,0x594B8D2E,0x5408ABF7,0x50C9B640,0x4E8EE645,0x4A4FFBF2,0x470CDD2B,0x43CDC09C,
0x7B827D21,0x7F436096,0x7200464F,0x76C15BF8,0x68860BFD,0x6C47164A,0x61043093,0x65C52D24,
0x119B4BE9,0x155A565E,0x18197087,0x1CD86D30, 0x29F3D35, 0x65E2082, 0xB1D065B, 0xFDC1BEC,
0x3793A651,0x3352BBE6,0x3E119D3F,0x3AD08088,0x2497D08D,0x2056CD3A,0x2D15EBE3,0x29D4F654,
0xC5A92679,0xC1683BCE,0xCC2B1D17,0xC8EA00A0,0xD6AD50A5,0xD26C4D12,0xDF2F6BCB,0xDBEE767C,
0xE3A1CBC1,0xE760D676,0xEA23F0AF,0xEEE2ED18,0xF0A5BD1D,0xF464A0AA,0xF9278673,0xFDE69BC4,
0x89B8FD09,0x8D79E0BE,0x803AC667,0x84FBDBD0,0x9ABC8BD5,0x9E7D9662,0x933EB0BB,0x97FFAD0C,
0xAFB010B1,0xAB710D06,0xA6322BDF,0xA2F33668,0xBCB4666D,0xB8757BDA,0xB5365D03,0xB1F740B4
};

//***************************************************
//* Вычисление CRC-32 массива
//***************************************************
uint32_t calc_crc32(char* buf, uint32_t len) {
  
uint32_t crc=0;
uint32_t i;

for(i=0;i<len;i++) {
  crc=(((uint32_t)(buf[i]&0xff)) | (crc<<8)) ^ crctab[crc>>24];
}

// добавляем 4 нуля поля КС
for(i=0;i<4;i++) {
  crc=crctab[crc>>24] ^ (crc<<8);
}
return crc;
}

//***************************************************
//* Определение размер массива КС
//***************************************************
uint32_t calc_crcsize() {

uint32_t filesize;
  
// определяем размер NV-файла
fseek(nvf,0,SEEK_END);
filesize=ftell(nvf);
// printf("\n fsize=%i crcoff=%i",filesize,crcoff);
if (filesize == crcoff) return 0;
// размер массива КС
return filesize-crcoff-(crcmode == 1 ? 4 : 8);
} 
  
//***************************************************
//* Проверка CRC-массива
//***************************************************
int test_crc() {
  
uint32_t crcsize;
uint32_t* csblock;
uint8_t buf[4096]; // буфер для одного блока данных
uint32_t crc;
uint32_t blocksize=4096;
uint32_t blknum;
uint32_t i;
int res=0;

// проверяем флаг наличия CRC
if (nvhd.crcflag == 0) return 0;
if (nvhd.crcflag == 8) return -2; // V7R22 пока не поддерживается
crcsize=calc_crcsize();
if (crcsize == 0) return 0;
// выделяем место под CRC-блок
csblock=malloc(crcsize);
// читаем массив CRC из файла
fseek(nvf,crcoff+4,SEEK_SET);
fread(csblock,crcsize,1,nvf);

// Число проверяемых блоков по 4096 байт
blknum=crcsize>>2;
// переезжаем на начало области данных
fseek(nvf,nvhd.ctrl_size,SEEK_SET);

// Вычисляем CRC блоков и проверяем ее
for (i=0;i<blknum;i++) {
  // коррекция размера последнего блока
  if ((crcoff-ftell(nvf)) < 4096) blocksize=crcoff-ftell(nvf);
  // читаем блок
  fread(buf,blocksize,1,nvf);
  crc=calc_crc32(buf,blocksize);
  if (crc != csblock[i]) {
//      printf("\n Блок %i: Ошибка CRC",i);
     res=-1;
  }   
}
free(csblock);  
return res;
}  


//***************************************************
//* Вычисление CRC-массива
//***************************************************
void recalc_crc() {
  
uint32_t crcsize;
uint32_t* csblock;
uint32_t* oldcsblock;
uint8_t buf[4096]; // буфер для одного блока данных
uint32_t blocksize=4096;
uint32_t i;

// проверяем флаг наличия CRC
if (crcmode != 1 && crcmode != 3) return; // только для блочной CRC

crcsize=calc_crcsize();

// выделяем место под CRC-блок
csblock=malloc(crcsize);
oldcsblock=malloc(crcsize);

// читаем массив старых КС
fseek(nvf,crcoff+4,SEEK_SET);
fread(oldcsblock,crcsize,1,nvf);

// переезжаем на начало области данных
fseek(nvf,nvhd.ctrl_size,SEEK_SET);

// Вычисляем CRC блоков и проверяем ее
for (i=0;i<(crcsize>>2);i++) {
  // коррекция размера последнего блока
  if ((crcoff-ftell(nvf)) < 4096) blocksize=crcoff-ftell(nvf);
  // читаем блок
  fread(buf,blocksize,1,nvf);
  csblock[i]=calc_crc32(buf,blocksize);
}
// пишем массив CRC в файл
if (memcmp(oldcsblock, csblock, crcsize) != 0) {
  if (fseek(nvf,crcoff+4,SEEK_SET) != 0) 
    printf("\n ! Ошибка позиционирования к массиву CRC");
  else
    fwrite(csblock,crcsize,1,nvf);
}

free(csblock);  
free(oldcsblock);  
}  

//****************************************************
//* Вычисление CRC управляющих структур
//****************************************************
uint32_t calc_ctrl_crc() {
  
char* buf;
uint32_t crc;

buf=malloc(nvhd.ctrl_size-4);
if (buf == 0) {
  printf("\n Ошибка выделения буфера управляющих структур");
  exit(1);
}  
fseek(nvf,0,SEEK_SET);
fread(buf,nvhd.ctrl_size-4,1,nvf);
crc=calc_crc32(buf,nvhd.ctrl_size-4);
free(buf);

return crc;  
}


//**********************************************
//* Загрузка CRC отдельных ячеек 
//**********************************************
uint32_t load_item_crc(int item) {
  
int idx;
uint32_t crc;

idx=itemidx(item);
if (idx == -1) return 0; // не найдена
fseek(nvf,itemoff_idx(idx)+itemlen(item),SEEK_SET);
fread(&crc,4,1,nvf);
return crc;
}


//**********************************************
//* Расчет CRC отдельных ячеек 
//**********************************************
uint32_t calc_item_crc(int item) {

char* buf;
uint32_t crc;

buf=malloc(itemlen(item));
  
load_item(item,buf);
crc=calc_crc32(buf,itemlen(item)),
free(buf);
if (crc == 0) crc=0x5b637eb3;
return crc;
}

//**********************************************
//* Перерасчет CRC отдельных ячеек 
//**********************************************
void restore_item_crc(int item) {
  
int idx;
uint32_t crc;

if (crcmode != 2) return; // только для типов 2
crc=calc_item_crc(item);

idx=itemidx(item);
if (idx == -1) return; // не найдена
fseek(nvf,itemoff_idx(idx)+itemlen(item),SEEK_SET);
fwrite(&crc,4,1,nvf);
}

//**********************************************
//* Проверка CRC отдельных ячеек 
//**********************************************
int verify_item_crc(int item) {

if (calc_item_crc(item) != load_item_crc(item)) return 0;
return 1;
}
