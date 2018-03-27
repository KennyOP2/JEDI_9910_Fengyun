/****************************************************************************
 *
 *            Copyright (c) 2003-2007 by HCC Embedded
 *
 * This software is copyrighted by and is the sole property of
 * HCC.  All rights, title, ownership, or other interests
 * in the software remain the property of HCC.  This
 * software may only be used in accordance with the corresponding
 * license agreement.  Any unauthorized use, duplication, transmission,
 * distribution, or disclosure of this software is expressly forbidden.
 *
 * This Copyright notice may not be removed or modified without prior
 * written consent of HCC.
 *
 * HCC reserves the right to modify this software without notice.
 *
 * HCC Embedded
 * Budapest 1133
 * Vaci ut 110
 * Hungary
 *
 * Tel:  +36 (1) 450 1302
 * Fax:  +36 (1) 450 1303
 * http: www.hcc-embedded.com
 * email: info@hcc-embedded.com
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "common/fat.h"
#include "common/common.h"
#include "common/port_f.h"
#include "chkdsk.h"

#if (FN_CAPI_USED)
void _fw_mutex_put(FW_MULTI *fm);
int _fw_settask(FW_MULTI **);
#endif

/****************************************************************************
 *
 * This module requires __malloc and __free function.
 * See the manual for more information.
 *
 ***************************************************************************/

#ifdef USE_MALLOC

#define F_FILE	FN_FILE
#define F_FIND	FN_FIND


#define SET_CLUSTER_BIT(ctbl,cluster) ctbl[cluster>>3]|=(1<<(cluster&7))
#define CLEAR_CLUSTER_BIT(ctbl,cluster) ctbl[cluster>>3]&=(unsigned char)(~((unsigned long)(1<<(cluster&7))))
#define GET_CLUSTER_BIT(ctbl,cluster) (ctbl[cluster>>3]&(1<<(cluster&7)))

#ifdef CHKDSK_LOG_ENABLE
static char *chkdsk_log;
static unsigned long chkdsk_log_pos;
static int chkdsk_log_dir=0;
static char *chkdsk_dir;
#endif

static F_DIRENTRY de[F_DEF_SECTOR_SIZE/sizeof(F_DIRENTRY)];
static char *full_path;
static int lost_pos;
static int dirlevel;


enum {
  FC_BAD_CLUSTER=100,
  FC_CROSS_CHAIN,
  FC_LAST_CLUSTER_CHANGED,
  FC_FILE,
  FC_DIR,
  FC_NOT_FOUND,
  FC_ERROR
};

#ifdef CHKDSK_LOG_ENABLE
enum {
  FC_LOG_DIRCHANGE=1,
  FC_LOG_DIRDEL,
  FC_LOG_DELETE__LOST,
  FC_LOG_DELETE__BAD_CLUSTER,
  FC_LOG_SIZE_CHANGE,
  FC_LOG_CLUSTER_CHANGE,
  FC_LOG_CROSS_LINK,
  FC_LOG_LOSTDIR_SAVE,
  FC_LOG_LOSTFILE_SAVE,
  FC_LOG_LOST_REMOVE,
  FC_LOG_LAST_CHANGED,
  FC_LOG_LOST_MOVE,
  FC_LOG_BACKDIR_ROOT,
  FC_LOG_FAT2_UPDATED,
  FC_LOG_DIR_DEPTH_EXCEEDED,
  FC_LOG_ALLOCATION_ERROR,
  FC_LOG_TMP,
#if F_LONGFILENAME
  FC_LOG_LFN_REMOVE,
#endif
  FC_LOG_NULL
};

static char *chkdsk_logmsg[] = {
                    "\nDirectory: ",
                    "  Directory entry deleted: ",
                    "  Lost entry deleted (found in a subdirectory): /",
                    "  Entry deleted (reserved/bad cluster): ",
                    "  File size changed: ",
                    "  Start cluster changed: ",
                    "  Entry deleted (cross linked chain): ",
                    "  Lost directory chain saved: ",
                    "  Lost file chain saved: ",
                    "  Lost chain removed (first cluster/cnt): ",
                    "  Last cluster changed (bad next cluster value): ",
                    "  Moving lost directory: /",
                    "  '..' changed to root: ",
                    "FAT2 updated according to FAT1.",
                    "  Maximum directory depth exceeded.",
                    "  Not enough memory to complete operation.",
                    "  file: ",
#if F_LONGFILENAME
                    "\nLong filename entry/entries removed. Count=",
#endif
                    "\0"
                  };
#endif



/*
**  chkdsk_setnamecnt
**
**  Convert position to string
**
**  Input:
**     name - name of file
**     pos - position
*/
static void chkdsk_setnamecnt(char *name, int pos)
{
  int i;

  i=7;
  while (pos)
  {
    name[i--]=(char)((pos%10)+'0');
    pos/=10;
  }
}

#ifdef CHKDSK_LOG_ENABLE
/*
**  chkdsk_putlog
**
**  Makes an entry in the log file.
**
**  Input:
**      errnum - error number
**      text - text to put
*/
static void chkdsk_putlog (unsigned char errnum, char *text)
{
  unsigned long len,alen;
  char *p;

  len=1;
  if (text) len+=strlen(text);
  alen=1+len;
  if (chkdsk_log_dir==0) alen+=strlen(chkdsk_dir)+1+1;

  if (CHKDSK_LOG_SIZE-chkdsk_log_pos>=alen)
  {
    p=chkdsk_log+chkdsk_log_pos;
    if (chkdsk_log_dir==0)
    {
      *(unsigned char *)p=FC_LOG_DIRCHANGE;
      ++p;
      (void)strcpy(p,chkdsk_dir);
      p+=strlen(chkdsk_dir)+1;
      chkdsk_log_dir=1;
    }
    *(unsigned char *)p=errnum;
    ++p;
    if (text) (void)strcpy(p,text);
         else *p=0;
    chkdsk_log_pos+=alen;
  }
}


/*
**  chkdsk_savelog
**
**  saves the log file to the disk.
**
**  Input:
**      fm - multitask information
*/
static int chkdsk_savelog (F_MULTI *fm)
{
  char log_name[]="LOG_0000.TXT";
  int cnt;
  int rc=FC_NO_ERROR;
  unsigned long code;
  char *p;
  F_FIND find;
  F_FILE *f;

  if (chkdsk_log_pos==0) return FC_NO_ERROR;

  cnt=0;
  do {
    chkdsk_setnamecnt(log_name,cnt++);
  } while (fn_findfirst(fm,log_name,&find)==0 && cnt<10000);

  if (cnt<10000)
  {
    f=fn_open(fm,log_name,"w");
    if (f)
    {
      p=chkdsk_log;
      while ((unsigned long)(p-chkdsk_log)<chkdsk_log_pos)
      {
        code=*(unsigned char *)p;
        ++p;
        (void)fn_write(fm,chkdsk_logmsg[code-1],1,strlen(chkdsk_logmsg[code-1]),f);
        (void)fn_write(fm,p,1,strlen(p),f);
        (void)fn_putc(fm,'\n',f);
        p+=strlen(p)+1;
      }
      (void)fn_putc(fm,'\n',f);
      (void)fn_putc(fm,'\n',f);

      (void)fn_close(fm,f);
    }
    else rc=FC_WRITE_ERROR;
  }

  return rc;
}
#endif



