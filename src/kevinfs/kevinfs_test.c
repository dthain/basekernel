#include "kevinfs.h"
#include "../string.h"
#include "../kmalloc.h"
#include "../kerneltypes.h"
#include "../fs.h"

void test_mount() {
	printf("test_mount\n");
	struct fs *f = fs_get("kevin");
	struct fs_volume *v = fs_volume_mount(f, 0);
	struct fs_dirent *d = fs_volume_root(v);
	printf("root fs_dirent size: %d\n", d->sz);
}

void test_lsdir_empty() {
	printf("test_lsdir_empty\n");
	struct fs *f = fs_get("kevin");
	struct fs_volume *v = fs_volume_mount(f, 0);
	struct fs_dirent *d = fs_volume_root(v);
	char buffer[1000];
	int n = fs_dirent_readdir(d, buffer, 1000);
	buffer[n] = 0;
	printf("%s\n", buffer);
}

void test_mkdir() {
	printf("test_mkdir\n");
	struct fs *f = fs_get("kevin");
	struct fs_volume *v = fs_volume_mount(f, 0);
	struct fs_dirent *d = fs_volume_root(v);
	fs_dirent_mkdir(d, "example_directory");
	char buffer[1000];
	int n = fs_dirent_readdir(d, buffer, 1000);
	buffer[n] = 0;
	printf("%s\n", buffer);
}

void test_rmdir() {
	printf("test_rmdir\n");
	struct fs *f = fs_get("kevin");
	struct fs_volume *v = fs_volume_mount(f, 0);
	struct fs_dirent *d = fs_volume_root(v);
	fs_dirent_rmdir(d, "example_directory");
	char buffer[1000];
	int n = fs_dirent_readdir(d, buffer, 1000);
	buffer[n] = 0;
	printf("%s\n", buffer);
}

void test_mkdir_to_max() {
	printf("test_mkdir_to_max\n");
	struct fs *f = fs_get("kevin");
	struct fs_volume *v = fs_volume_mount(f, 0);
	struct fs_dirent *d = fs_volume_root(v);
	uint32_t i;
	for (i = 0;; i++) {
		char filename[30] = "example";
		char id[4];
		strcat(filename, uint_to_string(i, id));
		printf("writing %s\n", filename);
		if(fs_dirent_mkdir(d, filename) < 0)
			break;
	}
	char buffer[1000];
	int n = fs_dirent_readdir(d, buffer, 1000);
	buffer[n] = 0;
	printf("%s\n", buffer);
}

void test_rmdir_to_min() {
	printf("test_rmdir_to_min\n");
	struct fs *f = fs_get("kevin");
	struct fs_volume *v = fs_volume_mount(f, 0);
	struct fs_dirent *d = fs_volume_root(v);
	uint32_t i;
	for (i = 0;; i++) {
		char filename[30] = "example";
		char id[4];
		strcat(filename, uint_to_string(i, id));
		if(fs_dirent_rmdir(d, filename) < 0)
			break;
	}
	char buffer[1000];
	int n = fs_dirent_readdir(d, buffer, 1000);
	buffer[n] = 0;
	printf("%s\n", buffer);
}

void test_write_file() {
	printf("test_file_read_write\n");
	struct fs *f = fs_get("kevin");
	struct fs_volume *v = fs_volume_mount(f, 0);
	struct fs_dirent *d = fs_volume_root(v);
	fs_dirent_mkfile(d, "read_write_file");
	char buffer[1000];
	char wbuffer[] = "This is the content of the kevinfs file.";
	int n = fs_dirent_readdir(d, buffer, 1000);
	buffer[n] = 0;
	printf("%s\n", buffer);

	struct fs_dirent *d_f = fs_dirent_namei(d, "read_write_file");
	struct fs_file *fp = fs_file_open(d_f, FILE_MODE_WRITE);
	printf("opened read_write_file for writing\n");
	int result = fs_file_write(fp, wbuffer, sizeof(wbuffer));
	printf("wrote %d bytes\n", result);
	fs_file_close(fp);
	printf("closed read_write_file\n");
	printf("opened read_write_file for reading\n");
	fp = fs_file_open(d_f, FILE_MODE_READ);
	printf("read %d bytes:\n", result);
	result = fs_file_read(fp, buffer, sizeof(wbuffer));
	printf("%s\n", buffer);
	fs_file_close(fp);
	printf("closed read_write_file\n");
	fs_dirent_unlink(d, "read_write_file");
	printf("deleted read_file\n");
	n = fs_dirent_readdir(d, buffer, 1000);
	buffer[n] = 0;
	printf("%s\n", buffer);
}

int kevinfs_test() {
	test_mount();
	test_lsdir_empty();
	test_mkdir();
	test_rmdir();
	test_mkdir_to_max();
	test_rmdir_to_min();
	test_write_file();

	return 0;
}
