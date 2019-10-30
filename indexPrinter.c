#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

void indexPrinter(const char* indexFileNm, const char* outputFileNm)
{
	/* 파일 읽어오기 */
	int rd = open(indexFileNm, O_RDONLY);
	int wd = open(outputFileNm, O_WRONLY | O_CREAT);

	char *buf;
	buf = (char*)malloc(256);
	
	if(rd > 0){
		while(read(rd, buf, 256)){
			write(wd, buf, 256);
			buf = (char*)calloc(256, sizeof(char));
		}
	}
	close(rd);
	close(wd);
}