/*
**  chkdsk_getlostname
**
**  Get next valid 'LOSTxxxx' entry in a specified directory
**
**  Input:
**     fm - multitask information
**     path - path where to find
**     lost_name - lost filename
**  Ouput:
**     FC_NO_ERROR - found a valid entry
**     FC_ERROR - no more entries available
*/
static int chkdsk_getlostname (F_MULTI *fm, char *path, char *lost_name)
{
  F_FIND find;

  /* Check for a valid filename to put the lost chain in */
  (void)fn_chdir(fm,path);

  if (lost_name[0]==0 || fn_findfirst(fm,lost_name,&find)==0)
  {
    (void)strcpy(lost_name,"LOST0000");
    do {
      chkdsk_setnamecnt(lost_name,lost_pos++);
    } while (fn_findfirst(fm,lost_name,&find)==0 && lost_pos<10000);
  }
  else return FC_NO_ERROR;

  if (lost_pos<10000) return FC_NO_ERROR;
                 else return FC_ERROR;
}


/*
**  chkdsk_checklostname
**
**  Checks if a name matches to 'LOSTxxxx'
**
**  Input: name - name to check
**  Output: FC_ERROR-no match / FC_NO_ERROR-match
*/
static int chkdsk_checklostname (char *name)
{
  unsigned int i;
  char lost[]="LOST";

  for (i=0;i<strlen(lost) && name[i]==lost[i];i++);
  if (i==strlen(lost))
  {
    for (i=4;i<8 && name[i]>='0' && name[i]<='9';i++);
    if (i==8 && name[8]==0)
    {
      return FC_NO_ERROR;
    }
  }

  return FC_ERROR;
}


/*
**  chkdsk_getname
**
**  Converts a directory entry name/ext entry to a string
**
**  Input:
**     fde - pointer to directory entry
**     s - pointer to the destination string
*/
static void chkdsk_getname (F_DIRENTRY *fde, char *s)
{
  int i,j=0;

  for (i=0;i<8 && fde->name[i]!=' ';i++) s[i]=fde->name[i];
  if (fde->ext[0]!=' ')
  {
    s[i++]='.';
    for (j=i,i=0;i<3 && fde->ext[i];i++) s[j+i]=fde->ext[i];
  }
  s[j+i]=0;
}




/*
**  chkdsk_checkclusters
**
**  Check if a cluster chain is valid. If bad/empty or cross chained element found in chain, then
**  change last valid FAT entry to be the last cluster.
**
**  Input:
**     vi - volume information
**     ctbl - pointer to FAT entry usage table
**     cluster - chain start cluster
**     cluster_num - number of clusters in chain
**     param - parameters
**  Output:
**     FC_CLUSTER_ERROR - error writing/reading cluster value
**     FC_LAST_CLUSTER_CHANGED - last cluster changed
**     FC_CROSS_CHAIN - chain first element is cross linked
**     FC_BAD_CLUSTER - chain first element is invalid
**     FC_NO_ERROR - chain valid
*/
static int chkdsk_checkclusters (F_VOLUME *vi, unsigned char *ctbl, unsigned long cluster, unsigned long *cluster_num, int param)
{
  unsigned long next_cluster,prev_cluster;
  int rc=FC_NO_ERROR;
  unsigned long min_cluster;

  min_cluster=2;
  if (vi->mediatype==F_FAT32_MEDIA) min_cluster++;
  *cluster_num=0;

  if (cluster>=min_cluster && cluster!=F_CLUSTER_RESERVED && cluster!=F_CLUSTER_BAD)
  {
    prev_cluster=0;
    for (;;)
    {
      if (GET_CLUSTER_BIT(ctbl,cluster))
      {
        /* Cross chain */
        if (prev_cluster && (param&CHKDSK_ERASE_BAD_CHAIN)==0)
        {
          /* Fix: Change last cluster in chain */
          if (_f_setclustervalue(vi,prev_cluster,F_CLUSTER_LAST)) rc=FC_CLUSTER_ERROR;
                                                             else rc=FC_LAST_CLUSTER_CHANGED;
        }
        else rc=FC_CROSS_CHAIN;
        break;
      }
      SET_CLUSTER_BIT(ctbl,cluster);

      if (_f_getclustervalue(vi,cluster,&next_cluster))
      {
        rc=FC_CLUSTER_ERROR;
        break;
      }
      (*cluster_num)++;

      if (next_cluster<min_cluster || next_cluster==F_CLUSTER_RESERVED || next_cluster==F_CLUSTER_BAD)
      {
        /* Invalid cluster value in chain */
        /* Fix: Mark actual cluster as last cluster */
        if ((param&CHKDSK_ERASE_BAD_CHAIN)==0)
        {
          if (_f_setclustervalue(vi,cluster,F_CLUSTER_LAST)) rc=FC_CLUSTER_ERROR;
                                                        else rc=FC_LAST_CLUSTER_CHANGED;
        }
        else rc=FC_BAD_CLUSTER;
        break;
      }
      else if (next_cluster>=F_CLUSTER_RESERVED) break;	/* End of chain */

      prev_cluster=cluster;
      cluster=next_cluster;
    }
  }
  else if (cluster) rc=FC_BAD_CLUSTER;

  return rc;
}




/*
**  chkdsk_removechain
**
**  Remove a chain from FAT table.
**
**  Input:
**     vi - volume information
**     ctbl - pointer to the check table
**     cluster - start cluster of chain
**     size - number of clusters to remove
**     lost - deleting when handling lost chains
**
**  Output: 0-successfull
**          other-failed
*/
static int chkdsk_removechain (F_VOLUME *vi, unsigned char *ctbl, unsigned long cluster, unsigned long size, int lost)
{
  unsigned long next_cluster;
  while (size--)
  {
    if (_f_getclustervalue(vi,cluster,&next_cluster)) return FC_CLUSTER_ERROR;
    if (_f_setclustervalue(vi,cluster,0)) return FC_CLUSTER_ERROR;
    if (lost) SET_CLUSTER_BIT(ctbl,cluster);
         else CLEAR_CLUSTER_BIT(ctbl,cluster);
    cluster=next_cluster;
  }
  return FC_NO_ERROR;
}




/*
**  chkdsk_getnextentry
**
**  Gets the next entry from a directory structure.
**
**  Input:
**     vi - volume information
**     act_pos - actual directory position
**     dirsectorbuf - buffer where actual directory sector is stored
**     direntry - pointer to the new directory entry (in dirsectorbuf)
**     dirsector - atcual sector number (what dirsectorbuf holds)
**     changed - when set save sector at sector change
**  Output:
**     FC_READ_ERROR - sector read error
**     FC_WRITE_ERROR - sector write error
**     FC_CLUSTER_ERROR - error reading next cluster
**     FC_NO_ERROR - directory entry valid
**     FC_ERROR - no more directory entries
*/
static int chkdsk_getnextentry (F_VOLUME *vi, F_POS *act_pos, F_DIRENTRY *dirsectorbuf, F_DIRENTRY **direntry, unsigned long *dirsector, int *changed)
{
  while (act_pos->cluster<F_CLUSTER_RESERVED)
  {
    for (;act_pos->sector<act_pos->sectorend;act_pos->sector++)
    {
      if (*dirsector!=act_pos->sector)
      {
        if (_f_readsector(vi,(char *)dirsectorbuf,act_pos->sector,1)) return FC_READ_ERROR;
        *dirsector=act_pos->sector;
      }

      for (;act_pos->pos<F_DEF_SECTOR_SIZE/sizeof(F_DIRENTRY);act_pos->pos++)
      {
        F_DIRENTRY *ade=dirsectorbuf+(act_pos->pos);

        if (ade->name[0]!=0 && ade->name[0]!=F_DELETED_CHAR)
        {
          *direntry=ade;
          ++act_pos->pos;
          return FC_NO_ERROR;
        }
      }

      if (*changed)
      {
        if (_f_writesector(vi,(char *)dirsectorbuf,act_pos->sector,1)) return FC_WRITE_ERROR;
        *changed=0;
      }

      act_pos->pos=0;
    }

    if (act_pos->cluster==0)
    {
      if (vi->mediatype==F_FAT32_MEDIA) act_pos->cluster=vi->bootrecord.rootcluster;
                                   else return FC_ERROR;
    }

    {
      unsigned long next_cluster;
      if (_f_getclustervalue(vi,act_pos->cluster,&next_cluster)) return FC_CLUSTER_ERROR;
      if (next_cluster>=F_CLUSTER_RESERVED) return FC_ERROR;
      _f_clustertopos(vi,next_cluster,act_pos);
    }
  }

  return FC_ERROR;
}


