/*
# This file is part of Primer Pooler v1.41 (c) 2016-18 Silas S. Brown.  For Wen.
# 
# This program is free software; you can redistribute and
# modify it under the terms of the General Public License
# as published by the Free Software Foundation; either
# version 3 of the License, or any later version.
#
# This program is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY.  See the GNU General
# Public License for more details.
*/
/* load-common.c: file-loading code common to all widths,
   generally just utils & look-before-loading.
   The bitwidth-dependent part is in 64-128.h etc. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *loadAndClose(FILE *f) {
  /* Read a file (which must be opened for reading in binary mode and positioned at the start) into a single null-terminated string, and close the file.  Caller must free() if non-NULL return */
  long size; char *r=NULL;
  if (!fseek(f,0L,SEEK_END) && ((size=ftell(f)) >= 0)
      && !fseek(f,0L,SEEK_SET) && (r=malloc(size+1))!=NULL) {
    r[size] = 0;
    if(!fread(r,size,1,f) && size) { free(r); r=NULL; }
  }
  fclose(f); return r;
}
int numPrimers(const char *fileData, int *maxLen,int *numTags) {
  /* count number of primer strings for memory allocation.
     Also sets maxLen to max # bases found in 1 primer.
     Tags are counted separately
     (we assume *numTags=0 on entry) */
  int r=0,minL=-1,maxL=0,isTag=0,maxTag=0;
  fileData += strspn(fileData,"\r\n\xef\xbb\xbf");
  while(*fileData) {
    int basesFound = 0;
    size_t lineEnd,start=0,l=0; do { // multiline seq?
      l = strcspn(fileData+start,"\r\n\xef\xbb\xbf"); /* stray BOMs at start of lines other than the first are sometimes possible if a Windows Notepad file has subsequently been edited on a non-Windows system */
      int i; for(i=0; i<(int)l; i++) if(strchr("ABCDGHKMNRSTVWYabcdghkmnrstvwy",fileData[start+i])) basesFound++;
      lineEnd = l + start;
      start = strspn(fileData+lineEnd,"\r\n\xef\xbb\xbf") + lineEnd;
    } while(*fileData!='>' && fileData[start] && fileData[start]!='>');
    if (lineEnd && *fileData != '>') {
      if(isTag) {
        (*numTags)++;
        maxTag = ((basesFound > maxTag) ? basesFound : maxTag);
      } else {
        r++;
        minL = ((minL<0 || basesFound < minL) ? basesFound : minL);
        maxL = ((basesFound > maxL) ? basesFound : maxL);
      }
    } else if(!strncmp(fileData, ">tag", 4) && lineEnd==5)
      /* e.g. >tagF or >tagR */
      isTag = 1;
    else if(*fileData == '>') isTag = 0;
    fileData += start;
  }
  if(!r) { fputs("No sequences found in this file\n",stderr); return -1; }
  fprintf(stderr,"%d primers + %d tags\nShortest primer length is %d bases; longest is %d\n",r,*numTags,minL,maxL);
  if(maxTag) {
    minL+=maxTag; maxL+=maxTag;
    fprintf(stderr,"(max tag length is %d, so max tagged primer length is %d)\n",maxTag,maxL); // TODO: longest *actual* tagged primer length might depend on *which* tag is added to which primer; would need more accounting if want to report that here, but this should be OK for allocation purposes except in the extremely rare case where this maxL exceeds checkLenLimit below but would not do so if accounted more carefully
  }
  int checkLenLimit(int maxLen); // bit-common.c
  *maxLen = maxL; return checkLenLimit(maxL) ? -1 : r;
}
