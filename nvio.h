extern FILE* nvf;
// Хранилище заголовка файла
extern struct nvfile_header nvhd;
// каталог файлов
extern struct nv_file flist[];
// каталог ячеек
extern struct nv_item* itemlist;

void fdump(char buffer[],unsigned int len,unsigned int base, FILE* dump);
void print_filelist();
void print_itemlist();
void print_hd_info();
void extract_files();
void extract_item(int item);
void item_to_file(int item, char* prefix);
void extract_all_item();
int load_item(int item, char* buf);
int save_item(int item, char* buf);
int itemlen(int item);
void brute(int flag);
void print_data();
void  write_imei(char* imei);
void  write_serial(char* serial);
void dump_item(uint32_t item);
void extract_comp_items(int32_t fid);
void mass_import(char* dir);
uint32_t calc_ctrl_crc();
uint32_t item_crc(int item);
int32_t itemidx(int item);
uint32_t itemoff_idx(int idx);
