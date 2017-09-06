#ifndef _NVIO_C
#define _NVIO_C

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#ifndef WIN32
#include <sys/stat.h>
#include <sys/types.h>
#else
#include <windows.h>
#include <direct.h>
#include "printf.h"
#endif

#include "nvfile.h"
#include "nvio.h"
#include "nvio.h"
#include "nvid.h"
#include "nvcrc.h"
#include "sha2.h"

// Хранилище заголовка файла
struct nvfile_header nvhd;
// Каталог файлов
struct nv_file flist[100];
// каталог ячеек
struct nv_item* itemlist;

#ifdef MODEM 
// флаг прямой работы с nvram-файлом вместо интерфейса ядра
extern int32_t kernelflag;
#endif

//******************************************************
// Получение смещения до начала файла по номеру файла
//******************************************************
uint32_t fileoff(int fid) {

int i;
for (i=0;i<nvhd.file_num;i++) {
  if (flist[i].id == fid) return flist[i].offset;
}
printf("\n - Ошибка структуры файла - компоненты #%i не существует\n",fid);
exit(1);
}

//******************************************************
// Получение индкса по номеру файла
//******************************************************
int32_t fileidx(int fid) {

int i;

for (i=0;i<nvhd.file_num;i++) {
  if (flist[i].id == fid) return i;
}
return -1;
}


//******************************************************
// Получение смещения до начала ячейки по ее индексу
//******************************************************
uint32_t itemoff_idx(int idx) {

return itemlist[idx].off+fileoff(itemlist[idx].file_id);
}


//******************************************************
//* Получение индекса ячейки по ее id
//*  возврат -1 - ячейка не найдена
//******************************************************
int32_t itemidx(int item) {
  
int i;

for(i=0;i<nvhd.item_count;i++) {
  if (itemlist[i].id == item) return i;
}
return -1;
}

//******************************************************
// Получение смещения до начала ячейки по ее номеру
//******************************************************
int32_t itemoff (int item) {

int idx=itemidx(item);
if (item == -1) return -1;
return itemoff_idx(idx);
}

//******************************************************
// Получение размера ячейки по ее номеру
//******************************************************
int32_t itemlen (int item) {

int idx=itemidx(item);
if (idx == -1) return -1;
return itemlist[idx].len;
}

  
//***********************
//* Дамп области памяти *
//***********************

void fdump(char buffer[],unsigned int len,unsigned int base, FILE* dump) {
unsigned int i,j;
char ch;

for (i=0;i<len;i+=16) {
  fprintf(dump,"%08x: ",(base+i));
  for (j=0;j<16;j++){
   if ((i+j) < len) fprintf(dump,"%02x ",buffer[i+j]&0xff);
   else fprintf(dump,"   ");
  }
  fprintf(dump," *");
  for (j=0;j<16;j++) {
   if ((i+j) < len) {
    ch=buffer[i+j];
    if ((ch < 0x20)|(ch > 0x80)) ch='.';
    fputc(ch,dump);
   }
   else fputc(' ',dump);
  }
  fprintf(dump,"*\n");
}
}

//**********************************************
//* Вывод информации из заголовка 
//**********************************************
void print_hd_info() {
  
printf("\n Версия файла NVRAM:       %i (0x%x)",nvhd.version,nvhd.version);
printf("\n Номер модема:             %i",nvhd.modem_num);
printf("\n Идентификатор устройства: %s",nvhd.product_version);
printf("\n Тип CRC:                  ");
switch (crcmode) {
  case 0:
    printf("Нет");
    break;
    
  case 1:
    printf("Блочная, тип 1");
    break;
    
  case 2:
    printf("Индивидуальная, тип 8");
    break;
    
  default:
    printf("Неподдерживаемая в этой версии утилиты, тип %x",nvhd.crcflag);
    break;
}    
  
}


//**********************************************
//* Вывод списка компонентных файлов 
//**********************************************
void print_filelist() {
  
int i;

printf("\n\n --- Каталог файлов ----\n");
printf("\n ID  позиция   размер  ---имя---\n--------------------------------------");

for(i=0;i<nvhd.file_num;i++) {
  printf("\n %2i  %08x  %6i  %s",flist[i].id,flist[i].offset,flist[i].size,flist[i].name);
}  
  
printf("\n * Всего файлов: %i\n",nvhd.file_num);
}
  
