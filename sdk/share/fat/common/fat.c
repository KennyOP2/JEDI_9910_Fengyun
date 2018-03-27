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

#include "fat.h"
#include "port_f.h"
#include "common.h"


#if (!F_LONGFILENAME)

/****************************************************************************
 *
 *  THIS MODULE IS FOR FAT WITHOUT LONG FILE NAMES ONLY 8+3 NAMES
 *  PLEASE CHECK UDEFS_F.H WITH F_LONGFILENAME DECLARATION
 *
 ***************************************************************************/

/****************************************************************************
 *
 * _f_getdirsector
 *
 * read a directory sector
 *
 * INPUTS
 *
 * vi - volume pointer
 * sector - which sector is needed
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/

int _f_getdirsector(F_VOLUME *vi,unsigned long sector)
{
	int ret=F_NO_ERROR;

	if (vi->direntrysector!=sector)
	{
		ret=_f_readsector(vi,vi->direntry,sector,1);
		if (!ret) vi->direntrysector=sector;
		else vi->direntrysector=(unsigned long)(-1);
	}

	return ret;
}

/****************************************************************************
 *
 * _f_writedirsector
 *
 * Write directory sector, which was last read
 *
 * INPUTS
 *
 * vi - volume pointer
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/

int _f_writedirsector(F_VOLUME *vi)
{
    if (vi->direntrysector!=(unsigned long)(-1))
    {
   	    return _f_writesector(vi,vi->direntry,vi->direntrysector,1);
    }
    return F_ERR_INVALIDSECTOR;
}

/****************************************************************************
 *
 * _f_checknameinv
 *
 * checking name special characters
 *
 * INPUTS
 *
 * name - filename (e.g.: filename)
 * len - length to check
 *
 * RETURNS
 *
 * 0 - if no contains invalid character
 * other - if contains any invalid character
 *
 ***************************************************************************/

static int _f_checknameinv(const char *name, int len)
{
	int inspace=0;
	int a=0;

	for (a=0; a<len || (len==-1); a++)
	{
		char ch=name[a];

		if (!ch)
		{
			if (len==-1) return 0; /* terminated normally */
			else return 1;		   /* zero char in the name */
		}

		if (!inspace)
		{
			if (ch==' ') inspace=1;	 /* cannot be space inside the name */
			if (ch>0 && ch<0x20) return 1;
			if (ch=='|') return 1;
			if (ch=='<') return 1;
			if (ch=='>') return 1;
			if (ch=='/') return 1;
			if (ch=='\\') return 1;
			if (ch==':') return 1;
			if (ch==34) return 1; /* " */
		}
		else if (ch!=' ') return 1; /* no inspace allowed */
	}

	return 0;
}

/****************************************************************************
 *
 * _f_checkname
 *
 * checking filename and extension for special characters
 *
 * INPUTS
 *
 * name - filename (e.g.: filename)
 * ext - extension of file (e.g.: txt)
 *
 * RETURNS
 *
 * 0 - if no contains invalid character
 * other - if contains any invalid character
 *
 ***************************************************************************/

static int _f_checkname(char *name,char *ext)
{
	if (name[0]==F_DELETED_CHAR) return 1; /* cannot started with this char! */

	if (_f_checknameinv(name,F_MAXNAME)) return 1;
	if (_f_checknameinv(ext,F_MAXEXT)) return 1;

	return 0;
}

/****************************************************************************
 *
 * _f_checknamewc
 *
 * checking filename and extension for wildcard character
 *
 * INPUTS
 *
 * name - filename (e.g.: filename)
 * ext - extension of file (e.g.: txt)
 *
 * RETURNS
 *
 * 0 - if no contains wildcard character (? or *)
 * other - if contains any wildcard character
 *
 ***************************************************************************/

static int _f_checknamewc(char *name,char *ext)
{
	int a=0;

	for (a=0; a<F_MAXNAME; a++)
	{
		char ch=name[a];
		if (ch=='?') return 1;
		if (ch=='*') return 1;
	}

	for (a=0; a<F_MAXEXT; a++)
	{
		char ch=ext[a];
		if (ch=='?') return 1;
		if (ch=='*') return 1;
	}

	return _f_checkname(name,ext);
}


/****************************************************************************
 *
 * _f_setnameext
 *
 * convert a string into filename and extension separatelly, the terminator
 * character could be zero char, '/' or '\'
 *
 * INPUTS
 *
 * s - source string (e.g.: hello.txt)
 * name - where to store name (this array size has to be F_MAXNAME (8))
 * ext - where to store extension (this array size has to be F_MAXEXT (3))
 *
 * RETURNS
 *
 * length of the used bytes from source string array
 *
 ***************************************************************************/

static int _f_setnameext(const char *s, char *name, char *ext)
{
	int len,extlen=0;
	int a;
	int setext=1;

	for (len=0;;)
	{
		char ch=s[len];
		if (ch==0) break;
		if (ch=='\\' || ch=='/') break;
		len++;					/* calculate len */
	}

	if (len && s[0]=='.')
	{
		if (len==1) goto dots;
		if (s[1]=='.') {
			if (len==2) goto dots;
			goto dots; /* maybe more than 2 dots */
		}
	}

	for (a=len; a>0; a--)
	{
		if (s[a-1]=='.')
		{
			int b;
			extlen=len-a+1;
			len=a-1;

			for (b=0; b<F_MAXEXT; b++)
			{
				if (b<extlen-1)
				{
					char ch=s[a++];
					if (ch>='a' && ch<='z') ch+='A'-'a';

					ext[b]=ch;
				}
				else ext[b]=32;
			}
			setext=0;
			break;
		}
	}

dots:
	if (setext)
	{
		for (a=0; a<F_MAXEXT; a++)
		{
			ext[a]=32;     /* fills with extension and zeroes */
		}
	}

	for (a=0; a<F_MAXNAME; a++)
	{
		if (a<len)
		{
			char ch=s[a];
			if (ch>='a' && ch<='z')
			{
				ch+='A'-'a';
			}
			name[a]=ch;
		}
		else name[a]=32;
	}

	return len+extlen;
}

/****************************************************************************
 *
 * _f_setfsname
 *
 * convert a single string into F_NAME structure
 *
 * INPUTS
 *
 * fm - multi structure pointer
 * name - combined name with drive,path,filename,extension used for source
 * fsname - where to fill this structure with separated drive,path,name,ext
 *
 * RETURNS
 *
 * 0 - if successfully
 * other - if name contains invalid path or name
 *
 ***************************************************************************/

static int _f_setfsname(F_MULTI *fm,const char *name,F_NAME *fsname)
{
	char s[FN_MAXPATH];
	unsigned int pathpos=0;
	unsigned int namepos=0;
	unsigned int a;

	s[0]=0;

	if (!name[0]) return 1; /* no name */

	if (name[1]==':')
	{
		int drv=_f_toupper(name[0]);
		if (drv>='A' && drv<='Z')
		{
			fsname->drivenum=drv-'A';
			name+=2; /* skip drive number */
		}
		else return 1; /* invalid drive */
	}
	else
	{
		fsname->drivenum=fn_getdrive(fm);
		if (fsname->drivenum==-1) return 1;
	}

	if (name[0]!='/' && name[0]!='\\')
	{
   		if (fn_getdcwd(fm,fsname->drivenum,fsname->path,FN_MAXPATH)) return 1; /* error */

		for (pathpos=0; fsname->path[pathpos];)
		{
			fsname->path[pathpos]=fsname->path[pathpos+1]; /* remove starting dir separator */
			pathpos++;
		}

		pathpos--;
	}

	for (;;)
	{
		/*char ch=_f_toupper(*name++);*/
		char ch=*name++;
		ch=(char)_f_toupper(ch);

		if (!ch) break;

		if (ch=='/' || ch=='\\')
		{
			if (pathpos)
			{
				if (fsname->path[pathpos-1]=='/') return 1; /* not allowed double  */
				if (fsname->path[pathpos-1]=='\\') return 1; /* not allowed double  */

				if (pathpos>=FN_MAXPATH-2) return 1; /* path too long */
				fsname->path[pathpos++]=F_SEPARATORCHAR;
			}

			for (;namepos;)
			{
				if (s[namepos-1]!=' ') break;
				namepos--;		  /* remove end spaces */
			}

			for (a=0; a<namepos; a++)
			{
				if (pathpos>=FN_MAXPATH-2) return 1; /* path too long */
				fsname->path[pathpos++]=s[a];
			}
			namepos=0;
			continue;
		}

  		if (ch==' ' && (!namepos)) continue; /* remove start spaces */

		if (namepos>=(sizeof(s)-2)) return 1; /* name too long */
		s[namepos++]=ch;
	}

	s[namepos]=0; /* terminates it */
	fsname->path[pathpos]=0;  /* terminates it */

	for (;namepos;)
	{
   		if (s[namepos-1]!=' ') break;
   		s[namepos-1]=0; /* remove end spaces */
   		namepos--;
	}

	if (!_f_setnameext(s,fsname->filename,fsname->fileext)) return 2; /* no name */

	if (fsname->filename[0]==' ') return 1; /* cannot be */

	return 0;
}


/****************************************************************************
 *
 * _f_findfile
 *
 * internal function to finding file in directory entry
 *
 * INPUTS
 *
 * vi - volumeinfo structure, where to find file
 * name - filename
 * ext - fileextension
 * pos - where to start searching, and contains current position
 * pde - store back the directory entry pointer
 *
 * RETURNS
 *
 * 0 - if file was not found
 * 1 - if file was found
 *
 ***************************************************************************/

static int _f_findfile(F_VOLUME *vi,char *name,char *ext,F_POS *pos, F_DIRENTRY **pde)
{
	while (pos->cluster<F_CLUSTER_RESERVED)
	{
		for (;pos->sector<pos->sectorend; pos->sector++)
		{
			F_DIRENTRY *de=(F_DIRENTRY*)(vi->direntry+sizeof(F_DIRENTRY)*pos->pos);

			if (_f_getdirsector(vi,pos->sector)) return 0; /* notfound */

			for (; pos->pos<(vi->bootrecord.bytes_per_sector)/sizeof(F_DIRENTRY);de++,pos->pos++)
			{
				unsigned int b,ok;

				if (!de->name[0]) return 0;	 /* last entry, not found */
				if (de->name[0]==F_DELETED_CHAR) continue; /* deleted */
				if (de->attr & F_ATTR_VOLUME) continue;

				for (b=0,ok=1; b<sizeof(de->name); b++)
				{
					if (de->name[b]!=name[b])
					{
						ok=0;
						break;
					}
				}

				if (!ok) continue;

				for (b=0,ok=1; b<sizeof(de->ext); b++)
				{
					if (de->ext[b]!=ext[b])
					{
						ok=0;
						break;
					}
				}

				if (ok)
				{
					if (pde) *pde=de;
					return 1;
				}
			}

			pos->pos=0;
		}

 		if (!pos->cluster)
		{
			if (vi->mediatype==F_FAT32_MEDIA)
			{
				pos->cluster=vi->bootrecord.rootcluster;
			}
			else return 0; /* root directory */
		}

		{
			unsigned long nextcluster;
			if (_f_getclustervalue(vi,pos->cluster,&nextcluster)) return 0; /* not found */

	   		if (nextcluster<2 || nextcluster>=F_CLUSTER_RESERVED) return 0; /* eof */

			_f_clustertopos(vi,nextcluster,pos);
		}

	}

	return 0;
}

