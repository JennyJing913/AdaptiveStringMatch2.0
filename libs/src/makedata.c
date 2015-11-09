#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <string.h>
#include <assert.h>
#include "common.h"
#include "makedata.h"

static int Seed;
#define ACMa 16807
#define ACMm 2147483647
#define ACMq 127773         
#define ACMr 2836
#define hi (Seed / ACMq)
#define lo (Seed % ACMq)

void cre_rand_text(const char *filename, long text_len, int low, int high)
{
     FILE *fp_text = Fopen(filename, "w");

     if (low < 0 || low > high || high > 255) {
         fprintf(stderr, "Illegal characters set!\n");
         exit(EXIT_FAILURE);
     }
     
     srand(time(NULL));
     
     while (text_len--)
         putc(rand_range(low, high), fp_text);
	
     Fclose(fp_text);
}

void cre_rand_pats(const char *filename, long pat_num, int min_pat_len, int max_pat_len, int low, int high)
{
    FILE *fp_text = Fopen(filename, "w");
    int pat_len;
    int ch;

    srand(time(NULL));

    while (pat_num--) {
        pat_len = rand_range(min_pat_len, max_pat_len); 
        while (pat_len--) {
            do {
                ch = rand_range(low, high);
               } while (ch == '\0' || ch == '\n');
               putc(ch, fp_text);
          }
          putc('\n', fp_text);
     }

     Fclose(fp_text);
}

unsigned char buf[1024*1024]; /* 1MB buffer */
  
static int fst = 1;

/* ��1~alphabet�е��ַ����buf��ǰn���ַ�*/
/* static void fill_buffer(unsigned char *buf, int n, int alphabet) */
/* { */
/*     int i; */
/*     long test; */
/*     struct timeval t; */
     
/*     if (fst) { */
/*         gettimeofday(&t, NULL); */
/*         Seed = t.tv_sec * t.tv_usec; */
/*         fst = 0; */
/*     } */
 
/*    for (i = 0; i < n; i++) { */
/*         Seed = ((test = ACMa * lo - ACMr * hi) > 0) ? test : test + ACMm; */
/*         buf[i] = 1 + ((double)Seed) * alphabet / ACMm; */
/*     } */
/* } */

/* ���� file_name, ��СΪ n MB, �ַ���Ϊalphabet  */
/* void fill_file(char const *file_name, int n, int alphabet) */
/* { */
/*     FILE *ofile; */

/*     if ((n < 0) || (n > 4096)) { */
/*         fprintf (stderr,"Error: file length (which is given in MB) must" */
/*                  " be between 0 and 4096 (= 4 GB file)\n"); */
/*         exit(EXIT_FAILURE); */
/*     } */

/*     if ((alphabet < 1) || (alphabet > 256)) { */
/*         fprintf (stderr,"Error: alphabet size must be between 1 and 256\n"); */
/*         exit(EXIT_FAILURE); */
/*     } */

/*     if (alphabet == 256) { */
/*         fprintf (stderr,"Warning: several indexes need alphabet size < 256" */
/*                  " to work properly.\n Generating anyway.\n"); */
/*     } */

/*     if ((ofile = fopen(file_name,"w")) == NULL) { */
/*         perror(file_name); */
/*         exit(EXIT_FAILURE); */
/*     } */

/*     while (n--) { */
/*         fill_buffer(buf, 1024*1024, alphabet); /\* ������� *\/ */
/*         if (fwrite (buf,1024*1024,1,ofile) != 1) { /\* �ٽ����д���ļ� *\/ */
/*             fprintf (stderr,"Error: cannot write %s\n",file_name); */
/*             fprintf(stderr, "%s\n", strerror(errno)); */
/*             fclose(ofile); */
/*             unlink (file_name); */
/*             exit(EXIT_FAILURE); */
/*         } */
/*     } */

/*     fclose(ofile); */

/*     fprintf (stderr,"File %s successfully generated\n",file_name); */
/* } */

int aleat (int top)
{
    long test;
    struct timeval t;
     
    if (fst) {
        gettimeofday(&t, NULL);
        Seed = t.tv_sec * t.tv_usec;
        fst = 0;
    }
     
    Seed = ((test = ACMa * lo - ACMr * hi) > 0) ? test : test + ACMm;
    return ((double) Seed) * top / ACMm;
}