//**********************************************
//* Вывод списка ячеек nvram
//**********************************************
void print_itemlist() {
  
int i;
uint32_t badflag=0;
printf("\n\n --- Каталог ячеек ----\n");
printf("\n NVID   FID  позиция   размер  приоритет  имя\n-----------------------------------------------");

for (i=0;i<nvhd.item_count;i++) {
   printf("\n %-5i  %2i   %08x  %4i    %x   %s",
 	itemlist[i].id, 
 	itemlist[i].file_id, 
 	itemoff_idx(i),
 	itemlist[i].len, 
 	itemlist[i].priority,
 	find_desc(itemlist[i].id)); 


  if ((crcmode ==2) && !verify_item_crc(itemlist[i].id)) {
    printf("\n ! ** Ошибка CRC ячейки.\n");
    badflag++;
  }   
}
printf("\n\n * Всего ячеек: %i\n",nvhd.item_count);
if ((crcmode == 2) && (badflag != 0)) printf(" ! Ячеек с ошибкой CRC: %i\n",badflag);
    
}

//**********************************************
//* Извлечение компонентных файлов
//**********************************************
void extract_files() {

int i;
char filename[100];
char* buf;
FILE* out;

#ifndef WIN32
mkdir ("component",0777);
#else
_mkdir("component");
#endif
for(i=0;i<nvhd.file_num;i++) {
 fseek(nvf,flist[i].offset,SEEK_SET);
 printf("\n Извлечение файла %s",flist[i].name);
 sprintf(filename,"component/%s",flist[i].name);
 out=fopen(filename,"wb");
 buf=malloc(flist[i].size);
 fread(buf,1,flist[i].size,nvf);
 fwrite(buf,1,flist[i].size,out);
 free(buf);
 fclose(out);
}
printf("\n");
}

//**********************************************
//* Исвлечение ячеек по номеру компоненты
//**********************************************
void extract_comp_items(int32_t fn) {


char dirname[100];  
int i,fidx;

fidx=fileidx(fn);
if (fidx == -1) {
  printf("\n Компоненты %i не существует\n",fn);
  return;
}

printf("\n Извлечение ячеек компоненты %i (%s)\n\n",fn,flist[fidx].name);
sprintf(dirname,"COMP%i/",fn);
#ifndef WIN32
mkdir(dirname,0777);
#else
_mkdir(dirname);
#endif

for(i=0;i<nvhd.item_count;i++) {
  if (itemlist[i].file_id != fn) continue;
  printf("\r Ячейка %i",itemlist[i].id);
  item_to_file(itemlist[i].id,dirname);
}  
printf("\n\n");

}

//**********************************************
//* Дамп ячейки nvram
//**********************************************
void dump_item(uint32_t item) {

int len;
char buf[4100];
char* desc;
int32_t idx,fidx;    
int32_t fn;

idx=itemidx(item);
if (idx == -1) {
    printf("\n - Ячейка %i не найдена\n",item);
    return;
}

fseek(nvf,itemoff_idx(idx),SEEK_SET);
fread(buf,itemlist[idx].len,1,nvf);
len=itemlist[idx].len;
fn=itemlist[idx].file_id;
fidx=fileidx(fn);

printf("\n -- Ячейка # %i: %i байт -- Компонент %i (%s) -- ",item,len,fn,flist[fidx].name);
desc=find_desc(item);
if (strlen(desc) != 0) printf(" Имя: %s --",desc);
printf("-----\n");
fdump(buf,len,0,stdout);
if ((crcmode == 2) && !verify_item_crc(item)) printf("\n ! Ошибка CRC ячейки");
printf("\n");
}


//**********************************************
//* Загрузка ячейки в буфер
//**********************************************
int load_item(int item, char* buf) {
  
int idx;

idx=itemidx(item);
if (idx == -1) return -1; // не найдена
fseek(nvf,itemoff_idx(idx),SEEK_SET);
fread(buf,itemlist[idx].len,1,nvf);
return itemlist[idx].len;
}



//**********************************************
//* Запись ячейки из буфера
//**********************************************
int save_item(int item, char* buf) {
  
int idx;
int res;

idx=itemidx(item);
if (idx == -1) return 0; // не найдена

fseek(nvf,itemoff_idx(idx),SEEK_SET);
res=fwrite(buf,itemlist[idx].len,1,nvf);
if (res != 1) {
    printf("\nОшибка записи ячейки %i",item);
    perror("");
    return 0;
}    
restore_item_crc(item); // восстанавливаем CRC ячейки
return 1; // ok
}


//**********************************************
//* Извлечение одной ячейки в файл
//**********************************************
void item_to_file(int item, char* prefix) {
  
char filename[100];
char buf[4100];
FILE* out;
int len;

sprintf(filename,"%s%05i.nvm",prefix,item);
len=load_item(item,buf);
if (len == -1) {
  printf("\n - Ячейка %i не найдена\n",item);
  exit(1);
}  
out=fopen(filename,"wb");
fwrite(buf,1,len,out);
fclose(out);
if ((crcmode == 2) && !verify_item_crc(item)) printf("\n Ячейка %i: ошибка CRC",item);
}

