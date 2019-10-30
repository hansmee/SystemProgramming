#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

/* Index 구조체 */
typedef struct index{
	int chap;
	int clause;
	int location;
}Index;

/* Index 구조체 배열을 재할당 시켜주면서 배열을 늘림 */
Index *reallocIndex(Index *now, int size, int ch, int cl, int lo){
	now = (Index *)realloc(now, sizeof(Index)*size);
	now[size-1].chap = ch;
	now[size-1].clause = cl;
	now[size-1].location = lo;
	return now;
}

/* 트라이(trie) 노드 구조체 */
typedef struct node{
	struct node *child[28];	// 'a'-'z', ''', '-'
	int isEnd;				// 단어의 끝인지
	int size;				// 단어 길이
	int num;				// 단어 개수
	Index *idx;				// Index 배열
}Node;

int count = 0;
/* 노드 할당 */
Node *newNode(){
	Node *new = (Node *)malloc(sizeof(Node));
	new->isEnd = 0; 
	new->num = 0;
	new->size = 0;
	new->idx = NULL;
	for(int i = 0; i < 28; i++)
		new->child[i] = 0;
	return new;
}

/* root를 받아와서 문자에 맞는 child에 갈 때 */
/* child가 없으면 생성 후 이동, 아니면 그냥 이동 */
void insert(Node *root, char *str, int size, Index *id){
	Node *now = root;
	for(int i = 0; i < size; i++){
		if(str[i] == '\''){
			if(!now->child[26])
				now->child[26] = newNode();
			now = now->child[26];
		}else if(str[i] == '-'){
			if(!now->child[27])
				now->child[27] = newNode();
			now = now->child[27];
		}else{
			if(!now->child[str[i]-'a'])
				now->child[str[i]-'a'] = newNode();
			now = now->child[str[i]-'a'];
		}
	}
	now->isEnd = 1;
	now->size = size;
	
	now->idx = reallocIndex(now->idx, now->num+1, id->chap, id->clause, id->location);
	now->num++;
}

/* 트라이 순회하면서 인덱스의 수를 count */
void printtree(Node *now, char *str, int d){
	if(now->isEnd) 
		count++;
	for(int i = 0; i < 28; i++){
		if(now->child[i]){
			if(i == 26){
				str[d] = '\'';
				str[d+1] = 0;
				printtree(now->child[i], str, d+1);
			}else if(i == 27){
				str[d] = '-';
				str[d+1] = 0;
				printtree(now->child[i], str, d+1);
			}
			else{
				str[d] = i+'a';
				str[d+1] = 0;
				printtree(now->child[i], str, d+1);
			}
		}
	}
}

/* int를 string으로 변환 후 문자열의 길이 리턴 */
int InttoString(int x, char* str){
	int i = 0, m = 1, k;
	while(m <= x){
		m *= 10;
		i++;
	}
	k = i;
	str[i--] = 0;
	while(x > 0){
		str[i--] = x % 10 + '0';
		x /= 10;
	}
	return k;
}

/* str + ' ' */
int AddSpc(char* str, int size){
	str[size] = ' ';
	return size+1;
}

/* str + ',' + ' ' */
int AddComSpc(char* str, int size){
	str[size] = ',';
	str[size+1] = ' ';
	return size+2;
}

/* str + ':' */
int AddCol(char* str, int size){
	str[size] = ':';
	return size+1;
}

/* str + '\n' */
int AddEnt(char* str, int size){
	str[size] = '\n';
	return size+1;
}