/***************************************************************************
 *
 * _f_cmp_wname
 *
 * function for checking a nem with wild card to original
 *
 * INPUTS
 *
 * wname - wild card name (e.g. *.* or ?a.*)
 * name - original name to check
 *
 * RETURNS
 *
 * 0 - if not equal
 * 1 - if equal or match
 *
 ***************************************************************************/

int _f_cmp_wname (const char *wname, const char *name)
{
	for (;;)
	{
		char ch=(char)_f_toupper(*name);
		char wch=(char)_f_toupper(*wname);
		if ((!ch) && (!wch)) return 1;
		if ((!wch) && (ch=='.'))
		{
			name++;
			continue;
		}
		if (wch==ch)
		{
			name++;
			wname++;
			continue;
		}
		if (wch=='?')
		{
			if (ch) name++;
			wname++;
			continue;
		}
		if (wch=='*')
		{
			wname++;
			wch=(char)_f_toupper(*wname);
			wname++;
			if (!wch) return 1;

			for (;;)
			{
				ch=(char)_f_toupper(*name);
				if (ch==wch)
				{
					name++;
					break;
				}
				if ((!ch) && (wch=='.')) break;
				if (!ch) return 0;
				name++;
			}
			continue;
		}

		return 0;
	}
}

/****************************************************************************
 *
 * _f_to_string
 *
 * Convert 8+3 name to string
 *
 * INPUTS
 *
 * r - where to convert filename
 * name - 8 chars original name
 * ext - 3 chars original extension
 *
 ***************************************************************************/

static void _f_to_string(char *r, char *name, char *ext)
{
	int i,j;

	for (i=j=0;i<F_MAXNAME && name[i]!=' ';r[j++]=name[i++])
	  ;
	r[j++]='.';
	for (i=0;i<F_MAXEXT && ext[i]!=' ';r[j++]=ext[i++])
	  ;

	r[j]=0;
}

/****************************************************************************
 *
 * _fs_findfilewc
 *
 * internal function to finding file in directory entry with wild card
 *
 * INPUTS
 *
 * vi - volumeinfo structure, where to find file
 * name - filename
 * ext - fileextension
 * pos - where to start searching, and contains current position
 * pde - store back the directory entry pointer
 *
 * RETURNS
 *
 * 0 - if file was not found
 * 1 - if file was found
 *
 ***************************************************************************/

static int _f_findfilewc(F_VOLUME *vi,char *name,char *ext,F_POS *pos, F_DIRENTRY **pde)
{
	char wname[F_MAXNAME+F_MAXEXT+2];

	_f_to_string(wname,name,ext);

	while (pos->cluster<F_CLUSTER_RESERVED)
	{
		for (;pos->sector<pos->sectorend; pos->sector++)
		{
			F_DIRENTRY *de=(F_DIRENTRY*)(vi->direntry+sizeof(F_DIRENTRY)*pos->pos);

			if (_f_getdirsector(vi,pos->sector)) return 0; /* not found */

			for (; pos->pos<(vi->bootrecord.bytes_per_sector)/sizeof(F_DIRENTRY);de++,pos->pos++)
			{
				if (!de->name[0]) return 0;						 /* empty,last */
				if (de->name[0]==F_DELETED_CHAR) continue; /* deleted */
				if (de->attr & F_ATTR_VOLUME) continue;

				{
					char oname[F_MAXNAME+F_MAXEXT+2];
					_f_to_string(oname,(char*)(de->name),(char*)(de->ext));
					if (_f_cmp_wname(wname,oname))
					{
						if (pde) *pde=de;
						return 1;
					}
				}
			}

			pos->pos=0;
		}

 		if (!pos->cluster)
		{
   			if (vi->mediatype==F_FAT32_MEDIA)
			{
				pos->cluster=vi->bootrecord.rootcluster;
			}
			else return 0; /* root directory */
		}

		{
			unsigned long nextcluster;
			if (_f_getclustervalue(vi,pos->cluster,&nextcluster)) return 0; /* not found */

	   		if (nextcluster<2 || nextcluster>=F_CLUSTER_RESERVED) return 0; /* eof */

			_f_clustertopos(vi,nextcluster,pos);
		}
	}

	return 0;
}

/****************************************************************************
 *
 * _f_getfilename
 *
 * create a complete filename from name and extension
 *
 * INPUTS
 *
 * dest - where to store filename
 * name - name of the file
 * ext - extension of the file
 *
 ***************************************************************************/

static void _f_getfilename(char *dest, char *name,char *ext)
{
	int a,len;

	for (len=a=F_MAXNAME; a; a--,len--)
	{
		if (name[a-1]!=' ') break;
	}

	for (a=0; a<len; a++) *dest++=*name++;


	for (len=a=F_MAXEXT; a; a--,len--)
	{
		if (ext[a-1]!=' ') break;
	}
	if (len) *dest++='.';

	for (a=0; a<len; a++) *dest++=*ext++;

	*dest=0; /* terminateit */
}

/****************************************************************************
 *
 * _f_findpath
 *
 * finding out if path is valid in F_NAME and
 * correct path info with absolute path (removes relatives)
 *
 * INPUTS
 *
 * vi - volumeinfo
 * fsname - filled structure with path,drive
 * pos - where to start searching, and contains current position
 *
 * RETURNS
 *
 * 0 - if path was not found or invalid
 * 1 - if path was found
 *
 ***************************************************************************/

static int _f_findpath(F_VOLUME *vi,F_NAME *fsname,F_POS *pos)
{
	char *path=fsname->path;
	char *mpath=path;
	F_DIRENTRY *de;

	_f_clustertopos(vi,0,pos);

	for (;*path;)
	{
		char name[F_MAXNAME];
		char ext[F_MAXEXT];

		int len=_f_setnameext(path,name,ext);

		if (pos->cluster==0 && len==1 && name[0]=='.')
		{
			_f_clustertopos(vi,0,pos);
		}
		else
		{
			if (!_f_findfile(vi,name,ext,pos,&de)) return 0;
			if (!(de->attr & F_ATTR_DIR ) ) return 0;

			_f_clustertopos(vi,_f_getdecluster(vi,de),pos);
		}


		if (name[0]=='.')
		{
			if (len==1)
			{
				path+=len;

				if (!(*path))
				{
					if (mpath!=fsname->path) mpath--; /* if we are now at the top */
					break;
				}
				path++;
				continue;
			}

			if (name[1]!='.') return 0; /* invalid name */
			if (len!=2) return 0; /* invalid name ! */

			path+=len;

			if (mpath==fsname->path) return 0; /* we are in the top */

			mpath--; /* no on separator */
			for (;;)
			{
				if (mpath==fsname->path) break; /* we are now at the top */
				mpath--;
				if (*mpath==F_SEPARATORCHAR)
				{
					mpath++;
					break;
				}
			}

			if (!(*path))
			{
				if (mpath!=fsname->path) mpath--; /* if we are now at the top */
				break;
			}
			path++;
			continue;

		}
		else {
			if (path==mpath)
			{							/* if no was dots just step */
				path+=len;
				mpath+=len;
			}
			else
			{
				int a;
				for (a=0; a<len;a++) *mpath++=*path++;	 /* copy if in different pos */
			}
		}

		if (!(*path)) break;
		path++;
		*mpath++=F_SEPARATORCHAR; /* add separator */
	}

	*mpath=0; /* terminate it */

	return 1;
}

/****************************************************************************
 *
 * fn_getdcwd
 *
 * getting a drive current working directory
 *
 * INPUTS
 *
 * fm - multi structure pointer
 * drivenum - drive number of which drive current folder needed
 * buffer - where to store current working folder
 * maxlen - buffer length (possible size is FN_MAXPATH)
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/

int fn_getdcwd(F_MULTI *fm,int drivenum, char *buffer, int maxlen )
{
	F_VOLUME *vi;
	int a;
	char *cwd;
	int ret;

	if (drivenum < 0 || drivenum>=FN_MAXVOLUME) return F_SETLASTERROR(F_ERR_INVALIDDRIVE);

	vi=&f_filesystem.volumes[drivenum];

	cwd=fm->f_vols[drivenum].cwd;

	ret=_f_mutex_get(fm,vi);
	if (ret) return F_SETLASTERROR(ret);

	if (_f_checkstatus(vi,0))
	{
		fm->f_vols[drivenum].cwd[0]=0; /* remove cwd */
	}

	if (!maxlen) return F_SETLASTERROR(F_NO_ERROR);

	maxlen--;	/* need for termination */

	if (maxlen)
	{
	  *buffer++=F_SEPARATORCHAR;
	  maxlen--;
	}

	for (a=0; a<maxlen; a++)
	{
		char ch=cwd[a];
		buffer[a]=ch;
		if (!ch) break;
	}

	buffer[a]=0;	/* add terminator at the end */

	return F_SETLASTERROR(F_NO_ERROR);
}

/****************************************************************************
 *
 * fn_getcwd
 *
 * getting a current working directory of current drive
 *
 * INPUTS
 *
 * fm - multi structure pointer
 * buffer - where to store current working folder
 * maxlen - buffer length (possible size is FN_MAXPATH)
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/

int fn_getcwd(F_MULTI *fm,char *buffer, int maxlen )
{
	F_VOLUME *vi;
	int a;
	char *cwd;
	int ret;
	int drivenum=fn_getdrive(fm);

	if (drivenum < 0 || drivenum>=FN_MAXVOLUME) return F_SETLASTERROR(F_ERR_INVALIDDRIVE);

	vi=&f_filesystem.volumes[drivenum];

	cwd=fm->f_vols[drivenum].cwd;

	ret=_f_mutex_get(fm,vi);
	if (ret) return F_SETLASTERROR(ret);

	if (_f_checkstatus(vi,0))
	{
		fm->f_vols[drivenum].cwd[0]=0; /* remove cwd */
	}

	if (!maxlen) return F_SETLASTERROR(F_NO_ERROR);

	maxlen--;	/* need for termination */

	if (maxlen)
	{
	  *buffer++=F_SEPARATORCHAR;
	  maxlen--;
	}

	for (a=0; a<maxlen; a++)
	{
		char ch=cwd[a];
		buffer[a]=ch;
		if (!ch) break;
	}

	buffer[a]=0;	/* add terminator at the end */

	return F_SETLASTERROR(F_NO_ERROR);
}

