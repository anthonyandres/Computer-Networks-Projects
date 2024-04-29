/*
 * test.c
 * 
 * Copyright 2023 antho <antho@ANTHONYLAPTOP>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>


#define BUFLEN 256

int main(int argc, char **argv)
{
	//char	*bp, buf[BUFLEN];
	char buffer[99999];
	char data[99999];
	//int 	n, bytes_to_read, wordCount = 0;
	FILE *file;

	file = fopen("test.txt", "rb");
	
	if(file != NULL){//check if file is valid
		printf("file exists\n");
		while(fgets(buffer, sizeof(buffer), file)){//read data from file
			strcat(data, buffer);
		}
		fclose(file);
	}
	else{
		printf("file does not exist\n");
	}
	int length = sizeof(data)/sizeof(data[0]);
	printf("size of data Array: %d\n", length);
	printf(data);
	
	//--------FOR USE IN CLIENT DOWNLOAD------//
	FILE *newfile = fopen("second.txt", "w");
	int stringLength = strlen(data);

	printf("\n\nlength of data until terminating character: %d\n", stringLength); 
	int i;	
	for(i = 0; i < stringLength; i++){
		   fprintf(newfile, "%c", data[i]);
	   }
	   
	//FOR SERVER WRITING TO CLIENT WITH 100 BYTE PACKAGES//  
	FILE *second = fopen("second.txt", "r");
	int j, a = 0, b = 100;
	char c;
	char buff[1000];

	//
	FILE *testFile = fopen("arbitratyDumbo.txt", "w");
	char *rbuf = "arbitraty dumbo";
	int strLen = strlen(rbuf);
	   int count = 0;
	   for(i = 0; i < strLen; i++){
		   fprintf(testFile, "%c", rbuf[i]);
	   }
	return 0;
}