#if F_LONGFILENAME
/*
**  chkdsk_deldirentries
**
**  Delete entries from a directory
**
**  Input:
**     vi - volume information
**     start_pos - start position
**     cnt - number of entries to delete
**  Output:
**     FC_READ_ERROR - sector read error
**     FC_WRITE_ERROR - sector write error
**     FC_NO_ERROR - success
**     FC_ERROR - no more directory entries
*/
static int chkdsk_deldirentries (F_VOLUME *vi, F_POS *start_pos, int cnt)
{
  F_DIRENTRY de0[F_DEF_SECTOR_SIZE/sizeof(F_DIRENTRY)];
  F_DIRENTRY *fde;
  unsigned long dirsector;
  int changed;
  int rc;

  dirsector=(unsigned long)(-1);
  changed=0;
  while ((rc=chkdsk_getnextentry(vi,start_pos,&(de0[0]),&fde,&dirsector,&changed))==FC_NO_ERROR)
  {
    fde->name[0]=F_DELETED_CHAR;
    changed=1;
    if (--cnt==0)
    {
      if (_f_writesector(vi,(char *)&(de0[0]),start_pos->sector,1)) return FC_WRITE_ERROR;
      return FC_NO_ERROR;
    }
  }

  return rc;
}
#endif


/*
**  chkdsk_lost_changebackdir
**
**  Change '..' of LOSTxxxx directories in root directory to point to root directory
**  when needed.
**
**  Input:
**     vi - volume information
**  Output:
**     FC_READ_ERROR - sector read error
**     FC_WRITE_ERROR - sector write error
**     FC_NO_ERROR - success
*/
static int chkdsk_lost_changebackdir (F_VOLUME *vi)
{
  F_DIRENTRY de0[F_DEF_SECTOR_SIZE/sizeof(F_DIRENTRY)];
  F_DIRENTRY de1[F_DEF_SECTOR_SIZE/sizeof(F_DIRENTRY)];
  F_DIRENTRY *fde;
  F_POS start_pos,tmp_pos;
  unsigned long dirsector;
  int changed;
  int rc;

  unsigned long start_cluster;
  char fullname[13];

  dirsector=(unsigned long)(-1);
  changed=0;
  _f_clustertopos(vi,0,&start_pos);
  while ((rc=chkdsk_getnextentry(vi,&start_pos,&(de0[0]),&fde,&dirsector,&changed))==FC_NO_ERROR)
  {
    if (fde->attr&F_ATTR_VOLUME) continue;
    if ((fde->attr&F_ATTR_DIR)==0) continue;

    chkdsk_getname(fde,fullname);
    start_cluster=_f_getdecluster(vi,fde);

    if (chkdsk_checklostname(fullname)==FC_NO_ERROR)
    {
      _f_clustertopos(vi,start_cluster,&tmp_pos);
      if (_f_readsector(vi,(char *)&(de1[0]),tmp_pos.sector,1)) return FC_READ_ERROR;
      start_cluster=_f_getdecluster(vi,&(de1[1]));

      if (start_cluster)
      {
        _f_setdecluster(vi,&(de1[1]),0);
        if (_f_writesector(vi,(char *)&(de1[0]),tmp_pos.sector,1)) return FC_WRITE_ERROR;
#ifdef CHKDSK_LOG_ENABLE
        chkdsk_putlog(FC_LOG_BACKDIR_ROOT,fullname);
#endif
      }
    }
  }

  if (rc==FC_ERROR) rc=FC_NO_ERROR;
  return rc;
}



/*
**  chkdsk_lost_checkcluster
**
**  Check if cluster match occurs, if yes delete LOSTxxxx from root directory.
**
**  Input:
**     vi - volume information
**     cluster - cluster to comapre to
**  Output:
**     FC_READ_ERROR - sector read error
**     FC_WRITE_ERROR - sector write error
**     FC_NO_ERROR - success
**     FC_ERROR - not found
*/
static int chkdsk_lost_checkcluster (F_VOLUME *vi, unsigned long cluster)
{
  F_DIRENTRY de0[F_DEF_SECTOR_SIZE/sizeof(F_DIRENTRY)];
  F_DIRENTRY *fde;
  F_POS start_pos;
  unsigned long dirsector;
  int changed;
  int rc;

  unsigned long start_cluster;
  char fullname[13];


  dirsector=(unsigned long)(-1);
  changed=0;
  _f_clustertopos(vi,0,&start_pos);
  while ((rc=chkdsk_getnextentry(vi,&start_pos,&(de0[0]),&fde,&dirsector,&changed))==FC_NO_ERROR)
  {
    if (fde->attr&F_ATTR_VOLUME) continue;

    chkdsk_getname(fde,fullname);
    start_cluster=_f_getdecluster(vi,fde);

    if (chkdsk_checklostname(fullname)==FC_NO_ERROR)
    {
      if (start_cluster==cluster)
      {
        fde->name[0]=F_DELETED_CHAR;
        if (_f_writesector(vi,(char *)&(de0[0]),start_pos.sector,1)) return FC_WRITE_ERROR;
#ifdef CHKDSK_LOG_ENABLE
        chkdsk_putlog(FC_LOG_DELETE__LOST,fullname);
#endif
        return FC_NO_ERROR;
      }
    }
  }

  return rc;
}