/* ���ַ���forbid�е��ַ�����,ת��Ϊ������ת���ַ����д���*forbide�� */
static void parse_forbid(char const *forbid, char ** forbide)
{
     int len, i, j;

     len = strlen(forbid);

     *forbide = MALLOC(len + 1, char);
     
     for(i = 0, j = 0; i < len; i++) { /* ����forbid�ַ��� */
         if (forbid[i] != '\\') {
	       if(forbid[i] != '\n')
		    (*forbide)[j++] = forbid[i];
	  } else { 
	       i++;
	       if(i == len) {
		    /* forbid[i-1] = '\0'; */
		    (*forbide)[j] = '\0';
		    fprintf (stderr, "Not correct forbidden string: only one \\\n");
		    return;
	       }
	       switch (forbid[i]) {
		   case'n':  (*forbide)[j++] = '\n'; break;
		   case'\\': (*forbide)[j++] = '\\'; break;
		   case'b':  (*forbide)[j++] = '\b'; break;				
		   case'e':  (*forbide)[j++] = '\e'; break;
		   case'f':  (*forbide)[j++] = '\f'; break;
		   case'r':  (*forbide)[j++] = '\r'; break;
		   case't':  (*forbide)[j++] = '\t'; break;
		   case'v':  (*forbide)[j++] = '\v'; break;
		   case'a':  (*forbide)[j++] = '\a'; break;
		   case'c':     /* \c�ַ�����(0~255) */
			if(i + 3 >= len) {
			     /* forbid[i-1] = '\0'; */
			     (*forbide)[j] = '\0';
			     fprintf (stderr, "Not correct forbidden string: 3 digits after \\c\n");
			     return;
			}
			(*forbide)[j++] = (forbid[i+1]-48)*100 + (forbid[i+2]-48)*10 + (forbid[i+3]-48); 
			i += 3;
			break;					
		   default:
			fprintf (stdout, "Unknown escape sequence '\\%c'in forbidden string\n", forbid[i]);
			break;
	       }
	  }
     }
     (*forbide)[j] = '\0';
}

/* ���ļ�file��, ��ȡpat_num��ģʽ��, ����ģʽ���ļ�pat_file, �����ֲ�:min_pat_len~max_pat_len, ��ֹ���ַ��ڱ�����forbid�� */
void extract_pats(char const *text_filename, long pat_num, int min_pat_len, int max_pat_len, char const *pat_filename, char const *forbid)
{
    int pat_len, n, file_len;
    struct stat sdata;
    FILE *ifile, *ofile;
    unsigned char *buff;
    char *forbide = NULL;
    
    if (stat(text_filename, &sdata) != 0) {
        fprintf(stderr, "Error: cannot stat file %s\n", text_filename);
        exit(EXIT_FAILURE);
    }
    
    file_len = sdata.st_size; 	/* length of file */

    if (min_pat_len <= 0 || max_pat_len <= 0 || min_pat_len > max_pat_len || max_pat_len > file_len) {
        fprintf(stderr,"Error: pat length must be >= 1 and <= file length");
        exit(EXIT_FAILURE);
    }

    if (pat_num < 1) {
        fprintf(stderr, "Error: number of patterns must be >= 1\n");
        exit(EXIT_FAILURE);
    }
     
    parse_forbid(forbid, &forbide);

    ifile = Fopen (text_filename, "r");

    buff = MALLOC(file_len, unsigned char);
    
    if (fread (buff, file_len, 1, ifile) != 1)  { /* read the whole file into buf */
        fprintf(stderr, "Can not fread file: %s\n", text_filename);
        exit(EXIT_FAILURE);
    }
    
    Fclose(ifile);

    ofile = Fopen(pat_filename, "w");

    for (n = 0; n < pat_num; n++) {
        int starting_pos, i;
        pat_len = rand_range(min_pat_len, max_pat_len);
        do { /*��ÿһ����������ʼλ��,����λ�õ�pat�Ƿ������ֹ�ַ�,�������,�����²���һ����ʼλ��,ֱ����λ�õ�pat�������κν�ֹ�ַ� */
            starting_pos = aleat(file_len - pat_len + 1);
            for (i = 0; i < pat_len; i++)
                if (strchr(forbide, buff[starting_pos+i])) break;
        } while (i < pat_len);
	
        for (i = 0; i < pat_len; i++)
            putc(buff[starting_pos+i], ofile);
        
        putc('\n', ofile);      /* ģʽ������Ϊ��λ */
    }

    Fclose(ofile);
	  
    printf("File %s successfully generated\n", pat_filename);

    free(buff); free(forbide);
}

//static double cal_sd(patset_t *pat_set);
static void ins_pat(patnode_t *, patset_t *);
static void read_pats(FILE *, patset_t *);

