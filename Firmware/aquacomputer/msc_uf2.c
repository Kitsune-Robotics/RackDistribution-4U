#include "msc_uf2.h"

#include "parameters.h"

#if ENABLE_UF2_LOADER

#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico.h"
#include "tusb.h"

#include <string.h>

#define DISK_BLOCK_SIZE 512
#define DISK_BLOCK_NUM 4096
#define FAT_SECTORS 16
#define ROOT_SECTORS 32
#define FS_SECTORS (1 + FAT_SECTORS + ROOT_SECTORS)
#define DATA_START FS_SECTORS

#define IMAGE_MAX (96 * 1024)
#define UF2_MAGIC0 0x0A324655u
#define UF2_MAGIC1 0x9E5D5157u
#define UF2_MAGIC_END 0x0AB16F30u
#define UF2_FLAG_FAMILYID 0x00002000u
#define UF2_FAMILY_RP2040 0xe48bff56u
#define UF2_PAYLOAD 256
#define UF2_MAX_BLOCKS (IMAGE_MAX / UF2_PAYLOAD)

#define README                                                                       \
  "Drop a .uf2 here to update firmware.\r\n"                                         \
  "--Joe :3\r\n"

typedef struct __attribute__((packed)) {
  uint32_t magic0;
  uint32_t magic1;
  uint32_t flags;
  uint32_t target_addr;
  uint32_t payload_size;
  uint32_t block_no;
  uint32_t num_blocks;
  uint32_t family_id;
  uint8_t data[476];
  uint32_t magic_end;
} uf2_block_t;

static_assert(sizeof(uf2_block_t) == 512, "UF2 block is 512 bytes");

static uint8_t g_fs[FS_SECTORS][DISK_BLOCK_SIZE];
static uint8_t g_image[IMAGE_MAX];
static uint8_t g_got[(UF2_MAX_BLOCKS + 7) / 8];
static uint32_t g_num_blocks;
static uint32_t g_got_count;
static uint32_t g_image_size;
static volatile bool g_apply_pending;
static bool g_ejected;
static bool g_fs_ready;

static void init_fs(void) {
  memset(g_fs, 0, sizeof(g_fs));

  uint8_t *b = g_fs[0];
  b[0] = 0xEB;
  b[1] = 0x3C;
  b[2] = 0x90;
  memcpy(b + 3, "MSDOS5.0", 8);
  b[0x0B] = 0x00;
  b[0x0C] = 0x02;
  b[0x0D] = 1;
  b[0x0E] = 1;
  b[0x10] = 1;
  b[0x11] = 0x00;
  b[0x12] = 0x02;
  b[0x15] = 0xF8;
  b[0x16] = (uint8_t)FAT_SECTORS;
  b[0x18] = 1;
  b[0x1A] = 1;
  b[0x20] = (uint8_t)DISK_BLOCK_NUM;
  b[0x21] = (uint8_t)(DISK_BLOCK_NUM >> 8);
  b[0x24] = 0x80;
  b[0x26] = 0x29;
  b[0x27] = 0x34;
  b[0x28] = 0x12;
  memcpy(b + 0x2B, "RACKDIST   ", 11);
  memcpy(b + 0x36, "FAT16   ", 8);
  b[0x1FE] = 0x55;
  b[0x1FF] = 0xAA;

  g_fs[1][0] = 0xF8;
  g_fs[1][1] = 0xFF;
  g_fs[1][2] = 0xFF;
  g_fs[1][3] = 0xFF;
  g_fs[1][4] = 0xFF;
  g_fs[1][5] = 0xFF;

  uint8_t *root = g_fs[1 + FAT_SECTORS];
  memcpy(root, "RACKDIST   ", 11);
  root[11] = 0x08;

  memcpy(root + 32, "README  TXT", 11);
  root[32 + 11] = 0x20;
  root[32 + 26] = 2;
  uint32_t rlen = sizeof(README) - 1;
  root[32 + 28] = (uint8_t)rlen;
  root[32 + 29] = (uint8_t)(rlen >> 8);

  g_fs_ready = true;
}

