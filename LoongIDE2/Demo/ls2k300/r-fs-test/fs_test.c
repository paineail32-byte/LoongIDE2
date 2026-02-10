/*
 * fs_test_funcs.c
 *
 * created: 2025-02-26
 *  author:
 */

#include "bsp.h"

#if BSP_USE_FS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/dirent.h>
#include <sys/stat.h>

#include "fs_test.h"

//-----------------------------------------------------------------------------
// Common functions
//-----------------------------------------------------------------------------

/*
 * 磁盘是否存在, 存在返回 1
 */
int disk_exists(const char *diskname)
{
    return (access(diskname, 0) == 0) ? 1 : 0;
}

/*
 * 目录是否存在, 存在返回 1
 */
int directory_exists(const char *pathname)
{
    struct stat st;

    if (stat(pathname, &st) == 0)
    {
        return (st.st_mode & S_IFDIR) ? 1 : 0;
    }

    return 0;
}

/*
 * 文件是否存在, 存在返回 1
 */
int file_exists(const char *filename)
{
    struct stat st;

    if (stat(filename, &st) == 0)
    {
        return (st.st_mode & S_IFREG) ? 1 : 0;
    }

    return 0;
}

//-----------------------------------------------------------------------------
// list directory contents
//-----------------------------------------------------------------------------

void list_dir(const char *path)
{
    DIR *dir;
    struct dirent *de;

    if (!directory_exists(path))
    {
        printf("Directory %s not exists\r\n", path);
        return;
    }

    printf("Contents of %s\r\n", path);

    dir = opendir(path);

    if (dir)
    {
        de = readdir(dir);

        while (de)
        {
            if (de->d_attrs & S_IFDIR)
                printf("  <dir>   %s\r\n", de->d_name);
            else
                printf("          %s\r\n", de->d_name);

            de = readdir(dir);
        }

        printf("\r\n");
        closedir(dir);

    }
    else
    {
        printf("  <empty>\r\n");
    }
}

//-----------------------------------------------------------------------------
// 目录操作
//-----------------------------------------------------------------------------

#if TEST_DIR

void dir_test(char *pathname)
{
    char ppath[256], *p;
    int  n=0;

    /*
     * 从 new_path 获取父目录
     */
    p = fs_extract_path(pathname, &n);

    if (p && (n > 0))
    {
        snprintf(ppath, 255, "%s", p);
        p[n] = '/';
    }
    else
    {
        printf("  invalid new path name \"%s\"\r\n", pathname);
        return;
    }

    if (!directory_exists(ppath))
    {
        printf("  parent path \"%s\" not exists\r\n", ppath);
        return;
    }

    /*
     * 创建目录
     */
    #if TEST_DIR_MK
    {
        list_dir(ppath);                /* list 内容 */

        if (directory_exists(pathname))
        {
            printf("  directory %s exists\r\n", pathname);
        }
        else
        {
            printf("  directory %s not exists\r\n", pathname);

            if (mkdir(pathname, 0777) == 0)
                printf("  create directory %s successful\r\n", pathname);
            else
                printf("  create directory %s fail\r\n", pathname);

            list_dir(ppath);            /* list 内容 */
        }
    }
    #endif

    /*
     * 删除目录
     */
    #if TEST_DIR_RM
    {
        #if !TEST_DIR_MK
        {
            list_dir(ppath);            /* list 内容 */
        }
        #endif

        if (!directory_exists(pathname))
        {
            printf("  directory %s not exists\r\n", pathname);
        }
        else
        {
            if (rmdir(pathname) == 0)
                printf("  remove directory %s successful\r\n", pathname);
            else
                printf("  remove directory %s fail\r\n", pathname);

            list_dir(ppath);            /* list 内容 */
        }
    }
    #endif

}

#else

void dir_test(char *pathname)
{
    return;
}

#endif // #if TEST_DIR


//-----------------------------------------------------------------------------
// 文件测试
//-----------------------------------------------------------------------------

#if TEST_FILE

/*
 * 使用 open() read() write() close() 等函数
 */
void file_test_1(char *filename)
{
    char path[256], *p;
    int  n=0;
    int  f1;

    /*
     * 从 new_path 获取父目录
     */
    p = fs_extract_path(filename, &n);

    if (p && (n > 0))
    {
        snprintf(path, 255, "%s", p);
        p[n] = '/';
    }
    else
    {
        printf("  invalid file name \"%s\"\r\n", filename);
        return;
    }

    if (!directory_exists(path))
    {
        printf("  parent path \"%s\" not exists\r\n", path);
        return;
    }

    /*
     * 打开(创建)文件
     */
    f1 = open(filename, O_RDWR | O_CREAT);
    if (f1 < 0)
    {
        printf("  open file %s fail\r\n", filename);
        return;
    }

    list_dir(path);

    /*
     * 写文件
     */
    #if TEST_FILE_WRITE
    {
        const char wrbuf[] = "Hello, I'm loongarch64 LS2K0300.";
        int n_wr = strlen(wrbuf);

        ftruncate(f1, 0);

        if (write(f1, (const void *)wrbuf, (size_t)n_wr) == n_wr)
            printf("  write %i bytes into file %s successful\r\n", n_wr, filename);
        else
            printf("  write file %s fail\r\n", filename);
    }
    #endif

    /*
     * 读文件
     */
    #if TEST_FILE_READ
    {
        char rdbuf[128] = { 0 };
        int n_rd;

        lseek(f1, 0, SEEK_SET);

        n_rd = read(f1, (void *)rdbuf, 32);
        if (n_rd > 0)
        {
            printf("  read %i bytes from file %s successful\r\n", n_rd, filename);
            printf("  contents:\r\n    %s\r\n", rdbuf);
        }
        else
            printf("  read file %s fail\r\n", filename);
    }
    #endif

    /*
     * 关闭文件
     */
    close(f1);

    /*
     * 删除文件
     */
    #if TEST_FILE_DEL
    {
        if (file_exists(filename))
        {
            printf("  Try delete file %s\r\n", filename);
            if (unlink(filename) == 0)
                printf("  --- delete successful\r\n");
            else
                printf("  --- delete fail\r\n");
        }
        else
        {
            printf("  file %s not exists\r\n", filename);
        }
    }
    #endif

    list_dir(path);
}

