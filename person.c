#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "person.h"
//필요한 경우 헤더 파일과 함수를 추가할 수 있음

// 과제 설명서대로 구현하는 방식은 각자 다를 수 있지만 약간의 제약을 둡니다.
// 레코드 파일이 페이지 단위로 저장 관리되기 때문에 사용자 프로그램에서 레코드 파일로부터 데이터를 읽고 쓸 때도
// 페이지 단위를 사용합니다. 따라서 아래의 두 함수가 필요합니다.
// 1. readPage(): 주어진 페이지 번호의 페이지 데이터를 프로그램 상으로 읽어와서 pagebuf에 저장한다
// 2. writePage(): 프로그램 상의 pagebuf의 데이터를 주어진 페이지 번호에 저장한다
// 레코드 파일에서 기존의 레코드를 읽거나 새로운 레코드를 쓰거나 삭제 레코드를 수정할 때나
// 위의 readPage() 함수를 호출하여 pagebuf에 저장한 후, 여기에 필요에 따라서 새로운 레코드를 저장하거나
// 삭제 레코드 관리를 위한 메타데이터를 저장합니다. 그리고 난 후 writePage() 함수를 호출하여 수정된 pagebuf를
// 레코드 파일에 저장합니다. 반드시 페이지 단위로 읽거나 써야 합니다.
//
// 주의: 데이터 페이지로부터 레코드(삭제 레코드 포함)를 읽거나 쓸 때 페이지 단위로 I/O를 처리해야 하지만,
// 헤더 레코드의 메타데이터를 저장하거나 수정하는 경우 페이지 단위로 처리하지 않고 직접 레코드 파일을 접근해서 처리한다.

//
// 페이지 번호에 해당하는 페이지를 주어진 페이지 버퍼에 읽어서 저장한다. 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
//
void readPage(FILE *fp, char *pagebuf, int pagenum)
{
	fseek(fp, (PAGE_SIZE * pagenum) + 16, SEEK_SET);
	if(fread(pagebuf, PAGE_SIZE, 1, fp) != 1) {
		fprintf(stderr, "fread error\n");
		exit(1);
	}
}

//
// 페이지 버퍼의 데이터를 주어진 페이지 번호에 해당하는 레코드 파일의 위치에 저장한다. 
// 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
//
void writePage(FILE *fp, const char *pagebuf, int pagenum)
{
	fseek(fp, (PAGE_SIZE * pagenum) + 16, SEEK_SET);
	if(fwrite(pagebuf, PAGE_SIZE, 1, fp) != 1) {
		fprintf(stderr, "fwrite error\n");
		exit(1);
	}
}

//
// 새로운 레코드를 저장할 때 터미널로부터 입력받은 정보를 Person 구조체에 먼저 저장하고, pack() 함수를 사용하여
// 레코드 파일에 저장할 레코드 형태를 recordbuf에 만든다. 
// 
void pack(char *recordbuf, const Person *p)
{
	strcpy(recordbuf, p->id);
	strcat(recordbuf, "#");
	strcat(recordbuf, p->name);
	strcat(recordbuf, "#");
	strcat(recordbuf, p->age);
	strcat(recordbuf, "#");
	strcat(recordbuf, p->addr);
	strcat(recordbuf, "#");
	strcat(recordbuf, p->phone);
	strcat(recordbuf, "#");
	strcat(recordbuf, p->email);
	strcat(recordbuf, "#");
}

// 
// 아래의 unpack() 함수는 recordbuf에 저장되어 있는 레코드를 구조체로 변환할 때 사용한다.
//
void unpack(const char *recordbuf, Person *p)
{
	char* buf = (char*)malloc(sizeof(recordbuf));
	strcpy(buf, recordbuf);
	char* ptr = strtok(buf, "#");
	strcpy(p->id, ptr);
	ptr = strtok(NULL, "#");
	strcpy(p->name, ptr);
	ptr = strtok(NULL, "#");
	strcpy(p->age, ptr);
	ptr = strtok(NULL, "#");
	strcpy(p->addr, ptr);
	ptr = strtok(NULL, "#");
	strcpy(p->phone, ptr);
	ptr = strtok(NULL, "#");
	strcpy(p->email, ptr);
	free(buf);
}