/****************************************************************************
 *
 * fn_findfirst
 *
 * find a file(s) or directory(s) in directory
 *
 * INPUTS
 *
 * fm - multi structure pointer
 * filename - filename (with or without wildcards)
 * find - where to store found file information
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/

int fn_findfirst(F_MULTI *fm,const char *filename,FN_FIND *find)
{
	F_VOLUME *vi;
	int ret;

	if (_f_setfsname(fm,filename,&find->findfsname)) return F_SETLASTERROR(F_ERR_INVALIDNAME); /* invalid name */
	if (_f_checkname(find->findfsname.filename,find->findfsname.fileext)) return F_SETLASTERROR(F_ERR_INVALIDNAME);/* invalid name, wildcard is ok */

	ret=_f_getvolume(fm,find->findfsname.drivenum,&vi);
	if (ret) return F_SETLASTERROR(ret);

	if (!_f_findpath(vi,&find->findfsname,&find->pos)) return F_SETLASTERROR(F_ERR_INVALIDDIR); /* search for path */

	ret=fn_findnext(fm,find);
	return F_SETLASTERROR(ret);
}

/****************************************************************************
 *
 * fn_findnext
 *
 * find further file(s) or directory(s) in directory
 *
 * INPUTS
 *
 * fm - multi structure pointer
 * find - where to store found file information (findfirst should call 1st)
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/

int fn_findnext(F_MULTI *fm,FN_FIND *find)
{
	F_VOLUME *vi;
	F_DIRENTRY *de;
	int a;
	int ret;
	unsigned long ofsize;

	ret=_f_getvolume(fm,find->findfsname.drivenum,&vi);
	if (ret) return F_SETLASTERROR(ret);

	if (!_f_findfilewc(vi,find->findfsname.filename,find->findfsname.fileext,&find->pos,&de)) return F_SETLASTERROR(F_ERR_NOTFOUND);

	for (a=0;a<F_MAXNAME; a++) find->name[a]=de->name[a];
	for (a=0;a<F_MAXEXT; a++) find->ext[a]=de->ext[a];

	_f_getfilename(find->filename,(char*)de->name,(char*)de->ext);

	find->attr=de->attr;
	find->cdate=_f_get16bitl(de->cdate);
	find->ctime=_f_get16bitl(de->ctime);
	find->filesize=_f_get32bitl(de->filesize);
	find->cluster=_f_getdecluster(vi,de);

	if (_f_findopensize(&ofsize,find->findfsname.drivenum,&find->pos))
	{
		find->filesize=ofsize;
	}

	find->pos.pos++; /* goto next position */

	return F_SETLASTERROR(F_NO_ERROR);
}

/****************************************************************************
 *
 * fn_chdir
 *
 * change current working directory
 *
 * INPUTS
 *
 * fm - multi structure pointer
 * dirname - new working directory name
 *
 * RETURNS
 *
 * 0 - if successfully
 * other - if any error
 *
 ***************************************************************************/

int fn_chdir(F_MULTI *fm,const char *dirname)
{
	F_POS pos;
	F_VOLUME *vi;
	F_NAME fsname;
	int len;
	int a;
	int ret;

	ret=_f_setfsname(fm,dirname,&fsname);

	if (ret==1) return F_SETLASTERROR(F_ERR_INVALIDNAME); /* invalid name */
	if (_f_checknamewc(fsname.filename,fsname.fileext)) return F_SETLASTERROR(F_ERR_INVALIDNAME);/* invalid name */

	ret=_f_getvolume(fm,fsname.drivenum,&vi);
	if (ret) return F_SETLASTERROR(ret);

	for (len=0;fsname.path[len];len++)
	  ;
	if (len && (fsname.filename[0]!=32 || fsname.fileext[0]!=32)) fsname.path[len++]=F_SEPARATORCHAR;

	_f_getfilename(fsname.path+len,fsname.filename,fsname.fileext);

	if (!(_f_findpath(vi,&fsname,&pos))) return F_SETLASTERROR(F_ERR_NOTFOUND);

	for (a=0; a<FN_MAXPATH;a++) vi->cwd[a]=fsname.path[a]; /* safety uses of vi->cwd */

	return F_SETLASTERROR(F_NO_ERROR);
}

/****************************************************************************
 *
 * fn_rename
 *
 * Rename file or directory
 *
 * INPUTS
 *
 * fm - multi structure pointer
 * filename - filename or directory name (with or without path)
 * newname - new name of the file or directory (without path)
 *
 * RETURNS
 *
 * 0 - if successfully
 * other - if any error
 *
 ***************************************************************************/

int fn_rename(F_MULTI *fm,const char *filename, const char *newname)
{
	F_POS posdir;
	F_POS pos;
	F_VOLUME *vi;
	F_DIRENTRY *de;
	F_NAME fsname;
	char name[F_MAXNAME];
	char ext[F_MAXEXT];
	int a;
	int ret;

	if (_f_setfsname(fm,filename,&fsname)) return F_SETLASTERROR(F_ERR_INVALIDNAME); /* invalid name */
	if (_f_checknamewc(fsname.filename,fsname.fileext)) return F_SETLASTERROR(F_ERR_INVALIDNAME);/* invalid name */

	ret=_f_getvolume(fm,fsname.drivenum,&vi);
	if (ret) return F_SETLASTERROR(ret);

	if (!(_f_findpath(vi,&fsname,&posdir))) return F_SETLASTERROR(F_ERR_INVALIDDIR);

	pos.cluster = posdir.cluster;
	pos.sector	= posdir.sector;
	pos.sectorend=posdir.sectorend;
	pos.pos=posdir.pos;

	if (fsname.filename[0]=='.') return F_SETLASTERROR(F_ERR_INVALIDNAME);

	if (!_f_findfile(vi,fsname.filename,fsname.fileext,&pos,&de)) return F_SETLASTERROR(F_ERR_NOTFOUND);

	if (de->attr & F_ATTR_READONLY) return F_SETLASTERROR(F_ERR_ACCESSDENIED);      /* readonly */

	if (_f_checklocked(fsname.drivenum,&pos)) return F_SETLASTERROR(F_ERR_LOCKED);

	if (_f_checknameinv(newname,-1)) return F_SETLASTERROR(F_ERR_INVALIDNAME);/* invalid name */
	if (!_f_setnameext(newname,name,ext)) return F_SETLASTERROR(F_ERR_INVALIDNAME); /* no length invalid name */
	if (_f_checknamewc(name,ext)) return F_SETLASTERROR(F_ERR_INVALIDNAME);/* invalid name */

	if (name[0]=='.') return F_SETLASTERROR(F_ERR_INVALIDNAME);
	if (name[0]==' ') return F_SETLASTERROR(F_ERR_INVALIDNAME);

	if (_f_findfile(vi,name,ext,&posdir,0)) return F_SETLASTERROR(F_ERR_DUPLICATED);

	ret=_f_getdirsector(vi,pos.sector);
	if (ret) return F_SETLASTERROR(ret);

	for (a=0;a<F_MAXNAME; a++) de->name[a]=name[a];
	for (a=0;a<F_MAXEXT; a++) de->ext[a]=ext[a];

	ret=_f_writedirsector(vi);
	return F_SETLASTERROR(ret);
}

/****************************************************************************
 *
 * fn_move
 *
 * move file or directory
 *
 * INPUTS
 *
 * fm - multi structure pointer
 * filename - filename or directory name (with or without path)
 * newname - new name of the file or directory (with path)
 *
 * RETURNS
 *
 * 0 - if successfully
 * other - if any error
 *
 ***************************************************************************/

int fn_move(F_MULTI *fm,const char *filename, const char *newname)
{
	F_POS posdir;
	F_POS posdir2;
	F_POS pos;
	F_POS pos2;
	F_VOLUME *vi;
	F_DIRENTRY *de;
	F_DIRENTRY *de2;
	F_NAME fsname;
	F_NAME fsnamenew;
	int ret;
	F_DIRENTRY oldde;

	if (_f_setfsname(fm,filename,&fsname)) return F_SETLASTERROR(F_ERR_INVALIDNAME); /* invalid name */
	if (_f_checknamewc(fsname.filename,fsname.fileext)) return F_SETLASTERROR(F_ERR_INVALIDNAME);/* invalid name */

	ret=_f_getvolume(fm,fsname.drivenum,&vi);
	if (ret) return F_SETLASTERROR(ret);

	if (!(_f_findpath(vi,&fsname,&posdir))) return F_SETLASTERROR(F_ERR_INVALIDDIR);

	pos.cluster = posdir.cluster;
	pos.sector	= posdir.sector;
	pos.sectorend=posdir.sectorend;
	pos.pos=posdir.pos;

	if (fsname.filename[0]=='.') return F_SETLASTERROR(F_ERR_INVALIDNAME);

	if (!_f_findfile(vi,fsname.filename,fsname.fileext,&pos,&de)) return F_SETLASTERROR(F_ERR_NOTFOUND);

	if (de->attr & F_ATTR_READONLY) return F_SETLASTERROR(F_ERR_ACCESSDENIED);      /* readonly */

	(void)_memcpy(&oldde,de,sizeof(F_DIRENTRY));

	if (de->attr & F_ATTR_DIR)
	{
		unsigned long cluster=_f_getdecluster(vi,&oldde);

		if (_f_setfsname(fm,newname,&fsnamenew)) return F_SETLASTERROR(F_ERR_INVALIDNAME); /* invalid name */
		if (_f_checknamewc(fsnamenew.filename,fsnamenew.fileext)) return F_SETLASTERROR(F_ERR_INVALIDNAME);/* invalid name */

		if (fsnamenew.drivenum != fsname.drivenum) return F_SETLASTERROR(F_ERR_INVALIDDRIVE); /* can't move on different drive */

		if (!(_f_findpath(vi,&fsnamenew,&posdir2))) return F_SETLASTERROR(F_ERR_INVALIDDIR);

		pos2.cluster = posdir2.cluster;
		pos2.sector	= posdir2.sector;
		pos2.sectorend=posdir2.sectorend;
		pos2.pos=posdir2.pos;

		if (fsnamenew.filename[0]=='.') return F_SETLASTERROR(F_ERR_INVALIDNAME);

		if (_f_findfile(vi,fsnamenew.filename,fsnamenew.fileext,&pos2,&de2)) return F_SETLASTERROR(F_ERR_DUPLICATED);

		{ /* check if target directory is not in original's subdirectory */
			unsigned long cl=pos2.cluster;
			F_POS p;
			F_DIRENTRY *de3=(F_DIRENTRY*)(vi->direntry+sizeof(F_DIRENTRY)*1);
			for(;;)
			{
				if (cl==cluster) return F_SETLASTERROR(F_ERR_INVALIDDIR);

				_f_clustertopos(vi,cl,&p);

				ret=_f_getdirsector(vi,p.sector);
				if (ret) return F_SETLASTERROR(ret);

				if (de3->name[0]!='.' || de3->name[1]!='.' || de3->name[2]!=32) break;

				cl=_f_getdecluster(vi,de3);
			}
		}

		ret=_f_addentry(vi,&fsnamenew,&posdir2,&de2);
		if (ret) return F_SETLASTERROR(ret); /* couldnt be added */

		(void)_memcpy(oldde.name,de2->name,8);		 /* update name and ext */
		(void)_memcpy(oldde.ext,de2->ext,3);
		(void)_memcpy((void *)de2,&oldde,sizeof(F_DIRENTRY)); /* restore all properties */

		ret=_f_writedirsector(vi);
		if (ret) return F_SETLASTERROR(ret); /* couldnt be written */

		ret=_f_getdirsector(vi,posdir.sector);
		if (ret) return F_SETLASTERROR(ret);

		de->name[0]=F_DELETED_CHAR;	/* removes oldname */

		ret=_f_writedirsector(vi);

		cluster=_f_getdecluster(vi,&oldde); /* set .. to current */
		_f_clustertopos(vi,cluster,&posdir);

		ret=_f_getdirsector(vi,posdir.sector);
		if (ret) return F_SETLASTERROR(ret);

		de=(F_DIRENTRY*)(vi->direntry+sizeof(F_DIRENTRY)*1);

		if (de->name[0]!='.' || de->name[1]!='.') return F_SETLASTERROR(F_ERR_INVALIDDIR);

		_f_setdecluster(vi,de,posdir2.cluster);

		ret=_f_writedirsector(vi);
		return F_SETLASTERROR(ret);
	}
	else
	{
		if (_f_checklocked(fsname.drivenum,&pos)) return F_SETLASTERROR(F_ERR_LOCKED);

		if (_f_setfsname(fm,newname,&fsnamenew)) return F_SETLASTERROR(F_ERR_INVALIDNAME); /* invalid name */
		if (_f_checknamewc(fsnamenew.filename,fsnamenew.fileext)) return F_SETLASTERROR(F_ERR_INVALIDNAME);/* invalid name */

		if (fsnamenew.drivenum != fsname.drivenum) return F_SETLASTERROR(F_ERR_INVALIDDRIVE); /* can't move on different drive */

		if (!(_f_findpath(vi,&fsnamenew,&posdir2))) return F_SETLASTERROR(F_ERR_INVALIDDIR);

		pos2.cluster = posdir2.cluster;
		pos2.sector	= posdir2.sector;
		pos2.sectorend=posdir2.sectorend;
		pos2.pos=posdir2.pos;

		if (fsname.filename[0]=='.') return F_SETLASTERROR(F_ERR_INVALIDNAME);

		if (_f_findfile(vi,fsnamenew.filename,fsnamenew.fileext,&pos2,&de2)) return F_SETLASTERROR(F_ERR_DUPLICATED);

		ret=_f_addentry(vi,&fsnamenew,&posdir2,&de2);
		if (ret) return F_SETLASTERROR(ret); /* couldnt be added */

		(void)_memcpy(oldde.name,de2->name,8);		 /* update name and ext */
		(void)_memcpy(oldde.ext,de2->ext,3);
		(void)_memcpy((void *)de2,&oldde,sizeof(F_DIRENTRY)); /* restore all properties */

		ret=_f_writedirsector(vi);
		if (ret) return F_SETLASTERROR(ret); /* couldnt be written */

		ret=_f_getdirsector(vi,posdir.sector);
		if (ret) return F_SETLASTERROR(ret);

		de->name[0]=F_DELETED_CHAR;	/* removes oldname */

		ret=_f_writedirsector(vi);
		return F_SETLASTERROR(ret);
	}
}