/* 트라이 순회하면서 단어, 단어 수, Index 출력 */
/* 재귀함수 돌면서 빈 str을 한 글자씩 뒤에 붙여나가면서 str을 한 단어로 완성 */
void PrintTree(Node *now, char *str, int d, int fd){
	if(now->isEnd){
		int len = 0;
		char *tmp = (char*)malloc(15);
		len = AddCol(str, now->size);
		len = AddSpc(str, len);
		write(fd, str, len);

		len = InttoString(now->num, tmp);
		len = AddComSpc(tmp, len);
		write(fd, tmp, len);
		tmp = (char*)calloc(15, sizeof(char));

		for(int i = 0; i < now->num-1; i++){
			len = InttoString(now->idx[i].chap, tmp);
			len = AddCol(tmp, len);
			write(fd, tmp, len);
			tmp = (char*)calloc(15, sizeof(char));
			
			len = InttoString(now->idx[i].clause, tmp);
			len = AddCol(tmp, len);
			write(fd, tmp, len);
			tmp = (char*)calloc(15, sizeof(char));
			
			len = InttoString(now->idx[i].location, tmp);
			len = AddComSpc(tmp, len);
			write(fd, tmp, len);
			tmp = (char*)calloc(15, sizeof(char));
		}
		
		len = InttoString(now->idx[now->num-1].chap, tmp);
		len = AddCol(tmp, len);
		write(fd, tmp, len);
		tmp = (char*)calloc(15, sizeof(char));

		len = InttoString(now->idx[now->num-1].clause, tmp);
		len = AddCol(tmp, len);
		write(fd, tmp, len);
		tmp = (char*)calloc(15, sizeof(char));
			
		len = InttoString(now->idx[now->num-1].location, tmp);
		len = AddEnt(tmp, len);
		write(fd, tmp, len);
		tmp = (char*)calloc(15, sizeof(char));

	}
	for(int i = 0; i < 28; i++){
		if(now->child[i]){
			if(i == 26){
				str[d] = '\'';
				str[d+1] = 0;
				PrintTree(now->child[i], str, d+1, fd);
			}else if(i == 27){
				str[d] = '-';
				str[d+1] = 0;
				PrintTree(now->child[i], str, d+1, fd);
			}
			else{
				str[d] = i+'a';
				str[d+1] = 0;
				PrintTree(now->child[i], str, d+1, fd);
			}
		}
	}
}

Node *root;	// root 선언

/* 장 수: c_chap / 절 수: cla_count / 인덱스 수: count / 단어 수: word_count */
/* 각각의 int형을 문자열로 변환 */
int Change(int chap, int clause, int index, int word, char* str){
	int len = 0, cur = 0;
	char tmp[10];

	len = InttoString(chap, tmp);
	for(int i = 0; i < len; i++)
		str[i] = tmp[i];
	str[len] = ' ';
	cur = len + 1;


	len = InttoString(clause, tmp);
	for(int i = 0; i < len; i++)
		str[cur+i] = tmp[i];
	cur = cur +  len +1;
	str[cur-1] = ' ';


	len = InttoString(index, tmp);
	for(int i = 0; i < len; i++)
		str[cur+i] = tmp[i];
	cur = cur +len+1;
	str[cur-1] = ' ';


	len = InttoString(word, tmp);
	for(int i = 0; i < len; i++)
		str[cur+i] = tmp[i];
	cur = cur + len+1;
	str[cur-1] = '\n';


	return cur;
}

