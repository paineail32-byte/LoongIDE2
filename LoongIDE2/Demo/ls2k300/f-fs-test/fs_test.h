/*
 * fs_test.h
 *
 * created: 2025-02-26
 *  author: 
 */

#ifndef _FS_TEST_H
#define _FS_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 被测试设备
 */
#if BSP_USE_EMMC
#define TEST_EMMC_RW    0       // "/mmc0"
#endif

#if BSP_USE_SATA
#define TEST_SATA_RW    1       // "/hda"
#endif

#if USE_YAFFS2
#define TEST_YAFFS2_RW  1       // "/ndd"
#endif

#if BSP_USE_USB
#define TEST_USB_RW     1       // "/usbd0"
#endif

/*
 * 测试项目
 */
#define TEST_DIR_MK         1
#define TEST_DIR_RM         1
#define TEST_DIR_LIST       1
#define TEST_DIR            (TEST_DIR_MK | TEST_DIR_RM | TEST_DIR_LIST)

#define TEST_FILE_CREAT     1
#define TEST_FILE_DEL       1
#define TEST_FILE_READ      1
#define TEST_FILE_WRITE     1
#define TEST_FILE           (TEST_FILE_CREAT | TEST_FILE_DEL | TEST_FILE_READ | TEST_FILE_WRITE)

//-------------------------------------------------------------------------------------------------
// 目录和文件读写测试
//-------------------------------------------------------------------------------------------------

/*
void dir_test(char *pathname);
void file_test_1(char *filename);
void file_test_2(char *filename);
 */
 
int do_fs_rw_test(void);

//-------------------------------------------------------------------------------------------------

int disk_exists(const char *diskname);
int file_exists(const char *filename);
int directory_exists(const char *pathname);
void list_dir(const char *path);

//-------------------------------------------------------------------------------------------------
// fs.h
//-------------------------------------------------------------------------------------------------

/*
 * use cwd normalize pathname
 *
 * 1. add "/" at header
 * 2. remove "../" in pathname
 */
extern char *fs_normalize_path_with_cwd(const char *path, char *resolved, int remove_last_backslash);

/*
 * likely: from filename "/usbd0/a1/dog.txt":
 *         return "/usbd0" & set filename[6]=0, *pos=6
 */
extern char *fs_extract_disk_name(char *filename, int *pos);

/*
 * likely: from filename "/usbd0/a1/dog.txt":
           return "/usbd0/a1" & set filename[9]=0, *pos=9
 */
extern char *fs_extract_path(char *filename, int *pos);

#ifdef __cplusplus
}
#endif

#endif // _FS_TEST_H

