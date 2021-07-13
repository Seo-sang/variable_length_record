#ifndef	_PERSON_H_
#define	_PERSON_H_

#define MAX_RECORD_SIZE	100 // including 6 delimeters
#define PAGE_SIZE		256 // 수정 가능함	
#define HEADER_AREA_SIZE	(PAGE_SIZE/10)
#define DATA_AREA_SIZE	(PAGE_SIZE - HEADER_AREA_SIZE)

typedef struct _Person
{
	char id[14];		//주민번호
	char name[18];		//이름
	char age[4];		//나이
	char addr[22];	    	//주소
	char phone[16];		//전화번호
	char email[26];		//이메일 주소
} Person;

#endif