/*
**  chkdsk_lost_movelostdir
**
**  Check if dst_pos cluster matches to /LOSTxxxx '..' directory, if yes
**  move /LOSTxxxx to dst_pos
**
**  Input:
**     fm - multitask information
**     vi - volume information
**     dst_pos - destination directory position
**  Output:
**     FC_READ_ERROR - sector read error
**     FC_WRITE_ERROR - sector write error
**     FC_NO_ERROR - success
*/
static int chkdsk_lost_movelostdir (F_MULTI *fm, F_VOLUME *vi, F_POS *dst_pos)
{
  F_DIRENTRY de0[F_DEF_SECTOR_SIZE/sizeof(F_DIRENTRY)];
  F_DIRENTRY de1[F_DEF_SECTOR_SIZE/sizeof(F_DIRENTRY)];
  F_DIRENTRY de2;
  F_DIRENTRY *fde;
  F_POS start_pos,tmp_pos;
  unsigned long dirsector;
  int changed;
  int rc;
  int i;

  unsigned long start_cluster;
  char fullname[13];

  dirsector=(unsigned long)(-1);
  changed=0;
  _f_clustertopos(vi,0,&start_pos);
  while ((rc=chkdsk_getnextentry(vi,&start_pos,&(de0[0]),&fde,&dirsector,&changed))==FC_NO_ERROR)
  {
    if (fde->attr&F_ATTR_VOLUME) continue;
    if ((fde->attr&F_ATTR_DIR)==0) continue;

    chkdsk_getname(fde,fullname);
    start_cluster=_f_getdecluster(vi,fde);

    if (chkdsk_checklostname(fullname)==FC_NO_ERROR)
    {
      for (i=0;i<8 && full_path[i+1]==fullname[i];i++);

      if (i<8)
      {
        _f_clustertopos(vi,start_cluster,&tmp_pos);
        if (_f_readsector(vi,(char *)&(de1[0]),tmp_pos.sector,1)) return FC_READ_ERROR;
        start_cluster=_f_getdecluster(vi,&(de1[1]));
        if (start_cluster==dst_pos->cluster)
        {
          /* Move /LOSTnnnn to dst_pos/LOSTxxxx */
          F_NAME fsname;
          char tmpname[13];

          (void)_memcpy(&de2,fde,sizeof(F_DIRENTRY));
          fde->name[0]=F_DELETED_CHAR;
          if (_f_writesector(vi,(char *)&(de0[0]),start_pos.sector,1)) return FC_WRITE_ERROR;

          (void)strcpy(tmpname,fullname);
          if (chkdsk_getlostname(fm,full_path,tmpname)==0)
          {
#if F_LONGFILENAME
            (void)_memcpy((void *)(fsname.lname),(void *)tmpname,8);
            fsname.lname[8]=0;
#else
            (void)_memcpy((void *)(fsname.filename),(void *)tmpname,8);
            (void)_memset(fsname.fileext,' ',3);
#endif
            if (_f_addentry(vi,&fsname,dst_pos,&fde)==0)
            {
              (void)_memcpy((void *)((char *)fde+F_MAXNAME+F_MAXEXT),(void *)((char *)(&de2)+F_MAXNAME+F_MAXEXT),sizeof(F_DIRENTRY)-F_MAXNAME-F_MAXEXT);
              if (_f_writedirsector(vi)) return FC_WRITE_ERROR;
            }
            else return FC_WRITE_ERROR;
          }

#ifdef CHKDSK_LOG_ENABLE
          chkdsk_putlog(FC_LOG_DIRCHANGE,full_path);
          chkdsk_putlog(FC_LOG_LOST_MOVE,fullname);
#endif
          return FC_NO_ERROR;
        }
      }
    }
  }

  if (rc==FC_ERROR) rc=FC_NO_ERROR;
  return rc;
}



/*
**  chkdsk_files
**
**  Go throught all the directories and subderectories and check for inconsistencies.
**  Checks:
**      cluster chains,
**      '.'/'..' in directories,
**      size of files.
**
**  Input:
**     vi - volume information
**     ctbl - pointer to the check table
**     act_pos - actual directory position
**     back_pos - previous directory position
**     lost - directory is a lost chain
**     param - parameters
**     depth - actual directory depth
**  Output:
**     FC_READ_ERROR - sector read error
**     FC_WRITE_ERROR - sector write error
**     FC_CLUSTER_ERROR - error reading next cluster
**     FC_NO_ERROR - success
*/
static int chkdsk_files (F_VOLUME *vi, unsigned char *ctbl, F_POS *act_pos, F_POS *back_pos, int lost, int param, int depth)
{
  F_POS npos,fpos;
  F_POS *first_pos=&fpos;
  F_POS *next_pos=&npos;
  unsigned long start_cluster;
  unsigned long calc_size=0,real_size;
  unsigned long tmp;

  char fullname[13];

  int changed=0;
  int rc;
#ifdef CHKDSK_LOG_ENABLE
  int dirpos=0;
#endif



#ifdef CHKDSK_LOG_ENABLE
  chkdsk_log_dir=0;
#endif

  if (depth>CHKDSK_MAX_DIR_DEPTH)
  {
#ifdef CHKDSK_LOG_ENABLE
	chkdsk_putlog(FC_LOG_DIR_DEPTH_EXCEEDED,NULL);
#endif
    return FC_ERROR;
  }

  (void)_memcpy((void *)first_pos,(void *)act_pos,sizeof(F_POS));

  while (act_pos->cluster<F_CLUSTER_RESERVED)
  {
    for (;act_pos->sector<act_pos->sectorend;act_pos->sector++)
    {
      if (_f_readsector(vi,(char *)&(de[0]),act_pos->sector,1)) return FC_READ_ERROR;

      for (;act_pos->pos<F_DEF_SECTOR_SIZE/sizeof(F_DIRENTRY);act_pos->pos++)
      {
        F_DIRENTRY *ade=&de[act_pos->pos];

        if (ade->name[0]==0 || ade->name[0]==F_DELETED_CHAR || ade->attr&F_ATTR_VOLUME) continue;

        chkdsk_getname(ade,fullname);
        start_cluster=_f_getdecluster(vi,ade);

        if ( !(ade->attr&F_ATTR_DIR) || (ade->attr&F_ATTR_DIR && fullname[0]!='.') )
        {
          if (lost)
          {
            rc=chkdsk_lost_checkcluster(vi,start_cluster);
            if (rc==FC_NO_ERROR) continue;
            if (rc!=FC_ERROR) return rc;
          }

          /* Check cluster chain and get the number of clusters */
          rc=chkdsk_checkclusters(vi,ctbl,start_cluster,&calc_size,param);

          if (rc==FC_BAD_CLUSTER)
          {
            /* bad or reserved cluster in chain */
            /* remove file */
            (void)chkdsk_removechain (vi,ctbl,start_cluster,calc_size,lost);
            ade->name[0]=F_DELETED_CHAR;
            changed=1;
#ifdef CHKDSK_LOG_ENABLE
            chkdsk_putlog(FC_LOG_DELETE__BAD_CLUSTER,fullname);
#endif
          }
          else if (rc==FC_CROSS_CHAIN)
          {
            /* cluster in chain used by an other file */
            (void)chkdsk_removechain (vi,ctbl,start_cluster,calc_size,lost);
            ade->name[0]=F_DELETED_CHAR;
            changed=1;
#ifdef CHKDSK_LOG_ENABLE
            chkdsk_putlog(FC_LOG_CROSS_LINK,fullname);
#endif
          }
          else if (rc==FC_LAST_CLUSTER_CHANGED)
          {
#ifdef CHKDSK_LOG_ENABLE
            chkdsk_putlog(FC_LOG_LAST_CHANGED,fullname);
#endif
            rc=FC_NO_ERROR;
          }
        }
        else rc=FC_NO_ERROR;


        if (rc==FC_NO_ERROR)
        {
          if (ade->attr&F_ATTR_DIR)
          {
            if (fullname[0]=='.' || (fullname[0]=='.' && fullname[1]=='.'))
            {
              if (back_pos)
              {
                if (fullname[1]=='.')
                {
                  if (start_cluster!=back_pos->cluster)
                  {
                    /* Error, '..' pointing to a wrong place */
#ifdef CHKDSK_LOG_ENABLE
                    chkdsk_putlog(FC_LOG_CLUSTER_CHANGE,fullname);
#endif
                    _f_setdecluster(vi,ade,back_pos->cluster);
                    changed=1;
                  }
                }
                else
                {
                  if (start_cluster!=act_pos->cluster)
                  {
                    /* Error, '.' pointing to a wrong place */
#ifdef CHKDSK_LOG_ENABLE
                    chkdsk_putlog(FC_LOG_CLUSTER_CHANGE,fullname);
#endif
                    _f_setdecluster(vi,ade,act_pos->cluster);
                    changed=1;
                  }
                }
              }
              else
              {
                /* Error, root directory contains '.' / '..' directory, delete them */
#ifdef CHKDSK_LOG_ENABLE
                chkdsk_putlog(FC_LOG_DIRDEL,fullname);
#endif
                ade->name[0]=F_DELETED_CHAR;
                changed=1;
              }
            }
            else
            {
              if (changed)
              {
                if (_f_writesector(vi,(char *)&(de[0]),act_pos->sector,1)) return FC_WRITE_ERROR;
                changed=0;
              }

              _f_clustertopos(vi,start_cluster,next_pos);
#ifdef CHKDSK_LOG_ENABLE
              dirpos=strlen(chkdsk_dir);
              if (dirpos+strlen(fullname)+2<F_MAXPATH-1)
              {
                if (back_pos) (void)strcat(chkdsk_dir,"/");
                (void)strcat(chkdsk_dir,fullname);
              }
              else
              {
                (void)strcat(chkdsk_dir,"-");
              }
#endif
              ++depth;
              rc=chkdsk_files(vi,ctbl,next_pos,first_pos,lost,param,depth);
              --depth;
              if (rc) return rc;

#ifdef CHKDSK_LOG_ENABLE
              chkdsk_dir[dirpos]=0;
              chkdsk_log_dir=0;
#endif
              if (_f_readsector(vi,(char *)&(de[0]),act_pos->sector,1)) return FC_READ_ERROR;
            }
          }
          else
          {
            /* Do size check, compare max. size calculated from number of clusters and the
               size stored in the directory entry */
            tmp=vi->bootrecord.sector_per_cluster*F_DEF_SECTOR_SIZE;
            calc_size*=tmp;
            real_size=_f_get32bitl(ade->filesize);
            if (calc_size-real_size>=tmp)
            {
              /* Error, not possible file size stored in the directory entry */
#ifdef CHKDSK_LOG_ENABLE
              char tmps[64];
              (void)sprintf(tmps,"%s  orig.size:%ld  new size:%ld",fullname,real_size,calc_size);
              chkdsk_putlog(FC_LOG_SIZE_CHANGE,tmps);
#endif
              _f_set32bitl(ade->filesize,calc_size);
              changed=1;
            }
          }
        }
      }

      if (changed)
      {
        if (_f_writesector(vi,(char *)&(de[0]),act_pos->sector,1)) return FC_WRITE_ERROR;
        changed=0;
      }

      act_pos->pos=0;
    }


    if (act_pos->cluster==0)
    {
      if (vi->mediatype==F_FAT32_MEDIA) act_pos->cluster=vi->bootrecord.rootcluster;
                                   else break;
    }

    {
      unsigned long next_cluster;
      if (_f_getclustervalue(vi,act_pos->cluster,&next_cluster)) return FC_CLUSTER_ERROR;
      if (next_cluster>=F_CLUSTER_RESERVED) break;
      _f_clustertopos(vi,next_cluster,act_pos);
    }
  }

  return FC_NO_ERROR;
}