//
// 새로운 레코드를 저장하는 기능을 수행하며, 터미널로부터 입력받은 필드값들을 구조체에 저장한 후 아래 함수를 호출한다.
//
void add(FILE *fp, const Person *p)
{
	char header_area[16];
	char recordbuf[MAX_RECORD_SIZE];
	char pagebuf[PAGE_SIZE];
	memset(recordbuf, 0, sizeof(recordbuf));
	pack(recordbuf, p);
	int pages, records, pnum, rnum;
	fseek(fp, 0, SEEK_SET);//삭제 레코드 주소
	if(fread(header_area, sizeof(header_area), 1, fp) != 1) {
		fprintf(stderr, "fread error\n");
		exit(1);
	}
	memcpy(&pages, header_area, sizeof(int)); //헤더 레코드 정보 불러오기
	memcpy(&records, header_area+4, sizeof(int));
	memcpy(&pnum, header_area+8, sizeof(int));
	memcpy(&rnum, header_area+12, sizeof(int));

	if((pages == 0) && (records == 0)) { //파일이 빈 경우
		pages++; records++;
		char inputpage[PAGE_SIZE];
		int newoffset = 0;
		int newlength = strlen(recordbuf);
		memset(inputpage, 0, sizeof(inputpage));
		int one = 1;
		memcpy(inputpage, &one, sizeof(one));
		memcpy(inputpage+4, &newoffset, sizeof(int));
		memcpy(inputpage+8, &newlength, sizeof(int));
		memcpy(inputpage + HEADER_AREA_SIZE, recordbuf, strlen(recordbuf));
		writePage(fp, inputpage, pages - 1);
		memcpy(header_area, &pages, sizeof(int));
		memcpy(header_area + 4, &records, sizeof(int));
		fseek(fp, 0, SEEK_SET);
		fwrite(header_area, sizeof(header_area), 1, fp);
	}
	else if((pnum == -1) &&(rnum == -1)) { //append 하는 경우
		readPage(fp, pagebuf, pages-1);
		int pagerecords;
		memcpy(&pagerecords, pagebuf, sizeof(int));
		int offset, length;
		memcpy(&offset, pagebuf+8 * (pagerecords-1) + 4, sizeof(int));
		memcpy(&length, pagebuf+8 * (pagerecords-1) + 8, sizeof(int));
		if(((HEADER_AREA_SIZE - 4) / 8) == pagerecords) { //page 새로 할당하는 경우
			char inputpage[PAGE_SIZE];
			int newoffset = 0, newlength = strlen(recordbuf);
			memset(inputpage, 0, sizeof(inputpage));
			int one = 1;
			memcpy(inputpage, &one, sizeof(one));
			memcpy(inputpage+4, &newoffset, sizeof(int));
			memcpy(inputpage+8, &newlength, sizeof(int));
			memcpy(inputpage + HEADER_AREA_SIZE, recordbuf, strlen(recordbuf));
			pages++;
			writePage(fp, inputpage, pages-1);
			records++;
			memcpy(header_area, &pages, sizeof(int));
			memcpy(header_area +4, &records, sizeof(int));
			fseek(fp, 0, SEEK_SET);
			fwrite(header_area, sizeof(header_area), 1, fp);
		}
		else { //page에 추가
			int newoffset, newlength;
			newoffset = offset+length;
			newlength = strlen(recordbuf);
			memcpy(pagebuf + 4 + (pagerecords * 8), &newoffset, sizeof(newoffset));
			memcpy(pagebuf + 8 + (pagerecords * 8), &newlength, sizeof(newlength));
			pagerecords++;
			memcpy(pagebuf, &pagerecords, sizeof(records));
			memcpy(pagebuf + HEADER_AREA_SIZE + (offset+length), recordbuf, newlength);
			writePage(fp, pagebuf, pages-1);
			records++;
			memcpy(header_area + 4, &records, sizeof(int));
			fseek(fp, 0, SEEK_SET);
			fwrite(header_area, sizeof(header_area), 1, fp);
		}
	}
	else { //삭제레코드 확인하는 경우
		int prevpage = -2, prevrecord = -2;
		char inputpage[PAGE_SIZE];
		char pagebuf[PAGE_SIZE];
		char prevbuf[PAGE_SIZE];
		char header_area[16];
		memset(header_area, 0, sizeof(header_area));
		memset(inputpage, 0, sizeof(inputpage));
		memset(pagebuf, 0, sizeof(inputpage));
		memset(prevbuf, 0, sizeof(prevpage));
		int pagerecord, del_length, del_offset;
		while((pnum != -1) && (rnum != -1)) { //삭제된 레코드들 순환하며 확인
			readPage(fp, pagebuf, pnum); //해당 페이지 읽음
			memcpy(&del_offset, pagebuf +4 + (8 * rnum), sizeof(int)); //offset과 length 확인
			memcpy(&del_length, pagebuf + 8 + (8 * rnum), sizeof(int));
			if(del_length < strlen(recordbuf)) { //길이가 모자라면
				prevpage = pnum;
				prevrecord = rnum;
				memcpy(&pnum, pagebuf + HEADER_AREA_SIZE + del_offset + 1, sizeof(int));//다음 위치로 이동
				memcpy(&rnum, pagebuf + HEADER_AREA_SIZE + del_offset + 5, sizeof(int));
				continue;
			}
			else {//길이가 맞으면
				if((prevpage == -2) && (prevrecord == -2)) { //이전 노드가 header record인 경우
					fseek(fp, 0, SEEK_SET);
					fread(header_area, sizeof(header_area), 1, fp);
					memcpy(header_area + 8, pagebuf + HEADER_AREA_SIZE +del_offset + 1, sizeof(int));//삭제된 위치 헤더에 저장
					memcpy(header_area + 12, pagebuf + HEADER_AREA_SIZE + del_offset + 5, sizeof(int));
					fseek(fp, 0, SEEK_SET);
					fwrite(header_area, sizeof(header_area), 1, fp);
				}
				else { //이전 노드가 header record가 아닌 경우
					int off, len;
					if(prevpage != pnum) { //이전 페이지와 지금 페이지가 다른 경우
						readPage(fp, prevbuf, prevpage);
						int pp = 0, pr = 0;
						memcpy(&off, prevbuf +4 + (8*prevrecord), sizeof(int));
						memcpy(&len, prevbuf +8 + (8*prevrecord), sizeof(int));
						memcpy(&pp, pagebuf + HEADER_AREA_SIZE + del_offset + 1, sizeof(int));
						memcpy(&pr, pagebuf + HEADER_AREA_SIZE + del_offset + 5, sizeof(int));
						memcpy(prevbuf + HEADER_AREA_SIZE + off + 1, &pp, sizeof(int));
						memcpy(prevbuf + HEADER_AREA_SIZE + off + 5, &pr, sizeof(int));
						writePage(fp, prevbuf, prevpage);
					}
					else {
						memcpy(&off, pagebuf + 4 + (8 * prevrecord), sizeof(int));
						memcpy(pagebuf + HEADER_AREA_SIZE + off + 1, pagebuf+HEADER_AREA_SIZE + del_offset + 1, sizeof(int));
						memcpy(pagebuf + HEADER_AREA_SIZE + off + 5, pagebuf + HEADER_AREA_SIZE + del_offset + 5, sizeof(int));
					}
				}
				memcpy(pagebuf + HEADER_AREA_SIZE + del_offset, recordbuf, strlen(recordbuf));
				writePage(fp, pagebuf, pnum);
				break;
			}
		}
		if((pnum == -1) && (rnum == -1)) {
			memset(pagebuf, 0, sizeof(pagebuf));
			readPage(fp, pagebuf, pages-1);
			int pagerecords;
			memcpy(&pagerecords, pagebuf, sizeof(int));
			int offset, length;
			memcpy(&offset, pagebuf + 8 * (pagerecords-1) + 4, sizeof(int));
			memcpy(&length, pagebuf + 8 * (pagerecords-1) + 8, sizeof(int));
			fseek(fp, 0, SEEK_SET);
			fread(header_area, sizeof(header_area), 1, fp);
			if(((HEADER_AREA_SIZE - 4) / 8) == pagerecords) { //page 새로 할당하는 경우
				char inputpage[PAGE_SIZE];
				int newoffset = 0, newlength = strlen(recordbuf);
				memset(inputpage, 0, sizeof(inputpage));
				int one = 1;
				memcpy(inputpage, &one, sizeof(one)); //쓰려는 page에 추가
				memcpy(inputpage+4, &newoffset, sizeof(int));
				memcpy(inputpage+8, &newlength, sizeof(int));
				memcpy(inputpage + HEADER_AREA_SIZE, recordbuf, strlen(recordbuf));
				writePage(fp , inputpage, pages);
				pages++;
				records++;
				memcpy(header_area, &pages, sizeof(int));
				memcpy(header_area +4, &records, sizeof(int));
				fseek(fp, 0, SEEK_SET);
				fwrite(header_area, sizeof(header_area), 1, fp);
			}
			else { //page에 추가
				int newoffset, newlength;
				newoffset = offset+length;
				newlength = strlen(recordbuf);
				memcpy(pagebuf + 4 + (pagerecords * 8), &newoffset, sizeof(newoffset));
				memcpy(pagebuf + 8 + (pagerecords * 8), &newlength, sizeof(newlength));
				pagerecords++;
				memcpy(pagebuf, &pagerecords, sizeof(pagerecords));
				memcpy(pagebuf + HEADER_AREA_SIZE + (offset+length), recordbuf, newlength);
				writePage(fp, pagebuf, pages-1);
				records++;
				memcpy(header_area+4, &records, sizeof(int));
				fseek(fp, 0, SEEK_SET);
				fwrite(header_area, sizeof(header_area), 1, fp);
			}
		}
	}
}

