/* 

2016. 11. 08.
File merge 

20113288 Hyunsu_So

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

#define BUFFER_SIZE 2048
#define PART_BUFFER_SIZE 200
#define FILE_NUMBER 2

typedef struct file_info{

	int fp; // file point
	int lp; // line point
	int size;
	char buf_in[BUFFER_SIZE];
	char buf_out[PART_BUFFER_SIZE];
	int line;

}File_info;

int main(int argc, char *argv[])
{
	FILE *file[FILE_NUMBER], *fout;
	int i, j = 0;
	char temp;
	int end = 0;
	int total_line = 0;
	File_info file_info[FILE_NUMBER];
	
	struct timeval before, after;
	int duration;
	int ret = 1;

	if (argc != 4) {
		fprintf(stderr, "usage: %s file1 file2 fout\n", argv[0]);
		goto leave0;
	}
	for (i = 0; i < FILE_NUMBER; i++){
		if ((file[i] = fopen(argv[i + 1], "rt")) == NULL) {
			perror(argv[i + 1]);
			goto leave1;
		}
	}
	if ((fout = fopen(argv[3], "wt")) == NULL) {
		perror(argv[3]);
		goto leave2;
	}

	// init file info
	for (i = 0; i < FILE_NUMBER; i++){
		(file_info + i)->fp = 0;
		(file_info + i)->lp = 0;
		(file_info + i)->line = 0;
		(file_info + i)->size = fread((file_info + i)->buf_in, 1, BUFFER_SIZE, file[i]);
	}

	gettimeofday(&before, NULL);
	while (1){

		// finish
		end = 0;
		for (i = 0; i < FILE_NUMBER; i++){
			if ((file_info + i)->size == 0){
				end++;
			}
		}
		if (end == FILE_NUMBER){
			break;
		}
		
		// process line
		for (i = 0; i < FILE_NUMBER; i++){
			while(1){

				// read buffer size
				if ((file_info + i)->fp == (file_info + i)->size){
					(file_info + i)->size = fread((file_info + i)->buf_in, 1, BUFFER_SIZE, file[i]);
					(file_info + i)->fp = 0;
				}

				if ((file_info + i)->size == 0){
					break;
				}
				(file_info + i)->buf_out[(file_info + i)->lp++] = (file_info + i)->buf_in[(file_info + i)->fp];
				
				// end line
				if ((file_info + i)->buf_in[(file_info + i)->fp] == '\n'){
					for (j = 0; j < (file_info + i)->lp / 2; j++){
						temp = (file_info + i)->buf_out[j];
						(file_info + i)->buf_out[j] = (file_info + i)->buf_out[(file_info + i)->lp - j - 1];
						(file_info + i)->buf_out[(file_info + i)->lp - j - 1] = temp;
            		}
					fwrite((file_info + i)->buf_out, 1, (file_info + i)->lp, fout);
					(file_info + i)->fp++;
					(file_info + i)->line++;
					(file_info + i)->lp = 0;
					break;
				}
				(file_info + i)->fp++;
			}
		}
	}
	gettimeofday(&after, NULL);

	duration = (after.tv_sec - before.tv_sec) * 1000000 + (after.tv_usec - before.tv_usec);
	printf("Processing time = %d.%06d sec\n", duration / 1000000, duration % 1000000);
	
	for (i = 0; i < FILE_NUMBER; i++){
		printf("file_%d line : %d \n", i + 1, file_info[i].line);
		total_line += (file_info + i)->line;
	
	}
	printf("total lien : %d \n", total_line);
	ret = 0;

leave2:
	fclose(fout);
leave1:
	for (i = 0; i < FILE_NUMBER; i++){
		fclose(file[i]);
	}
leave0:
	return ret;
}

