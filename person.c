#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "person.h"
//�ʿ��� ��� ��� ���ϰ� �Լ��� �߰��� �� ����

// ���� ������� �����ϴ� ����� ���� �ٸ� �� ������ �ణ�� ������ �Ӵϴ�.
// ���ڵ� ������ ������ ������ ���� �����Ǳ� ������ ����� ���α׷����� ���ڵ� ���Ϸκ��� �����͸� �а� �� ����
// ������ ������ ����մϴ�. ���� �Ʒ��� �� �Լ��� �ʿ��մϴ�.
// 1. readPage(): �־��� ������ ��ȣ�� ������ �����͸� ���α׷� ������ �о�ͼ� pagebuf�� �����Ѵ�
// 2. writePage(): ���α׷� ���� pagebuf�� �����͸� �־��� ������ ��ȣ�� �����Ѵ�
// ���ڵ� ���Ͽ��� ������ ���ڵ带 �аų� ���ο� ���ڵ带 ���ų� ���� ���ڵ带 ������ ����
// ���� readPage() �Լ��� ȣ���Ͽ� pagebuf�� ������ ��, ���⿡ �ʿ信 ���� ���ο� ���ڵ带 �����ϰų�
// ���� ���ڵ� ������ ���� ��Ÿ�����͸� �����մϴ�. �׸��� �� �� writePage() �Լ��� ȣ���Ͽ� ������ pagebuf��
// ���ڵ� ���Ͽ� �����մϴ�. �ݵ�� ������ ������ �аų� ��� �մϴ�.
//
// ����: ������ �������κ��� ���ڵ�(���� ���ڵ� ����)�� �аų� �� �� ������ ������ I/O�� ó���ؾ� ������,
// ��� ���ڵ��� ��Ÿ�����͸� �����ϰų� �����ϴ� ��� ������ ������ ó������ �ʰ� ���� ���ڵ� ������ �����ؼ� ó���Ѵ�.

//
// ������ ��ȣ�� �ش��ϴ� �������� �־��� ������ ���ۿ� �о �����Ѵ�. ������ ���۴� �ݵ�� ������ ũ��� ��ġ�ؾ� �Ѵ�.
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
// ������ ������ �����͸� �־��� ������ ��ȣ�� �ش��ϴ� ���ڵ� ������ ��ġ�� �����Ѵ�. 
// ������ ���۴� �ݵ�� ������ ũ��� ��ġ�ؾ� �Ѵ�.
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
// ���ο� ���ڵ带 ������ �� �͹̳ηκ��� �Է¹��� ������ Person ����ü�� ���� �����ϰ�, pack() �Լ��� ����Ͽ�
// ���ڵ� ���Ͽ� ������ ���ڵ� ���¸� recordbuf�� �����. 
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
// �Ʒ��� unpack() �Լ��� recordbuf�� ����Ǿ� �ִ� ���ڵ带 ����ü�� ��ȯ�� �� ����Ѵ�.
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
// ���ο� ���ڵ带 �����ϴ� ����� �����ϸ�, �͹̳ηκ��� �Է¹��� �ʵ尪���� ����ü�� ������ �� �Ʒ� �Լ��� ȣ���Ѵ�.
//
void add(FILE *fp, const Person *p)
{
	char header_area[16];
	char recordbuf[MAX_RECORD_SIZE];
	char pagebuf[PAGE_SIZE];
	memset(recordbuf, 0, sizeof(recordbuf));
	pack(recordbuf, p);
	int pages, records, pnum, rnum;
	fseek(fp, 0, SEEK_SET);//���� ���ڵ� �ּ�
	if(fread(header_area, sizeof(header_area), 1, fp) != 1) {
		fprintf(stderr, "fread error\n");
		exit(1);
	}
	memcpy(&pages, header_area, sizeof(int)); //��� ���ڵ� ���� �ҷ�����
	memcpy(&records, header_area+4, sizeof(int));
	memcpy(&pnum, header_area+8, sizeof(int));
	memcpy(&rnum, header_area+12, sizeof(int));

	if((pages == 0) && (records == 0)) { //������ �� ���
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
	else if((pnum == -1) &&(rnum == -1)) { //append �ϴ� ���
		readPage(fp, pagebuf, pages-1);
		int pagerecords;
		memcpy(&pagerecords, pagebuf, sizeof(int));
		int offset, length;
		memcpy(&offset, pagebuf+8 * (pagerecords-1) + 4, sizeof(int));
		memcpy(&length, pagebuf+8 * (pagerecords-1) + 8, sizeof(int));
		if(((HEADER_AREA_SIZE - 4) / 8) == pagerecords) { //page ���� �Ҵ��ϴ� ���
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
		else { //page�� �߰�
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
	else { //�������ڵ� Ȯ���ϴ� ���
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
		while((pnum != -1) && (rnum != -1)) { //������ ���ڵ�� ��ȯ�ϸ� Ȯ��
			readPage(fp, pagebuf, pnum); //�ش� ������ ����
			memcpy(&del_offset, pagebuf +4 + (8 * rnum), sizeof(int)); //offset�� length Ȯ��
			memcpy(&del_length, pagebuf + 8 + (8 * rnum), sizeof(int));
			if(del_length < strlen(recordbuf)) { //���̰� ���ڶ��
				prevpage = pnum;
				prevrecord = rnum;
				memcpy(&pnum, pagebuf + HEADER_AREA_SIZE + del_offset + 1, sizeof(int));//���� ��ġ�� �̵�
				memcpy(&rnum, pagebuf + HEADER_AREA_SIZE + del_offset + 5, sizeof(int));
				continue;
			}
			else {//���̰� ������
				if((prevpage == -2) && (prevrecord == -2)) { //���� ��尡 header record�� ���
					fseek(fp, 0, SEEK_SET);
					fread(header_area, sizeof(header_area), 1, fp);
					memcpy(header_area + 8, pagebuf + HEADER_AREA_SIZE +del_offset + 1, sizeof(int));//������ ��ġ ����� ����
					memcpy(header_area + 12, pagebuf + HEADER_AREA_SIZE + del_offset + 5, sizeof(int));
					fseek(fp, 0, SEEK_SET);
					fwrite(header_area, sizeof(header_area), 1, fp);
				}
				else { //���� ��尡 header record�� �ƴ� ���
					int off, len;
					if(prevpage != pnum) { //���� �������� ���� �������� �ٸ� ���
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
			if(((HEADER_AREA_SIZE - 4) / 8) == pagerecords) { //page ���� �Ҵ��ϴ� ���
				char inputpage[PAGE_SIZE];
				int newoffset = 0, newlength = strlen(recordbuf);
				memset(inputpage, 0, sizeof(inputpage));
				int one = 1;
				memcpy(inputpage, &one, sizeof(one)); //������ page�� �߰�
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
			else { //page�� �߰�
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
// �ֹι�ȣ�� ��ġ�ϴ� ���ڵ带 ã�Ƽ� �����ϴ� ����� �����Ѵ�.
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
	FILE *fp;  // ���ڵ� ������ ���� ������
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