//---------------------------------------------------------
//---------------------------------------------------------

/*
 * 使用 fopen() fread() fwrite() fclose() 等函数
 */
void file_test_2(char *filename)
{
    char  path[256], *p;
    int   n=0;
    FILE *f2;

    /*
     * 从 new_path 获取父目录
     */
    p = fs_extract_path(filename, &n);

    if (p && (n > 0))
    {
        snprintf(path, 255, "%s", p);
        p[n] = '/';
    }
    else
    {
        printf("  invalid file name \"%s\"\r\n", filename);
        return;
    }

    if (!directory_exists(path))
    {
        printf("  parent path \"%s\" not exists\r\n", path);
        return;
    }

    /*
     * 打开(创建)文件
     */
    f2 = fopen(filename, "w+");
    if (f2 == NULL)
    {
        printf("  open file %s fail\r\n", filename);
        return;
    }

    list_dir(path);

    /*
     * 写文件
     */
    #if TEST_FILE_WRITE
    {
        const char wrbuf[] = "Hello, I'm loongarch64 LS2K0300.";
        int n_wr = strlen(wrbuf);

        // ftruncate(fd, 0);

        if (fwrite(wrbuf, 1, (size_t)n_wr, f2) == n_wr)
            printf("  write %i bytes into file %s successful\r\n", (int)n_wr, filename);
        else
            printf("  write file %s fail\r\n", filename);
    }
    #endif

    /*
     * 读文件
     */
    #if TEST_FILE_READ
    {
        char rdbuf[128] = { 0 };
        int  n_rd;

        fseek(f2, 0, SEEK_SET);

        n_rd = fread((void *)rdbuf, 1, 32, f2);
        if (n_rd > 0)
        {
            printf("  read %i bytes from file %s successful\r\n", n_rd, filename);
            printf("  contents:\r\n    %s\r\n", rdbuf);
        }
        else
            printf("  read file %s fail\r\n", filename);
    }
    #endif

    /*
     * 关闭文件
     */
    fclose(f2);

    /*
     * 删除文件
     */
    #if TEST_FILE_DEL
    {
        if (file_exists(filename))
        {
            printf("  Try delete file %s\r\n", filename);
            if (unlink(filename) == 0)
                printf("  --- delete successful\r\n");
            else
                printf("  --- delete fail\r\n");
        }
        else
            printf("  file %s not exists\r\n", filename);
    }
    #endif

    list_dir(path);
}

#else

void file_test_1(char *filename)
{
    return;
}

void file_test_2(char *filename)
{
    return;
}

#endif // #if TEST_FILE


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// 读写测试函数
//-----------------------------------------------------------------------------

int do_fs_rw_test(void)
{
    #if TEST_SATA_RW
    {
        static int is_sata_tested = 0;

        if (!is_sata_tested)    // 只执行一次
        {
            if (disk_exists("/hda"))
            {
                dir_test("/hda/ddd1");

            #if 1
                file_test_1("/hda/a1/hello1.txt");
            #else
                file_test_2("/hda/a1/hello2.txt");
            #endif

                is_sata_tested = 1;
            }
        }
    }
    #endif

    #if TEST_EMMC_RW
    {
        static int is_emmc_tested = 0;

        if (!is_emmc_tested)    // 只执行一次
        {
            if (disk_exists("/mmc0"))
            {
                dir_test("/mmc0/ddd1");

            #if 1
                file_test_1("/mmc0/a1/hello1.txt");
            #else
                file_test_2("/mmc0/a1/hello2.txt");
            #endif

                is_emmc_tested = 1;
            }
        }
    }
    #endif

    #if TEST_YAFFS2_RW
    {
        static int is_yaffs_tested = 0;

        if (!is_yaffs_tested)    // 只执行一次
        {
            if (disk_exists("/ndd"))
            {
                dir_test("/ndd/ddd1");

            #if 1
                file_test_1("/ndd/a1/hello1.txt");
            #else
                file_test_2("/ndd/a1/hello2.txt");
            #endif

                is_yaffs_tested = 1;
            }
        }
    }
    #endif

    #if TEST_USB_RW
    {
        extern int unmount(const char *mount_point);
        
        static int is_usb_tested = 0;

        if (!is_usb_tested)     // 只执行一次
        {
            /*
             * 等待 U 盘插入
             */
            if (disk_exists("/usbd0"))
            {
        #if 1
                dir_test("/usbd0/ddd2");

            #if 0
                file_test_1("/usbd0/a2/hello1.txt");
            #else
                file_test_2("/usbd0/a2/hello2.txt");
            #endif
        #else
                printk("/usbd0 inserted...\r\n");
        #endif
    
                unmount("/usbd0");
                is_usb_tested = 1;
            }
        }
    }
    #endif

    return 0;
}



#endif // #if BSP_USE_FS

//-----------------------------------------------------------------------------
/*
 * @@ END
 */