/****************************************************************************
 *
 * _f_getintfile
 *
 * allocate an internal file handler
 *
 * RETURNS
 *
 * internal file pointer if successful
 * zero if there was no free file pointer (see F_MAXFILES)
 *
 ***************************************************************************/

static FN_FILEINT *_f_getintfile(void)
{
	int a;

	if (f_mutex_get(&fat_gmutex)) return 0;

	for (a=0; a<F_MAXFILES; a++)
	{
		if (f_filesystem.files[a].mode==FN_FILE_CLOSE)
		{
			f_filesystem.files[a].file.reference=(void *)((f_filesystem.drvbldnum<<F_MAXFILES_SHIFT)+a);

			f_filesystem.files[a].WrDataCache.size=0;
			f_filesystem.files[a].WrDataCache.ptr=NULL;

			f_filesystem.files[a].mode=FN_FILE_LOCKED; /* for avoiding reentrancy */

			(void)f_mutex_put(&fat_gmutex);

			return &f_filesystem.files[a];
		}
	}

	(void)f_mutex_put(&fat_gmutex);

	return 0;		/* no more file could be opened  */
}

/****************************************************************************
 *
 * _f_addentry
 *
 * Add a new directory entry into driectory list
 *
 * INPUTS
 *
 * vi - volumeinfo
 * fs_name - filled structure what to add into directory list
 * pos - where directory cluster chains starts
 * pde - F_DIRENTRY pointer where to store the entry where it was added
 *
 * RETURNS
 *
 * 0 - if successfully added
 * other - if any error (see FS_xxx errorcodes)
 *
 ***************************************************************************/

int _f_addentry(F_VOLUME *vi,F_NAME *fsname,F_POS *pos,F_DIRENTRY **pde)
{
	int ret;

	if (!fsname->filename[0]) return F_ERR_INVALIDNAME;
	if (fsname->filename[0]=='.') return F_ERR_INVALIDNAME;

	while (pos->cluster<F_CLUSTER_RESERVED)
	{
		for (;pos->sector<pos->sectorend; pos->sector++)
		{
			F_DIRENTRY *de=(F_DIRENTRY*)(vi->direntry+sizeof(F_DIRENTRY)*pos->pos);

			ret=_f_getdirsector(vi,pos->sector);
			if (ret) return ret;

			for (; pos->pos<(vi->bootrecord.bytes_per_sector)/sizeof(F_DIRENTRY);de++,pos->pos++)
			{
				if ((!de->name[0]) || (de->name[0]==F_DELETED_CHAR))
				{
					_f_initentry(de,fsname->filename,fsname->fileext);

					_f_set16bitl(de->crtdate,f_getdate());  /* if there is realtime clock then creation date could be set from */
					_f_set16bitl(de->crttime,f_gettime());  /* if there is realtime clock then creation time could be set from */
					_f_set16bitl(de->lastaccessdate,f_getdate());  /* if there is realtime clock then creation date could be set from */

					if (pde) *pde=de;
					return F_NO_ERROR;
				}
			}

			pos->pos=0;
		}

		if (!pos->cluster)
		{
			if (vi->mediatype==F_FAT32_MEDIA)
			{
				pos->cluster=vi->bootrecord.rootcluster;
			}
			else return F_ERR_NOMOREENTRY; /* root directory, cannot continue */
		}

		{
			unsigned long cluster;

			ret=_f_getclustervalue(vi,pos->cluster,&cluster); /* try to get next cluster */
			if (ret) return ret;

			if (cluster<F_CLUSTER_RESERVED)
			{
				_f_clustertopos(vi,cluster,pos);
			}
			else
			{
				ret=_f_alloccluster(vi,&cluster);	/* get a new one */
				if (ret) return ret;

				if (cluster<F_CLUSTER_RESERVED)
				{
					FN_FILEINT *f=_f_getintfile();
					if (!f) return F_ERR_NOMOREENTRY;

					_f_clustertopos(vi,cluster,&f->pos);


					(void)_memset((void *)(f->data),0,(vi->bootrecord.bytes_per_sector));
					while(f->pos.sector<f->pos.sectorend)
					{
						while ((ret=_f_writesector(vi,f->data,f->pos.sector,1))!=F_NO_ERROR)
						{
							if (vi->state!=F_STATE_WORKING) 
							{
								f->mode=FN_FILE_CLOSE;
								return ret; /* check if still working volume */
							}

							ret=_f_dobadblock(vi,f);
							if (ret) 
							{
								f->mode=FN_FILE_CLOSE;
								return ret;
							}
						}
						f->pos.sector++;
					}

					ret=_f_setclustervalue(vi,f->pos.cluster,F_CLUSTER_LAST);
					if (ret) 
					{
						f->mode=FN_FILE_CLOSE; /* release fileentry */
						return ret;
					}

					ret=_f_setclustervalue(vi,pos->cluster,f->pos.cluster);
					if (ret) 
					{
						f->mode=FN_FILE_CLOSE; /* release fileentry */
						return ret;
					}

					ret=_f_writefatsector(vi);
					if (ret) 
					{
						f->mode=FN_FILE_CLOSE; /* release fileentry */
						return ret;
					}

					_f_clustertopos(vi,f->pos.cluster,pos);

					f->mode=FN_FILE_CLOSE; /* release fileentry */
				}
				else return F_ERR_NOMOREENTRY;
			}
		}
	}

	return F_ERR_NOMOREENTRY;
}


/****************************************************************************
 *
 * fn_mkdir
 *
 * making a new directory
 *
 * INPUTS
 *
 * fm - multi structure pointer
 * dirname - new directory name
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/

int fn_mkdir(F_MULTI *fm,const char *dirname)
{
	FN_FILEINT *f;
	F_POS posdir;
	F_POS pos;
	F_DIRENTRY *de;
	F_NAME fsname;
	F_VOLUME *vi;
	unsigned long cluster;
	unsigned int a;
	int ret;

	if (_f_setfsname(fm,dirname,&fsname)) return F_SETLASTERROR(F_ERR_INVALIDNAME); /* invalid name */
	if (_f_checknamewc(fsname.filename,fsname.fileext)) return F_SETLASTERROR(F_ERR_INVALIDNAME);/* invalid name */

	ret=_f_getvolume(fm,fsname.drivenum,&vi);
	if (ret) return F_SETLASTERROR(ret);

	if (!_f_findpath(vi,&fsname,&posdir)) return F_SETLASTERROR(F_ERR_INVALIDDIR);

	pos.cluster = posdir.cluster;
	pos.sector	= posdir.sector;
	pos.sectorend=posdir.sectorend;
	pos.pos=posdir.pos;

	if (fsname.filename[0]=='.') return F_SETLASTERROR(F_ERR_INVALIDNAME);
	if (_f_findfile(vi,fsname.filename,fsname.fileext,&pos,&de)) return F_SETLASTERROR(F_ERR_DUPLICATED);

	pos.cluster = posdir.cluster;
	pos.sector	= posdir.sector;
	pos.sectorend=posdir.sectorend;
	pos.pos=posdir.pos;

	ret=_f_alloccluster(vi,&cluster);
	if (ret) return F_SETLASTERROR(ret);

	ret=_f_addentry(vi,&fsname,&pos,&de);
	if (ret) return F_SETLASTERROR(ret); /* couldnt be added */

	de->attr |= F_ATTR_DIR;		/* set as directory */

/*  */

	f=_f_getintfile();
	if (!f) return F_SETLASTERROR(F_ERR_NOMOREENTRY);

	_f_clustertopos(vi,cluster,&f->pos);

	_f_setdecluster(vi,de,cluster); /* new dir */