//**********************************************
//* Извлечение всех ячеек в файл
//**********************************************
void extract_all_item() {

int i;

#ifndef WIN32
mkdir ("item",0777);
#else
_mkdir("item");
#endif
printf("\n");
for(i=0;i<nvhd.item_count;i++) {
  printf("\r Ячейка %i длина %i",itemlist[i].id,itemlist[i].len);
  item_to_file(itemlist[i].id,"item/");
}  
printf("\n\n");
}

//**********************************************
//* Импорт всех ячеек из указанного каталога
//**********************************************
void mass_import(char* dir) {
  
int i;
char filename[200];
uint32_t fsize;
FILE* in;
char ibuf[4100];
int idx;

printf("\n Импорт ячеек:\n\n");
for (i=0;i<65536;i++) {
  sprintf(filename,"%s/%05i.nvm",dir,i);
  in=fopen(filename,"rb");
  if (in == 0) continue; // такого файла у нас нет
  // грузим файл в буфер и заодно определяем размер его
  fsize=fread(ibuf,1,2054,in);
  fclose(in);
  // проверяем параметры ячейки в образе
  idx=itemidx(i);
  if (idx == -1) {
    printf("\r Ячейка %i не найдена в образе nvram\n",i);
    continue;
  }  
  if (itemlist[idx].len != fsize) {  
    printf("\r Ячейка %i: размер ячейки (%i) не соответствует размеру файла (%i)\n",i,itemlist[idx].len,fsize);
    continue;
  }  
  // импорт ячейки  
  printf("\r Ячейка %i: ",i);
  fseek(nvf,itemoff_idx(idx),SEEK_SET);
  fwrite(ibuf,itemlist[idx].len,1,nvf);
  restore_item_crc(i);
  printf("OK");
}
}
  
  
//**********************************************
//* Поск оем и симлок - кодов
//*  1 - oem
//*  2 - simlock
//**********************************************
void brute(int flag) {
  
char buf[128];
int item=50502;
int code;
SHA2_CTX ctx;
char scode[50];

uint8_t hash1[32],hash2[32];

if (flag == 2) item=50503;
if (itemlen(item) != 128) {
  printf("\n - Ячейка %i не найдена или повреждена\n",item);
  return;
}
load_item(item,buf);
memcpy(hash1,buf,32);
memcpy(hash2,buf+64,32);

printf("\n");
for(code=0;code<99999999;code++) {
  if  (code % 1000000 == 0) {
    printf("\r* Поиск %s-кода: %i%%... ",(flag == 1)?"OEM":"Simlock",code/1000000); 
    fflush(stdout);
  }  
  // хеш нашего кода
  SHA256Init(&ctx);
  sprintf(scode,"%08i",code);
  SHA256Update(&ctx,scode,8);
  SHA256Final(scode,&ctx);
  // хеш суммы  
  SHA256Init(&ctx);
  SHA256Update(&ctx,scode,32);
  SHA256Update(&ctx,hash2,32);  
  SHA256Final(scode,&ctx);
  // сравниваем хеш с лежащим в nvram
  if (memcmp(scode,hash1,32) == 0) {
    printf("\r Найден %s-код: %08i\n",(flag == 1)?"OEM":"Simlock",code);
    return;
  }
}  
printf("\n%s-код не найден\n",(flag == 1)?"OEM":"Simlock");
}

//************************************************
//* Запись нового IMEI
//************************************************
void write_imei(char* imei) {

unsigned char binimei[16];
int i,f,j;  
char cbuf[7];
int csum;

if (strlen(imei) != 15) {
  printf("\n Неправильная длина IMEI");
  return;
}

for (i=0;i<15;i++) {
  if ((imei[i] >= '0') && (imei[i] <='9')) {
    binimei[i] = imei[i]-'0'; 
    continue;
  }  
  printf("\n Неправильный символ в строке IMEI - %c\n",imei[i]);
  return;
}
binimei[15]=0;
// Проверяем контрольную сумму IMEI
j=0;
for (i=1;i<14;i+=2) cbuf[j++]=binimei[i]*2;
csum=0;
for (i=0;i<7;i++) {
  f=(int)cbuf[i]/10;
  csum = csum + f + (int)((cbuf[i]*10) - f*100)/10;
}  
for (i=0;i<13;i+=2) csum += binimei[i];
 
if ((((int)csum/10)*10) == csum) csum=0;
else csum=( (int)csum/10 + 1)*10 - csum;
if (binimei[14] != csum) {
  printf("\n IMEI имеет неправильную контрольную сумму !\n Правильный IMEI: ");
  for (i=0;i<14;i++) printf("%1i",binimei[i]);
  printf("%1i",csum);
  binimei[14]=csum;
  printf("\n IMEI скорректирован");
}  

if (save_item(0,binimei)) printf("\n IMEI успешно записан\n");
else printf("\n Ошибка поиска ячейки 0");
  
}
  
