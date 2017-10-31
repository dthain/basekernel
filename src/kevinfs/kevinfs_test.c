#include "kevinfs.h"
#include "../string.h"
#include "../kmalloc.h"
#include "../kerneltypes.h"
#include "kevinfs_transaction.h"
#include "../fs.h"

void test_mount() {
	struct fs *f = fs_get("kevin");
	struct volume *v = fs_mount(f, 0);
	struct dirent *d = fs_root(v);
	printf("dirent size: %d\n", d->sz);
}

//void test_lsdir_empty() {
//	printf("test_lsdir_empty\n");
//	kevinfs_lsdir();	
//}
//
//void test_mkdir() {
//	kevinfs_lsdir();
//	printf("test_mkdir\n");
//	kevinfs_mkdir("example");
//	kevinfs_lsdir();
//}
//
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
//void test_open_fd() {
//	int fd1 = kevinfs_open("fda", FILE_MODE_WRITE);
//	struct kevinfs_stat stat;
//	uint8_t buffer[] = "my name is kevin.";
//	uint8_t buffer2[sizeof(buffer) + 1];
//	kevinfs_lsdir();
//	printf("%d\n", fd1);
//	printf("%u\n", sizeof(buffer));
//	printf("result_write %d\n", kevinfs_write(fd1, buffer, sizeof(buffer)));
//	kevinfs_close(fd1);
//	kevinfs_stat("fda", &stat);
//	printf("%u %u %u %u %u\n", stat.inode_number, stat.is_directory, stat.size, stat.links, stat.num_blocks);
//	kevinfs_link("fda", "link");
//	fd1 = kevinfs_open("link", FILE_MODE_READ);
//	kevinfs_lseek(fd1, 3);
//	printf("result_read %d\n", kevinfs_read(fd1, buffer2, sizeof(buffer)));
//	buffer2[sizeof(buffer)] = 0;
//	printf("response? %s\n", buffer2);
//	printf("%d\n", kevinfs_close(fd1));
//	printf("%d\n", kevinfs_close(fd1));
//	kevinfs_lsdir();
//	printf("%d\n", kevinfs_unlink("fda"));
//	kevinfs_lsdir();
//	printf("%d\n", kevinfs_unlink("link"));
//
//	fd1 = kevinfs_open("example", FILE_MODE_WRITE);
//	kevinfs_stat("example", &stat);
//	printf("%u %u %u %u %u\n", stat.inode_number, stat.is_directory, stat.size, stat.links, stat.num_blocks);
//	kevinfs_lsdir();
//}
//
//void test_chdir() {
//	kevinfs_mkdir("dir");
//	kevinfs_lsdir();
//	kevinfs_chdir("dir");
//	kevinfs_lsdir();
//	kevinfs_mkdir("dir2");
//	kevinfs_lsdir();
//	kevinfs_chdir("..");
//	kevinfs_lsdir();
//	kevinfs_chdir(".");
//	kevinfs_lsdir();
//}
//
//void test_stat() {
//	struct kevinfs_stat stat;
//	kevinfs_mkdir("dur");
//	kevinfs_stat("dur", &stat);
//	printf("%u %u %u %u %u\n", stat.inode_number, stat.is_directory, stat.size, stat.links, stat.num_blocks);
//	kevinfs_stat(".", &stat);
//	printf("%u %u %u %u %u\n", stat.inode_number, stat.is_directory, stat.size, stat.links, stat.num_blocks);
//}
//
//void test_rollback() {
//	struct kevinfs_transaction bad_list;
//	kevinfs_transaction_init(&bad_list);
//	struct kevinfs_inode *node = kmalloc(sizeof(struct kevinfs_inode));
//	uint8_t shouldnt_write[FS_BLOCKSIZE];
//	memset(node, 0, sizeof(struct kevinfs_inode));
//	node->inode_number = 1;
//	node->direct_addresses_len = 1;
//	node->direct_addresses[0] = 0;
//	node->sz = 0;
//
//	kevinfs_lsdir();
//	kevinfs_transaction_stage_inode(&bad_list, node, FS_TRANSACTION_DELETE);
//	kevinfs_transaction_stage_data(&bad_list, 15, shouldnt_write, FS_TRANSACTION_MODIFY);
//
//	kevinfs_transaction_commit(&bad_list);
//	kevinfs_lsdir();
//}

int kevinfs_test() {
	//test_lsdir_empty();
	//test_mkdir_to_max();
	//test_rmdir_to_min();
	//test_open_fd();
	//test_chdir();
	//test_rollback();
	test_mount();

	return 0;
}
