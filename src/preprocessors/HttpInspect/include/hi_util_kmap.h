/****************************************************************************
 *
 * Copyright (C) 2003-2012 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 ****************************************************************************/
 
/*
*   kmap.h
*
*   Keyword Trie based Map Table
*
*   Author: Marc Norton
*
*/

#ifndef KTRIE_H
#define KTRIE_H

#define ALPHABET_SIZE 256

/*
*
*/
typedef struct _keynode {

  struct  _keynode * next;

  unsigned char * key;
  int             nkey;
  void          * userdata;  /* data associated with this pattern */
  
} KEYNODE;

/*
*
*/
typedef struct _kmapnode {

  int      nodechar;  /* node character */

  struct  _kmapnode * sibling; 
  struct  _kmapnode * child; 

  KEYNODE * knode;

} KMAPNODE;

/*
*
*/
typedef void (*KMapUserFreeFunc)(void *p);

typedef struct _kmap {

  KMAPNODE * root[256];  /* KTrie nodes */

  KEYNODE  * keylist; // list of key+data pairs
  KEYNODE  * keynext; // findfirst/findnext node

  KMapUserFreeFunc userfree;  // fcn to free user data
 
  int        nchars; // # character nodes

  int        nocase;

} KMAP;

/*
*  PROTOTYPES
*/
KMAP * KMapNew ( KMapUserFreeFunc userfree );
void   KMapSetNoCase( KMAP * km, int flag );
int    KMapAdd ( KMAP * km, void * key, int ksize, void * userdata );
void * KMapFind( KMAP * km, void * key, int ksize );
void * KMapFindFirst( KMAP * km );
void * KMapFindNext ( KMAP * km );
KEYNODE * KMapFindFirstKey( KMAP * km );
KEYNODE * KMapFindNextKey ( KMAP * km );
void KMapDelete(KMAP *km);

#endif