void indexBuilder(const char* inputFileNm, const char* indexFileNm)
{
	/* Write your own C code */
	int rd = open(inputFileNm, O_RDONLY);
	int wd = open(indexFileNm, O_WRONLY | O_CREAT);

	
	int c_chap, c_clause, c_loc; // current location
	int l, p_clause; // p_clause: 바로 전 절
	char word[100]; // current word
	int word_size = 0; 
	int isIn = 0, wasEn = 0, isCutChap = 0, isCutCla = 0, wasSpc = 0, isFirst = 0, EndisEnd = 0, EndisChap = 0, EndisCla = 0;
	int cla_count = 0, word_count = 0;

	char *buf;
	buf = (char*)malloc(256);
	
	char c;
	int d1 = 0, d2 = 0; // d1: 장, d2: 절
	char *tmp = (char*)malloc(100);

	root = newNode();
	Index *id = (Index *)malloc(sizeof(Index));

	if(rd > 0){
		while(read(rd, buf, 256)){
			
			/* EndisChap: 버퍼의 끝이 장/절일 때 */
			if(EndisChap && buf[0] >= '0' && buf[0] <= '9'){
				isCutChap = 1;
				wasEn = 1;
			}
			if(EndisCla && buf[0] >= '0' && buf[0] <= '9'){
				isCutCla = 1;
			}
			/* isCutChap: 장/절이 잘렸을 때 */
			if(!isCutChap)
				d1 = 0;
			if(!isCutCla)
				d2 = 0;

			EndisChap = 0, EndisCla = 0;
			for(int i = 0; i < 256; i++){
				c = buf[i];

				if(c == '\n') 
					wasEn = 1;

				if(isIn){
					if(wasEn){
						// 지금 몇 장에 있는지, 
						if(c <= '9' && c >= '0'){
							c_chap = (int)c - 48;
							if(isCutChap) {
								c_chap += 10*d1;
								isCutChap = 0;
								cla_count--;
							}
							while(1){
								/* 장이 10 이상일 경우 */
								if(i >= 255) {
									d1 = c_chap;
									EndisChap = 1;
									cla_count++;
									break;
								}
								i++;
								if(buf[i] == ':') {
									cla_count++;
									break;
								}
								c_chap = 10*c_chap + (int)buf[i] - 48;
							}
							if(i < 256) 
								wasEn = 0;
						}
						p_clause = c_clause;
					}else{
						// 지금 몇 절에 있는지,
						if(c <= '9' && c >= '0'){
							c_clause = (int)c - 48;
							if(isCutCla) {
								c_clause += 10*d2;
								isCutCla = 0;
							}
							while(1){
								/* 절이 10 이상일 경우 */
								if(i >= 255) {
									d2 = c_clause;
									EndisCla = 1;
									break;
								}
								i++;
								if(buf[i] == ':') break;
								c_clause = 10*c_clause + (int)buf[i] - 48;
							}
							isFirst = 1;
						}else{
							// 단어 확인, - 문자일 경우 word에 현재 단어 저장
							if((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
									c == '\'' || c == '-'){
								if(c >= 'A' && c <= 'Z') c += 32; // 소문자로 바꾸기
								l = c_loc;
								if(wasSpc){
									word_count++; // 단어 수 세기
									wasSpc = 0;
								}
								word[word_size] = c; // 현재 단어 저장
								word_size++;
							}else{ // 띄어쓰기 확인, - 단어를 count해주면서 현재 저장된 word와 현재 Index를 root에 추가
								if(c == ' '){
									wasSpc = 1;
									if(isFirst){
										if(c_clause > 1){
											id->chap = c_chap;
											id->clause = c_clause - 1;
										}else {
											id->chap = c_chap - 1;
											id->clause = p_clause;
										}
										id->location = l - word_size;
									}else{		
										id->chap = c_chap;
										id->clause = c_clause;
										id->location = l - word_size;
									}
									if(word_size > 0)
							 			insert(root, word, word_size, id); // 현재 단어 넣어주기
									word_size = 0; // 현재 단어 초기화
									if(isFirst){
										c_loc = 0;
										l = 0;
										isFirst = 0;
									}
								}
								if(i == 255) EndisEnd = 1;
							}
						}
						c_loc++;
					}
				}else{ // 1장 1절 시작되기 전 건너 뜀
					if(c <= '9' && c >= '0')
						isIn = 1, i--;
				}
				isCutChap = 0, isCutCla = 0;
			}
			isCutChap = 0, isCutCla = 0;
			buf = (char*)calloc(256, sizeof(char)); // 버퍼 초기화
		}
	}

	/* 마지막 단어 */
	id->chap = c_chap;
	id->clause = c_clause;
	id->location = l - word_size;
	insert(root, word, word_size, id);

	printtree(root, tmp, 0);

	int len = 0;
	for(len = 0; inputFileNm[len]!='.'; len++)
		tmp[len] = inputFileNm[len];
	tmp[len] = ' ';

	write(wd, tmp, len+1);
	tmp = (char*)calloc(100, sizeof(char));


	len = Change(c_chap, cla_count, count, word_count, tmp);
	write(wd, tmp, len);

	tmp = (char*)calloc(100, sizeof(char));
	PrintTree(root, tmp, 0, wd);
	
	close(wd);
}