/*  */

	de=(F_DIRENTRY *)(f->data);

	_f_initentry(de,".       ","   ");
	de->attr = F_ATTR_DIR;		/* set as directory */
	_f_setdecluster(vi,de,cluster);	 /* current */
	de++;

	_f_initentry(de,"..      ","   ");
	de->attr = F_ATTR_DIR;		/* set as directory */
	_f_setdecluster(vi,de,posdir.cluster); /* parent */
	de++;

	for (a=2; a<(vi->bootrecord.bytes_per_sector)/sizeof(F_DIRENTRY); a++,de++)
	{
		(void)_memset((void *)de,0,sizeof(F_DIRENTRY));
	}

	ret=_f_writesector(vi,f->data,f->pos.sector,1);
	if (ret) 
	{
		f->mode=FN_FILE_CLOSE; /* release fileentry */
		return F_SETLASTERROR(ret);
	}

	f->pos.sector++;

	de=(F_DIRENTRY *)(f->data);

	for (a=0; a<2; a++,de++)
	{
		(void)_memset((void *)de,0,sizeof(F_DIRENTRY));
	}

	while(f->pos.sector<f->pos.sectorend)
	{
		de=(F_DIRENTRY *)(f->data);

		while ((ret=_f_writesector(vi,f->data,f->pos.sector,1))!=F_NO_ERROR)
		{
		 	if (vi->state!=F_STATE_WORKING) 
			{
				f->mode=FN_FILE_CLOSE; /* release fileentry */
				return F_SETLASTERROR(ret); /* check if still working volume */
			}
		 	ret=_f_dobadblock(vi,f);
		 	if (ret) 
			{
				f->mode=FN_FILE_CLOSE; /* release fileentry */
				return F_SETLASTERROR(ret);
			}
		}


		f->pos.sector++;
	}

	ret=_f_setclustervalue(vi,f->pos.cluster,F_CLUSTER_LAST);
	if (ret) 
	{
		f->mode=FN_FILE_CLOSE; /* release fileentry */
		return F_SETLASTERROR(ret);
	}

	f->mode=FN_FILE_CLOSE; /* release fileentry */

	ret=_f_writefatsector(vi);
	if (ret) return F_SETLASTERROR(ret);

#ifdef FATCACHE_ENABLE
	ret=_f_fatcache_flush(vi);
	if (ret) return F_SETLASTERROR(ret);
#endif

	ret=_f_writedirsector(vi); /* writing  direntry */
	return F_SETLASTERROR(ret);
}

/****************************************************************************
 *
 * fn_filelength
 *
 * Get a file length
 *
 * INPUTS
 *
 * fm - multi structure pointer
 * filename - file whose length is needed
 *
 * RETURNS
 *
 * length of the file or -1 if any error
 *
 ***************************************************************************/

long fn_filelength(F_MULTI *fm,const char *filename)
{
	F_VOLUME *vi;
	F_POS pos;
	F_DIRENTRY *de;
	F_NAME fsname;
	unsigned long ofsize;

	if (_f_setfsname(fm,filename,&fsname)) 
	{
	 	F_SETLASTERROR_NORET(F_ERR_INVALIDNAME);
		return -1; /* invalid name */
	}
	if (_f_checknamewc(fsname.filename,fsname.fileext)) 
	{
	 	F_SETLASTERROR_NORET(F_ERR_INVALIDNAME);
		return -1;/* invalid name */
	}

	if (_f_getvolume(fm,fsname.drivenum,&vi)) 
	{
	 	F_SETLASTERROR_NORET(F_ERR_INVALIDDRIVE);
		return -1; /* can't get the volume */
	}

	if (!_f_findpath(vi,&fsname,&pos)) 
	{
	 	F_SETLASTERROR_NORET(F_ERR_INVALIDDIR);
		return -1;
	}
	if (!_f_findfile(vi,fsname.filename,fsname.fileext,&pos,&de)) 
	{
	 	F_SETLASTERROR_NORET(F_ERR_NOTFOUND);
		return -1;
	}

	if (de->attr & F_ATTR_DIR) 
	{
		F_SETLASTERROR_NORET(F_ERR_ACCESSDENIED);
		return -1;				/* directory */
	}

	if (_f_findopensize(&ofsize,fsname.drivenum,&pos))
	{
		F_SETLASTERROR_NORET(F_NO_ERROR);
		return (long)ofsize;
	}

	F_SETLASTERROR_NORET(F_NO_ERROR);
	return (long)_f_get32bitl(de->filesize);
}

/****************************************************************************
 *
 * fn_open
 *
 * open a file for reading/writing/appending
 *
 * INPUTS
 *
 * fm - multi structure pointer
 * filename - which file need to be opened
 * mode - string how to open ("r"-read, "w"-write, "w+"-overwrite, "a"-append
 *
 * RETURNS
 *
 * FN_FILE pointer if successfully
 * 0 - if any error
 *
 ***************************************************************************/

