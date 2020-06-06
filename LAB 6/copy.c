#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <sys/mman.h> 
#include <fcntl.h> 
#include <unistd.h>

#define  DEFAULT_CHUNK  262144  /* 256k */

int mmapCopy(int inp_fd, int out_fd)
{
	printf("Copying using MMAP\n");
	struct stat file_status;
	fstat(inp_fd,&file_status);
	size_t filesize = file_status.st_size;


	lseek(inp_fd,0,SEEK_SET); // set to begining of file

	size_t size = DEFAULT_CHUNK;
 	char        *data, *dst;
 	off_t offset = 0;

	ftruncate(out_fd, filesize);
	
	while(filesize > 0)
	{
		if(filesize < size)
		{
		size = filesize;
		filesize = 0;
		}
		else
		{
		 filesize = filesize - size;
		}

		if((data=mmap(NULL,size,PROT_READ,MAP_SHARED,inp_fd,offset))==MAP_FAILED)
		{
		perror("mmap error, mapping input");
	
			if((munmap(data,size))==-1)printf("munmap error : input");
		return -1;
		}

		if((dst=mmap(NULL,size,PROT_READ | PROT_WRITE,MAP_SHARED,out_fd,offset))==MAP_FAILED)
            	{
		perror("mmap error, mapping output");
                	if((munmap(dst,size))==-1)printf("munmap error : output");
                return -1;
            	}

		memcpy(dst,data,size);
		
		if((munmap(dst,size))==-1)printf("munmap error : output");
		if((munmap(data,size))==-1)printf("munmap error : input");

		offset = offset + size;
	}
}

int readWriteCopy(int inp_fd, int out_fd)
{
	//Function will not handle closing files
	printf("Copying using Read/Write\n");
	const size_t size = DEFAULT_CHUNK;
	char        *data, *ptr, *end;
	ssize_t      bytes;
	
	//ALLOCATING BUFFER
	data = malloc(size);
    	if (!data) {
	perror("Buffer cannot be allocated");

	return -1;
    	}

	while (1) {
	//READING
	// -------------------------------------------
	bytes = read(inp_fd, data, size);
		if (bytes < 0) {
		perror("Reading data failed");
         	free(data); // dealocating buffer

            	return -1; // error during reading data
        	}
		else
		{
			if (bytes == 0) // EOF
            		break; // break coping loop
		}
	//WRITING
	// -------------------------------------------
		ptr = data; // begin
        	end = data + bytes; // end

		while (ptr < end) { // ensure that whole buffer will be written if possible			
			bytes = write(out_fd, ptr, (size_t)(end - ptr)); // write to output file (end-ptr) starting at ptr
			if (bytes <= 0) { // number of written bytes is <=0 means error in writing
                	
			free(data); // dealocate buffer

			perror("Writing data failed");
			return -1; 
			}
			else
			{
			ptr += bytes; // in case we couldn't write whole buffer shift pointer
			}
		}// write while
	}
	
	free(data); // dealocate buffer
	return 0; // everything is OK
}

int openFiles(char * input, char * output, int * inp_fd, int * out_fd)
{
	if(inp_fd == NULL || out_fd == NULL || input == NULL || output == NULL)return -1;

	*inp_fd = open(input, O_RDONLY);
 	if (*inp_fd ==-1) 
    	{ 
        perror(input); 
	return -1; 
    	}

	*out_fd = open(output, O_RDWR | O_CREAT | O_TRUNC);
 	if (*out_fd ==-1) 
    	{ 
        perror(output);
	return -1; 
    	}

	return 0;
}

int closeFiles(int inp_fd, int out_fd)
{
	int error_code = 0;
	if(inp_fd != -1){
		if (close(inp_fd) < 0)  
		{ 
        	perror("Error closing file"); 
		error_code = -1;
		}
	}
	if(out_fd != -1){
		if (close(out_fd) < 0)  
		{ 
        	perror("Error closing file");
		error_code = -1;
		}
	}
	return error_code; 
}

int performCopy(char * input, char * output, int MODE)
{

     printf("File input: %s\n", input);  
     printf("File output: %s\n", output);


	int in_fd, out_fd;
	if(openFiles(input, output, &in_fd, &out_fd) == -1)
	{
	closeFiles(in_fd, out_fd);
	return -1;
	}

	int error_code = 0;

	if(MODE == 0)readWriteCopy(in_fd, out_fd);
	if(MODE == 2)mmapCopy(in_fd, out_fd);
	closeFiles(in_fd, out_fd);

	return 0;
	
}


void displayHelp()
{
	printf("copy [-m] <file_name> <new_file_name>\ncopy [-h]\n");
	printf("Without option -m use read() and write() functions to copy file contents.\nIf the option -m is given, maps files to memory regions with mmap()\n\tand copy the file with memcpy() instead.\n");
	printf("If the option -h is given or the program is called without arguments\n\tprint out some help information. \n");
}

int main(int argc, char *argv[])
{

	int opt;
	int MODE = 0;

	while((opt = getopt(argc, argv, "hm")) != -1)
	{  
        	switch(opt)  
        	{
       		case 'h': if(MODE!= 0){printf("To many arguments\n");exit(0);}else MODE = 1;  break;
      		case 'm': if(MODE!= 0){printf("To many arguments\n");exit(0);} else  MODE = 2; break;
		case '?': printf("Unknow argument\n");exit(0); break;
		}
	}

	int start_id = optind;
	int extra_args = argc - optind;

	if((MODE == 0 || MODE == 1) && extra_args == 0){displayHelp();exit(0);} // no args
	if(MODE == 1 && extra_args != 0){printf("To many arguments\n");exit(0);} // no args
	if(MODE == 0  && extra_args != 0 && extra_args != 2){printf("Wrong combination of arguments\n");exit(0);}
	if(MODE == 2 && extra_args < 2){printf("Missing argument: file name\n");exit(0);}
	if(MODE == 2 && extra_args > 2){printf("To many arguments: file name\n");exit(0);}

	performCopy(argv[start_id], argv[start_id+1], MODE);


	return 0;
}