static void handle_uf2(const uf2_block_t *blk) {
  if (blk->magic0 != UF2_MAGIC0 || blk->magic1 != UF2_MAGIC1 ||
      blk->magic_end != UF2_MAGIC_END) {
    return;
  }
  if (blk->payload_size > UF2_PAYLOAD || blk->block_no >= UF2_MAX_BLOCKS) {
    return;
  }
  if ((blk->flags & UF2_FLAG_FAMILYID) && blk->family_id != UF2_FAMILY_RP2040) {
    return;
  }
  if (blk->target_addr < XIP_BASE) {
    return;
  }

  uint32_t off = blk->target_addr - XIP_BASE;
  if (off + blk->payload_size > IMAGE_MAX) {
    return;
  }
  if (blk->num_blocks == 0 || blk->num_blocks > UF2_MAX_BLOCKS) {
    return;
  }
  if (g_num_blocks == 0) {
    g_num_blocks = blk->num_blocks;
    memset(g_got, 0, sizeof(g_got));
    g_got_count = 0;
    g_image_size = 0;
  } else if (blk->num_blocks != g_num_blocks) {
    return;
  }

  memcpy(g_image + off, blk->data, blk->payload_size);
  if (off + blk->payload_size > g_image_size) {
    g_image_size = off + blk->payload_size;
  }

  uint32_t byte = blk->block_no / 8;
  uint8_t bit = (uint8_t)(1u << (blk->block_no % 8));
  if ((g_got[byte] & bit) == 0) {
    g_got[byte] |= bit;
    g_got_count++;
  }
  if (g_got_count == g_num_blocks) {
    g_apply_pending = true;
  }
}

bool msc_uf2_ready_to_apply(void) { return g_apply_pending; }

void __not_in_flash_func(msc_uf2_apply)(void) {
  uint32_t erase = (g_image_size + FLASH_SECTOR_SIZE - 1u) &
                   ~(FLASH_SECTOR_SIZE - 1u);
  uint32_t prog =
      (g_image_size + FLASH_PAGE_SIZE - 1u) & ~(FLASH_PAGE_SIZE - 1u);
  if (prog > IMAGE_MAX) {
    prog = IMAGE_MAX;
  }
  if (erase < prog) {
    erase = (prog + FLASH_SECTOR_SIZE - 1u) & ~(FLASH_SECTOR_SIZE - 1u);
  }

  uint32_t ints = save_and_disable_interrupts();
  flash_range_erase(0, erase);
  flash_range_program(0, g_image, prog);
  (void)ints;
  *(volatile uint32_t *)0xe000ed0c = 0x05FA0004u;
  while (true) {
  }
}

void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16],
                        uint8_t product_rev[4]) {
  (void)lun;
  memcpy(vendor_id, "Kitsune ", 8);
  memcpy(product_id, "RACKDIST        ", 16);
  memcpy(product_rev, "1.0 ", 4);
}

bool tud_msc_test_unit_ready_cb(uint8_t lun) {
  (void)lun;
  if (g_ejected) {
    tud_msc_set_sense(lun, SCSI_SENSE_NOT_READY, 0x3a, 0x00);
    return false;
  }
  return true;
}

void tud_msc_capacity_cb(uint8_t lun, uint32_t *block_count, uint16_t *block_size) {
  (void)lun;
  *block_count = DISK_BLOCK_NUM;
  *block_size = DISK_BLOCK_SIZE;
}

bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start,
                           bool load_eject) {
  (void)lun;
  (void)power_condition;
  if (load_eject && !start) {
    g_ejected = true;
  }
  return true;
}

int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer,
                          uint32_t bufsize) {
  (void)lun;
  if (!g_fs_ready) {
    init_fs();
  }
  if (lba >= DISK_BLOCK_NUM) {
    return -1;
  }

  const uint8_t *src;
  if (lba < FS_SECTORS) {
    src = g_fs[lba] + offset;
  } else if (lba == DATA_START) {
    static const char readme[] = README;
    static uint8_t cluster[DISK_BLOCK_SIZE];
    memset(cluster, 0, sizeof(cluster));
    memcpy(cluster, readme, sizeof(readme) - 1);
    src = cluster + offset;
  } else {
    memset(buffer, 0, bufsize);
    return (int32_t)bufsize;
  }
  memcpy(buffer, src, bufsize);
  return (int32_t)bufsize;
}

bool tud_msc_is_writable_cb(uint8_t lun) {
  (void)lun;
  return !g_apply_pending;
}

int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset,
                           uint8_t *buffer, uint32_t bufsize) {
  (void)lun;
  if (!g_fs_ready) {
    init_fs();
  }
  if (lba >= DISK_BLOCK_NUM) {
    return -1;
  }
  if (lba < FS_SECTORS) {
    memcpy(g_fs[lba] + offset, buffer, bufsize);
  } else if (offset == 0 && bufsize >= sizeof(uf2_block_t)) {
    handle_uf2((const uf2_block_t *)buffer);
  }
  return (int32_t)bufsize;
}

int32_t tud_msc_scsi_cb(uint8_t lun, uint8_t const scsi_cmd[16], void *buffer,
                        uint16_t bufsize) {
  (void)buffer;
  (void)bufsize;
  (void)scsi_cmd;
  tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);
  return -1;
}

#endif