FN_FILE *fn_open(F_MULTI *fm,const char *filename,const char *mode)
{
	F_DIRENTRY *de;
	F_NAME fsname;
	F_VOLUME *vi;
	char m_mode=FN_FILE_CLOSE;
	FN_FILEINT *f=0;
	FN_FILEINT *fapp=0;
	int ret;

	if (mode[0]=='r')
	{
		if (mode[1]==0) m_mode=FN_FILE_RD;
		else if (mode[1]=='+')
		{
			if (mode[2]==0) m_mode=FN_FILE_RDP;
		}
	}
	else if (mode[0]=='w')
	{
		if (mode[1]==0) m_mode=FN_FILE_WR;
		else if (mode[1]=='+')
		{
			if (mode[2]==0) m_mode=FN_FILE_WRP;
		}
	}
	else if (mode[0]=='a')
	{
		if (mode[1]==0) m_mode=FN_FILE_A;
		else if (mode[1]=='+')
		{
			if (mode[2]==0) m_mode=FN_FILE_AP;
		}
	}

	if (_f_setfsname(fm,filename,&fsname)) 
	{
		F_SETLASTERROR_NORET(F_ERR_INVALIDNAME);
		return 0; /* invalid name */
	}

	if (_f_checknamewc(fsname.filename,fsname.fileext)) 
	{
		F_SETLASTERROR_NORET(F_ERR_INVALIDNAME);
		return 0;/* invalid name */
	}

	if (_f_getvolume(fm,fsname.drivenum,&vi)) 
	{
		F_SETLASTERROR_NORET(F_ERR_INVALIDDRIVE);
		return 0; /* cant open any */
	}

	f=_f_getintfile();
	if (!f) 
	{
		F_SETLASTERROR_NORET(F_ERR_NOMOREENTRY);
		return 0; /* no more file could be opened  */
	}

	f->drivenum=fsname.drivenum;
	f->relpos=0;
	f->abspos=0;
	f->filesize=0;
	f->pos.prevcluster=0;
	f->modified=0;
	f->state=0;
	f->syncfile=0;

	if (fsname.filename[0]=='.') 
	{
		F_SETLASTERROR_NORET(F_ERR_INVALIDNAME);
		f->mode=FN_FILE_CLOSE; /* release fileentry */
		return 0;
	}

	switch (m_mode)
	{
		case FN_FILE_RDP: /* r+ */
		case FN_FILE_RD: /* r */

			if (!_f_findpath(vi,&fsname,&f->dirpos)) 
			{
				F_SETLASTERROR_NORET(F_ERR_INVALIDDIR);
				f->mode=FN_FILE_CLOSE; /* release fileentry */
				return 0;
			}
			if (!_f_findfile(vi,fsname.filename,fsname.fileext,&f->dirpos,&de)) 
			{
				F_SETLASTERROR_NORET(F_ERR_NOTFOUND);
				f->mode=FN_FILE_CLOSE; /* release fileentry */
				return 0;
			}

   			if (de->attr & F_ATTR_DIR) 
			{
				F_SETLASTERROR_NORET(F_ERR_INVALIDDIR);
				f->mode=FN_FILE_CLOSE; /* release fileentry */
				return 0;		   /* directory */
			}

			if (m_mode==FN_FILE_RDP)
			{
				if (de->attr & F_ATTR_READONLY) 
				{
					F_SETLASTERROR_NORET(F_ERR_ACCESSDENIED);
					f->mode=FN_FILE_CLOSE; /* release fileentry */
					return 0;
				}
				if (_f_checklocked(fsname.drivenum,&f->dirpos)) 
				{
					F_SETLASTERROR_NORET(F_ERR_LOCKED);
					f->mode=FN_FILE_CLOSE; /* release fileentry */
					return 0; /* locked */
				}
			}
			else
			{
				if (_f_checkreadlocked(vi,fsname.drivenum,&f->dirpos,&fapp)) 
				{
					F_SETLASTERROR_NORET(F_ERR_LOCKED);
					f->mode=FN_FILE_CLOSE; /* release fileentry */
					return 0; /* locked for read */
				}
			}

			if (!fapp)	/* there is no same file opened for append */
			{
				f->startcluster=_f_getdecluster(vi,de);

				if (f->startcluster)
				{
					_f_clustertopos(vi,f->startcluster,&f->pos);

					f->filesize=_f_get32bitl(de->filesize);

#if F_MAXSEEKPOS
					_fn_initseekdivisor(f,vi);
#endif
					f->abspos=(unsigned long)-(vi->bootrecord.bytes_per_sector); /* forcing seek to read 1st sector! abspos=0; */
					ret=_f_fseek(vi,fm,f,0);
					if (ret)
					{
						F_SETLASTERROR_NORET(ret);
						f->mode=FN_FILE_CLOSE; /* release fileentry */
						return 0;
					}
				}
				else
				{
					f->pos.sector=0;
					f->pos.sectorend=0;
#if F_MAXSEEKPOS
					_fn_initseekdivisor(f,vi);
#endif
				}
			}
			else
			{
				f->pos.sector=0;
				f->pos.sectorend=0;

				f->filesize=fapp->filesize; /* set all size */
				f->startcluster=0;
				f->syncfile=fapp;

#if F_MAXSEEKPOS
				_fn_initseekdivisor(f,vi);
#endif

				if (fapp->startcluster)
				{
					f->startcluster=fapp->startcluster;
					_f_clustertopos(vi,f->startcluster,&f->pos);
					f->abspos=(unsigned long)-(vi->bootrecord.bytes_per_sector); /* forcing seek to read 1st sector! abspos=0; */
					ret=_f_fseek(vi,fm,f,0);
					if (ret)
					{
						F_SETLASTERROR_NORET(ret);
						f->mode=FN_FILE_CLOSE; /* release fileentry */
						return 0;
					}
				}
			}

#if F_UPDATELASTACCESSDATE
			if (m_mode==FN_FILE_RD) /* read mode access time updating, r+ is updated from close */
			{
				if (_f_get16bitl(de->lastaccessdate)!=f_getdate())
				{
					_f_set16bitl(de->lastaccessdate,f_getdate());  /* if there is realtime clock then lastaccess date could be set from */

					ret=_f_writedirsector(vi);
					if (ret)
					{
						F_SETLASTERROR_NORET(ret);
						f->mode=FN_FILE_CLOSE; /* release fileentry */
						return 0; /* Can't be updated */
					}
				}
			}
#endif

   			break;

   		case FN_FILE_AP:
   		case FN_FILE_A: /* a */

			if (!_f_findpath(vi,&fsname,&f->dirpos)) 
			{
				F_SETLASTERROR_NORET(F_ERR_INVALIDDIR);
				f->mode=FN_FILE_CLOSE; /* release fileentry */
				return 0;
			}

			f->pos.cluster=f->dirpos.cluster;
			f->pos.pos=f->dirpos.pos;
			f->pos.sector=f->dirpos.sector;
			f->pos.sectorend=f->dirpos.sectorend;

			if (_f_findfile(vi,fsname.filename,fsname.fileext,&f->dirpos,&de))
			{
	   			if (de->attr & F_ATTR_DIR) 
				{
					F_SETLASTERROR_NORET(F_ERR_INVALIDDIR);
					f->mode=FN_FILE_CLOSE; /* release fileentry */
					return 0;		   /* directory */
				}
				if (de->attr & F_ATTR_READONLY) 
				{
					F_SETLASTERROR_NORET(F_ERR_ACCESSDENIED);
					f->mode=FN_FILE_CLOSE; /* release fileentry */
					return 0;
				}

				if (_f_checkappendlocked(fsname.drivenum,&f->dirpos,f)) 
				{
					F_SETLASTERROR_NORET(F_ERR_LOCKED);
					f->mode=FN_FILE_CLOSE; /* release fileentry */
					return 0; /* locked */
				}

				f->startcluster=_f_getdecluster(vi,de);

				f->filesize=_f_get32bitl(de->filesize);
#if F_MAXSEEKPOS
				_fn_initseekdivisor(f,vi);
#endif

				if (f->startcluster)
				{
					_f_clustertopos(vi,f->startcluster,&f->pos);
					f->abspos=(unsigned long)-(vi->bootrecord.bytes_per_sector); /* forcing seek to read 1st sector! abspos=0; */

					ret=_f_fseek(vi,fm,f,f->filesize);
					if (ret)
					{
						f->mode=FN_FILE_CLOSE;
						F_SETLASTERROR_NORET(ret);
						f->mode=FN_FILE_CLOSE; /* release fileentry */
						return 0;
					}
				}
				else
				{
					f->pos.sector=0;
					f->pos.sectorend=0;
				}
			}
			else
			{

				f->dirpos.cluster=f->pos.cluster;
				f->dirpos.pos=f->pos.pos;
				f->dirpos.sector=f->pos.sector;
				f->dirpos.sectorend=f->pos.sectorend;

				_f_clustertopos(vi,f->dirpos.cluster,&f->pos);

		   		ret=_f_addentry(vi,&fsname,&f->dirpos,&de);
				if (ret)
				{
					F_SETLASTERROR_NORET(ret);
					f->mode=FN_FILE_CLOSE; /* release fileentry */
					return 0;  /* couldnt be added */
				}

				de->attr |= F_ATTR_ARC;		/* set as archiv */
				ret=_f_writedirsector(vi);
				if (ret)
				{
					F_SETLASTERROR_NORET(ret);
					f->mode=FN_FILE_CLOSE; /* release fileentry */
					return 0;
				}

				f->startcluster=0;
				f->pos.sector=0;
				f->pos.sectorend=0;
#if F_MAXSEEKPOS
				_fn_initseekdivisor(f,vi);
#endif
			}

	   		break;

	   	case FN_FILE_WR: /* w */
	   	case FN_FILE_WRP: /* w+ */
			if (!_f_findpath(vi,&fsname,&f->dirpos)) 
			{
				F_SETLASTERROR_NORET(F_ERR_INVALIDDIR);
				f->mode=FN_FILE_CLOSE; /* release fileentry */
				return 0;
			}

			_f_clustertopos(vi,f->dirpos.cluster,&f->pos);
			if (_f_findfile(vi,fsname.filename,fsname.fileext,&f->pos,&de))
			{
				unsigned long cluster=_f_getdecluster(vi,de);	   /* exist */

				if (de->attr & F_ATTR_DIR) 
				{
					F_SETLASTERROR_NORET(F_ERR_INVALIDDIR);
					f->mode=FN_FILE_CLOSE; /* release fileentry */
					return 0;		   /* directory */
				}
				if (de->attr & F_ATTR_READONLY) 
				{
					F_SETLASTERROR_NORET(F_ERR_ACCESSDENIED);
					f->mode=FN_FILE_CLOSE; /* release fileentry */
					return 0;
				}

				f->dirpos.cluster=f->pos.cluster;
				f->dirpos.pos=f->pos.pos;
				f->dirpos.sector=f->pos.sector;
				f->dirpos.sectorend=f->pos.sectorend;

				if (_f_checklocked(fsname.drivenum,&f->dirpos)) 
				{
					F_SETLASTERROR_NORET(F_ERR_LOCKED);
					f->mode=FN_FILE_CLOSE; /* release fileentry */
					return 0; /* locked */
				}

				de->filesize[0]=0;					/* reset size; */
				de->filesize[1]=0;					/* reset size; */
				de->filesize[2]=0;					/* reset size; */
				de->filesize[3]=0;					/* reset size; */

				de->attr |= F_ATTR_ARC;		/* set as archiv */
				de->clusterlo[0]=0;					/* no points anywhere */
				de->clusterlo[1]=0;					/* no points anywhere */
				de->clusterhi[0]=0;					/* no points anywhere */
				de->clusterhi[1]=0;					/* no points anywhere */

				_f_set16bitl(de->crtdate,f_getdate());  /* if there is realtime clock then creation date could be set from */
				_f_set16bitl(de->crttime,f_gettime());  /* if there is realtime clock then creation time could be set from */
				_f_set16bitl(de->lastaccessdate,f_getdate());  /* if there is realtime clock then creation date could be set from */

				ret=_f_writedirsector(vi);
				if (ret)
				{
					F_SETLASTERROR_NORET(ret);
					f->mode=FN_FILE_CLOSE; /* release fileentry */
					return 0;
				}

				ret=_f_removechain(vi,cluster); 
				if (ret)
				{
					F_SETLASTERROR_NORET(ret);
					f->mode=FN_FILE_CLOSE; /* release fileentry */
					return 0; /* remove  */
				}
			}
			else
			{
		   		ret=_f_addentry(vi,&fsname,&f->dirpos,&de);
				if (ret)
				{
					F_SETLASTERROR_NORET(ret);
					f->mode=FN_FILE_CLOSE; /* release fileentry */
					return 0;  /* couldnt be added */
				}

				de->attr |= F_ATTR_ARC;		/* set as archiv */
				ret=_f_writedirsector(vi);
				if (ret)
				{
					F_SETLASTERROR_NORET(ret);
					f->mode=FN_FILE_CLOSE; /* release fileentry */
					return 0;
				}
			}


			f->startcluster=0;
			f->pos.sector=0;
			f->pos.sectorend=0;

#if F_MAXSEEKPOS
			_fn_initseekdivisor(f,vi);
#endif
	   		break;

		default:
			F_SETLASTERROR_NORET(F_ERR_NOTUSEABLE);
			f->mode=FN_FILE_CLOSE; /* release fileentry */
			return 0; /* invalid mode */
	}

#ifdef USE_MALLOC
	f->WrDataCache.pos=(F_POS *)__malloc(sizeof(F_POS)*WR_DATACACHE_SIZE);
	if (f->WrDataCache.pos==NULL) 
	{
		F_SETLASTERROR_NORET(F_ERR_UNKNOWN);
		f->mode=FN_FILE_CLOSE; /* release fileentry */
		return 0;
	}
#else
	f->WrDataCache.pos=f->WrDataCache.posbuf;
#endif

	f->mode=m_mode; /*  set proper mode lock it  */

	F_SETLASTERROR_NORET(F_NO_ERROR);
	return (FN_FILE *)(f->file.reference);
}

/****************************************************************************
 *
 * fn_settimedate
 *
 * set a file time and date
 *
 * INPUTS
 *
 * fm - multi structure pointer
 * filename - which file time and date wanted to be set
 * ctime - new ctime of the file
 * cdate - new cdate of the file
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/

int fn_settimedate(F_MULTI *fm,const char *filename,unsigned short ctime,unsigned short cdate)
{
	F_POS pos;
	F_DIRENTRY *de;
	F_NAME fsname;
	F_VOLUME *vi;
	int ret;

	if (_f_setfsname(fm,filename,&fsname)) return F_SETLASTERROR(F_ERR_INVALIDNAME); /* invalid name */
	if (_f_checknamewc(fsname.filename,fsname.fileext)) return F_SETLASTERROR(F_ERR_INVALIDNAME);/* invalid name */

	ret=_f_getvolume(fm,fsname.drivenum,&vi);
	if (ret) return F_SETLASTERROR(ret);

	if (!_f_findpath(vi,&fsname,&pos)) return F_SETLASTERROR(F_ERR_INVALIDDIR);
	if (!_f_findfile(vi,fsname.filename,fsname.fileext,&pos,&de)) return F_SETLASTERROR(F_ERR_NOTFOUND);

	_f_set16bitl(de->ctime,ctime);
	_f_set16bitl(de->cdate,cdate);

	ret=_f_writedirsector(vi);
	return F_SETLASTERROR(ret);
}

/****************************************************************************
 *
 * fn_gettimedate
 *
 * get a file time and date
 *
 * INPUTS
 *
 * fm - multi structure pointer
 * filename - which file time and date wanted to be retrive
 * pctime - ctime of the file where to store
 * pcdate - cdate of the file where to store
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/

int fn_gettimedate(F_MULTI *fm,const char *filename,unsigned short *pctime,unsigned short *pcdate)
{
	F_POS pos;
	F_VOLUME *vi;
	F_DIRENTRY *de;
	F_NAME fsname;
	int ret;

	if (_f_setfsname(fm,filename,&fsname)) return F_SETLASTERROR(F_ERR_INVALIDNAME); /* invalid name */
	if (_f_checknamewc(fsname.filename,fsname.fileext)) return F_SETLASTERROR(F_ERR_INVALIDNAME);/* invalid name */

	ret=_f_getvolume(fm,fsname.drivenum,&vi);
	if (ret) return F_SETLASTERROR(ret);

	if (!_f_findpath(vi,&fsname,&pos)) return F_SETLASTERROR(F_ERR_INVALIDDIR);
	if (!_f_findfile(vi,fsname.filename,fsname.fileext,&pos,&de)) return F_SETLASTERROR(F_ERR_NOTFOUND);

	if (pctime) *pctime=_f_get16bitl(de->ctime);
	if (pcdate) *pcdate=_f_get16bitl(de->cdate);

	return F_SETLASTERROR(F_NO_ERROR);
}