//
// 주민번호와 일치하는 레코드를 찾아서 삭제하는 기능을 수행한다.
//
void delete(FILE *fp, const char *id)
{
	char header_area[16];
	char pagebuf[PAGE_SIZE];
	char recordbuf[MAX_RECORD_SIZE];
	int pages, pagerecords, pnum, rnum;
	int now = 0;
	char delbuf[9];
	char pid[14];
	char* ptr;
	memset(pid, 0, sizeof(pid));
	strcpy(delbuf, "*");
	memset(header_area, 0, sizeof(header_area));
	fseek(fp, 0, SEEK_SET);
	fread(header_area, sizeof(header_area), 1, fp);
	memcpy(&pages, header_area, sizeof(int));
	memcpy(&pnum, header_area + 8, sizeof(int));
	memcpy(&rnum, header_area + 12, sizeof(int));
	for(int i = 0; i < pages; i++) {
		readPage(fp, pagebuf, i);
		memcpy(&pagerecords, pagebuf, sizeof(int));
		int offset, length;
		for(int j = 0; j < pagerecords; j++) {
			memcpy(&offset, pagebuf + (j*8) + 4, sizeof(int));
			memcpy(&length, pagebuf + (j*8) + 8, sizeof(int));
			memset(recordbuf, 0, sizeof(recordbuf));
			memcpy(recordbuf, pagebuf + offset + HEADER_AREA_SIZE, length);
			if(recordbuf[0] == '*') continue;
			ptr = strtok(recordbuf, "#");
			strcpy(pid, ptr);
			if(strcmp(pid, id) == 0) {
				memcpy(delbuf + 1, &pnum, sizeof(int));
				memcpy(delbuf + 5, &rnum, sizeof(int));
				memcpy(pagebuf + offset + HEADER_AREA_SIZE, delbuf, sizeof(delbuf));
				writePage(fp, pagebuf, i);
				memcpy(header_area+8, &i, sizeof(int));
				memcpy(header_area+12, &j, sizeof(int));
				fseek(fp, 0, SEEK_SET);
				fwrite(header_area, sizeof(header_area), 1, fp);
				return;
			}
		}
	}
	return;
}

