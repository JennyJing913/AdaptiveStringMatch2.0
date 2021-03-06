#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"
#include "share.h"
#include "binary.h"
#include "common.h"
#include "map.h"

extern Queue_t *queue;
extern unsigned type_num[];

#define BUILD_N_MAP

#define BUILD_MAP_4_OR_16(n)						\
									\
static void build_map_##n(Expand_Node_t *expand_node, Pat_Num_t dif_ch_num) \
{									\
  Suffix_Node_t *cur_suf, *next_suf, **next_p;				\
  Map_##n##_t *map_##n;							\
  Expand_Node_t *expand_nodes_##n;					\
  Char_t *keys;								\
  Char_t ch;								\
  int i;								\
									\
  map_##n = CALLOC(1, Map_##n##_t);					\
  map_##n->dif_ch_num = dif_ch_num;					\
  keys = map_##n->keys;							\
  expand_nodes_##n = map_##n->expand_nodes;				\
  /* 把第一个串的第一个字符拷到第一个key中 */				\
  cur_suf = expand_node->next_level;					\
  keys[0] = *cur_suf->str;						\
  next_p = (Suffix_Node_t **) &expand_nodes_##n[0].next_level;		\
									\
  for (i = 0, cur_suf = expand_node->next_level; cur_suf; cur_suf = next_suf) {	\
    next_suf = cur_suf->next;						\
    if ((ch = *cur_suf->str) != keys[i]) {				\
      keys[++i] = ch; *next_p = NULL; /* 当前链表截止 */		\
      next_p = (Suffix_Node_t **) &expand_nodes_##n[i].next_level;	\
    }									\
									\
    if (cur_suf = cut_head(cur_suf, 1)) {				\
      *next_p = cur_suf; next_p = &cur_suf->next;			\
    } else								\
      set_bit(map_##n->pat_end_flag, i); /* 是模式尾 */			\
  }									\
									\
  expand_node->next_level = map_##n;					\
  expand_node->type = MAP_##n;						\
  type_num[MAP_##n]++;							\
									\
  for (i = 0; i < dif_ch_num; i++) /* 加入到队列 */			\
    if (expand_nodes_##n[i].next_level)					\
      in_queue(queue, expand_nodes_##n + i);				\
}

BUILD_MAP_4_OR_16(4)

BUILD_MAP_4_OR_16(16)

static void build_map_48(Expand_Node_t *expand_node, Pat_Num_t dif_ch_num)
{
  Suffix_Node_t *cur_suf, *next_suf, **next_p;
  Map_48_t *map_48 = CALLOC(1, Map_48_t);
  Expand_Node_t *expand_nodes_48 = map_48->expand_nodes;
  char *index = map_48->index;
  UC_t ch;
  int i = 0;
  
  memset(index, -1, 256);

  cur_suf = expand_node->next_level; ch = *cur_suf->str;
  index[ch] = 0;
  next_p = (Suffix_Node_t **) &expand_nodes_48[0].next_level;
  
  for (cur_suf = expand_node->next_level; cur_suf; cur_suf = next_suf) {
    next_suf = cur_suf->next;
    ch = *cur_suf->str;

    if (index[ch] != i) {
      index[ch] = ++i;
      *next_p = NULL; next_p = (Suffix_Node_t **) &expand_nodes_48[i].next_level;
    }
    
    if (cur_suf = cut_head(cur_suf, 1)) {
      *next_p = cur_suf; next_p = &cur_suf->next;
    } else  /* 模式串尾 */
      set_bit(map_48->pat_end_flag, i);
  }

  expand_node->next_level = map_48;
  expand_node->type = MAP_48;
  type_num[MAP_48]++;
  
  for (i = 0; i < dif_ch_num; i++)
    if (expand_nodes_48[i].next_level)
      in_queue(queue, expand_nodes_48 + i);
}

static void build_map_256(Expand_Node_t *expand_node)
{
  Suffix_Node_t *cur_suf, *next_suf, **next_p;
  Map_256_t *map_256 = CALLOC(1, Map_256_t);
  Expand_Node_t *expand_nodes_256 = map_256->expand_nodes;
  UC_t ch, new_ch;
  int n;
  
  cur_suf = expand_node->next_level; ch = *cur_suf->str;
  next_p = (Suffix_Node_t **) &expand_nodes_256[ch].next_level; /* 指向第一个链表头 */

  for (cur_suf = expand_node->next_level; cur_suf; cur_suf = next_suf) {
    next_suf = cur_suf->next;
    new_ch = *cur_suf->str;

    if (new_ch != ch) { /* 终止上一个链表,跳转到新链表头 */
      *next_p = NULL; next_p = (Suffix_Node_t **) &expand_nodes_256[new_ch].next_level; 
      ch = new_ch;
    }

    if (cur_suf = cut_head(cur_suf, 1)) {
      *next_p = cur_suf; next_p = &cur_suf->next;
    } else
      set_bit(map_256->pat_end_flag, ch);
  }
  
  expand_node->next_level = map_256;
  expand_node->type = MAP_256;
  type_num[MAP_256]++;

  for (n = 256, expand_node = map_256->expand_nodes; n; expand_node++, n--)
    if (expand_node->next_level)
      in_queue(queue, expand_node);
}

void build_map(Expand_Node_t *expand_node, Pat_Num_t dif_ch_num)
{
  if (dif_ch_num <= 4)
    build_map_4(expand_node, dif_ch_num);
  else if (dif_ch_num <= 16)
    build_map_16(expand_node, dif_ch_num);
  else if (dif_ch_num <= 48)
    build_map_48(expand_node, dif_ch_num);
  else
    build_map_256(expand_node);
}

Expand_Node_t *match_map_4(Map_4_t *map_4, Char_t const **text, Bool_t *is_pat_end)
{
  Char_t ch = **text, *key;
  Expand_Node_t *expand_node;
  int n, i;
  
  for (n = map_4->dif_ch_num, key = map_4->keys;
       n && ch > *key; n--, key++) /* key是有序排列的 */
    ;
  
  if (n == 0 || ch < *key) /* 匹配失败 */
    return NULL;

  i = key - map_4->keys;
  expand_node = map_4->expand_nodes + i;
  *is_pat_end = test_bit(map_4->pat_end_flag, i);
  (*text)++;
  
  return expand_node;
}

Expand_Node_t *match_map_16(Map_16_t *map_16, Char_t const **text, Bool_t *is_pat_end)
{
  Char_t *keys = map_16->keys, ch = **text;
  Expand_Node_t *expand_node;
  char low = 0, high = map_16->dif_ch_num - 1, mid;
  
  while (low <= high) {
    mid = (low + high) >> 1;

    if (ch == keys[mid]) {
      expand_node = map_16->expand_nodes + mid;

      *is_pat_end = test_bit(map_16->pat_end_flag, mid);
      (*text)++;
      return expand_node;
    }  else if (ch < keys[mid])
      high = mid - 1;
    else
      low = mid + 1;
  }
  
  return NULL;
}

Expand_Node_t *match_map_48(Map_48_t *map_48, Char_t const **text, Bool_t *is_pat_end)
{
  UC_t ch = **text;
  char i;
  
  if ((i = map_48->index[ch]) == -1)
    return NULL;

  *is_pat_end = test_bit(map_48->pat_end_flag, i);
  (*text)++;
  
  return map_48->expand_nodes + i;
}

Expand_Node_t *match_map_256(Map_256_t *map_256, Char_t const **text, Bool_t *is_pat_end)
{
  UC_t ch = **text;
  
  if (*is_pat_end = test_bit(map_256->pat_end_flag, ch))
    (*text)++;
  
  return map_256->expand_nodes + ch;
}

void print_map_4(Map_4_t *map_4)
{
     int i;
     
     printf("\n4 map\n");
     for (i = 0; i < map_4->dif_ch_num; i++) {
	  printf("\n%c:\n ", map_4->keys[i]);
	  print_suffix(map_4->expand_nodes[i].next_level);
     }
}

void print_map_16(Map_16_t *map_16)
{
     int i;
     
     printf("\n16 map\n");
     for (i = 0; i < map_16->dif_ch_num; i++) {
	  printf("\n%c:\n ", map_16->keys[i]);
	  print_suffix(map_16->expand_nodes[i].next_level);
     }
}

void print_map_48(Map_48_t *map_48)
{
     int i;
     
     puts("\n48 map\n");
     for (i = 0; i < 256; i++)
	  if (map_48->index[i] != -1) {
	       printf("\n%c:\n ", i);
	       print_suffix(map_48->expand_nodes[map_48->index[i]].next_level);
	  }
}

void print_map_256(Map_256_t *map_256)
{
     int i;
     
     puts("\n256 map");
     for (i = 0; i < 256; i++)
       if (map_256->expand_nodes[i].next_level) {
	       printf("\n%c:\n ", i);
	       print_suffix(map_256->expand_nodes[i].next_level);
	  }
}


/* void build_map_4(Expand_Node_t *expand_node, Pat_Num_t dif_ch_num) */
/* { */
/*   Suffix_Node_t *cur_suf, *next_suf; */
/*   _4_Map_t *_4_map; */
/*   Char_t ch; */
/*   Char_t *keys; */
/*   int i, j; */

/*   _4_map = CALLOC(1, _4_Map_t); */
/*   memset(_4_map->keys, UCHAR_MAX, 4); */
/*   _4_map->dif_ch_num = dif_ch_num; */
/*   keys = _4_map->keys; */
  
/*   for (cur_suf = expand_node->next_level; cur_suf; cur_suf = next_suf) { */
/*     next_suf = cur_suf->next; */
/*     ch = *cur_suf->str; */
/*     for (i = 0; ch > keys[i]; i++) */
/*       ; */
/*     if (ch < keys[i]) { */
/*       for (j = dif_ch_num - 2; j >= i; j--) /\* 向后移动元素 *\/ */
/* 	keys[j+1] = keys[j]; */
/*       keys[i] = ch; /\* 插入元素 *\/ */
/*     } */
    
/*     if (cur_suf = cut_head(cur_suf, 1)) */
/*       insert_to_expand(_4_map->expand_nodes + i, cur_suf); */
/*     else */
/*       set_bit(_4_map->pat_end_flag, i); /\* 是模式尾 *\/ */
/*   } */
 
/*   expand_node->next_level = _4_map; */
/*   expand_node->type = _4_MAP; */
/*   type_num[_4_MAP]++; */
  
/*   for (i = 0; i < dif_ch_num; i++) /\* 加入到队列 *\/ */
/*     if ((expand_node = _4_map->expand_nodes + i)->next_level) */
/*       in_queue(queue, expand_node); */
/* } */

/* void build_16_map(Expand_Node_t *expand_node, Pat_Num_t dif_ch_num) */
/* { */
/*   Suffix_Node_t *cur_suf, *next_suf; */
/*   _16_Map_t *_16_map; */
/*   Char_t ch; */
/*   Char_t *keys; */
/*   int i, j; */

/*   _16_map = CALLOC(1, _16_Map_t); */
/*   memset(_16_map->keys, UCHAR_MAX, 16); */
/*   _16_map->dif_ch_num = dif_ch_num; */
/*   keys = _16_map->keys; */
  
/*   for (cur_suf = expand_node->next_level; cur_suf; cur_suf = next_suf) { */
/*     next_suf = cur_suf->next; */
/*     ch = *cur_suf->str; */
/*     for (i = 0; ch > keys[i]; i++) */
/*       ; */
/*     if (ch < keys[i]) { */
/*       for (j = dif_ch_num - 2; j >= i; j--) /\* 向后移动元素 *\/ */
/* 	keys[j+1] = keys[j]; */
/*       keys[i] = ch; /\* 插入元素 *\/ */
/*     } */
    
/*     if (cur_suf = cut_head(cur_suf, 1)) */
/*       insert_to_expand(_16_map->expand_nodes + i, cur_suf); */
/*     else */
/*       set_bit(_16_map->pat_end_flag, i); */
/*   } */
 
/*   expand_node->next_level = _16_map; */
/*   expand_node->type = _16_MAP; */
/*   type_num[_16_MAP]++; */
  
/*   for (i = 0; i < dif_ch_num; i++) /\* 加入到队列 *\/ */
/*     if ((expand_node = _16_map->expand_nodes + i)->next_level) */
/*       in_queue(queue, expand_node); */
/* } */

/* void build_48_map(Expand_Node_t *expand_node, Pat_Num_t dif_ch_num) */
/* { */
/*   _48_Map_t *_48_map = CALLOC(1, _48_Map_t); */
/*   Suffix_Node_t *cur_suf, *next_suf; */
/*   Char_t ch; */
/*   int i = 0, index; */
  
/*   memset(_48_map->index, -1, 256); */

/*   for (cur_suf = expand_node->next_level; cur_suf; cur_suf = next_suf) { */
/*     next_suf = cur_suf->next; */
 
/*     ch = *cur_suf->str; */
/*     if (_48_map->index[ch] == -1) */
/*       _48_map->index[ch] = i++; */
/*     index = _48_map->index[ch]; */
    
/*     if (cur_suf = cut_head(cur_suf, 1)) */
/*       insert_to_expand(_48_map->expand_nodes + index, cur_suf); */
/*     else  /\* 模式串尾 *\/ */
/*       set_bit(_48_map->pat_end_flag, index); */
/*   } */

  
/*   expand_node->next_level = _48_map; */
/*   expand_node->type = _48_MAP; */
/*   type_num[_48_MAP]++; */
  
/*   for (i = 0; i < dif_ch_num; i++) */
/*     if (_48_map->expand_nodes[i].next_level) */
/*       in_queue(queue, _48_map->expand_nodes + i); */
/* } */

/* void build_256_map(Expand_Node_t *expand_node) */
/* { */
/*   Suffix_Node_t *cur_suf, *next_suf; */
/*   _256_Map_t *_256_map = CALLOC(1, _256_Map_t); */
/*   Char_t ch; */
/*   int n; */

/*   for (cur_suf = expand_node->next_level; cur_suf; cur_suf = next_suf) { */
/*     next_suf = cur_suf->next; */
/*     ch = *cur_suf->str; */
/*     if (cur_suf = cut_head(cur_suf, 1)) */
/*       insert_to_expand(_256_map->expand_nodes + ch, cur_suf); */
/*     else */
/*       set_bit(_256_map->pat_end_flag, ch); */
/*   } */
  
/*   expand_node->next_level = _256_map; */
/*   expand_node->type = _256_MAP; */
/*   type_num[_256_MAP]++; */

/*   for (n = 256, expand_node = _256_map->expand_nodes; n; expand_node++, n--) */
/*     if (expand_node->next_level) */
/*       in_queue(queue, expand_node); */
/* } */