/****************************************************************************
 *
 * fn_stat
 *
 * get status information on a file
 *
 * INPUTS
 *
 * fm - multi structure pointer
 * filename - which file time and date wanted to be retrive
 * stat - pointer where to store information
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/

int fn_stat(F_MULTI *fm,const char *filename,F_STAT *stat)
{
	F_POS pos;
	F_VOLUME *vi;
	F_DIRENTRY *de;
	F_NAME fsname;
	int ret;
	unsigned long ofsize;

	if (_f_setfsname(fm,filename,&fsname)) return F_SETLASTERROR(F_ERR_INVALIDNAME); /* invalid name */
	if (_f_checknamewc(fsname.filename,fsname.fileext)) return F_SETLASTERROR(F_ERR_INVALIDNAME);/* invalid name */

	ret=_f_getvolume(fm,fsname.drivenum,&vi);
	if (ret) return F_SETLASTERROR(ret);

	if (!_f_findpath(vi,&fsname,&pos)) return F_SETLASTERROR(F_ERR_INVALIDDIR);
	if (!_f_findfile(vi,fsname.filename,fsname.fileext,&pos,&de)) return F_SETLASTERROR(F_ERR_NOTFOUND);

	stat->filesize=_f_get32bitl(de->filesize);
	stat->createdate=_f_get16bitl(de->crtdate);
	stat->createtime=_f_get16bitl(de->crttime);
	stat->modifieddate=_f_get16bitl(de->cdate);
	stat->modifiedtime=_f_get16bitl(de->ctime);
	stat->lastaccessdate=_f_get16bitl(de->lastaccessdate);
	stat->attr=de->attr;
	stat->drivenum=fsname.drivenum;

	if (_f_findopensize(&ofsize,fsname.drivenum,&pos))
	{
		stat->filesize=ofsize;
	}

	return F_SETLASTERROR(F_NO_ERROR);
}

/****************************************************************************
 *
 * fn_delete
 *
 * delete a file
 *
 * INPUTS
 *
 * fm - multi structure pointer
 * filename - file which wanted to be deleted (with or without path)
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/

int fn_delete(F_MULTI *fm,const char *filename)
{
	int ret;
	F_POS pos;
	F_DIRENTRY *de;
	F_NAME fsname;
	F_VOLUME *vi;

	if (_f_setfsname(fm,filename,&fsname)) return F_SETLASTERROR(F_ERR_INVALIDNAME); /* invalid name */
	if (_f_checknamewc(fsname.filename,fsname.fileext)) return F_SETLASTERROR(F_ERR_INVALIDNAME);/* invalid name */

	ret=_f_getvolume(fm,fsname.drivenum,&vi);
	if (ret) return F_SETLASTERROR(ret);

	if (fsname.filename[0]=='.') return F_SETLASTERROR(F_ERR_INVALIDNAME);

	if (!(_f_findpath(vi,&fsname,&pos))) return F_SETLASTERROR(F_ERR_INVALIDDIR);
	if (!_f_findfile(vi,fsname.filename,fsname.fileext,&pos,&de)) return F_SETLASTERROR(F_ERR_NOTFOUND);

	if (de->attr & F_ATTR_DIR) return F_SETLASTERROR(F_ERR_INVALIDDIR);		   /* directory */
	if (de->attr & F_ATTR_READONLY) return F_SETLASTERROR(F_ERR_ACCESSDENIED);      /* readonly */

	if (_f_checklocked(fsname.drivenum,&pos)) return F_SETLASTERROR(F_ERR_LOCKED);

	de->name[0]=F_DELETED_CHAR;	/* removes it */

	ret=_f_writedirsector(vi);
	if (ret) return F_SETLASTERROR(ret);

	ret=_f_removechain(vi,_f_getdecluster(vi,de));
#ifdef FATCACHE_ENABLE
	if (ret) return F_SETLASTERROR(ret);

	ret=_f_fatcache_flush(vi);
#endif
	return F_SETLASTERROR(ret);
}

/****************************************************************************
 *
 * fn_getattr
 *
 * get file attribute
 *
 * INPUTS
 *
 * fm - multi structure pointer
 * filename - which file attribute is needed
 * attr - pointer to a characterter where to store attribute information
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/

int fn_getattr(F_MULTI *fm,const char *filename,unsigned char *attr)
{
	F_POS pos;
	F_VOLUME *vi;
	F_DIRENTRY *de;
	F_NAME fsname;
	int ret;

	if (_f_setfsname(fm,filename,&fsname)) return F_SETLASTERROR(F_ERR_INVALIDNAME); /* invalid name */
	if (_f_checknamewc(fsname.filename,fsname.fileext)) return F_SETLASTERROR(F_ERR_INVALIDNAME);/* invalid name */

	ret=_f_getvolume(fm,fsname.drivenum,&vi);
	if (ret) return F_SETLASTERROR(ret);

	if (!_f_findpath(vi,&fsname,&pos)) return F_SETLASTERROR(F_ERR_INVALIDDIR);
	if (!_f_findfile(vi,fsname.filename,fsname.fileext,&pos,&de)) return F_SETLASTERROR(F_ERR_NOTFOUND);

	if (attr) *attr=de->attr;

	return F_SETLASTERROR(F_NO_ERROR);
}

/****************************************************************************
 *
 * fn_setattr
 *
 * set attribute of a file
 *
 * INPUTS
 *
 * fm - multi structure pointer
 * filename - which file attribute set
 * attr - new attribute of the file
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/

int fn_setattr(F_MULTI *fm,const char *filename,unsigned char attr)
{
	F_POS pos;
	F_DIRENTRY *de;
	F_NAME fsname;
	F_VOLUME *vi;
	int ret;

	if (_f_setfsname(fm,filename,&fsname)) return F_SETLASTERROR(F_ERR_INVALIDNAME); /* invalid name */
	if (_f_checknamewc(fsname.filename,fsname.fileext)) return F_SETLASTERROR(F_ERR_INVALIDNAME);/* invalid name */

	ret=_f_getvolume(fm,fsname.drivenum,&vi);
	if (ret) return F_SETLASTERROR(ret);

	if (!_f_findpath(vi,&fsname,&pos)) return F_SETLASTERROR(F_ERR_INVALIDDIR);
	if (!_f_findfile(vi,fsname.filename,fsname.fileext,&pos,&de)) return F_SETLASTERROR(F_ERR_NOTFOUND);

	attr&=F_ATTR_HIDDEN|F_ATTR_READONLY|F_ATTR_ARC|F_ATTR_SYSTEM; /* keep only valid bits */

	attr|=de->attr&(F_ATTR_DIR|F_ATTR_VOLUME); /* keep some original */

	if (de->attr==attr) return F_SETLASTERROR(F_NO_ERROR); /* its the same */

	de->attr=attr;

	ret=_f_writedirsector(vi);
	return F_SETLASTERROR(ret);
}

/****************************************************************************
 *
 * fn_rmdir
 *
 * Remove directory, only could be removed if empty
 *
 * INPUTS
 *
 * fm - multi structure pointer
 * dirname - which directory needed to be removed
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/

int fn_rmdir(F_MULTI *fm,const char *dirname)
{
	int ret;
	F_POS pos;
	F_DIRENTRY *de;
	F_NAME fsname;
	F_VOLUME *vi;
	FN_FILEINT *f;
	unsigned int a;

	if (_f_setfsname(fm,dirname,&fsname)) return F_SETLASTERROR(F_ERR_INVALIDNAME); /* invalid name */
	if (_f_checknamewc(fsname.filename,fsname.fileext)) return F_SETLASTERROR(F_ERR_INVALIDNAME);/* invalid name */

	ret=_f_getvolume(fm,fsname.drivenum,&vi);
	if (ret) return F_SETLASTERROR(ret);

	if (!(_f_findpath(vi,&fsname,&pos))) return F_SETLASTERROR(F_ERR_INVALIDDIR);

	if (fsname.filename[0]=='.') return F_SETLASTERROR(F_ERR_INVALIDNAME);

	if (!_f_findfile(vi,fsname.filename,fsname.fileext,&pos,&de)) return F_SETLASTERROR(F_ERR_NOTFOUND);

	if (!(de->attr & F_ATTR_DIR)) return F_SETLASTERROR(F_ERR_INVALIDDIR);		   /* not a directory */

	if (de->attr & F_ATTR_READONLY) return F_SETLASTERROR(F_ERR_ACCESSDENIED);

	f=_f_getintfile();
	if (!f) return F_SETLASTERROR(F_ERR_NOMOREENTRY);

	_f_clustertopos(vi,_f_getdecluster(vi,de),&f->pos);

	for (;;)
	{
		F_DIRENTRY *de2;
		ret=_f_getcurrsector(vi,f,NULL,0);

		if (ret==F_ERR_EOF) break;
		if (ret) 
		{
		 	f->mode=FN_FILE_CLOSE; /* release fileentry */
			return F_SETLASTERROR(ret);
		}

		de2=(F_DIRENTRY *)(f->data);

		for (a=0; a<(vi->bootrecord.bytes_per_sector)/sizeof(F_DIRENTRY); a++,de2++)
		{
			char ch=de2->name[0];
			if (!ch) break;
			if (ch==F_DELETED_CHAR) continue;
			if (ch=='.') continue;

		 	f->mode=FN_FILE_CLOSE; /* release fileentry */
			return F_SETLASTERROR(F_ERR_NOTEMPTY); /* something is there */
		}
		f->pos.sector++;
	}

 	f->mode=FN_FILE_CLOSE; /* release fileentry */

	de->name[0]=F_DELETED_CHAR;

	ret=_f_writedirsector(vi);
	if (ret) return F_SETLASTERROR(ret);

	ret=_f_removechain(vi,_f_getdecluster(vi,de));

#ifdef FATCACHE_ENABLE
	if (ret) return F_SETLASTERROR(ret);

	ret=_f_fatcache_flush(vi);
#endif

	return F_SETLASTERROR(ret);
}

