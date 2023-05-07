#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#define READ_LEN    (1024)  /*读取长度*/
#define FILL    (uint8_t)0xff

uint8_t read_buf[READ_LEN];/*读取缓冲*/
uint8_t fill_byte = FILL;/*填充字节*/
uint8_t fill_array[READ_LEN] = {[0 ... READ_LEN-1] = FILL};/*填充数组*/
char output_filename[128];



int main(int argc,char *argv[])
{
    if(argc <= 4)
    {
        printf("Please enter at least two file names");
        return -1;
    }

    uint8_t file_num = (argc-2)/2;
    uint32_t offset[file_num];
    FILE *file[file_num];
    char *filename[file_num];

    memset(offset,0,sizeof(offset));
    memset(file,0,sizeof(file));
    memset(filename,0,sizeof(filename));
    strcpy(output_filename,argv[argc-1]);
    for(int i = 0;i < file_num;i++)
    {
        char* endptr;
        filename[i] = argv[i+1];
        offset[i] = strtoimax(argv[i+1+file_num],&endptr,0);
    }


    FILE *fp_save = fopen(output_filename,"wb+"); 
    if(fp_save == NULL)
    {
        perror("tmpfile fail");
        return -1;
    }

    for(int i = 0;i < file_num;i++)
    {
        file[i] = fopen(filename[i],"rb");
        if(file[i] == NULL)
        {
            printf("open %s failed : %s\n", filename[i],strerror(errno));
            remove(output_filename);
            return -1;            
        }


        /*填充*/
        while((offset[i] - ftell(fp_save)) > READ_LEN)
        {
            fwrite(fill_array,sizeof(char),READ_LEN,fp_save);
        }
        while(offset[i] - ftell(fp_save))
        {
            fwrite(&fill_byte,sizeof(char),1,fp_save);
        }
        /*复制*/
        while(1)
        {
            uint16_t read_len = fread(read_buf,sizeof(char),READ_LEN,file[i]);
            uint16_t write_len = fwrite(read_buf,sizeof(char),read_len,fp_save);
            if(write_len != read_len)
            {
                perror("fwrite fail");
                fclose(file[i]);
                return -1;                 
            }
            if(read_len < READ_LEN)
            {
                fclose(file[i]);
                break;    
            }        
        }
        

    }



    fclose(fp_save);

    for(int i = 0;i < file_num;i++)
    {
        printf("%#010X \t%s\n",offset[i],filename[i]);
    }

    return 0;
}



/**
 * @brief   获取文件大小
 * 
 * @param   stream      
 * @return  uint64_t -1：获取失败 其他:获取成功
 */
 uint64_t get_file_size(FILE *stream)
{
	uint64_t file_size = -1;
	uint64_t cur_offset = ftell(stream);	// 获取当前偏移位置
	if (cur_offset == -1) {
		printf("ftell failed :%s\n", strerror(errno));
		return -1;
	}
    if (fseek(stream, 0, SEEK_END) != 0) {	// 移动文件指针到文件末尾
		printf("fseek failed: %s\n", strerror(errno));
		return -1;
	}
	file_size = ftell(stream);	// 获取此时偏移值，即文件大小
	if (file_size == -1) {
		printf("ftell failed :%s\n", strerror(errno));
	}
    if (fseek(stream, cur_offset, SEEK_SET) != 0) {	// 将文件指针恢复初始位置
		printf("fseek failed: %s\n", strerror(errno));
		return -1;
	}
	return file_size;
}