#if F_LONGFILENAME

/* If logging enbaled, count the number of entries deleted */
#ifdef CHKDSK_LOG_ENABLE
static int remove_cnt;
#endif
/*
**  chkdsk_lfn
**
**  Check long filename entries.
**  Delete if checksum,order number or clusterlo is bad.
**
**  Input:
**     vi - volume information
**     act_pos - position to start checking from
**     depth - actual directory depth
**  Output:
**     FC_READ_ERROR - sector read error
**     FC_WRITE_ERROR - sector write error
**     FC_CLUSTER_ERROR - error reading next cluster
**     FC_NO_ERROR - success
*/
static int chkdsk_lfn (F_VOLUME *vi, F_POS *act_pos, int depth)
{
  F_POS next_pos;
  int rc;
  int changed;

  int lfn_remove;
  F_POS lfn_pos;
  unsigned char lfn_chksum;
  unsigned char lfn_ord;
  unsigned char lfn_cnt;


  if (depth>CHKDSK_MAX_DIR_DEPTH)
  {
#ifdef CHKDSK_LOG_ENABLE
    chkdsk_putlog(FC_LOG_DIR_DEPTH_EXCEEDED,NULL);
#endif
    return FC_ERROR;
  }

  lfn_ord=0;
  lfn_cnt=0;
  lfn_chksum=0;
  lfn_remove=0;
  changed=0;
  (void)_memset((void *)(&lfn_pos),0,sizeof(F_POS));
  while (act_pos->cluster<F_CLUSTER_RESERVED)
  {
    for (;act_pos->sector<act_pos->sectorend;act_pos->sector++)
    {
      if (_f_readsector(vi,(char *)&(de[0]),act_pos->sector,1)) return FC_READ_ERROR;

      for (;act_pos->pos<F_DEF_SECTOR_SIZE/sizeof(F_DIRENTRY);act_pos->pos++)
      {
        F_DIRENTRY *ade=&de[act_pos->pos];

        if (ade->name[0] && ade->name[0]!=F_DELETED_CHAR)
        {
          if (ade->attr==F_ATTR_LFN)
          {
            F_LFN *lfn=(F_LFN *)ade;
            unsigned char pos;
            pos=(unsigned char)(lfn->ord&0x3f);
            if (pos>=1 && pos<=20 && _f_get16bitl(lfn->clusterlo)==0)
            {
              if (lfn_cnt)
              {
                if (lfn->ord+1!=lfn_ord || lfn_chksum!=lfn->chksum) lfn_remove=1;
                ++lfn_cnt;
              }
              else
              {
                lfn_remove=0;
                if ((lfn->ord&0x40)==0) lfn_remove=1;
                lfn_chksum=lfn->chksum;
                (void)_memcpy((void *)(&lfn_pos),(void *)act_pos,sizeof(F_POS));
                lfn_cnt=1;
              }
              lfn_ord=pos;
            }
            else
            {
              ade->name[0]=F_DELETED_CHAR;
              changed=1;
            }
            continue;
          }
          else if (ade->attr&F_ATTR_VOLUME)
          {
            lfn_remove=1;
          }
        }
        else if (lfn_cnt) lfn_remove=1;

        if (lfn_cnt)
        {
          if (lfn_remove==0)
          {
            unsigned char chksum;
            int i;

            chksum=0;
            for (i=0;i<8;i++) chksum=(unsigned char)((((chksum&1)<<7)|((chksum&0xfe)>>1))+ade->name[i]);
            for (i=0;i<3;i++) chksum=(unsigned char)((((chksum&1)<<7)|((chksum&0xfe)>>1))+ade->ext[i]);
            if (lfn_chksum!=chksum) lfn_remove=1;
          }

          if (lfn_remove)
          {
            if (changed && lfn_pos.sector!=act_pos->sector)
            {
              if (_f_writesector(vi,(char *)&(de[0]),act_pos->sector,1)) return FC_WRITE_ERROR;
              changed=0;
            }
            (void)chkdsk_deldirentries(vi,&lfn_pos,lfn_cnt);
            lfn_remove=0;
#ifdef CHKDSK_LOG_ENABLE
            ++remove_cnt;
#endif
            if (_f_readsector(vi,(char *)&(de[0]),act_pos->sector,1)) return FC_READ_ERROR;
          }

          lfn_cnt=0;
        }

        if (ade->name[0]==0 || ade->name[0]==F_DELETED_CHAR) continue;

        if (ade->attr&F_ATTR_VOLUME) continue;

        if (ade->attr&F_ATTR_DIR)
        {
          if (ade->name[0]!='.')
          {
            _f_clustertopos(vi,_f_getdecluster(vi,ade),&next_pos);
            ++depth;
            rc=chkdsk_lfn(vi,&next_pos,depth);
            --depth;
            if (rc) return rc;
            if (_f_readsector(vi,(char *)&(de[0]),act_pos->sector,1)) return FC_READ_ERROR;
          }
        }
      }

      if (changed)
      {
        if (_f_writesector(vi,(char *)&(de[0]),act_pos->sector,1)) return FC_WRITE_ERROR;
        changed=0;
      }

      act_pos->pos=0;
    }


    if (act_pos->cluster==0)
    {
      if (vi->mediatype==F_FAT32_MEDIA) act_pos->cluster=vi->bootrecord.rootcluster;
                                   else break;
    }

    {
      unsigned long next_cluster;
      if (_f_getclustervalue(vi,act_pos->cluster,&next_cluster)) return FC_CLUSTER_ERROR;
      if (next_cluster>=F_CLUSTER_RESERVED) break;
      _f_clustertopos(vi,next_cluster,act_pos);
    }
  }

  return FC_NO_ERROR;
}