/****************************************************************************
 *
 * fn_getlabel
 *
 * get a label of a media
 *
 * INPUTS
 *
 * fm - multi structure pointer
 * drivenum - drive number which label's is needed
 * label - char pointer where to store label
 * len - length of label buffer
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/

int fn_getlabel(F_MULTI *fm,int drivenum, char *label, long len)
{
	char tmplabel[11];
	long a;
	F_VOLUME *vi;
	F_POS pos;
	int ret;

	if (!len) return F_SETLASTERROR(F_NO_ERROR); /* how to store volume name? */

	ret=_f_getvolume(fm,drivenum,&vi);
	if (ret) return F_SETLASTERROR(ret);

	_f_clustertopos(vi,0,&pos);
	(void)_memcpy(tmplabel,vi->bootrecord.volume_name,11);

	while (pos.cluster<F_CLUSTER_RESERVED)
	{
	   	for (;pos.sector<pos.sectorend;pos.sector++)
		{
			F_DIRENTRY *de=(F_DIRENTRY*)(vi->direntry);

	   		ret=_f_getdirsector(vi,pos.sector);
	   		if (ret) return F_SETLASTERROR(ret);

	   		for (pos.pos=0; pos.pos<(vi->bootrecord.bytes_per_sector)/sizeof(F_DIRENTRY);de++,pos.pos++)
			{
   				if (!de->name[0]) goto vege;						 /* empty, last */
	   			if (de->name[0]==F_DELETED_CHAR) continue; /* deleted */
		   		if (de->attr == F_ATTR_VOLUME)
				{
			   		(void)_memcpy(tmplabel,de->name,8);
   					(void)_memcpy(tmplabel+8,de->ext,3);
   					goto vege;
   				}
   			}
	   	}

		if (!pos.cluster)
		{
			if (vi->mediatype==F_FAT32_MEDIA)
			{
				pos.cluster=vi->bootrecord.rootcluster;
			}
			else goto vege;
		}

		{
			unsigned long nextcluster;
			if (_f_getclustervalue(vi,pos.cluster,&nextcluster)) goto vege;

	   		if (nextcluster>=F_CLUSTER_RESERVED) goto vege;

			_f_clustertopos(vi,nextcluster,&pos);
		}
  	}

vege:
	len--;
	for (a=0; a<11 && a<len; a++)
	{
		char ch=tmplabel[a];
		if (ch>='a' && ch<='z') ch+='A'-'a';
		label[a]=tmplabel[a];
	}
	label [a]=0; /* terminate it, this is ok because of len--! */

	for (;a>=0; a--)
	{ /* cut the spaces */
		if (label[a]<=32) label[a]=0;
		else break;
	}

	return F_SETLASTERROR(F_NO_ERROR);
}

/****************************************************************************
 *
 * fn_setlabel
 *
 * set a label of a media
 *
 * INPUTS
 *
 * fm - multi structure pointer
 * drivenum - drive number which label's need to be set
 * label - new label for the media
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/

int fn_setlabel(F_MULTI *fm,int drivenum, const char *label)
{
	F_NAME fsname;
	int ret;
	char tmplabel[11];
	int a,b;
	F_VOLUME *vi;
	F_POS pos;
	F_DIRENTRY *de;

	ret=_f_getvolume(fm,drivenum,&vi);
	if (ret) return F_SETLASTERROR(ret);

	_f_clustertopos(vi,0,&pos);

	for (a=b=0; a<11; a++)
	{
		char ch=label[b];
		if (ch>='a' && ch<='z') ch+='A'-'a';

		if (ch>=32) b++;
		else ch=32;

		tmplabel[a]=ch;
	}

	while (pos.cluster<F_CLUSTER_RESERVED)
	{
	   	for (;pos.sector<pos.sectorend;pos.sector++)
		{
			de=(F_DIRENTRY*)(vi->direntry);

			ret=_f_getdirsector(vi,pos.sector);
			if (ret) return F_SETLASTERROR(ret);

			for (pos.pos=0; pos.pos<(vi->bootrecord.bytes_per_sector)/sizeof(F_DIRENTRY);de++,pos.pos++)
			{
   				if (!de->name[0]) goto vege;						 /* empty */
	   			if (de->name[0]==F_DELETED_CHAR) continue; /* deleted */
		   		if (de->attr == F_ATTR_VOLUME)
				{
					(void)_memcpy((void *)(de->name),(void *)tmplabel,8);
					(void)_memcpy((void *)(de->ext),(void *)(tmplabel+8),3);
					ret=_f_writedirsector(vi);
					return F_SETLASTERROR(ret);
				}
			}
   		}

		if (!pos.cluster)
		{
			if (vi->mediatype==F_FAT32_MEDIA)
			{
				pos.cluster=vi->bootrecord.rootcluster;
			}
			else break;
		}

		{
			unsigned long nextcluster;
			if (_f_getclustervalue(vi,pos.cluster,&nextcluster)) break;

	   		if (nextcluster>=F_CLUSTER_RESERVED) break;

			_f_clustertopos(vi,nextcluster,&pos);
		}
  	}
vege:
	_f_clustertopos(vi,0,&pos);

   fsname.drivenum=drivenum;
   (void)_memcpy(fsname.filename,(void *)tmplabel,F_MAXNAME);
   (void)_memcpy(fsname.fileext,(void *)(tmplabel+F_MAXNAME),F_MAXEXT);

	if (_f_addentry(vi,&fsname,&pos,&de)) return F_SETLASTERROR(F_ERR_NOMOREENTRY);  /* couldnt be added */

	de->attr=F_ATTR_VOLUME;
	ret=_f_writedirsector(vi);
	return F_SETLASTERROR(ret);
}

/****************************************************************************
 *
 * fn_truncate
 *
 * truncate a file to a specified length, filepointer will be set to the
 * end
 *
 * INPUTS
 *
 * fm - multi structure pointer
 * filename - which file need to be truncated
 * length - length of new file
 *
 * RETURNS
 *
 * filepointer or zero if fails
 *
 ***************************************************************************/

FN_FILE *fn_truncate(F_MULTI *fm,const char *filename,unsigned long length)
{
	F_DIRENTRY *de;
	F_NAME fsname;
	F_VOLUME *vi;
	char m_mode=FN_FILE_WRP;
	FN_FILEINT *f=0;
	int ret;

	if (length==0) 
	{
		/* setlasterror is called in all path in fn_open */
		return fn_open(fm,filename,"w+"); 
	}

	if (_f_setfsname(fm,filename,&fsname)) 
	{
	 	F_SETLASTERROR_NORET(F_ERR_INVALIDNAME);
		return 0; /* invalid name */
	}
	if (_f_checknamewc(fsname.filename,fsname.fileext)) 
	{
	 	F_SETLASTERROR_NORET(F_ERR_INVALIDNAME);
		return 0;/* invalid name */
	}

	if (_f_getvolume(fm,fsname.drivenum,&vi)) 
	{
	 	F_SETLASTERROR_NORET(F_ERR_INVALIDDRIVE);
		return 0; /* cant open any */
	}

	f=_f_getintfile();
	if (!f) 
	{
	 	F_SETLASTERROR_NORET(F_ERR_NOMOREENTRY);
		return 0; /* no more file could be opened  */
	}

	f->drivenum=fsname.drivenum;
	f->relpos=0;
	f->abspos=0;
	f->filesize=0;
	f->pos.prevcluster=0;
	f->modified=0;
	f->state=0;
	f->syncfile=0;

	if (fsname.filename[0]=='.') 
	{
	 	F_SETLASTERROR_NORET(F_ERR_INVALIDNAME);
	 	f->mode=FN_FILE_CLOSE; /* release fileentry */
		return 0;
	}

	if (!_f_findpath(vi,&fsname,&f->dirpos)) 
	{
	 	F_SETLASTERROR_NORET(F_ERR_INVALIDDIR);
	 	f->mode=FN_FILE_CLOSE; /* release fileentry */
		return 0;
	}
	if (!_f_findfile(vi,fsname.filename,fsname.fileext,&f->dirpos,&de)) 
	{
	 	F_SETLASTERROR_NORET(F_ERR_NOTFOUND);
	 	f->mode=FN_FILE_CLOSE; /* release fileentry */
		return 0;
	}
	if (_f_checklocked(fsname.drivenum,&f->dirpos)) 
	{
	 	F_SETLASTERROR_NORET(F_ERR_LOCKED);
	 	f->mode=FN_FILE_CLOSE; /* release fileentry */
		return 0; /* locked */
	}

   	if (de->attr & F_ATTR_DIR) 
	{
	 	F_SETLASTERROR_NORET(F_ERR_INVALIDDIR);
	 	f->mode=FN_FILE_CLOSE; /* release fileentry */
		return 0;		   /* directory */
	}

	f->startcluster=_f_getdecluster(vi,de);

	if (f->startcluster)
	{
		unsigned long slen;

		_f_clustertopos(vi,f->startcluster,&f->pos);

		f->filesize=_f_get32bitl(de->filesize);
#if F_MAXSEEKPOS
		_fn_initseekdivisor(f,vi);
#endif

		f->abspos=(unsigned long)-(vi->bootrecord.bytes_per_sector); /* forcing seek to read 1st sector! abspos=0; */

		if (length>f->filesize) slen=f->filesize;
		else slen=length;

		ret=_f_fseek(vi,fm,f,slen);
		if (ret) 
		{
	 		F_SETLASTERROR_NORET(ret);
		 	f->mode=FN_FILE_CLOSE; /* release fileentry */
			return 0;
		}
	}
	else
	{
		f->pos.sector=0;
		f->pos.sectorend=0;
#if F_MAXSEEKPOS
		_fn_initseekdivisor(f,vi);
#endif
	}

	if (length<f->filesize)
	{
		unsigned long cluster;

		ret=_f_getclustervalue(vi,f->pos.cluster,&cluster);
		if (ret) 
		{
	 		F_SETLASTERROR_NORET(ret);
		 	f->mode=FN_FILE_CLOSE; /* release fileentry */
			return 0;
		}

		ret=_f_removechain(vi,cluster);
		if (ret)
		{
	 		F_SETLASTERROR_NORET(ret);
		 	f->mode=FN_FILE_CLOSE; /* release fileentry */
			return 0;
		}

		ret=_f_setclustervalue(vi,f->pos.cluster,F_CLUSTER_LAST);
		if (ret) 
		{
	 		F_SETLASTERROR_NORET(ret);
		 	f->mode=FN_FILE_CLOSE; /* release fileentry */
			return 0;
		}

		f->filesize=length;
#if F_MAXSEEKPOS
		_fn_removeseekpos(f);
#endif
	}
	else if (length>f->filesize)
	{
		unsigned long remainingbytes=length-f->filesize;

		f->mode=m_mode; /*  lock it  */

		ret=_f_writezeros(vi,fm,f,remainingbytes);
		if (ret)
		{
	 		F_SETLASTERROR_NORET(ret);
		 	f->mode=FN_FILE_CLOSE; /* release fileentry */
			return 0;
		}
	}

#ifdef USE_MALLOC
	f->WrDataCache.pos=(F_POS *)__malloc(sizeof(F_POS)*WR_DATACACHE_SIZE);
	if (f->WrDataCache.pos==NULL) 
	{
 		F_SETLASTERROR_NORET(F_ERR_UNKNOWN);
	 	f->mode=FN_FILE_CLOSE; /* release fileentry */
		return 0;
	}
#else
	f->WrDataCache.pos=f->WrDataCache.posbuf;
#endif

	f->mode=m_mode; /*  lock it  */

	F_SETLASTERROR_NORET(F_NO_ERROR);
	return (FN_FILE *)(f->file.reference);
}

/****************************************************************************
 *
 * end of fat.c
 *
 ***************************************************************************/

#endif  /*  !F_LONGFILENAME  */

