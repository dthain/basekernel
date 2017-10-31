#include "kevinfs.h"
#include "../string.h"
#include "../kmalloc.h"
#include "../kerneltypes.h"
#include "kevinfs_transaction.h"
#include "../fs.h"

void test_mount() {
	printf("test_mount\n");
	struct fs *f = fs_get("kevin");
	struct volume *v = fs_mount(f, 0);
	struct dirent *d = fs_root(v);
	printf("root dirent size: %d\n", d->sz);
}

void test_lsdir_empty() {
	printf("test_lsdir_empty\n");
	struct fs *f = fs_get("kevin");
	struct volume *v = fs_mount(f, 0);
	struct dirent *d = fs_root(v);
	char buffer[1000];
	int n = fs_readdir(d, buffer, 1000);
	buffer[n] = 0;
	printf("%s\n", buffer);
}

void test_mkdir() {
	printf("test_mkdir\n");
	struct fs *f = fs_get("kevin");
	struct volume *v = fs_mount(f, 0);
	struct dirent *d = fs_root(v);
	fs_mkdir(d, "example_directory");
	char buffer[1000];
	int n = fs_readdir(d, buffer, 1000);
	buffer[n] = 0;
	printf("%s\n", buffer);
}

void test_rmdir() {
	printf("test_rmdir\n");
	struct fs *f = fs_get("kevin");
	struct volume *v = fs_mount(f, 0);
	struct dirent *d = fs_root(v);
	fs_rmdir(d, "example");
	char buffer[1000];
	int n = fs_readdir(d, buffer, 1000);
	buffer[n] = 0;
	printf("%s\n", buffer);
}

//void test_mkdir_to_max() {
//	uint32_t i;
//	printf("test_mkdir_to_max\n");
//	for (i = 0;; i++) {
//		char filename[30] = "example";
//		char id[4];
//		strcat(filename, uint_to_string(i, id));
//		printf("hello?");
//		printf("writing %s\n", filename);
//		if(kevinfs_mkdir(filename) < 0)
//			break;
//	}
//	kevinfs_lsdir();
//}
//
//void test_rmdir_to_min() {
//	uint32_t i;
//	printf("test_rmdir_to_min\n");
//	for (i = 0;; i++) {
//		char filename[30] = "example";
//		char id[4];
//		strcat(filename, uint_to_string(i, id));
//		if(kevinfs_rmdir(filename) < 0)
//			break;
//	}
//	kevinfs_lsdir();
//}
//
void test_open_fd() {
	printf("test_file_read_write\n");
	struct fs *f = fs_get("kevin");
	struct volume *v = fs_mount(f, 0);
	struct dirent *d = fs_root(v);
	fs_mkfile(d, "read_write_file");
	char buffer[1000];
	char wbuffer[] = "This is the content of the file.";
	int n = fs_readdir(d, buffer, 1000);
	buffer[n] = 0;
	printf("%s\n", buffer);

	struct dirent *d_f = fs_namei(d, "read_write_file");
	struct file *fp = fs_open(d_f, FILE_MODE_WRITE);
	printf("opened file for writing\n");
	int result = fs_write(fp, wbuffer, sizeof(wbuffer));
	printf("wrote %d bytes\n", result);
	fs_close(fp);
	printf("closed file\n");
	printf("opened file for reading\n");
	fp = fs_open(d_f, FILE_MODE_READ);
	printf("read %d bytes:\n", result);
	result = fs_read(fp, buffer, sizeof(wbuffer));
	printf("%s\n", buffer);
	fs_close(fp);
	printf("closed file\n");
	fs_unlink(d, "read_write_file");
	printf("deleted file\n");
	n = fs_readdir(d, buffer, 1000);
	buffer[n] = 0;
	printf("%s\n", buffer);
}

int kevinfs_test() {
	test_mount();
	test_lsdir_empty();
	test_mkdir();
	test_rmdir();
	//test_mkdir_to_max();
	//test_rmdir_to_min();
	test_open_fd();

	return 0;
}