//************************************************
//* Запись нового серийника
//************************************************
void  write_serial(char* serial) {

char* sptr;

// заменяем нули на 0xff
sptr=strchr(serial,0);
if (sptr != 0) *sptr=0xff;

if (save_item(6,serial)) printf("\n Серийный номер успешно записан\n");
else printf("\n Ошибка поиска ячейки 6");
  
}


//************************************************
//*  Печать идентификаторов и настроек
//************************************************
void print_data() {

char buf[2560];
char* bptr;
int i,badflag=0;

// CRC управяющей структуры
uint32_t crc_ctrl=0;

// NV 53525 - информации о продукте
struct {
     uint32_t index;          // +00 Hardware version number (large version number 1 + large version number 2), distinguish between different products * /
     uint32_t hwIdSub;        // +04 hardware sub-version number, distinguish between different versions of the product * /
     uint8_t name [32];       // +08 internal product name * /
     uint8_t namePlus [32];   // +28 Internal product name PLUS * /
     uint8_t hwVer [32];      // +48 Hardware version name * /
     uint8_t dloadId [32];    // +68 the name used in the upgrade * /
     uint8_t productId [32];  // +88 External Product Name * /
 } prodinfo;

// информация о wifi
int wicount=0;       // число wifi-записей
// char wissid[36][4];  // имена сетей
// char wikey[68][4];   // ключи
char wissid[5][36];  // имена сетей
char wikey[5][68];   // ключи

// вывод информации о продукте
i=load_item(53525,(char*)&prodinfo);
if ((i == sizeof(prodinfo))&&(prodinfo.index != 0)) {
  printf("\n Product Code: %i %i",prodinfo.index,prodinfo.hwIdSub);
  printf("\n Product Name: %s",prodinfo.name);
  printf("\n HW version  : %s",prodinfo.hwVer);
  printf("\n Dload ID    : %s",prodinfo.dloadId);
  printf("\n Product ID  : %s\n",prodinfo.productId);
}  

// вывод IMEI
load_item(0,buf);
printf("\n IMEI  : ");
for (i=0;i<15;i++) printf("%c",buf[i]+'0');
// вывод серийника
load_item(6,buf);
printf("\n Serial: %16.16s",buf);
// IP-адрес:
load_item(44,buf);
printf("\n IPaddr: %s",buf);

// вывод wifi-параметров
#ifndef WIN32
bzero(buf,256);
bzero(wissid,36*4);
bzero(wikey,68*4);
#else
memset(buf,0,256);
memset(wissid,0,36*4);
memset(wikey,0,68*4);
#endif
load_item(9111,buf);
if (buf[0] != 0) {
  // ssid
  bptr=buf;
  wicount=0;
  while (*bptr != 0) {
    strcpy(wissid[wicount],bptr);
    wicount++;
    bptr+=0x21;
  }
  
  // WPA key
   load_item(9110,buf);
   for (i=0;i<wicount;i++) {
      strncpy(wikey[i],buf+462+i*65,65);
   }
  
  printf("\n\n *** Параметры Wifi ***\n\n #  SSID                             KEY\n\
----------------------------------------------------------------------------------\n");
  for(i=0;i<wicount;i++) printf("\n %1i  %-32.32s %-32.32s",i,(char*)&wissid[i],(char*)&wikey[i]);
  printf("\n");

}

// Проверяем crc

// Выделяем CRC заголовка
if (crcmode == 2) {
  if ((nvhd.ctrl_size-nvhd.item_offset-nvhd.item_size) != 4) 
                       printf("\n CTRL CRC   : отсутствует поле CRC в заголовке - формат файла неверен"); 
  else {
   // выделяем CRC управяющей структуры
   fseek(nvf,nvhd.ctrl_size-4,SEEK_SET);
   fread(&crc_ctrl,4,1,nvf);
   if (crc_ctrl == calc_ctrl_crc())
     printf("\n CTRL CRC   : OK");
   else
     printf("\n CTRL CRC   : Error!");
  } 
}    
else printf("\n CTRL CRC   : Отсутствует");

// Проверяем CRC области данных
switch (crcmode) {
  case 0:
    printf("\n DATA CRC   : Отсутствует");
    break;
    
  case 1:  
    if (test_crc() == 0) printf("\n DATA CRC   : OK");
    else printf("\n DATA CRC   : Error!");
    break;
    
  case 2:
    for (i=0;i<nvhd.item_count;i++) {
     if ((crcmode ==2) && !verify_item_crc(itemlist[i].id))  badflag++;
    }   
    if (badflag != 0) printf("\n DATA CRC   : Ячеек с ошибкой CRC: %i",badflag);
    else printf("\n DATA CRC   : OK");
    break;
    
  default:
    printf("\n DATA CRC   : Неподдерживаемый тип");
  
}

printf("\n");
}

  
  
#endif // define _NVIO_C 