/* ���������ݽṹ */
patset_t *cre_pat_set(const char *pats_file_name)
{
    FILE *fp_pats; /*ģʽ���ļ�*/
    patset_t *pat_set;
     
    fp_pats = Fopen(pats_file_name, "r");

    pat_set = MALLOC(1, patset_t);
    pat_set->pats_file = strdup(pats_file_name);
    pat_set->pat_list = NULL;
    pat_set->total_pats = 0;
    pat_set->min_pat_len = MAX_PAT_LEN;
    pat_set->max_pat_len = 0;
    pat_set->mean_pat_len = 0;
    pat_set->total_pat_len = 0;
    memset(pat_set->pat_len_distri, 0, sizeof(int) * MAX_PAT_LEN);

    read_pats(fp_pats, pat_set);

    Fclose(fp_pats);

    return pat_set;
}

static double cal_sd(patset_t *); /*�����׼��*/
/* ���ζ�ȡģʽ�� */
static void read_pats(FILE *fp_pats, patset_t *pat_set)
{
    char buf[MAX_PAT_LEN+1]; /*ģʽ�����棬���1000���ַ����������з�*/
    int pat_len;
    long pats_readed = 0; /*�Ѿ������ģʽ����*/
    char *line_break; /*���з�ָ��*/

    while (fgets(buf, sizeof(buf), fp_pats)) {
        if (line_break = strchr(buf, '\n'))
            *line_break = '\0';
        if (pat_len = strlen(buf)) {
            ins_pat(cre_pat_node(buf), pat_set); /*����ģʽ�ڵ㣬�����뵽ģʽ����*/
            pats_readed++;
            pat_set->total_pats++;
            pat_set->total_pat_len += pat_len;
            pat_set->pat_len_distri[pat_len-1]++;
        }
    }

    pat_set->mean_pat_len = (double) pat_set->total_pat_len / pat_set->total_pats; /*����ģʽ��ƽ������*/
    pat_set->pat_len_sd = cal_sd(pat_set);
}

patnode_t *cre_pat_node(const char *pat_str)
{
    patnode_t *new_pat_node =  MALLOC(1, patnode_t);
    new_pat_node->pat_str = strdup(pat_str);
    new_pat_node->next = NULL;

    return new_pat_node;
}

static void ins_pat(patnode_t *pat_node, patset_t *pat_set)
{
    static patnode_t *tail = NULL; /*��̬��������ģʽ����βָ��*/
    int len;

    if (pat_set->total_pats == 0) { /*ģʽ��Ϊ��*/
        pat_set->pat_list = pat_node;/*���뵽ͷ*/
        tail = pat_node; 
    } else { /*����ģʽ������β��*/
        tail->next = pat_node;
        tail = pat_node;
    }

    if ((len = strlen(pat_node->pat_str)) < pat_set->min_pat_len)
        pat_set->min_pat_len = len;
    if (len > pat_set->max_pat_len)
        pat_set->max_pat_len = len;
}

void des_pat_set(patset_t *pat_set) /*����ģʽ����*/
{
    patnode_t *p, *next;

    /*�������ٴ������ÿ���ڵ�*/
    for (p = pat_set->pat_list; p; p = next) { 
        next = p->next;
        free(p->pat_str); /*����ģʽ�ַ���*/
        free(p); /*����ģʽ�ڵ�*/
    }

    free(pat_set->pats_file);
    free(pat_set); /*����ģʽ������*/
}

static double cal_sd(patset_t *pat_set) /*�����׼��*/
{
     double sd = 0;
     patnode_t *p;

     for (p = pat_set->pat_list; p; p = p->next)
         sd += pow(strlen(p->pat_str) - pat_set->mean_pat_len, 2);

     return sqrt(sd / pat_set->total_pats);
}

/*print the pat set info*/
#ifdef DEBUG

void print_pat_set(const patset_t *pat_set, int max) 
{
    patnode_t *p;
    int num;
    int i;

    printf("pats file: %s\n total pats: %ld\n max len: %d min len: %d mean len: %.2f sd: %.2f total len: %ld \n",
            pat_set->pats_file, 
            pat_set->total_pats,
            pat_set->max_pat_len, pat_set->min_pat_len, pat_set->mean_pat_len, pat_set->pat_len_sd, pat_set->total_pat_len
            );

    for (i = pat_set->min_pat_len; i <= pat_set->max_pat_len; i++)
        printf("len: %2d  num: %5d  proportion: %%%7f\n", i, pat_set->pat_len_distri[i], ((double) pat_set->pat_len_distri[i] / pat_set->total_pats) * 100);
    
    for (num = 1, p = pat_set->pat_list; p && num <= max; p = p->next, num++)
        printf("%4d: %s\n", num, p->pat_str);
}

#endif
