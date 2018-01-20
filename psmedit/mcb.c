/*
 * メモリ操作の共通関数
 */

/*
 * open() close() fstat()
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "mc.h"
#include "mcb.h"
#include "error.h"



/*
 * ファイルmcbfnameを元にして
 * memを初期化する
 *
 * 読み込んだサイズを返す
 */
int mcb_read_file(const char *mcbfname, void *mem)
{
	int fd;
	struct stat st;

	if((fd = open(mcbfname, O_RDONLY)) == -1)
		error_exit("open mcbfile");

	/* サイズチェック */
	if(fstat(fd, &st) == -1)
		error_exit("fstat mcbfile");
	if((st.st_size % SIZE_BLOCK != SIZE_MCB_HEAD) || st.st_size > SIZE_MCB_MAX)
		error_exit("mcbfile size");

	if(read(fd, mem, st.st_size) == -1)
		error_exit("read mcbfile");

	close(fd);

	return st.st_size;
}


/*
 * mcbのデータ部分を取得する
 */
void *mcb_get_data(const void *mcb)
{
	const unsigned char *m_mcb = mcb;

	return (void *)(m_mcb + SIZE_MCB_HEAD);
}