#endif




/*
**  chkdsk_findlostdir
**
**  1) Checks whether a directory with start cluster 'cluster' exists in the directory structure
**  2) Checks whether LOSTnnnn directory '..' exists in any other LOSTnnnn directory structure,
**     if yes then move entry from '/LOSTnnnn' to the proper place.
**
**  Input:
**     fm - multitask information
**     vi - volume information
**     cluster - first cluster of directory (used for case 1)
**     lost - look only for lost directories and check wheter it's member of any lost directory
**            structure.
**     depth - actual directory depth.
**  Output:
**     0 - found
**     1 - not found
*/
static int chkdsk_findlostdir (F_MULTI *fm, F_VOLUME *vi, unsigned long cluster, int lost, int depth)
{
  F_FIND *f;
  int len;
  char fstr[8];

  F_POS start_pos,dst_pos;
  int rc;

  if (depth>CHKDSK_MAX_DIR_DEPTH)
  {
#ifdef CHKDSK_LOG_ENABLE
    chkdsk_putlog(FC_LOG_DIR_DEPTH_EXCEEDED,NULL);
#endif
    return FC_ERROR;
  }

  _f_clustertopos(vi,0,&start_pos);
  if (lost && cluster==0)
  {
    (void)strcpy(fstr,"LOST*.*");
    ++cluster;
  }
  else
  {
    if (!cluster) return 0;
    (void)strcpy(fstr,"*.*");
  }


  f=(F_FIND *)__malloc(sizeof(F_FIND));
  if (f==NULL)
  {
#ifdef CHKDSK_LOG_ENABLE
    chkdsk_putlog(FC_LOG_ALLOCATION_ERROR,NULL);
#endif
    return FC_ERROR;
  }

  if (fn_findfirst(fm,fstr,f)==0)
  {
    do {
      if ((f->attr)&F_ATTR_DIR)
      {
        if (f->filename[0]!='.')
        {
          len=strlen(full_path);
          if (len+strlen(f->filename)+2<F_MAXPATH-1)
          {
            if (full_path[1]) (void)strcat(full_path,"/");
            (void)strcat(full_path,f->filename);
          }
          else
          {
            (void)strcat(full_path,"-");
          }

          if (lost)
          {
            _f_clustertopos(vi,f->cluster,&dst_pos);
            rc=chkdsk_lost_movelostdir(fm,vi,&dst_pos);
            if (rc==FC_NO_ERROR)
            {
              (void)fn_chdir(fm,f->filename);
              ++dirlevel;
              ++depth;
              rc=chkdsk_findlostdir(fm,vi,cluster,lost,depth);
              --depth;
              if (rc!=FC_NO_ERROR && rc!=FC_ERROR)
              {
                __free(f);
                return rc;
              }
              --dirlevel;
              if (dirlevel) (void)fn_chdir(fm,"..");
                       else (void)fn_chdir(fm,"/");
              full_path[len]=0;
            }
            else
            {
              __free(f);
              return rc;
            }
          }
          else
          {
            if (f->cluster!=cluster)
            {
              (void)fn_chdir(fm,f->filename);
              ++dirlevel;
              ++depth;
              rc=chkdsk_findlostdir(fm,vi,cluster,lost,depth);
              --depth;
              if (rc==FC_NO_ERROR)
              {
                __free(f);
                return FC_NO_ERROR;
              }
              --dirlevel;
              if (dirlevel) (void)fn_chdir(fm,"..");
                       else (void)fn_chdir(fm,"/");
              full_path[len]=0;
            }
            else
            {
              __free(f);
              return FC_NO_ERROR;
            }
          }
        }
      }
    } while (fn_findnext(fm,f)==0);
  }

  __free(f);
  return FC_ERROR;
}





