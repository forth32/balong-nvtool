uint32_t load_item_crc(int item);
int test_crc();
uint32_t calc_crc32(char* buf, uint32_t len);
uint32_t calc_item_crc(int item);
// тип CRC, используемый в файле
extern int32_t crcmode;

uint32_t restore_item_crc(int item);
int verify_item_crc(int item);