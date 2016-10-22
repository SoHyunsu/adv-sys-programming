/* 

2016. 10. 21.
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
#define FILE_SIZE 2

typedef struct file_info{
	int file_point;
	int size;
	char *buffer;
	char *part_buffer;
	int part_index;
	int line;
}File_info;

int main(int argc, char *argv[])
{
	FILE *file[FILE_SIZE], *fout;
	int i, j = 0;
	char temp;
	int end = 0;
	int total_line = 0;
	File_info file_info[FILE_SIZE];
	
	struct timeval before, after;
	int duration;
	int ret = 1;

	if (argc != 4) {
		fprintf(stderr, "usage: %s file1 file2 fout\n", argv[0]);
		goto leave0;
	}
	for (i = 0; i < FILE_SIZE; i++){
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
	for (i = 0; i < FILE_SIZE; i++){
		file_info[i].file_point = 0;
		file_info[i].buffer = (char*)malloc(sizeof(char) * BUFFER_SIZE);
		file_info[i].part_buffer = (char*)malloc(sizeof(char) * PART_BUFFER_SIZE);
		file_info[i].part_index = 0;
		file_info[i].line = 0;

		file_info[i].size = fread(file_info[i].buffer, 1, BUFFER_SIZE, file[i]);
	}

	gettimeofday(&before, NULL);
	while (1){

		// finish
		end = 0;
		for (i = 0; i < FILE_SIZE; i++){
			if (file_info[i].size == 0){
				end++;
			}
		}
		if (end == FILE_SIZE){
			break;
		}

		// process line
		for (i = 0; i < FILE_SIZE; i++){
			while(1){

				// read buffer size
				if (file_info[i].file_point == file_info[i].size){
					file_info[i].size = fread(file_info[i].buffer, 1, BUFFER_SIZE, file[i]);
					file_info[i].file_point = 0;
				}

				if(file_info[i].size == 0){
					break;
				}

				file_info[i].part_buffer[file_info[i].part_index++] = file_info[i].buffer[file_info[i].file_point];
				
				// end line
				if (file_info[i].buffer[file_info[i].file_point] == '\n'){
					for(j = 0; j < file_info[i].part_index / 2; j++){
						temp = file_info[i].part_buffer[j];
						file_info[i].part_buffer[j] = file_info[i].part_buffer[file_info[i].part_index - j - 1];
						file_info[i].part_buffer[file_info[i].part_index - j - 1] = temp;
            				}
					fwrite(file_info[i].part_buffer, 1, file_info[i].part_index, fout);
					file_info[i].file_point++;
					file_info[i].line++;
					file_info[i].part_index = 0;
					break;
				}
				file_info[i].file_point++;
			}
		}
	}
	gettimeofday(&after, NULL);

	duration = (after.tv_sec - before.tv_sec) * 1000000 + (after.tv_usec - before.tv_usec);
	printf("Processing time = %d.%06d sec\n", duration / 1000000, duration % 1000000);
	
	for (i = 0; i < FILE_SIZE; i++){
		printf("file_%d line : %d \n", i + 1, file_info[i].line);
		total_line += file_info[i].line;
	
	}
	printf("total lien : %d \n", total_line);
	ret = 0;

leave2:
	fclose(fout);
leave1:
	for (i = 0; i < FILE_SIZE; i++){
		fclose(file[i]);
	}
leave0:
	return ret;
}