/*
**  chkdsk_checkdir
**
**  Checks whether a cluster content is a directory or not.
**
**  Input:
**      fm - multitask information
**      vi - volume information
**      ctbl - cluster table
**      first_cluster - first cluster of the chain
**      csize - number of clusters in the chain
**      param - parameters
**  Output:
**      FC_NO_ERROR-seems to be a directory,
**      FC_ERROR-not a directory
**
*/
static int chkdsk_savelostchain (F_MULTI *fm, F_VOLUME *vi, unsigned char *ctbl, unsigned long first_cluster, unsigned long csize, int param)
{
  F_POS pos,dirpos;
  F_NAME fsname;
  F_DIRENTRY *direntry;
  char lost_name[9];
  unsigned char attr;

  unsigned long back_cluster;
  char str[13];
  int rc;
  int dir=0;
#ifdef CHKDSK_LOG_ENABLE
  int tmp;
#endif

  full_path[0]='/';
  full_path[1]=0;
  lost_name[0]=0;
  attr=F_ATTR_ARC;

  if (chkdsk_getlostname(fm,"/",lost_name)==0)
  {
    _f_clustertopos(vi,first_cluster,&pos);
    if (_f_readsector(vi,(char *)&(de[0]),pos.sector,1)) return FC_READ_ERROR;

    chkdsk_getname(&(de[0]),str);
    if (str[0]=='.' && str[1]==0 && de[0].attr&F_ATTR_DIR)
    {
      if (_f_getdecluster(vi,&(de[0]))==first_cluster)
      {
        chkdsk_getname(&(de[1]),str);
        if (str[0]=='.' && str[1]=='.' && str[2]==0 && de[1].attr&F_ATTR_DIR)
        {
          back_cluster=_f_getdecluster(vi,&(de[1]));
          _f_clustertopos(vi,back_cluster,&dirpos);
#ifdef CHKDSK_LOG_ENABLE
          tmp=strlen(chkdsk_dir);
          (void)strcat(chkdsk_dir,lost_name);
#endif
          rc=chkdsk_files(vi,ctbl,&pos,&dirpos,1,param,0);
#ifdef CHKDSK_LOG_ENABLE
          chkdsk_dir[tmp]=0;
#endif
          if (rc) return rc;

          (void)fn_chdir(fm,"/");
          if (chkdsk_findlostdir(fm,vi,back_cluster,0,0)==FC_NO_ERROR) dir=1;
          attr=F_ATTR_DIR;
        }
      }
    }

    if (dir==0) _f_clustertopos(vi,0,&dirpos);

#if F_LONGFILENAME
    (void)_memcpy((void *)(fsname.lname),(void *)lost_name,8);
    fsname.lname[8]=0;
#else
    (void)_memcpy((void *)(fsname.filename),(void *)lost_name,8);
    (void)_memset(fsname.fileext,' ',3);
#endif

    if (_f_addentry(vi,&fsname,&dirpos,&direntry)==0)
    {
      direntry->attr=attr;
      if ((attr&F_ATTR_DIR)==0) _f_set32bitl(direntry->filesize,csize*vi->bootrecord.sector_per_cluster*F_DEF_SECTOR_SIZE);

      _f_setdecluster(vi,direntry,first_cluster);
      _f_set16bitl(direntry->cdate,f_getdate());
      _f_set16bitl(direntry->ctime,f_gettime());
      if (_f_writedirsector(vi)) return FC_WRITE_ERROR;
    }
    else return FC_WRITE_ERROR;

#ifdef CHKDSK_LOG_ENABLE
    if (attr&F_ATTR_DIR)
    {
      chkdsk_putlog(FC_LOG_DIRCHANGE,full_path);
      chkdsk_putlog(FC_LOG_LOSTDIR_SAVE,lost_name);
    }
    else
    {
      chkdsk_putlog(FC_LOG_LOSTFILE_SAVE,lost_name);
    }
#endif
  }

  return FC_NO_ERROR;
}



/*
**  chkdsk_buildlostchains
**
**  Try to save lost chains.
**  Currently saves all lost chains, if there is a chain with bad end (no last cluster, cross chain, ...),
**  it puts the last cluster mark to the proper position.
**
**  Input:
**     fm - multitask information
**     vi - volume information
**     ctbl - check table
**     ctbl_size - size of check table
**     param - parameters
**  Output:
**     FC_READ_ERROR - sector read error
**     FC_WRITE_ERROR - sector write error
**     FC_CLUSTER_ERROR - error reading next cluster
**     FC_NO_ERROR - success
*/
int chkdsk_buildlostchains (F_MULTI *fm, F_VOLUME *vi, unsigned char *ctbl, int ctbl_size, int param)
{
  /* Try to recover lost chains */
  int ctbl_pos;
  unsigned char ctbl_tmp;

  unsigned long first_cluster,act_cluster,next_cluster;
  unsigned long max_cluster,min_cluster;
  unsigned long csize,i;

  int rc;
  int erase;


  min_cluster=2;
  if (vi->mediatype==F_FAT32_MEDIA) min_cluster++;
  max_cluster=_f_getmaxcluster(vi);

  csize=0;
  for (ctbl_pos=0;ctbl_pos<ctbl_size;ctbl_pos++)
  {
    while (ctbl[ctbl_pos]!=0xff)
    {
      /* Lost cluster */
      ctbl_tmp=ctbl[ctbl_pos];
      first_cluster=(ctbl_pos<<3);

      while (ctbl_tmp&1)
      {
        ++first_cluster;	/* contains the first lost cluster in the table */
        ctbl_tmp>>=1;
      }

      /* Go and find the whole chain */
      csize=0;
      act_cluster=first_cluster;
      if (param&CHKDSK_ERASE_LOST_CHAIN) erase=1;
                                    else erase=0;
      for (;;)
      {
        SET_CLUSTER_BIT(ctbl,act_cluster);
        ++csize;
        if (_f_getclustervalue(vi,act_cluster,&next_cluster)) return FC_CLUSTER_ERROR;
        if (next_cluster<min_cluster || next_cluster>=max_cluster || GET_CLUSTER_BIT(ctbl,next_cluster))
        {
          if (next_cluster!=F_CLUSTER_LAST)
          {
            if ((param&CHKDSK_ERASE_LOST_BAD_CHAIN)==0)
            {
              if (_f_setclustervalue(vi,act_cluster,F_CLUSTER_LAST)) return FC_CLUSTER_ERROR;
            }
            else erase=1;
          }
          break;
        }
        act_cluster=next_cluster;
      }

      act_cluster=first_cluster+1;
      for (i=0;i<max_cluster-min_cluster;i++)
      {
        if (act_cluster>=max_cluster) act_cluster=min_cluster;
        if (GET_CLUSTER_BIT(ctbl,act_cluster)==0)
        {
          if (_f_getclustervalue(vi,act_cluster,&next_cluster)) return FC_CLUSTER_ERROR;
          if (next_cluster==first_cluster)
          {
            SET_CLUSTER_BIT(ctbl,act_cluster);
            first_cluster=act_cluster;
            ++csize;
            i=0;
          }
        }
        ++act_cluster;
      }

      if (erase)
      {
        rc=chkdsk_removechain (vi,ctbl,first_cluster,csize,1);
        if (rc) return rc;
#ifdef CHKDSK_LOG_ENABLE
        {
          char s[16];
          (void)sprintf(s,"%ld/%ld",first_cluster,csize);
          chkdsk_putlog(FC_LOG_LOST_REMOVE,s);
        }
#endif
      }
      else
      {
        rc=chkdsk_savelostchain(fm,vi,ctbl,first_cluster,csize,param);
        if (rc) return rc;
      }
    }
  }

  rc=chkdsk_findlostdir(fm,vi,0,1,0);
  if (rc==FC_NO_ERROR || rc==FC_ERROR)
  {
    rc=chkdsk_lost_changebackdir(vi);
  }

  return rc;
}



/*
**  chkdsk_cmpfat
**
**  Compare FAT1 and FAT2 and if there is any difference make FAT2 equal to FAT1
**
**  Input:
**     vi - volume information
**     ctbl - pointer to check table (used as intermediate buffer to load FATs)
**     ctbl_size - size of ctbl
**  Output:
**     FC_NO_ERROR - success
**     FC_READ_ERROR - invalid sector in FAT1 and FAT2
*/
static int chkdsk_cmpfat (F_VOLUME *vi, unsigned char *ctbl, unsigned long ctbl_size)
{
  unsigned long i;
  unsigned long check_size;
  unsigned long fat_size;
  unsigned long act_size;
  int rc1,rc2,rc,alloc;
  unsigned char *fat1,*fat2;

  rc=FC_NO_ERROR;
  fat_size=vi->firstfat.num;
  check_size=(ctbl_size/F_DEF_SECTOR_SIZE)/2;
  if (check_size==0)
  {
    fat1=(unsigned char *)de;
    fat2=(unsigned char *)__malloc(F_DEF_SECTOR_SIZE);
    if (fat2==NULL) return FC_ALLOCATION_ERROR;
    check_size=1;
    alloc=1;
  }
  else
  {
    if (check_size>256) check_size=256;
    fat1=ctbl;
    fat2=ctbl+check_size*F_DEF_SECTOR_SIZE;
    alloc=0;
  }

  for (i=0;i<fat_size;)
  {
    if (check_size>(fat_size-i)) act_size=fat_size-i;
                            else act_size=check_size;

    rc1=_f_readsector(vi,fat1,vi->firstfat.sector+i,act_size);
    rc2=_f_readsector(vi,fat2,vi->firstfat.sector+fat_size+i,act_size);

    if (rc1 && rc2)
    {
      rc=FC_READ_ERROR;
      break;
    }

    if (rc1==0 && rc2==0)
    {
      if (memcmp(fat1,fat2,act_size*F_DEF_SECTOR_SIZE))
      {
        (void)_f_writesector(vi,fat1,vi->firstfat.sector+fat_size+i,act_size);
#ifdef CHKDSK_LOG_ENABLE
        chkdsk_putlog(FC_LOG_FAT2_UPDATED,NULL);
#endif
      }
    }

    i+=act_size;
  }

  if (alloc) __free(fat2);

  return rc;
}