int main(int argc, char *argv[])
{
	FILE *fp;  // 레코드 파일의 파일 포인터
	Person p;
	char header_area[16];
	memset(header_area, 0, sizeof(header_area));
	if((argc != 9) && (argc != 4)) {
		fprintf(stderr, "%d, usage : %s a <record file name> <field values list> | %s d <record file name> <field value>\n", argc, argv[0], argv[0]);
		exit(1);
	}
	if(argv[1][0] == 'a') {
		if(access(argv[2], F_OK) < 0) {
			if((fp = fopen(argv[2], "w+")) < 0) {
				fprintf(stderr, "%s file open error\n", argv[2]);
				exit(1);
			}
			int zeronum = 0;
			int onenum = -1;
			memcpy(header_area, &zeronum, sizeof(int));
			memcpy(header_area+sizeof(int), &zeronum, sizeof(int));
			memcpy(header_area+(2*sizeof(int)), &onenum, sizeof(int));
			memcpy(header_area+(3*sizeof(int)), &onenum, sizeof(int));
			fwrite(header_area, sizeof(header_area), 1, fp);
		}
		else {
			if((fp = fopen(argv[2], "r+")) < 0) {
				fprintf(stderr, "%s file open error\n", argv[2]);
				exit(1);
			}
		}
		strcpy(p.id, argv[3]);
		strcpy(p.name, argv[4]);
		strcpy(p.age, argv[5]);
		strcpy(p.addr, argv[6]);
		strcpy(p.phone, argv[7]);
		strcpy(p.email, argv[8]);
		add(fp, &p);
		fclose(fp);
	}
	else if(argv[1][0] == 'd') {
		if(access(argv[2] , F_OK) < 0) {
			fprintf(stderr, "%s is not exist. You can't find record.\n", argv[2]);
			exit(1);
		}
		else {
			if((fp = fopen(argv[2], "r+")) < 0) {
				fprintf(stderr, "%s file open error\n", argv[2]);
				exit(1);
			}
			delete(fp, argv[3]);
			fclose(fp);
		}
	}

	return 1;
}