/*
** fn_checkdisk
**
** Main routine for checkdisk.
**
** Input:
**    fm - multitask param
**    drivenum - number of the drive
**    param - parameter (CHKDSK_ERASE_BAD_CHAIN|CHKDSK_ERASE_LOST_CHAIN|CHKDSK_ERASE_LOST_BAD_CHAIN)
**
** Output:
**     FC_READ_ERROR - sector read error
**     FC_WRITE_ERROR - sector write error
**     FC_CLUSTER_ERROR - error reading next cluster
**     FC_ALLOCATION_ERROR - not enough memory
**     FC_NO_ERROR - success
**
*/

static int fn_checkdisk (F_MULTI *fm,int drivenum, int param)
{
  F_VOLUME *vi;
  F_POS first_pos;
  int rc;

  unsigned long ctbl_size;

  unsigned char *ctbl;

  unsigned long maxcluster;
  unsigned long i;



  rc=_f_getvolume(fm,drivenum,&vi);
  if (rc) return rc;

  if (vi->bootrecord.bytes_per_sector!=F_DEF_SECTOR_SIZE) return F_ERR_NOTSUPPSECTORSIZE;

  rc=fn_chdrive(fm,drivenum);
  if (rc) return rc;

#ifdef CHKDSK_LOG_ENABLE
  chkdsk_log=(char *)__malloc(CHKDSK_LOG_SIZE+F_MAXPATH);
  if (chkdsk_log==NULL) return FC_ALLOCATION_ERROR;
  (void)_memset(chkdsk_log,0,CHKDSK_LOG_SIZE+F_MAXPATH);
  chkdsk_dir=chkdsk_log+CHKDSK_LOG_SIZE;
  chkdsk_dir[0]='/';
  chkdsk_dir[1]=0;
  chkdsk_log_pos=0;
#endif

  full_path=(char *)__malloc(F_MAXPATH);
  if (full_path==NULL) return FC_ALLOCATION_ERROR;
  (void)_memset(full_path,0,F_MAXPATH);

  maxcluster=_f_getmaxcluster(vi);
  ctbl_size=(maxcluster>>3)+1;
  ctbl=(unsigned char *)__malloc(ctbl_size);
  if (ctbl==NULL) return FC_ALLOCATION_ERROR;

  rc=chkdsk_cmpfat(vi,ctbl,ctbl_size);	/* Compare and fix FAT1/FAT2 */
  if (rc==FC_NO_ERROR)
  {
    (void)_memset((void *)ctbl,0,ctbl_size);
    if (vi->mediatype==F_FAT32_MEDIA) ctbl[0]=0x07;	/* mark cluster 0,1,2 as used */
                                 else ctbl[0]=0x03;	/* mark cluster 0,1 as used */

    _f_clustertopos(vi,0,&first_pos);

    rc=chkdsk_files(vi,ctbl,&first_pos,NULL,0,param,0);
    if (rc==FC_NO_ERROR)
    {
      /*
      ** Go through FAT table and mark lost clusters.
      */
      for (i=0;i<maxcluster;i++)
      {
        unsigned long val;

        if (_f_getclustervalue(vi,i,&val))
        {
          rc=FC_CLUSTER_ERROR;
          break;
        }

        if (val && val!=F_CLUSTER_BAD && val!=F_CLUSTER_RESERVED)
        {
          if (GET_CLUSTER_BIT(ctbl,i)) CLEAR_CLUSTER_BIT(ctbl,i);
                                  else SET_CLUSTER_BIT(ctbl,i);
        }
      }

      if (vi->mediatype==F_FAT32_MEDIA) ctbl[0]&=~0x07;	/* mark cluster 0,1,2 as used */
                                   else ctbl[0]&=~0x03;	/* mark cluster 0,1 as used */

      if (i==maxcluster)
      {
        for (i=0;i<ctbl_size;ctbl[i++]^=0xff);	/* invert bits */
        lost_pos=0;
        dirlevel=0;

        rc=chkdsk_buildlostchains(fm,vi,ctbl,ctbl_size,param);

#if F_LONGFILENAME
        if (rc==FC_NO_ERROR)
        {
#ifdef CHKDSK_LOG_ENABLE
          remove_cnt=0;
#endif
          _f_clustertopos(vi,0,&first_pos);
          rc=chkdsk_lfn(vi,&first_pos,0);
#ifdef CHKDSK_LOG_ENABLE
          if (remove_cnt)
          {
            char tmp[8];
            (void)sprintf(tmp,"%d",remove_cnt);
            chkdsk_putlog(FC_LOG_LFN_REMOVE,tmp);
          }
#endif
        }
#endif
      }
    }
  }


  __free((void *)full_path);
  __free((void *)ctbl);

#ifdef CHKDSK_LOG_ENABLE
  if (rc==FC_NO_ERROR) rc=chkdsk_savelog(fm);
  __free((void *)chkdsk_log);
#endif

#ifdef FATCACHE_ENABLE
  (void)_f_fatcache_flush (vi);
#endif

  return rc;
}

/*
** f_checkdisk
**
** Main routine for checkdisk.
**
** Input:
**    drivenum - number of the drive
**    param - parameter (CHKDSK_ERASE_BAD_CHAIN|CHKDSK_ERASE_LOST_CHAIN|CHKDSK_ERASE_LOST_BAD_CHAIN)
**
** Output:
**     FC_READ_ERROR - sector read error
**     FC_WRITE_ERROR - sector write error
**     FC_CLUSTER_ERROR - error reading next cluster
**     FC_ALLOCATION_ERROR - not enough memory
**     FC_NO_ERROR - success
**
*/
int f_checkdisk (int drivenum, int param)
{
  int ret;
#if (!FN_CAPI_USED)
  F_MULTI *fm;
  if (fnGetTask(&fm)) return F_ERR_TASKNOTFOUND; /* busy! */
  ret=fn_checkdisk(fm,drivenum,param);
  _f_mutex_put(fm);
#else
  FW_MULTI *fm;
  if (_fw_gettask(&fm)) return F_ERR_TASKNOTFOUND; /* busy! */
  ret=fn_checkdisk(fm,drivenum,param);
  _fw_mutex_put(fm);
#endif

  return ret;
}

#else
#error USE_MALLOC needs to be enabled in order to use checkdisk.
#endif /* USE_MALLOC */

