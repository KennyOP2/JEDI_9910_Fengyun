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

/****************************************************************************
 *
 * Includes
 *
 ***************************************************************************/

#include "test_f.h"
#include "common/port_f.h"
#include "testdrv_f.h"
#include "common/debug.h"

/****************************************************************************
 *
 * defines
 *
 ***************************************************************************/

#define RIT_NUM_OF_RECORDS  100  /* define for rit test */

/****************************************************************************
 *
 *	structure
 *
 ***************************************************************************/

typedef struct
{
	unsigned long MagicNum;
	int Line;
	char Buf[87];
} rit_struct_TestFileSysEntry;

/****************************************************************************
 *
 *	static variables
 *
 ***************************************************************************/

static W_CHAR cwd[F_MAXPATH];
static F_FIND find;
static char buffer[32768];
static F_FILE *files[F_MAXFILES];
int curdrive;
#if TEST_FULL_COVERAGE
static long test_fat_media;
#endif

/****************************************************************************
 *
 * f_nameconv
 *
 * convert incoming mixed string to returned uppercase string
 * this functions depending on long file using
 *
 * INPUTS
 *
 * s - incomming string to convert
 *
 * RETURNS
 *
 * converted uppercase string
 *
 ***************************************************************************/

#if F_LONGFILENAME /* long file name contains exact names */

#ifdef HCC_UNICODE
static wchar tmpwch[F_MAXPATH];
#endif

static W_CHAR *f_nameconv(char *s)
{
#ifdef HCC_UNICODE
	return _towchar(tmpwch,s);
#else
	return s;
#endif
}

#else

static char stmp[1024];

static char *f_nameconv(char *s)
{
char *ss=stmp;
	for (;;)
	{
		char ch=*s++;
		if (ch>='a' && ch<='z')
		{
			ch+='A'-'a';
		}
		*ss++=ch;
		if (!ch) break;
	}
	return stmp;
}
#endif

/****************************************************************************
 *
 * STRCMP and STRWCMP
 *
 * compares to string
 *
 * INPUTS
 *
 * s1 - string 1
 * s2 - string 2
 *
 * RETURNS
 *
 * 0 - if identical
 * other if not identical
 *
 ***************************************************************************/

#ifdef HCC_UNICODE
static int STRWCMP (wchar *s1, wchar *s2)
{
  while (*s1)
  {
    if (*s1!=*s2) return 1;
    ++s1;
    ++s2;
  }
  if (*s2) return 1;
  return 0;
}
#endif

static int STRCMP (char *s1, char *s2)
{
  while (*s1)
  {
    if (*s1!=*s2) return 1;
    ++s1;
    ++s2;
  }
  if (*s2) return 1;
  return 0;
}

/****************************************************************************
 *
 * _f_checkcwd
 *
 * compares original working folder with current working folder
 *
 * INPUTS
 *
 * orig - original folder to compare
 *
 * RETURNS
 *
 * 0 - if identical
 * other if not identical
 *
 ***************************************************************************/

static int _f_checkcwd(W_CHAR *orig)
{
	int ret;

#ifdef HCC_UNICODE
	ret=f_wgetcwd(cwd,F_MAXPATH);
#else
	ret=f_getcwd(cwd,F_MAXPATH);
#endif

	if (ret) return ret;

#ifdef HCC_UNICODE
	if (STRWCMP(orig,cwd)) return -1;
#else
	if (STRCMP(orig,cwd)) return -1;
#endif

	cwd[0]=0; /* reset it */

#ifdef HCC_UNICODE
	ret=f_wgetdcwd(curdrive,cwd,F_MAXPATH);
#else
	ret=f_getdcwd(curdrive,cwd,F_MAXPATH);
#endif

	if (ret) return ret;

#ifdef HCC_UNICODE
	if (STRWCMP(orig,cwd)) return -1;
#else
	if (STRCMP(orig,cwd)) return -1;
#endif

	return 0;
}

/****************************************************************************
 *
 * checkfilecontent
 *
 * checking file content with f_read
 *
 * INPUTS
 *
 * nums - number of bytes need to be checked
 * value - compare bytes with this value
 * file - FILE pointer
 *
 * RETURNS
 *
 * 0 - if identical
 * other if not identical
 *
 ***************************************************************************/

static int	checkfilecontent(long nums,unsigned char value,F_FILE *file)
{
	unsigned char ch;

	while (nums--) {

		if (f_eof(file)) return 1; /* eof ? */

		if (1!=f_read(&ch,1,1,file)) return 1;
		if (ch!=value)
		{
		  return 1;
		}
	}

	return 0;
}

/****************************************************************************
 *
 * f_formatting
 *
 * test case
 *
 * RETURNS
 *
 * error code or 0 if success
 *
 ***************************************************************************/

static int f_formatting(void)
{
	int ret;

	_f_dump ("f_formatting");

	/* F_MAXFILES must be equal or grater than 3 for these test */
	if (F_MAXFILES<3) return _f_result(__LINE__,F_MAXFILES);

	/* FN_MAXVOLUME must be equal or grater than 2 for these test */
	if (FN_MAXVOLUME<2) return _f_result(__LINE__,FN_MAXVOLUME);

/* 1st power on */
	ret=_f_poweron();
	if (ret==F_ERR_NOTFORMATTED)
	{
		ret=f_chdrive(curdrive);	/* this is supported to select a drive which is not formatted */
		if (ret) return _f_result(__LINE__,ret);

		ret=f_format(curdrive,TEST_FAT_MEDIA);
		if (ret) return _f_result(__LINE__,ret);
	}
    
/* simple restart */
	ret=_f_poweroff();
	if (ret) return _f_result(__LINE__,ret);

	ret=_f_poweron();
	if (ret) return _f_result(__LINE__,ret);
    
/* checking formatting */
#if TEST_FULL_COVERAGE
 	testdrv_set(TESTDRV_STATE,2); /* signal 2 removed */
	ret=f_format(curdrive,TEST_FAT_MEDIA);
	if (ret!=F_ERR_CARDREMOVED) return _f_result(__LINE__,ret);

 	testdrv_set(TESTDRV_STATE,1); /* signal 1 removed */

	ret=f_format(curdrive,TEST_FAT_MEDIA);
	if (ret!=F_ERR_CARDREMOVED) return _f_result(__LINE__,ret);
#endif

	ret=f_format(curdrive,-1); /* invalid */
	if (ret!=F_ERR_INVFATTYPE) return _f_result(__LINE__,ret);

	ret=f_format(curdrive,TEST_FAT_MEDIA);
	if (ret) return _f_result(__LINE__,ret);

	ret=_f_poweroff();
	if (ret) return _f_result(__LINE__,ret);

	ret=_f_poweron();
	if (ret) return _f_result(__LINE__,ret);

	ret=f_findfirst("*.*",&find);
	if (ret!=F_ERR_NOTFOUND) return _f_result(__LINE__,ret);

	ret=f_findfirst("",&find);
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);

	ret=f_findfirst("a[",&find);
	if (ret!=F_ERR_NOTFOUND) return _f_result(__LINE__,ret);

/*
	ret=f_format(curdrive,F_FAT16_MEDIA);
	if (!curdrive)
	{
		if (ret!=F_ERR_MEDIATOOSMALL) return _f_result(__LINE__,ret);
	}
	else
	{
		if (ret) return _f_result(__LINE__,ret);
	}
*/

	ret=f_format(curdrive,TEST_FAT_MEDIA);
	if (ret) return _f_result(__LINE__,ret);

	_f_dump("passed...");
	return 0;
}

/****************************************************************************
 *
 * f_dirtest
 *
 * test case
 *
 * RETURNS
 *
 * error code or 0 if success
 *
 ***************************************************************************/

static int f_dirtest(void)
{
	int ret;
	F_FILE *file;

	_f_dump("f_dirtest");
/* creates a ab abc abcd */

	ret=f_mkdir("{a}");
	if (ret) return _f_result(__LINE__,ret);

	ret=f_mkdir("{a}.{a}");
	if (ret) return _f_result(__LINE__,ret);


	ret=f_rmdir("{a}");
	if (ret) return _f_result(__LINE__,ret);

	ret=f_rmdir("{a}.{a}");
	if (ret) return _f_result(__LINE__,ret);


	ret=f_mkdir("Test.Dir");
	if (ret) return _f_result(__LINE__,ret);

	ret=f_rmdir("Test.Dir");
	if (ret) return _f_result(__LINE__,ret);


	ret=f_mkdir("");
	if (!ret) return _f_result(__LINE__,ret);

	ret=f_mkdir(" ");
	if (!ret) return _f_result(__LINE__,ret);

	ret=f_mkdir("?");
	if (!ret) return _f_result(__LINE__,ret);

	ret=f_mkdir("a");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_mkdir("ab");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_mkdir("abc");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_mkdir("abca");
	if (ret) return _f_result(__LINE__,ret);

	ret=f_rename("ab","abc");
	if (ret!=F_ERR_DUPLICATED) return _f_result(__LINE__,ret);


/* creates directories in /a  -  a ab abc abcd */
	ret=f_mkdir("a/a");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_mkdir("a/ab");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_mkdir("a/abc");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_mkdir("a/abcd");
	if (ret) return _f_result(__LINE__,ret);

/* change into a/abcd and check cwd */
	ret=f_chdir("a\\.../b");
	if (!ret) return _f_result(__LINE__,ret);

	ret=f_chdir("a\\\\b");
	if (!ret) return _f_result(__LINE__,ret);

	ret=f_chdir("a/abcd");
	if (ret) return _f_result(__LINE__,ret);
	ret=_f_checkcwd(f_nameconv("/a/abcd"));
	if (ret) return _f_result(__LINE__,ret);

	ret=f_chdir("/a /abcd");
	if (ret) return _f_result(__LINE__,ret);
	ret=_f_checkcwd(f_nameconv("/a/abcd"));
	if (ret) return _f_result(__LINE__,ret);


/* make directory t change into t and check cwd="a/abcd/t" */
	ret=f_mkdir("t");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_chdir("t");
	if (ret) return _f_result(__LINE__,ret);
	ret=_f_checkcwd(f_nameconv("/a/abcd/t"));
	if (ret) return _f_result(__LINE__,ret);
	ret=f_chdir(".");
	if (ret) return _f_result(__LINE__,ret);
	ret=_f_checkcwd(f_nameconv("/a/abcd/t"));
	if (ret) return _f_result(__LINE__,ret);
	ret=f_chdir("../.");
	if (ret) return _f_result(__LINE__,ret);
	ret=_f_checkcwd(f_nameconv("/a/abcd"));
	if (ret) return _f_result(__LINE__,ret);

/* removing t dir */
	ret=f_rmdir("");
	if (!ret) return _f_result(__LINE__,ret);
	ret=f_rmdir("*.*");
	if (!ret) return _f_result(__LINE__,ret);

	ret=f_rmdir("notfound");
	if (!ret) return _f_result(__LINE__,ret);

	ret=f_rmdir("invalid/notfound");
	if (!ret) return _f_result(__LINE__,ret);


	ret=f_rmdir("t");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_chdir("t");
	if (ret!=F_ERR_NOTFOUND) return _f_result(__LINE__,ret);

/* removing /a dir */
	ret=f_rmdir("/ab");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_chdir("/ab");
	if (ret!=F_ERR_NOTFOUND) return _f_result(__LINE__,ret);

/* removing /a dir */
	ret=f_rmdir("../../a");
	if (ret!=F_ERR_NOTEMPTY) return _f_result(__LINE__,ret);

/* removing /abca dir */
	if (!curdrive)
	{
		ret=f_rmdir("a:/abca");
		if (ret) return _f_result(__LINE__,ret);
	}
	else if (curdrive==1)
	{
		ret=f_rmdir("b:/abca");
		if (ret) return _f_result(__LINE__,ret);
	}
	else if (curdrive==2)
	{
		ret=f_rmdir("c:/abca");
		if (ret) return _f_result(__LINE__,ret);
	}


/* changing invalid dirs */
	ret=f_chdir("");
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);
	ret=f_chdir(" ");
	if (ret) return _f_result(__LINE__,ret);
	ret=_f_checkcwd(f_nameconv("/a/abcd"));
	if (ret) return _f_result(__LINE__,ret);
	ret=f_chdir("?");
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);
	ret=f_chdir("*.*");
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);
	ret=_f_checkcwd(f_nameconv("/a/abcd"));
	if (ret) return _f_result(__LINE__,ret);

/* changing into /abc and removes subfolder from /a/ */
	ret=f_chdir("/abc");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_rmdir("/a/a");
	if (ret) return _f_result(__LINE__,ret);
	if (!curdrive)
	{
		ret=f_rmdir("a:../a/ab");
		if (ret) return _f_result(__LINE__,ret);
		ret=f_rmdir("A:/a/abc");
		if (ret) return _f_result(__LINE__,ret);
	}
	else if (curdrive==1)
	{
		ret=f_rmdir("b:../a/ab");
		if (ret) return _f_result(__LINE__,ret);
		ret=f_rmdir("B:/a/abc");
		if (ret) return _f_result(__LINE__,ret);
	}
	else if (curdrive==2)
	{
		ret=f_rmdir("c:../a/ab");
		if (ret) return _f_result(__LINE__,ret);
		ret=f_rmdir("C:/a/abc");
		if (ret) return _f_result(__LINE__,ret);
	}
	ret=f_rmdir(".././abc/.././a/../a/abcd");
	if (ret) return _f_result(__LINE__,ret);

/* some invalid rmdir */
	ret=f_rmdir(".");
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);
	ret=f_rmdir("..");
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);

/* create again abc remove abc */
	ret=f_mkdir(".././abc");
	if (ret!=F_ERR_DUPLICATED) return _f_result(__LINE__,ret);
	ret=f_rmdir("../abc");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_mkdir(".././abc");
	if (ret!=F_ERR_INVALIDDIR) return _f_result(__LINE__,ret); /* cwd is not exist */
	ret=f_chdir("/");
	if (ret) return _f_result(__LINE__,ret);

/* try . and .. in the root */
	ret=f_chdir(".");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_chdir("./././.");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_chdir("..");
	if (ret!=F_ERR_NOTFOUND) return _f_result(__LINE__,ret);
#ifdef HCC_UNICODE
	ret=_f_checkcwd(_towchar(tmpwch,"/")); /* root! */
#else
	ret=_f_checkcwd("/"); /* root! */
#endif
	if (ret) return _f_result(__LINE__,ret);

/* test . and .. in a and remove a */
	ret=f_chdir("a");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_chdir("..");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_chdir("a");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_chdir(".");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_chdir("a");
	if (ret!=F_ERR_NOTFOUND) return _f_result(__LINE__,ret);
	ret=f_chdir("./..");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_rmdir("a");
	if (ret) return _f_result(__LINE__,ret);

	ret=f_mkdir("a.a");
	if (ret) return _f_result(__LINE__,ret);

	ret=f_chdir("a.a");
	if (ret) return _f_result(__LINE__,ret);

	ret=f_rmdir("../a.a");
	if (ret) return _f_result(__LINE__,ret);

	ret=f_chdir("/");
	if (ret) return _f_result(__LINE__,ret);

#if F_LONGFILENAME
	ret=f_mkdir(".foo");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_mkdir("..foo");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_mkdir("...foo");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_mkdir("....foo");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_chdir("foo~1");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_chdir("../foo~2");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_chdir("../foo~3");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_chdir("../foo~4");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_chdir("../foo~5");
	if (ret!=F_ERR_NOTFOUND) return _f_result(__LINE__,ret);
	ret=f_chdir("..");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_rmdir(".foo");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_rmdir("..foo");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_rmdir("...foo");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_rmdir("....foo");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_rmdir(".....foo");
	if (ret!=F_ERR_NOTFOUND) return _f_result(__LINE__,ret);
#endif

	ret=f_mkdir("..foo");
#if F_LONGFILENAME
	if (ret) return _f_result(__LINE__,ret);
#else
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);
#endif

	ret=f_chdir("..foo");
#if F_LONGFILENAME
	if (ret) return _f_result(__LINE__,ret);
	ret=f_chdir("..");
	if (ret) return _f_result(__LINE__,ret);
#else
	if (ret!=F_ERR_NOTFOUND) return _f_result(__LINE__,ret);
#endif

	ret=f_chdir(".foo");
#if F_LONGFILENAME
	if (ret!=F_ERR_NOTFOUND) return _f_result(__LINE__,ret);
#else
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);
#endif
	ret=f_chdir("foo");
	if (ret!=F_ERR_NOTFOUND) return _f_result(__LINE__,ret);

	/* test .foo */
	file=f_open("..foo/.foo","w+");
#if	F_LONGFILENAME
	if (!file) return _f_result(__LINE__,0);

	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);

	ret=f_delete("..foo/.foo");
	if (ret) return _f_result(__LINE__,ret);

#else
	if (file) return _f_result(__LINE__,0);

	ret=f_getlasterror();
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);
#endif


	ret=f_rmdir("..foo");
#if F_LONGFILENAME
	if (ret) return _f_result(__LINE__,ret);
#else
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);
#endif

/* check if all are removed */
	ret=f_findfirst("*.*",&find);
	if (ret!=F_ERR_NOTFOUND) return _f_result(__LINE__,ret);

	_f_dump("passed...");
	return 0;
}

/****************************************************************************
 *
 * f_findingtest
 *
 * test case
 *
 * RETURNS
 *
 * error code or 0 if success
 *
 ***************************************************************************/

static int f_findingtest(void)
{
	int ret,cou;
	F_FILE *file;

	_f_dump("f_findingtest");

/* check empty */
	ret=f_findfirst("*.*",&find);
	if (ret!=F_ERR_NOTFOUND) return _f_result(__LINE__,ret);

/* create Hello.dir */
	ret=f_mkdir("Hello.dir");
	if (ret) return _f_result(__LINE__,ret);

/* check if it is exist, and only exist */
	ret=f_findfirst("*.*",&find);
	if (ret) return _f_result(__LINE__,ret);

#ifdef HCC_UNICODE
	if (STRCMP(find.filename,"Hello.dir")) return _f_result(__LINE__,0);
#else
	if (STRCMP(find.filename,f_nameconv("Hello.dir"))) return _f_result(__LINE__,0);
#endif
	if (find.attr!=F_ATTR_DIR) return _f_result(__LINE__,0);
	ret=f_findnext(&find);
	if (ret!=F_ERR_NOTFOUND) return _f_result(__LINE__,ret);

/* check some not founds */
	ret=f_findfirst("q*.*",&find);
	if (ret!=F_ERR_NOTFOUND) return _f_result(__LINE__,ret);
	ret=f_findfirst("Hello.",&find);
	if (ret!=F_ERR_NOTFOUND) return _f_result(__LINE__,ret);
	ret=f_findfirst("a/*.*",&find);
	if (ret!=F_ERR_INVALIDDIR) return _f_result(__LINE__,ret);
	ret=f_findfirst(".",&find);
	if (ret!=F_ERR_NOTFOUND) return _f_result(__LINE__,ret);
	ret=f_findfirst("..",&find);
	if (ret!=F_ERR_NOTFOUND) return _f_result(__LINE__,ret);
	ret=f_findfirst("?e.*",&find);
	if (ret!=F_ERR_NOTFOUND) return _f_result(__LINE__,ret);
	ret=f_findfirst("*.",&find);
	if (ret!=F_ERR_NOTFOUND) return _f_result(__LINE__,ret);
	ret=f_findfirst("*.?",&find);
	if (ret!=F_ERR_NOTFOUND) return _f_result(__LINE__,ret);
	ret=f_findfirst("*.??",&find);
	if (ret!=F_ERR_NOTFOUND) return _f_result(__LINE__,ret);


/* check some founds */
	ret=f_findfirst("*.dir",&find);
	if (ret) return _f_result(__LINE__,ret);
	ret=f_findfirst("*.d?r",&find);
	if (ret) return _f_result(__LINE__,ret);
	ret=f_findfirst("*.d??",&find);
	if (ret) return _f_result(__LINE__,ret);
	ret=f_findfirst("*.???",&find);
	if (ret) return _f_result(__LINE__,ret);
	ret=f_findfirst("?ello.???",&find);
	if (ret) return _f_result(__LINE__,ret);
	ret=f_findfirst("he??o.dir",&find);
	if (ret) return _f_result(__LINE__,ret);
	ret=f_findfirst("he?*.dir",&find);
	if (ret) return _f_result(__LINE__,ret);
	ret=f_findfirst("HELLO.DIR",&find); /* no capitals sensitivity in find!! */
	if (ret) return _f_result(__LINE__,ret);

/* change into hello.dir */
	ret=f_chdir("hello.dir");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_findfirst("*.*",&find);
	if (ret) return _f_result(__LINE__,ret);
	ret=f_findfirst("..",&find);
	if (ret) return _f_result(__LINE__,ret);
	ret=f_findfirst("??",&find);
	if (ret) return _f_result(__LINE__,ret);
	ret=f_findfirst(".",&find);
	if (ret) return _f_result(__LINE__,ret);
	ret=f_findfirst("k*.*",&find);
	if (ret!=F_ERR_NOTFOUND) return _f_result(__LINE__,ret);
	ret=f_findfirst("*.",&find);
	if (ret) return _f_result(__LINE__,ret);
	if (STRCMP(find.filename,".")) return _f_result(__LINE__,0);

	ret=f_findnext(&find);
	if (ret) return _f_result(__LINE__,ret);
	if (STRCMP(find.filename,"..")) return _f_result(__LINE__,0);

	ret=f_findnext(&find);
	if (ret!=F_ERR_NOTFOUND) return _f_result(__LINE__,ret);


	ret=f_findfirst("*.a",&find);
	if (ret!=F_ERR_NOTFOUND) return _f_result(__LINE__,ret);

/* creating testdir and find it */
	ret=f_mkdir("testdir");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_findfirst("*.",&find);
	if (ret) return _f_result(__LINE__,ret);

	if (STRCMP(find.filename,".")) return _f_result(__LINE__,0);
	ret=f_findnext(&find);
	if (ret) return _f_result(__LINE__,ret);
	if (STRCMP(find.filename,"..")) return _f_result(__LINE__,0);

	ret=f_findnext(&find);
	if (ret) return _f_result(__LINE__,ret);
#ifdef HCC_UNICODE
	if (STRCMP(find.filename,"testdir")) return _f_result(__LINE__,0);
#else
	if (STRCMP(find.filename,f_nameconv("testdir"))) return _f_result(__LINE__,0);
#endif
	ret=f_findfirst("*.*",&find);
	if (ret) return _f_result(__LINE__,ret);
	if (STRCMP(find.filename,".")) return _f_result(__LINE__,0);
	ret=f_findnext(&find);
	if (ret) return _f_result(__LINE__,ret);
	if (STRCMP(find.filename,"..")) return _f_result(__LINE__,0);
	ret=f_findnext(&find);
	if (ret) return _f_result(__LINE__,ret);
#ifdef HCC_UNICODE
	if (STRCMP(find.filename,"testdir")) return _f_result(__LINE__,0);
#else
	if (STRCMP(find.filename,f_nameconv("testdir"))) return _f_result(__LINE__,0);
#endif
	ret=f_findnext(&find);
	if (ret!=F_ERR_NOTFOUND) return _f_result(__LINE__,ret);

/* search exact file */
	ret=f_findfirst("testDir",&find); /* no capitals! */
	if (ret) return _f_result(__LINE__,ret);
#ifdef HCC_UNICODE
	if (STRCMP(find.filename,"testdir")) return _f_result(__LINE__,0);
#else
	if (STRCMP(find.filename,f_nameconv("testdir"))) return _f_result(__LINE__,0);
#endif
	ret=f_findnext(&find);
	if (ret!=F_ERR_NOTFOUND) return _f_result(__LINE__,ret);


/* go back to root and remove dirs */
	ret=f_chdir("\\");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_rmdir("Hello.dir/testdir");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_rmdir("Hello.dir");
	if (ret) return _f_result(__LINE__,ret);

/* check if all are removed */
	ret=f_findfirst("*.*",&find);
	if (ret!=F_ERR_NOTFOUND) return _f_result(__LINE__,ret);

	file=f_open("1","w+");
	if (!file) return _f_result(__LINE__,0);

#if TEST_FULL_COVERAGE
 	testdrv_set(TESTDRV_STATE,2); /* signal 2 removed */
	ret=f_close(file);
	if (ret!=F_ERR_CARDREMOVED) return _f_result(__LINE__,ret);

	ret=f_close(file);	   /* already closed by card remove */
	if (ret!=F_ERR_NOTOPEN) return _f_result(__LINE__,ret);
#else
	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);
#endif

	file=f_open("2","w+");
	if (!file) return _f_result(__LINE__,0);

#if TEST_FULL_COVERAGE
 	testdrv_set(TESTDRV_STATE,2); /* signal 2 removed */
	ret=f_flush(file);
	if (ret!=F_ERR_CARDREMOVED) return _f_result(__LINE__,ret);
	ret=f_close(file);	   /* already closed by card remove */
	if (ret!=F_ERR_NOTOPEN) return _f_result(__LINE__,ret);
#else
	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);
#endif


	file=f_open("3","w+");
	if (!file) return _f_result(__LINE__,0);
	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);

	file=f_open("4","w+");
	if (!file) return _f_result(__LINE__,0);
	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);

	file=f_open("5","w+");
	if (!file) return _f_result(__LINE__,0);
	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);

	cou=0;
	while (f_findfirst("*.*", &find) == F_NO_ERROR)
	{
		ret=f_delete(find.filename);
		if (ret) return _f_result(__LINE__,ret);
		cou++;
	}

	if (cou!=5) return _f_result(__LINE__,cou);


/* check if all are removed */
	ret=f_findfirst("*.*",&find);
	if (ret!=F_ERR_NOTFOUND) return _f_result(__LINE__,ret);


	_f_dump("passed...");
	return 0;
}

/****************************************************************************
 *
 * f_powerfail
 *
 * test case
 *
 * RETURNS
 *
 * error code or 0 if success
 *
 ***************************************************************************/

static int f_powerfail(void)
{
	int ret;

	_f_dump ("f_powerfail");

/* checking if its power fail system (RAMDRIVE is not powerfail!) */
	ret=f_mkdir("testdir");
	if (ret) return _f_result(__LINE__,ret);
	ret=_f_poweroff();
	if (ret) return _f_result(__LINE__,ret);
	ret=_f_poweron();
	if (ret) return _f_result(__LINE__,ret);
	ret=f_findfirst("testdir",&find);
	if (ret) return _f_result(__LINE__,ret);

/* checking formatting */
	ret=f_format(curdrive,TEST_FAT_MEDIA);
	if (ret) return _f_result(__LINE__,ret);
	ret=_f_poweroff();
	if (ret) return _f_result(__LINE__,ret);
	ret=_f_poweron();
	if (ret) return _f_result(__LINE__,ret);
	ret=f_findfirst("*.*",&find);
	if (ret!=F_ERR_NOTFOUND) return _f_result(__LINE__,ret);

/* checking formatting, 1st creating */
	ret=f_format(curdrive,TEST_FAT_MEDIA);
	if (ret) return _f_result(__LINE__,ret);
	ret=f_mkdir("testdir");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_findfirst("testdir",&find);
	if (ret) return _f_result(__LINE__,ret);
#ifdef HCC_UNICODE
	if (STRCMP(find.filename,"testdir")) return _f_result(__LINE__,0);
#else
	if (STRCMP(find.filename,f_nameconv("testdir"))) return _f_result(__LINE__,0);
#endif

	ret=_f_poweroff();
	if (ret) return _f_result(__LINE__,ret);
	ret=_f_poweron();
	if (ret) return _f_result(__LINE__,ret);
	ret=f_findfirst("*.*",&find);
	if (ret) return _f_result(__LINE__,ret);
#ifdef HCC_UNICODE
	if (STRCMP(find.filename,"testdir")) return _f_result(__LINE__,0);
#else
	if (STRCMP(find.filename,f_nameconv("testdir"))) return _f_result(__LINE__,0);
#endif

/* checking formatting, 2nd creating */
	ret=f_format(curdrive,TEST_FAT_MEDIA);
	if (ret) return _f_result(__LINE__,ret);
	ret=f_mkdir("testdir");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_findfirst("testdir",&find);
	if (ret) return _f_result(__LINE__,ret);
#ifdef HCC_UNICODE
	if (STRCMP(find.filename,"testdir")) return _f_result(__LINE__,0);
#else
	if (STRCMP(find.filename,f_nameconv("testdir"))) return _f_result(__LINE__,0);
#endif
	ret=f_mkdir("testdir2");
	if (ret) return _f_result(__LINE__,ret);
	ret=f_findfirst("testdir2",&find);
	if (ret) return _f_result(__LINE__,ret);
#ifdef HCC_UNICODE
	if (STRCMP(find.filename,"testdir2")) return _f_result(__LINE__,0);
#else
	if (STRCMP(find.filename,f_nameconv("testdir2"))) return _f_result(__LINE__,0);
#endif

	ret=_f_poweroff();
	if (ret) return _f_result(__LINE__,ret);
	ret=_f_poweron();
	if (ret) return _f_result(__LINE__,ret);
	ret=f_findfirst("*.*",&find);
	if (ret) return _f_result(__LINE__,ret);
#ifdef HCC_UNICODE
	if (STRCMP(find.filename,"testdir")) return _f_result(__LINE__,0);
#else
	if (STRCMP(find.filename,f_nameconv("testdir"))) return _f_result(__LINE__,0);
#endif
	ret=f_findnext(&find);
	if (ret) return _f_result(__LINE__,ret);
#ifdef HCC_UNICODE
	if (STRCMP(find.filename,"testdir2")) return _f_result(__LINE__,0);
#else
	if (STRCMP(find.filename,f_nameconv("testdir2"))) return _f_result(__LINE__,0);
#endif
	ret=f_findnext(&find);
	if (ret!=F_ERR_NOTFOUND) return _f_result(__LINE__,ret);


/* checking empty */
	ret=_f_poweroff();
	if (ret) return _f_result(__LINE__,ret);
	ret=_f_poweron();
	if (ret) return _f_result(__LINE__,ret);
	ret=f_format(curdrive,TEST_FAT_MEDIA);
	if (ret) return _f_result(__LINE__,ret);
	ret=_f_poweroff();
	if (ret) return _f_result(__LINE__,ret);
	ret=_f_poweron();
	if (ret) return _f_result(__LINE__,ret);
	ret=f_findfirst("*.*",&find);
	if (ret!=F_ERR_NOTFOUND) return _f_result(__LINE__,ret);


	_f_dump("passed...");
	return 0;
}

/****************************************************************************
 *
 * f_seeking
 *
 * test case
 *
 * RETURNS
 *
 * error code or 0 if success
 *
 ***************************************************************************/

static int f_seeking(long sectorsize)
{
	F_FILE *file,*file2,*file3;
	int ret;
	long size;
	long pos;
	long a,b;

	if (sectorsize == 128) _f_dump("f_seeking with 128");
	else if (sectorsize == 256) _f_dump("f_seeking with 256");
	else if (sectorsize == 512) _f_dump("f_seeking with 512");
	else if (sectorsize == 1024) _f_dump("f_seeking with 1024");
	else if (sectorsize == 2048) _f_dump("f_seeking with 2048");
	else if (sectorsize == 4096) _f_dump("f_seeking with 4096");
	else if (sectorsize == 8192) _f_dump("f_seeking with 8192");
	else if (sectorsize == 16384) _f_dump("f_seeking with 16384");
	else if (sectorsize == 32768) _f_dump("f_seeking with 32768");
	else _f_dump("f_seeking with random");

/* check empty file seeking */
	file=f_open("test.bin ","w+");
	if (!file) return _f_result(__LINE__,0);

	ret=f_seek(file,sectorsize,F_SEEK_SET);
	if (ret) return _f_result(__LINE__,ret);

	pos=f_tell(file);
	if (pos!=sectorsize) return _f_result(__LINE__,pos);

	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);

	size=f_filelength("test.bin");
	if (size!=sectorsize) return _f_result(__LINE__,size);


	ret=f_delete("test.bin ");
	if (ret) return _f_result(__LINE__,ret);


/* checking sector boundary seekeng */
	file3=f_open("test1.bin ","w+");
	if (!file) return _f_result(__LINE__,0);

	file=f_open("test.bin ","w+");	/* also test spacecutting at the end */
	if (!file) return _f_result(__LINE__,0);

	ret=f_chdir("test.bin/a");
	if (!ret) return _f_result(__LINE__,ret);

/* check lock */
	file2=f_open("test.bin","w+");
	if (file2) return _f_result(__LINE__,0);

	ret=f_getlasterror();
	if (ret!=F_ERR_LOCKED) return _f_result(__LINE__,ret);

	file2=f_open("test.bin","r");
	if (file2) return _f_result(__LINE__,0);

	ret=f_getlasterror();
	if (ret!=F_ERR_LOCKED) return _f_result(__LINE__,ret);

	file2=f_open("test.bin","r+");
	if (file2) return _f_result(__LINE__,0);

	ret=f_getlasterror();
	if (ret!=F_ERR_LOCKED) return _f_result(__LINE__,ret);

	file2=f_open("test.bin","a");
	if (file2) return _f_result(__LINE__,0);

	ret=f_getlasterror();
	if (ret!=F_ERR_LOCKED) return _f_result(__LINE__,ret);

	file2=f_open("test.bin","a+");
	if (file2) return _f_result(__LINE__,0);

	ret=f_getlasterror();
	if (ret!=F_ERR_LOCKED) return _f_result(__LINE__,ret);
	
	ret=f_delete("Test.Bin");
	if (ret!=F_ERR_LOCKED) return _f_result(__LINE__,ret);

	ret=f_rename("Test.Bin","cannot");
	if (ret!=F_ERR_LOCKED) return _f_result(__LINE__,ret);

	ret=f_close(file3);
	if (ret) return _f_result(__LINE__,0);

	ret=f_delete("Test1.Bin");
	if (ret) return _f_result(__LINE__,ret);

	file2=f_truncate("Test.Bin",16380);
	if (file2) return _f_result(__LINE__,0);

	ret=f_getlasterror();
	if (ret!=F_ERR_LOCKED) return _f_result(__LINE__,ret);


/* write sectorsize times 0 */
	(void)_memset(buffer,0,(unsigned int)sectorsize);
	size=f_write(buffer,1,sectorsize,file);
	if (size!=sectorsize) return _f_result(__LINE__,size);
	pos=f_tell(file);
	if (pos!=sectorsize) return _f_result(__LINE__,pos);

	ret=f_seek(file,0,-1);   /* invalid seek */
	if (ret!=F_ERR_NOTUSEABLE) return _f_result(__LINE__,ret);

/* seek back and read some */
	ret=f_seek(file,0,F_SEEK_SET);   /* seek back */
	if (ret) return _f_result(__LINE__,ret);
	pos=f_tell(file);
	if (pos) return _f_result(__LINE__,pos);
	size=f_read(buffer,1,sectorsize,file);
	if (size!=sectorsize) return _f_result(__LINE__,size);
	pos=f_tell(file);
	if (pos!=sectorsize) return _f_result(__LINE__,pos);

/* fake read at eof */
	size=f_read(buffer,1,2,file); /* eof! */
	if (size!=0) return _f_result(__LINE__,size);
	pos=f_tell(file);
	if (pos!=sectorsize) return _f_result(__LINE__,pos);

/* seek beyond eof */
	ret=f_seek(file,sectorsize+8,F_SEEK_SET); /* beyond eof! */
	if (ret) return _f_result(__LINE__,ret);
	pos=f_tell(file);
	if (pos!=sectorsize+8) return _f_result(__LINE__,pos);

/* seek back to eof*/
	ret=f_seek(file,sectorsize,F_SEEK_SET);
	if (ret) return _f_result(__LINE__,ret);
	pos=f_tell(file);
	if (pos!=sectorsize) return _f_result(__LINE__,pos);


/* writing sectorsize times 1 at the end */
	(void)_memset(buffer,1,(unsigned int)sectorsize);
	size=f_write(buffer,1,sectorsize,file);
	if (size!=sectorsize) return _f_result(__LINE__,size);
	pos=f_tell(file);
	if (pos!=sectorsize*2) return _f_result(__LINE__,pos);

/* seeking back and read 1byte less */
	ret=f_seek(file,0,F_SEEK_SET);
	if (ret) return _f_result(__LINE__,ret);
	pos=f_tell(file);
	if (pos) return _f_result(__LINE__,pos);
	size=f_read(buffer,1,sectorsize-1,file);
	if (size!=sectorsize-1) return _f_result(__LINE__,size);
	pos=f_tell(file);
	if (pos!=sectorsize-1) return _f_result(__LINE__,pos);


/* write 2 times 2 */
	(void)_memset(buffer,2,(unsigned int)sectorsize);
	size=f_write(buffer,1,2,file);
	if (size!=2) return _f_result(__LINE__,size);
	pos=f_tell(file);
	if (pos!=sectorsize+1) return _f_result(__LINE__,pos);

/* read 2 bytes */
	size=f_read(buffer,1,2,file);
	if (size!=2) return _f_result(__LINE__,size);
	pos=f_tell(file);
	if (pos!=sectorsize+3) return _f_result(__LINE__,pos);


/* write 4 times 3 */
	(void)_memset(buffer,3,(unsigned int)sectorsize);
	size=f_write(buffer,1,4,file);
	if (size!=4) return _f_result(__LINE__,size);
	pos=f_tell(file);
	if (pos!=sectorsize+3+4) return _f_result(__LINE__,pos);

/* seek at 2 */
	ret=f_seek(file,2,F_SEEK_SET); /* here *f->sector */
	if (ret) return _f_result(__LINE__,ret);
	pos=f_tell(file);
	if (pos!=2) return _f_result(__LINE__,pos);

/* write 6 times 4 */
	(void)_memset(buffer,4,(unsigned int)sectorsize);
	size=f_write(buffer,1,6,file);
	if (size!=6) return _f_result(__LINE__,size);
	pos=f_tell(file);
	if (pos!=8) return _f_result(__LINE__,pos);

/* seek end -4 */
	ret=f_seek(file,-4,F_SEEK_END);
	if (ret) return _f_result(__LINE__,ret);
	pos=f_tell(file);
	if (pos!=2*sectorsize-4) return _f_result(__LINE__,pos);

/* read 2 bytes */
	size=f_read(buffer,1,2,file);
	if (size!=2) return _f_result(__LINE__,size);
	pos=f_tell(file);
	if (pos!=2*sectorsize-2) return _f_result(__LINE__,pos);

/* write 8 times 5 */
	(void)_memset(buffer,5,(unsigned int)sectorsize);
	size=f_write(buffer,1,8,file);
	if (size!=8) return _f_result(__LINE__,size);
	pos=f_tell(file);
	if (pos!=2*sectorsize+6) return _f_result(__LINE__,pos);

/* seek to the begining */
	ret=f_seek(file,-f_tell(file)*2,F_SEEK_END);
	if (ret!=F_ERR_INVALIDPOS) return _f_result(__LINE__,ret);

	pos=f_tell(file);
	if (pos!=2*sectorsize+6) return _f_result(__LINE__,pos);

	ret=f_seek(file,-(2*sectorsize+6),F_SEEK_END);
	if (ret) return _f_result(__LINE__,ret);

	pos=f_tell(file);
	if (pos) return _f_result(__LINE__,pos);

/* seek to the end */
	ret=f_seek(file,2*sectorsize+6,F_SEEK_SET);
	if (ret) return _f_result(__LINE__,ret);




	pos=f_tell(file);
	if (pos!=2*sectorsize+6) return _f_result(__LINE__,pos);

/* write 2 times 6 */
	(void)_memset(buffer,6,(unsigned int)sectorsize);
	size=f_write(buffer,1,2,file);
	if (size!=2) return _f_result(__LINE__,size);
	pos=f_tell(file);
	if (pos!=2*sectorsize+8) return _f_result(__LINE__,pos);

/* seek to the begining */
	(void)f_seek(file,-(2*sectorsize+8),F_SEEK_CUR);
	if (ret) return _f_result(__LINE__,ret);
	pos=f_tell(file);
	if (pos) return _f_result(__LINE__,pos);

/* read 2 times sector */
	size=f_read(buffer,1,sectorsize,file);
	if (size!=sectorsize) return _f_result(__LINE__,size);
	pos=f_tell(file);
	if (pos!=sectorsize) return _f_result(__LINE__,pos);

	size=f_read(buffer,1,sectorsize,file);
	if (size!=sectorsize) return _f_result(__LINE__,size);
	pos=f_tell(file);
	if (pos!=2*sectorsize) return _f_result(__LINE__,pos);

/* write 1 once 7 */
	(void)_memset(buffer,7,(unsigned int)sectorsize);
	size=f_write(buffer,1,1,file);
	if (size!=1) return _f_result(__LINE__,size);
	pos=f_tell(file);
	if (pos!=2*sectorsize+1) return _f_result(__LINE__,pos);

/* close it */
	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);

	ret=f_rmdir("test.bin"); /* no directory! */
	if (!ret) return _f_result(__LINE__,ret);

	ret=f_seek(file,0,F_SEEK_SET);   /* file ptr is invalid*/
	if (ret!=F_ERR_NOTOPEN) return _f_result(__LINE__,ret);

	ret=f_seek((F_FILE*)-1,0,F_SEEK_SET);   /* file ptr is invalid*/
	if (ret!=F_ERR_NOTOPEN) return _f_result(__LINE__,ret);


/* check the result */
	size=f_filelength("test.bin");
	if (size!=2*sectorsize+8) return _f_result(__LINE__,size);

/* opens it */
	file=f_open("  test.bin","r"); /* also test begining spaces removing */
	if (!file) return _f_result(__LINE__,size);

	file2=f_open("test.bin","r"); /* check read lock */
	if (!file) return _f_result(__LINE__,size);

	size=f_write(buffer,1,512,file); /* write fails because of 'r' mode */
	if (size!=0) return _f_result(__LINE__,size);

	if (checkfilecontent(2,0,file)) return _f_result(__LINE__,0);
	if (checkfilecontent(6,4,file)) return _f_result(__LINE__,0);
	if (checkfilecontent(sectorsize-8-1,0,file)) return _f_result(__LINE__,0);
	if (checkfilecontent(2,2,file)) return _f_result(__LINE__,0);
	if (checkfilecontent(2,1,file)) return _f_result(__LINE__,0);
	if (checkfilecontent(4,3,file)) return _f_result(__LINE__,0);
	if (checkfilecontent(sectorsize-7-2,1,file)) return _f_result(__LINE__,0);
	if (checkfilecontent(2,5,file)) return _f_result(__LINE__,0);
	if (checkfilecontent(1,7,file)) return _f_result(__LINE__,0);
	if (checkfilecontent(5,5,file)) return _f_result(__LINE__,0);
	if (checkfilecontent(2,6,file)) return _f_result(__LINE__,0);

/* check pos result */
	pos=f_tell(file);
	if (pos!=2*sectorsize+8) return _f_result(__LINE__,pos);

/* this has to be eof */
	ret=f_eof(file);
	if (!ret) return _f_result(__LINE__,pos);

	pos=f_eof((F_FILE*)(-1)); /* this is invalid */
	if (!ret) return _f_result(__LINE__,pos);

	pos=f_tell((F_FILE*)(-1)); /* this is invalid */
	if (pos!=-1) return _f_result(__LINE__,pos);


/* close it */
	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);

	ret=f_close(file2);
	if (ret) return _f_result(__LINE__,ret);


/* deletes it */
	ret=f_delete(" ");
	if (!ret) return _f_result(__LINE__,ret);

	ret=f_delete("test.b?n");
	if (!ret) return _f_result(__LINE__,ret);

	ret=f_delete("test1.bin");
	if (!ret) return _f_result(__LINE__,ret);

	ret=f_delete("a/test1.bin");
	if (!ret) return _f_result(__LINE__,ret);

	ret=f_delete("test.bin");
	if (ret) return _f_result(__LINE__,ret);

/* seeking beyond eof */
	file=f_open("seek.bin","w+");
	if (!file) return _f_result(__LINE__,0);

	ret=f_seek(file,1024,F_SEEK_CUR);
	if (ret) return _f_result(__LINE__,ret);

	pos=f_tell(file);
	if (pos!=1024) return _f_result(__LINE__,pos);

	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);

	size=f_filelength("seek.bin");
	if (size!=1024) return _f_result(__LINE__,size);


	file=f_open("seek.bin","w");
	if (!file) return _f_result(__LINE__,0);

	ret=f_seek(file,1024,F_SEEK_CUR);
	if (ret) return _f_result(__LINE__,ret);

	pos=f_tell(file);
	if (pos!=1024) return _f_result(__LINE__,pos);

	ret=f_flush(file);
	if (ret) return _f_result(__LINE__,ret);

	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);

	size=f_filelength("seek.bin");
	if (size!=1024) return _f_result(__LINE__,size);


	file=f_open("seek.bin","r+");
	if (!file) return _f_result(__LINE__,0);

	ret=f_seek(file,1028,F_SEEK_CUR);
	if (ret) return _f_result(__LINE__,ret);

	pos=f_tell(file);
	if (pos!=1028) return _f_result(__LINE__,pos);

	ret=f_flush(file);
	if (ret) return _f_result(__LINE__,ret);

	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);

	size=f_filelength("seek.bin");
	if (size!=1028) return _f_result(__LINE__,size);


	file=f_open("seek.bin","a+");
	if (!file) return _f_result(__LINE__,0);

	ret=f_seek(file,1024,F_SEEK_CUR);
	if (ret) return _f_result(__LINE__,ret);

	ret=f_flush(file);
	if (ret) return _f_result(__LINE__,ret);

	pos=f_tell(file);
	if (pos!=2052) return _f_result(__LINE__,pos);

	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);

	size=f_filelength("seek.bin");
	if (size!=2052) return _f_result(__LINE__,size);


	file=f_open("seek.bin","a");
	if (!file) return _f_result(__LINE__,0);

	ret=f_seek(file,1024,F_SEEK_CUR);
	if (ret) return _f_result(__LINE__,ret);

	pos=f_tell(file);
	if (pos!=3076) return _f_result(__LINE__,pos);

	ret=f_flush(file);
	if (ret) return _f_result(__LINE__,ret);

	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);

	size=f_filelength("seek.bin");
	if (size!=3076) return _f_result(__LINE__,size);


	ret=f_delete("seek.bin");
	if (ret) return _f_result(__LINE__,ret);


/* seeking into the file */
	for (a=0; a<sectorsize; a++)
	{
		buffer[a]=(char)(0x30+a/7);
	}

	file=f_open("t.bin","w+");
	if (!file) return _f_result(__LINE__,0);

	size=f_write(buffer,1,sectorsize,file);
	if (size!=sectorsize) return _f_result(__LINE__,size);

	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);


	file=f_open("t.bin","r");
	if (!file) return _f_result(__LINE__,0);

	for (a=0; a<sectorsize; a+=256)
	{
		long rsize=512;
		if (rsize+a>sectorsize) rsize=sectorsize-a;

		ret=f_seek(file,a,F_SEEK_SET);
		if (ret) return _f_result(__LINE__,ret);

		size=f_read(buffer,1,rsize,file);
		if (size!=rsize) return _f_result(__LINE__,size);

		for (b=0; b<rsize; b++)
		{
			if (buffer[b]!=(char)(0x30+(a+b)/7))
			{
				return _f_result(__LINE__,a); /* content error */
			}
		}
	}
	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);

	ret=f_delete("t.bin");
	if (ret) return _f_result(__LINE__,ret);

	/* increase test */

	file=f_open("s.bin","w+");
	if (!file) return _f_result(__LINE__,0);

	(void)_memset(buffer,1,(unsigned int)sectorsize);

	size=f_write(buffer,1,10,file);
	if (size!=10) return _f_result(__LINE__,size);

	pos=f_tell(file);
	if (pos!=10) return _f_result(__LINE__,pos);

	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);



	file=f_open("s.bin","r+");
	if (!file) return _f_result(__LINE__,0);

	ret=f_seek(file,0,F_SEEK_END);
	if (ret) return _f_result(__LINE__,ret);

	pos=f_tell(file);
	if (pos!=10) return _f_result(__LINE__,pos);

	(void)_memset(buffer,2,(unsigned int)sectorsize);

	size=f_write(buffer,1,10,file);
	if (size!=10) return _f_result(__LINE__,size);

	pos=f_tell(file);
	if (pos!=20) return _f_result(__LINE__,pos);

	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);



	ret=f_delete("s.bin");
	if (ret) return _f_result(__LINE__,ret);


	_f_dump("passed...");
	return 0;
}

/****************************************************************************
 *
 * f_opening
 *
 * test case
 *
 * RETURNS
 *
 * error code or 0 if success
 *
 ***************************************************************************/

static int f_opening(void)
{
	F_FILE *file;
	F_FILE *file2;
	long ret,size,pos;
	int a;
	char sname[5];

	_f_dump("f_opening");

/* test non existing file open r, r+ */
	file=f_open("file.bin","r");
	if (file) return _f_result(__LINE__,0);
	file=f_open("file.bin","r+");
	if (file) return _f_result(__LINE__,0);

/* test non existing appends	"a" a+ */
	file=f_open("file.bin","a");
	if (!file) return _f_result(__LINE__,0);
	file2=f_open("file.bin","a+");  /* open again */
	if (file2) return _f_result(__LINE__,0);
	ret=f_close(file);
	if (ret) return _f_result(__LINE__,1);
	ret=f_close(file2);
	if (ret!=F_ERR_NOTOPEN) return _f_result(__LINE__,2);


/* try to creates it w */
	file=f_open("file.bin","w");
	if (!file) return _f_result(__LINE__,0);

/* write 512 times 1 */
	(void)_memset(buffer,1,512); /* set all 1 */
	size=f_write(buffer,1,512,file); /* test write */
	if (size!=512) return _f_result(__LINE__,size);

/* go back, and read it */
	ret=f_rewind(file); /* back to the begining */
	if (ret) return _f_result(__LINE__,ret); /* it should fail */
	size=f_read(buffer,1,512,file); /* test read */
	if (size) return _f_result(__LINE__,size); /* it should fail */

#if F_FINDOPENFILESIZE==0
	size=f_filelength("file.bin");
	if (size) return _f_result(__LINE__,size); /* has to be zero */
#endif
/* close and check size */
	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);
	size=f_filelength("file.bin");
	if (size!=512) return _f_result(__LINE__,size);

/* try to owerwrites it it */
	file=f_open("file.bin","w+");
	if (!file) return _f_result(__LINE__,0);

/* close and check size */
	size=f_filelength("file.bin");
	if (size) return _f_result(__LINE__,size); /* has to be zero */
	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);
	size=f_filelength("file.bin");
	if (size) return _f_result(__LINE__,size);



/* test non existing appends	"a"  */
	file=f_open("file.bin","a");
	if (!file) return _f_result(__LINE__,0);

/* write 512 times 1 */
	(void)_memset(buffer,1,512); /* set all 1 */
	size=f_write(buffer,1,512,file); /* test write */
	if (size!=512) return _f_result(__LINE__,size);

/* go back, and read it */
	ret=f_rewind(file); /* back to the begining */
	size=f_read(buffer,1,512,file); /* test read */
	if (size) return _f_result(__LINE__,size); /* it should fail */

/* write 256 times 2 */
	(void)_memset(buffer,2,512); /* set all 2 */
	size=f_write(buffer,1,256,file); /* test write */
	if (size!=256) return _f_result(__LINE__,size);
	pos=f_tell(file);
	if (pos!=256) return _f_result(__LINE__,pos); /* position has to be 512 */
#if F_FINDOPENFILESIZE==0
	size=f_filelength("file.bin");
	if (size) return _f_result(__LINE__,size); /* has to be zero */
#endif
/* close and check size */
	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);
	size=f_filelength("file.bin");
	if (size!=512) return _f_result(__LINE__,size);


/* test non existing appends a+ */
	file=f_open("file.bin","a+");
	if (!file) return _f_result(__LINE__,0);
	pos=f_tell(file);
	if (pos!=512) return _f_result(__LINE__,pos); /* position has to be 512 */

/* write 512 times 3 */
	(void)_memset(buffer,3,512); /* set all 3 */
	size=f_write(buffer,1,512,file); /* test write */
	if (size!=512) return _f_result(__LINE__,size);

/* go back, and read it */
	ret=f_rewind(file); /* back to the begining */
	if (ret) return _f_result(__LINE__,ret); /* it should fail */
	size=f_read(buffer,1,512,file); /* test read */
	if (size!=512) return _f_result(__LINE__,size); /* it should fail */
	pos=f_tell(file);
	if (pos!=512) return _f_result(__LINE__,pos); /* position has to be 512 */

#if F_FINDOPENFILESIZE==0
	size=f_filelength("file.bin");
	if (size!=512) return _f_result(__LINE__,size); /* has to be zero */
#endif
/* close and check size */
	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);
	size=f_filelength("file.bin");
	if (size!=1024) return _f_result(__LINE__,size);

/* close again! */
	ret=f_close(file);
	if (ret!=F_ERR_NOTOPEN) return _f_result(__LINE__,pos);


/* test invalid modes file open */
	file=f_open("file.bin"," ");
	if (file) return _f_result(__LINE__,0);
	file=f_open("file.bin","r ");
	if (file) return _f_result(__LINE__,0);
	file=f_open("file.bin","a ");
	if (file) return _f_result(__LINE__,0);
	file=f_open("file.bin","w ");
	if (file) return _f_result(__LINE__,0);
	file=f_open("file.bin","r+ ");
	if (file) return _f_result(__LINE__,0);
	file=f_open("file.bin","a+ ");
	if (file) return _f_result(__LINE__,0);
	file=f_open("file.bin","w+ ");
	if (file) return _f_result(__LINE__,0);
	file=f_open("file.bin","+r");
	if (file) return _f_result(__LINE__,0);
	file=f_open("file.bin","");
	if (file) return _f_result(__LINE__,0);

/* test invalid filename open */
	file=f_open("??","w+");
	if (file) return _f_result(__LINE__,0);

	file=f_open(".","w+");
	if (file) return _f_result(__LINE__,0);

	file=f_open("..","w+");
	if (file) return _f_result(__LINE__,0);

	file=f_open(". .","w+");
	if (file) return _f_result(__LINE__,0);

	file=f_open("*","w+");
	if (file) return _f_result(__LINE__,0);

/* test invalid path open */
	file=f_open("/invalid/path/w.bin","w+");
	if (file) return _f_result(__LINE__,0);

	file=f_open("/invalid/path/r.bin","r+");
	if (file) return _f_result(__LINE__,0);

	file=f_open("/invalid/path/a.bin","a+");
	if (file) return _f_result(__LINE__,0);

/* open max files */
#if F_LONGFILENAME
	for (a=0; a<F_MAXFILES-1; a++)
#else
	for (a=0; a<F_MAXFILES; a++)
#endif
	{
		sname[0]=(char)('0'+(a/10));
		sname[1]=(char)('0'+(a%10));
		sname[2]=0;

		file=f_open(sname,"w+");
		if (!file) return _f_result(__LINE__,a);

		files[a]=file;
	}

	file=f_open("test.bin","w+"); /* maximum files reached */
	if (file) return _f_result(__LINE__,0);


/* simple restart */
	ret=_f_poweroff();
	if (ret) return _f_result(__LINE__,ret);

	ret=_f_poweron();
	if (ret) return _f_result(__LINE__,ret);

/* open max files again */
#if F_LONGFILENAME
	for (a=0; a<F_MAXFILES-1; a++)
#else
	for (a=0; a<F_MAXFILES; a++)
#endif
	{
		sname[0]=(char)('0'+(a/10));
		sname[1]=(char)('0'+(a%10));
		sname[2]=0;

		file=f_open(sname,"w+");
		if (!file) return _f_result(__LINE__,a);

		files[a]=file;
	}

	file=f_open("test.bin","w+"); /* maximum files reached */
	if (file) return _f_result(__LINE__,0);



/* open max files again */
#if F_LONGFILENAME
	for (a=0; a<F_MAXFILES-1; a++)
#else
	for (a=0; a<F_MAXFILES; a++)
#endif
	{
		ret=f_close(files[a]);
		if (ret) return _f_result(__LINE__,ret);
	}

	ret=f_close((F_FILE*)F_MAXFILES);
	if (ret!=F_ERR_NOTOPEN) return _f_result(__LINE__,ret);

/* open max files again */
#if F_LONGFILENAME
	for (a=0; a<F_MAXFILES-1; a++)
#else
	for (a=0; a<F_MAXFILES; a++)
#endif
	{
		sname[0]=(char)('0'+(a/10));
		sname[1]=(char)('0'+(a%10));
		sname[2]=0;

		ret=f_delete(sname);
		if (ret) return _f_result(__LINE__,ret);
	}

/* test invalid filenames */
	file=f_open("a:","w+");
	if (file) return _f_result(__LINE__,0);

	file=f_open("aaa:","w+");
	if (file) return _f_result(__LINE__,0);

	file=f_open("aaa<","w+");
	if (file) return _f_result(__LINE__,0);

	file=f_open("aaa>","w+");
	if (file) return _f_result(__LINE__,0);

	file=f_open("aaa|","w+");
	if (file) return _f_result(__LINE__,0);

	file=f_open("aaa/","w+");
	if (file) return _f_result(__LINE__,0);

	file=f_open("aaa\\","w+");
	if (file) return _f_result(__LINE__,0);

	file=f_open("aaa///","w+");
	if (file) return _f_result(__LINE__,0);

	file=f_open("aaa\\\\","w+");
	if (file) return _f_result(__LINE__,0);

#if F_LONGFILENAME==0
	file=f_open("aa a","w+"); /* in long file name the space is allowed in file name */
	if (file) return _f_result(__LINE__,0);
#endif

/* test invalid extensions */
	file=f_open("a.a:a","w+");
	if (file) return _f_result(__LINE__,0);

	file=f_open("a.aa:","w+");
	if (file) return _f_result(__LINE__,0);

	file=f_open("a.aa<","w+");
	if (file) return _f_result(__LINE__,0);

	file=f_open("a.aa>","w+");
	if (file) return _f_result(__LINE__,0);

	file=f_open("a.aa|","w+");
	if (file) return _f_result(__LINE__,0);

	file=f_open("a.aa/","w+");
	if (file) return _f_result(__LINE__,0);

	file=f_open("a.aa\\","w+");
	if (file) return _f_result(__LINE__,0);

	file=f_open("a.a//","w+");
	if (file) return _f_result(__LINE__,0);

	file=f_open("a.a\\\\","w+");
	if (file) return _f_result(__LINE__,0);

#if F_LONGFILENAME==0
	file=f_open("aaa.a a","w+"); /* in long file name the space is allowed in file name */
	if (file) return _f_result(__LINE__,0);
#endif

	file=f_open("aaa.a*a","w+");
	if (file) return _f_result(__LINE__,0);

	file=f_open("aaa.a?a","w+");
	if (file) return _f_result(__LINE__,0);

#if F_LONGFILENAME==0
	file=f_open("aaa. a","w+");  /* in long file name the space is allowed in file name */
	if (file) return _f_result(__LINE__,0);
#endif

/* invalid drives */
	file=f_open("f:/aaa.aa","w+");
	if (file) return _f_result(__LINE__,0);

	file=f_open("1:/aaa.aa","w+");
	if (file) return _f_result(__LINE__,0);

	file=f_open("]:/aaa.aa","w+");
	if (file) return _f_result(__LINE__,0);

	file=f_open("A:/a a/aaa.aa","w+");
	if (file) return _f_result(__LINE__,0);


    sname[0]=34; /* invalid char " */
	sname[1]=0;

	file=f_open(sname,"w+");
	if (file) return _f_result(__LINE__,0);

    sname[0]=-1; /* valid char */
	sname[1]='.';
    sname[2]=-1; /* valid char */
	sname[3]=0;

	file=f_open(sname,"w+");
	if (!file) return _f_result(__LINE__,0);

	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);

	ret=f_delete(sname);
	if (ret) return _f_result(__LINE__,ret);


    sname[0]=F_DELETED_CHAR; /* invalid char at the begining! 8+3 */
	sname[1]='.';
    sname[2]=F_DELETED_CHAR; /* valid char */
	sname[3]=0;

	file=f_open(sname,"w+");
	if (file) return _f_result(__LINE__,0);

	sname[0]='a';
    sname[1]=F_DELETED_CHAR; /* valid char */
	sname[2]='.';
    sname[3]=F_DELETED_CHAR; /* valid char */
	sname[4]=0;

	file=f_open(sname,"w+");
	if (!file) return _f_result(__LINE__,0);

	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);

	ret=f_delete(sname);
	if (ret) return _f_result(__LINE__,ret);



    sname[0]=10; /* invalid char */
	sname[1]=0;

	file=f_open(sname,"w+");
	if (file) return _f_result(__LINE__,0);

	sname[0]='a';
	sname[1]='.';
    sname[2]=10; /* invalid char */
	sname[3]=0;

	file=f_open(sname,"w+");
	if (file) return _f_result(__LINE__,0);


/* Create 100 files */
	ret=f_mkdir("test");
	if (ret) return _f_result(__LINE__,ret);

	ret=f_chdir("test");
	if (ret) return _f_result(__LINE__,ret);

	for (a=0; a<100; a++)
	{
		sname[0]=(char)('0'+(a/10));
		sname[1]=(char)('0'+(a%10));
		sname[2]=0;

		file=f_open(sname,"w+");
		if (!file) return _f_result(__LINE__,a);

		ret=f_close(file);
		if (ret) return _f_result(__LINE__,a);
	}

	ret=f_chdir("..");
	if (ret) return _f_result(__LINE__,ret);

	ret=f_rmdir("test"); /* not empty */
	if (ret!=F_ERR_NOTEMPTY) return _f_result(__LINE__,ret);

	ret=f_chdir("test");
	if (ret) return _f_result(__LINE__,ret);

	for (a=0; a<100; a++)
	{
		sname[0]=(char)('0'+(a/10));
		sname[1]=(char)('0'+(a%10));
		sname[2]=0;

		ret=f_delete(sname);
		if (ret) return _f_result(__LINE__,a);
	}

	ret=f_chdir("..");
	if (ret) return _f_result(__LINE__,ret);

	ret=f_rmdir("test");
	if (ret) return _f_result(__LINE__,ret);


	/* test .foo */
	file=f_open(".foo","w+");
#if	F_LONGFILENAME
	if (!file) return _f_result(__LINE__,0);

	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);

	ret=f_delete("foo");
	if (ret!=F_ERR_NOTFOUND) return _f_result(__LINE__,ret);

	ret=f_delete(".foo");
	if (ret) return _f_result(__LINE__,ret);

#else
	if (file) return _f_result(__LINE__,0);
#endif


	_f_dump("passed...");
	return 0;
}


/****************************************************************************
 *
 * f_appending
 *
 * test case
 *
 * RETURNS
 *
 * error code or 0 if success
 *
 ***************************************************************************/

static int f_appending(void)
{
	F_FILE *file;
	long size,a,tsize,pos,b;
	int ret;

	_f_dump("f_appending");

	ret=f_format(curdrive,TEST_FAT_MEDIA);
	if (ret) return _f_result(__LINE__,ret);

	for (tsize=0,a=0; a<32; a++)
	{
		file=f_open("ap.bin","a");
		if (!file) return _f_result(__LINE__,0);

		(void)_memset(buffer,(char)a,sizeof(buffer));
		size=f_write(buffer,1,a+128,file);
		if (size!=a+128) return _f_result(__LINE__,size);

#if F_FINDOPENFILESIZE==0
		size=f_filelength("ap.bin");
		if (size != tsize) return _f_result(__LINE__,size);
#endif

		tsize+=a+128;

		ret=f_close(file);
		if (ret) return _f_result(__LINE__,ret);

		size=f_filelength("ap.bin");
		if (size != tsize) return _f_result(__LINE__,size);
	}

	file=f_open("ap.bin","r");
	if (!file) return _f_result(__LINE__,0);

	/* can't seek beyond eof in "r" mode */
	ret=f_seek(file,2,F_SEEK_END);
	if (ret!=F_ERR_NOTOPEN) return _f_result(__LINE__,ret);

	for (tsize=0,a=0; a<32; a++)
	{
		if (checkfilecontent(a+128,(unsigned char)a,file)) return _f_result(__LINE__,a);
	}

	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);

	for (tsize=0,a=0; a<32; a++)
	{
		file=f_open("ap.bin","r");
		if (!file) return _f_result(__LINE__,0);

		ret=f_seek(file,tsize,F_SEEK_SET);
		if (ret) return _f_result(__LINE__,ret);

		pos=f_tell(file);
		if (pos!=tsize) return _f_result(__LINE__,pos);

		size=f_read(buffer,1,a+128,file);
		if (size!=a+128) return _f_result(__LINE__,size);

		for (b=0; b<a+128; b++)
		{
			if (buffer[b]!=(char)a) return _f_result(__LINE__,a);
		}

		tsize+=a+128;

		pos=f_tell(file);
		if (pos!=tsize) return _f_result(__LINE__,pos);

		ret=f_close(file);
		if (ret) return _f_result(__LINE__,ret);
	}

	ret=f_close(file);
	if (ret!=F_ERR_NOTOPEN) return _f_result(__LINE__,ret);


	ret=f_delete("ap.bin");
	if (ret) return _f_result(__LINE__,ret);


	_f_dump("passed...");
	return 0;
}


/****************************************************************************
 *
 * f_writing
 *
 * test case
 *
 * RETURNS
 *
 * error code or 0 if success
 *
 ***************************************************************************/

static int f_writing(void)
{
	F_FILE *file;
	F_FILE *filerd;
	char *ptr;
	long size,i;
	int ret;
	F_SPACE before,after;

	_f_dump("f_writing");

#if TEST_FULL_COVERAGE
 	testdrv_set(TESTDRV_STATE,2); /* signal 2 removed */
	ret=f_getfreespace(curdrive,&before);
	if (ret!=F_ERR_CARDREMOVED) return _f_result(__LINE__,ret);
#endif

	ret=f_getfreespace(curdrive,&before);
	if (ret) return _f_result(__LINE__,ret);

	for (i=0; i<16; i++)
	{
		file=f_open("wr.bin","w");
		if (!file) return _f_result(__LINE__,0);

		(void)_memset(buffer,(char)i,sizeof(buffer));
		size=f_write(buffer,1,i*128,file);
		if (size!=i*128) return _f_result(__LINE__,size);

		ret=f_close(file);
		if (ret) return _f_result(__LINE__,ret);

		size=f_filelength("wr.bin");
		if (size != i*128) return _f_result(__LINE__,size);

		file=f_open("wr.bin","r");
		if (!file) return _f_result(__LINE__,0);

		if (checkfilecontent(i*128,(unsigned char)i,file)) return _f_result(__LINE__,i);

		ret=f_close(file);
		if (ret) return _f_result(__LINE__,ret);
	}


	for (i=0; i<16; i++)
	{
		file=f_open("wr.bin","w+");
		if (!file) return _f_result(__LINE__,0);

		(void)_memset(buffer,(char)i,sizeof(buffer));
		size=f_write(buffer,1,i*128,file);
		if (size!=i*128) return _f_result(__LINE__,size);

		ret=f_close(file);
		if (ret) return _f_result(__LINE__,ret);

		size=f_filelength("wr.bin");
		if (size != i*128) return _f_result(__LINE__,size);

		file=f_open("wr.bin","r+");
		if (!file) return _f_result(__LINE__,0);

		if (checkfilecontent(i*128,(unsigned char)i,file)) return _f_result(__LINE__,i);

		ret=f_close(file);
		if (ret) return _f_result(__LINE__,ret);
	}

	ret=f_getfreespace(curdrive,&after);
	if (ret) return _f_result(__LINE__,ret);

	if (before.bad!=after.bad) return _f_result(__LINE__,0);
	if (before.free==after.free) return _f_result(__LINE__,0);
	if (before.used==after.used) return _f_result(__LINE__,0);
	if (before.total!=after.total) return _f_result(__LINE__,0);

	if (before.used+before.free!=after.used+after.free) return _f_result(__LINE__,0);

	ret=f_delete("wr.bin");
	if (ret) return _f_result(__LINE__,ret);

	ret=f_getfreespace(curdrive,&after);
	if (ret) return _f_result(__LINE__,ret);

	if (before.bad!=after.bad) return _f_result(__LINE__,0);
	if (before.free!=after.free) return _f_result(__LINE__,0);
	if (before.used!=after.used) return _f_result(__LINE__,0);
	if (before.total!=after.total) return _f_result(__LINE__,0);



	ret=f_format(curdrive,TEST_FAT_MEDIA);
	if (ret) return _f_result(__LINE__,ret);

	ret=f_getfreespace(curdrive,&before);
	if (ret) return _f_result(__LINE__,ret);


    {
		unsigned char a[2048];
		unsigned char b[16];
		unsigned long *la=(unsigned long *)a;

		for (i=0;i<2048/4;i++) *(la+i)=(unsigned long)i;

		file=f_open("wr.bin","w");
		if (file)
		{
			(void)f_write(a,1,2048,file);
			(void)f_close(file);
		}
		else
		{
			return _f_result(__LINE__,0);
		}

		file=f_open("wr.bin","r+");
		if (file)
		{
			for (i=0;i<2048;)a[i++]=0;
			(void)f_read(a,1,2048,file);
			for (i=0;i<2048/4 && *(la+i)==(unsigned long)i;)i++;

			if (i==2048/4)
			{
				(void)f_seek(file,2032,F_SEEK_SET);
				for (i=0;i<16;i++) b[i]=(unsigned char)(0x10+i);
				(void)f_write(b,1,16,file);
				(void)f_close(file);

				file=f_open("wr.bin","r");
				if (file)
				{
					for (i=0;i<2048;)a[i++]=0;
					(void)f_read(a,1,2048,file);
					for (i=0;i<(2048-16)/4 && *(la+i)==(unsigned long)i;)i++;
					if (i==(2048-16)/4)
					{
						for (i=0;i<16 && a[i+2032]==0x10+i;)i++;
						if (i!=16) return _f_result(__LINE__,0);
					}
					else return _f_result(__LINE__,0);
				}
				else return _f_result(__LINE__,0);
			}
			else return _f_result(__LINE__,0);

			(void)f_close(file);
		}
		else return _f_result(__LINE__,0);
    }

	ret=f_format(curdrive,TEST_FAT_MEDIA);
	if (ret) return _f_result(__LINE__,ret);

	if (before.bad!=after.bad) return _f_result(__LINE__,0);
	if (before.free!=after.free) return _f_result(__LINE__,0);
	if (before.used!=after.used) return _f_result(__LINE__,0);
	if (before.total!=after.total) return _f_result(__LINE__,0);

	/* write during read */
	ptr=buffer+2048;

	file=f_open("wrrd.bin","a+");
	if (!file) return _f_result(__LINE__,0);

	filerd=f_open("wrrd.bin","r");
	if (!filerd) return _f_result(__LINE__,0);

	(void)_memset(ptr,0x55,10);
	size=f_write(ptr,1,10,file); /* 6 write in */
	if (size!=10) return _f_result(__LINE__,size);
	if (checkfilecontent(10,0x55,filerd)) return _f_result(__LINE__,0);

	(void)_memset(ptr,0x56,20);
	size=f_write(ptr,1,20,file); /* 1 write into */
	if (size!=20) return _f_result(__LINE__,size);
	if (checkfilecontent(20,0x56,filerd)) return _f_result(__LINE__,0);

	(void)_memset(ptr,0x57,512);
	size=f_write(ptr,1,512,file); /* 4 write whole */
	if (size!=512) return _f_result(__LINE__,size);
	if (checkfilecontent(512,0x57,filerd)) return _f_result(__LINE__,0);

	(void)f_seek(file,-10,F_SEEK_CUR);
	if (ret) return _f_result(__LINE__,ret);
	(void)_memset(ptr,0x58,512+20);
	size=f_write(ptr,1,512+20,file); /* 5 write over */
	if (size!=512+20) return _f_result(__LINE__,size);
	if (checkfilecontent(512+10,0x58,filerd)) return _f_result(__LINE__,0);

	(void)_memset(ptr,0x59,512-10);
	size=f_write(ptr,1,512-10,file); /* 2 finish end */
	if (size!=512-10) return _f_result(__LINE__,size);
	if (checkfilecontent(512-10,0x59,filerd)) return _f_result(__LINE__,0);

	(void)_memset(ptr,0x5a,128);
	size=f_write(ptr,1,128,file); /* 6 write in */
	if (size!=128) return _f_result(__LINE__,size);
	if (checkfilecontent(128,0x5a,filerd)) return _f_result(__LINE__,0);

	(void)_memset(ptr,0x5a,512);
	size=f_write(ptr,1,512,file); /* 1 over end */
	if (size!=512) return _f_result(__LINE__,size);
	if (checkfilecontent(512,0x5a,filerd)) return _f_result(__LINE__,0);

	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);

	ret=f_close(filerd);
	if (ret) return _f_result(__LINE__,ret);

	ret=f_delete("wrrd.bin");
	if (ret) return _f_result(__LINE__,ret);


	/* test read during write */
	file=f_open("rdwr.bin","a+");
	if (!file) return _f_result(__LINE__,0);

	filerd=f_open("rdwr.bin","r");
	if (!filerd) return _f_result(__LINE__,0);

	(void)_memset(ptr,0x55,512);
	size=f_write(ptr,1,512,file);
	if (size!=512) return _f_result(__LINE__,size);
	(void)_memset(ptr,0x56,256);
	size=f_write(ptr,1,256,file);
	if (size!=256) return _f_result(__LINE__,size);

	/* 1 get in from out */
	size=f_read(buffer,1,512+256,filerd);
	if (size!=512+256) return _f_result(__LINE__,size);

	for (i=0; i<512; i++)
	{
		if (buffer[i]!=0x55) 
		{
			return _f_result(__LINE__,buffer[i]);
		}
	}
	for (; i<256; i++)
	{
		if (buffer[i]!=0x56) 
		{
			return _f_result(__LINE__,buffer[i]);
		}
	}

	(void)_memset(ptr,0x57,256);
	size=f_write(ptr,1,256,file);
	if (size!=256) return _f_result(__LINE__,size);
	(void)_memset(ptr,0x58,512*2);
	size=f_write(ptr,1,512*2,file);
	if (size!=512*2) return _f_result(__LINE__,size);
	(void)_memset(ptr,0x59,256);
	size=f_write(ptr,1,256,file);
	if (size!=256) return _f_result(__LINE__,size);
	(void)f_seek(file,-512,F_SEEK_CUR);
	if (ret) return _f_result(__LINE__,ret);
	(void)_memset(ptr,0x5a,128);
	size=f_write(ptr,1,128,file);
	if (size!=128) return _f_result(__LINE__,size);

	/* 2 get over */
	size=f_read(buffer,1,256+512+512+256,filerd);
	if (size!=256+512+512+256) return _f_result(__LINE__,size);

	for (i=0; i<256; i++)
	{
		if (buffer[i]!=0x57) return _f_result(__LINE__,buffer[i]);
	}
	for (; i<256+512; i++)
	{
		if (buffer[i]!=0x58) return _f_result(__LINE__,buffer[i]);
	}
	for (; i<128; i++)
	{
		if (buffer[i]!=0x5a) return _f_result(__LINE__,buffer[i]);
	}
	for (; i<128; i++)
	{
		if (buffer[i]!=0x58) return _f_result(__LINE__,buffer[i]);
	}
	for (; i<256; i++)
	{
		if (buffer[i]!=0x59) return _f_result(__LINE__,buffer[i]);
	}

	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);

	ret=f_close(filerd);
	if (ret) return _f_result(__LINE__,ret);

	ret=f_delete("rdwr.bin");
	if (ret) return _f_result(__LINE__,ret);


	_f_dump("passed...");
	return 0;
}

/****************************************************************************
 *
 * f_dots
 *
 * test case
 *
 * RETURNS
 *
 * error code or 0 if success
 *
 ***************************************************************************/

static int f_dots(void)
{
	int ret;
	long a,size;
	F_FILE *file;

	_f_dump("f_dots");

	ret=f_mkdir("/tt");
	if (ret) return _f_result(__LINE__,ret);

	ret=f_chdir("/tt");
	if (ret) return _f_result(__LINE__,ret);

	ret=f_rename("invalid","test");
	if (!ret) return _f_result(__LINE__,ret);

	ret=f_rename("","test");
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);

	ret=f_rename("?","test");
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);

	ret=f_rename("invalid/a","test");
	if (!ret) return _f_result(__LINE__,ret);

	ret=f_rename(".","test");
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);

	ret=f_rename("..","test");
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);

	ret=f_rmdir(".");
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);

	ret=f_rmdir("..");
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);

	ret=f_chdir(".");
	if (ret) return _f_result(__LINE__,ret);

	ret=_f_checkcwd(f_nameconv("/tt"));
	if (ret) return _f_result(__LINE__,ret);

	ret=f_delete(".");
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);

	ret=f_delete("..");
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);

	ret=f_mkdir(".");
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);

	ret=f_mkdir("..");
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);

	ret=f_mkdir("...");
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);

	for (a=0; a<6; a++)
	{
		char *mode;
		switch (a)
		{
			case 0: mode="r"; break;
			case 1: mode="r+"; break;
			case 2: mode="w"; break;
			case 3: mode="w+"; break;
			case 4: mode="a"; break;
			case 5: mode="a+"; break;
			default: return _f_result(__LINE__,a);
		}

		file=f_open(".",mode);
		if (file) return _f_result(__LINE__,a);
		file=f_open("..",mode);
		if (file) return _f_result(__LINE__,a);
		file=f_open("...",mode);
		if (file) return _f_result(__LINE__,a);
	}

	size=f_filelength(".");
	if (size!=-1) return _f_result(__LINE__,size);
	size=f_filelength("..");
	if (size!=-1) return _f_result(__LINE__,size);
	size=f_filelength("...");
	if (size!=-1) return _f_result(__LINE__,size);


	ret=f_chdir("...");
	if (ret!=F_ERR_NOTFOUND) return _f_result(__LINE__,ret);

	ret=f_chdir("..");
	if (ret) return _f_result(__LINE__,ret);

	ret=f_rmdir("tt");
	if (ret) return _f_result(__LINE__,ret);


	_f_dump("passed...");
	return 0;
}

/****************************************************************************
 *
 * f_rit
 *
 * test case
 *
 * RETURNS
 *
 * error code or 0 if success
 *
 ***************************************************************************/

static int f_rit(void)
{
	long i;
	int ret;
	F_FILE* File;
	rit_struct_TestFileSysEntry Entry;
	long Pos;
	char Ch;
	long Founded;

	_f_dump("f_rit");

	File = f_open("MyTest","a+");
	if (!File)
	{
		return _f_result(__LINE__,0);
	}

	ret=f_seteof(File);
	if (ret!=F_ERR_NOTOPEN)	return _f_result(__LINE__,0);

	/*  add records   */
	for(i=0 ; i<RIT_NUM_OF_RECORDS ; i++)
	{
		Ch = (char)(i % 10);
		Entry.MagicNum = 0xbcbcbcbc;
		Entry.Line = (int)i;
		Entry.Buf[0] = Ch;
		Entry.Buf[10] = (char)(Ch+1);

		if(F_NO_ERROR != f_seek(File,0,F_SEEK_END))
		{
			/*  error   */
			return _f_result(__LINE__,i); /*  Fail, could not go to the end of the file  */
		}

		if(sizeof(rit_struct_TestFileSysEntry) != f_write((void*)&Entry,1,sizeof(rit_struct_TestFileSysEntry),File))
		{
			/*  error   */
			return _f_result(__LINE__,i); /*  Fail, could not write new entry  */
		}

		Pos = f_tell(File);
		if(((Pos / (long)sizeof(rit_struct_TestFileSysEntry))-1) != i)
		{
			/*  error   */
			return _f_result(__LINE__,i); /*  Fail, wrong file position  */
		}
		if(F_NO_ERROR != f_seek(File,(long)Pos - (long)sizeof(rit_struct_TestFileSysEntry),F_SEEK_SET))
		{
			/*  error   */
			return _f_result(__LINE__,i); /*  Fail, could not go to new entry position  */
		}

		if(sizeof(rit_struct_TestFileSysEntry) != f_read((void*)&Entry,1,sizeof(rit_struct_TestFileSysEntry),File))
		{
			/*  error   */
			return _f_result(__LINE__,i); /*  Fail, could not read the new entry  */
		}
		if((Entry.MagicNum != 0xbcbcbcbc) ||
			(Entry.Line != (int)i) ||
			(Entry.Buf[0] != Ch) ||
			(Entry.Buf[10] != Ch+1))
		{
			return _f_result(__LINE__,i); /* Fail, the new entry is corrupted" */
		}
	}

	ret=f_close(File);
	if (ret) return _f_result(__LINE__,ret);


	/* -----------------  */

	/* Open file again */
	File = f_open("MyTest","a+");
	if (!File) return _f_result(__LINE__,0);

	/*  read records   */
	for(i=0 ; i<RIT_NUM_OF_RECORDS ; i++)
	{
		Ch = (char)(i % 10);

		if(F_NO_ERROR != f_seek(File,0,F_SEEK_SET))
		{
			/*  error   */
			return _f_result(__LINE__,0); /*  Fail, could not go to the start of the file  */
		}

		Founded = 0;
		while(sizeof(rit_struct_TestFileSysEntry) == f_read((void*)&Entry,1,sizeof(rit_struct_TestFileSysEntry),File))
		{
			if((Entry.MagicNum == 0xbcbcbcbc) &&
				(Entry.Line == (int)i) &&
				(Entry.Buf[0] == Ch) &&
				(Entry.Buf[10] == Ch+1))
			{
				Founded = 1;
				break;
			}
		}
		if(!Founded) return _f_result(__LINE__,0); /*  Entry not founded  */
	}

	ret=f_close(File);
	if (ret) return _f_result(__LINE__,ret);

	ret=f_delete("MyTest");
	if (ret) return _f_result(__LINE__,ret);

	_f_dump("passed...");

	return 0;
}

/****************************************************************************
 *
 * f_truncating
 *
 * test case
 *
 * RETURNS
 *
 * error code or 0 if success
 *
 ***************************************************************************/

static int f_truncating(void)
{
	F_FILE *file;
	long size;
	int ret;

    _f_dump("f_truncating");

	file=f_open("test.bin","w+");
	if (!file) return _f_result(__LINE__,0);

	(void)_memset(buffer,1,16384);
	size=f_write(buffer,1,16384,file);
	if (size!=16384) return _f_result(__LINE__,size);

	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);


	file=f_truncate("",16380);
	if (file) return _f_result(__LINE__,0);

	ret=f_getlasterror();
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);

	file=f_truncate("?",16380);
	if (file) return _f_result(__LINE__,0);

	ret=f_getlasterror();
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);

	file=f_truncate("/.foo",16380);
	if (file) return _f_result(__LINE__,0);

	file=f_truncate("invalid/foo",16380);
	if (file) return _f_result(__LINE__,0);

	ret=f_getlasterror();
	if (ret!=F_ERR_INVALIDDIR) return _f_result(__LINE__,ret);


	file=f_truncate("/foo",16380);
	if (file) return _f_result(__LINE__,0);


	file=f_truncate("test.bin",16380);
	if (!file) return _f_result(__LINE__,0);
	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);
	size=f_filelength("test.bin");
	if (size!=16380) return _f_result(__LINE__,size);


	file=f_truncate("test.bin",16384);
	if (!file) return _f_result(__LINE__,0);
	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);
	size=f_filelength("test.bin");
	if (size!=16384) return _f_result(__LINE__,size);


	file=f_truncate("test.bin",8100);
	if (!file) return _f_result(__LINE__,0);
	(void)_memset(buffer,2,92);
	size=f_write(buffer,1,92,file);
	if (size!=92) return _f_result(__LINE__,size);

	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);
	size=f_filelength("test.bin");
	if (size!=8192) return _f_result(__LINE__,size);


	file=f_truncate("test.bin",1);
	if (!file) return _f_result(__LINE__,0);
	(void)_memset(buffer,3,2);
	size=f_write(buffer,1,2,file);
	if (size!=2) return _f_result(__LINE__,size);

	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);
	size=f_filelength("test.bin");
	if (size!=3) return _f_result(__LINE__,size);

	file=f_open("test.bin","r+");
	if (!file) return _f_result(__LINE__,0);

	ret=f_ftruncate(file,1);
	if (ret) return _f_result(__LINE__,ret);

	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);

	size=f_filelength("test.bin");
	if (size!=1) return _f_result(__LINE__,size);


	file=f_open("test.bin","r+");
	if (!file) return _f_result(__LINE__,0);

	ret=f_ftruncate(file,10000);
	if (ret) return _f_result(__LINE__,ret);

	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);

	size=f_filelength("test.bin");
	if (size!=10000) return _f_result(__LINE__,size);


	_f_dump("passed...");
	return 0;
}

/****************************************************************************
 *
 * f_chk
 *
 * test case
 *
 * RETURNS
 *
 * error code or 0 if success
 *
 ***************************************************************************/

#if TEST_CHKDSK_ENABLED
static int f_chk(void)
{
	int ret;

    _f_dump("f_chk");

	ret=f_checkdisk(curdrive,CHKDSK_ERASE_BAD_CHAIN|CHKDSK_ERASE_LOST_CHAIN|CHKDSK_ERASE_LOST_BAD_CHAIN);
	if (ret) return _f_result(__LINE__,ret);

	_f_dump("passed...");

	return 0;
}
#endif

/****************************************************************************
 *
 * f_seekpos
 *
 * test case
 *
 * RETURNS
 *
 * error code or 0 if success
 *
 ***************************************************************************/

static int f_seekpos(void)
{
	int ret;
	long i,a,b,size;
	F_FILE *file=0;

    _f_dump("f_seekpos");

	for (b=0; b<5; b++)
	{
		if (!b)
		{
			file=f_open("seek.bin","w+");
			if (!file) return _f_result(__LINE__,0);

			for (a=0; a<256; a++)
			{
	   			(void)_memset(buffer,(unsigned char)a,1024);
	   			size=f_write(buffer,1,1024,file);
	   			if (size!=1024) return _f_result(__LINE__,size);
			}
		}
		else if (b==1)
		{
			file=f_open("seek.bin","r");
			if (!file) return _f_result(__LINE__,0);

		}
		else if (b==2)
		{
			file=f_open("seek.bin","r");
			if (!file) return _f_result(__LINE__,0);

			for (a=0; a<256; a++)
			{
	   			if (checkfilecontent(1024,(unsigned char)a,file)) return _f_result(__LINE__,0);
			}
		}
		else if (b==3)
		{
			file=f_open("seek.bin","a+");
			if (!file) return _f_result(__LINE__,0);

		}
		else if (b==4)
		{
			file=f_open("seek.bin","r");
			if (!file) return _f_result(__LINE__,0);

			ret=f_seek(file,256*1024,F_SEEK_SET);   /* seek back */
			if (ret) return _f_result(__LINE__,ret);
		}

		ret=f_seek(file,128*1024,F_SEEK_SET);   /* seek back */
		if (ret) return _f_result(__LINE__,ret);
		if (checkfilecontent(1024,128,file)) return _f_result(__LINE__,0);

		ret=f_seek(file,64*1024,F_SEEK_SET);   /* seek back */
		if (ret) return _f_result(__LINE__,ret);
		if (checkfilecontent(1024,64,file)) return _f_result(__LINE__,0);

		ret=f_seek(file,192*1024,F_SEEK_SET);   /* seek back */
		if (ret) return _f_result(__LINE__,ret);
		if (checkfilecontent(1024,192,file)) return _f_result(__LINE__,0);


		ret=f_seek(file,32*1024,F_SEEK_SET);   /* seek back */
		if (ret) return _f_result(__LINE__,ret);
		if (checkfilecontent(1024,32,file)) return _f_result(__LINE__,0);

		ret=f_seek(file,224*1024,F_SEEK_SET);   /* seek back */
		if (ret) return _f_result(__LINE__,ret);
		if (checkfilecontent(1024,224,file)) return _f_result(__LINE__,0);

		{
			long c=F_MAXSEEKPOS;
			if (!c) c=8;
			for (a=0; a<256; a+=c)
			{
	   			ret=f_seek(file,a*1024,F_SEEK_SET);   /* seek back */
	   			if (ret) return _f_result(__LINE__,ret);
	   			if (checkfilecontent(1024,(unsigned char)a,file)) return _f_result(__LINE__,a);
			}
		}

		for (a=0; a<256; a++)
		{
   			ret=f_seek(file,a*1024,F_SEEK_SET);   /* seek back */
   			if (ret) return _f_result(__LINE__,ret);
   			if (checkfilecontent(1024,(unsigned char)a,file)) return _f_result(__LINE__,0);
		}

		ret=f_close(file);
		if (ret) return _f_result(__LINE__,ret);
	}

/* truncate test */
	file=f_truncate("seek.bin",251*1024);
	if (!file) return _f_result(__LINE__,0);

	for (a=0; a<251; a++)
	{
   		ret=f_seek(file,a*1024,F_SEEK_SET);   /* seek back */
   		if (ret) return _f_result(__LINE__,ret);
   		if (checkfilecontent(1024,(unsigned char)a,file)) return _f_result(__LINE__,0);
	}

	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);



	file=f_truncate("seek.bin",128*1024);
	if (!file) return _f_result(__LINE__,0);

	for (a=0; a<128; a++)
	{
   		ret=f_seek(file,a*1024,F_SEEK_SET);   /* seek back */
   		if (ret) return _f_result(__LINE__,ret);
   		if (checkfilecontent(1024,(unsigned char)a,file)) return _f_result(__LINE__,0);
	}


  	ret=f_seek(file,126*1024,F_SEEK_SET);   /* seek back */
  	if (ret) return _f_result(__LINE__,ret);
	ret=f_seteof(file);
  	if (ret) return _f_result(__LINE__,ret);


	for (a=0; a<126; a++)
	{
   		ret=f_seek(file,a*1024,F_SEEK_SET);   /* seek back */
   		if (ret) return _f_result(__LINE__,ret);
   		if (checkfilecontent(1024,(unsigned char)a,file)) return _f_result(__LINE__,0);
	}

  	ret=f_seek(file,64*1024,F_SEEK_SET);   /* seek back */
  	if (ret) return _f_result(__LINE__,ret);
	ret=f_seteof(file);
  	if (ret) return _f_result(__LINE__,ret);


	for (a=0; a<64; a++)
	{
   		ret=f_seek(file,a*1024,F_SEEK_SET);   /* seek back */
   		if (ret) return _f_result(__LINE__,ret);
   		if (checkfilecontent(1024,(unsigned char)a,file)) return _f_result(__LINE__,0);
	}


	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);



	ret=f_delete("seek.bin");
	if (ret) return _f_result(__LINE__,ret);


/* seekpos test */
   file=f_open("32k.bin","w+");
	if (!file) return  _f_result(__LINE__,0);

	for (a=0; a<32768; a++) buffer[a]=(char)(a/512);

	size=f_write(buffer,1,32768,file);
	if (size!=32768) return _f_result(__LINE__,size);

	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);

	for (a=0; a<32768; a+=512) buffer[a]=0;

	file=f_open("32k.bin","r");
	if (!file) return  _f_result(__LINE__,0);

	ret=f_seek(file,8192*0,F_SEEK_SET);
	if (ret) return _f_result(__LINE__,ret);
	size=f_read(buffer,1,8192,file);
	if (size!=8192) return _f_result(__LINE__,size);
	if (buffer[0]!=0) return _f_result(__LINE__,buffer[0]);

	ret=f_seek(file,8192*1,F_SEEK_SET);
	if (ret) return _f_result(__LINE__,ret);
	size=f_read(buffer,1,8192,file);
	if (size!=8192) return _f_result(__LINE__,size);
	if (buffer[0]!=16) return _f_result(__LINE__,buffer[0]);

	ret=f_seek(file,8192*2,F_SEEK_SET);
	if (ret) return _f_result(__LINE__,ret);
	size=f_read(buffer,1,8192,file);
	if (size!=8192) return _f_result(__LINE__,size);
	if (buffer[0]!=32) return _f_result(__LINE__,buffer[0]);

	ret=f_seek(file,8192*3,F_SEEK_SET);
	if (ret) return _f_result(__LINE__,ret);
	size=f_read(buffer,1,8192,file);
	if (size!=8192) return _f_result(__LINE__,size);
	if (buffer[0]!=48) return _f_result(__LINE__,buffer[0]);

	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);

/* detailed seekpos test */
	for (i=0; i<32768-8192; i+=128)
	{
		file=f_open("32k.bin","r");
		if (!file) return  _f_result(__LINE__,0);

		ret=f_seek(file,i,F_SEEK_SET);
		if (ret) return _f_result(__LINE__,ret);
		size=f_read(buffer,1,8192,file);
		if (size!=8192) return _f_result(__LINE__,size);
		if (buffer[0]!=i/512) return _f_result(__LINE__,buffer[0]);

		for (a=0; a<32768; a++) buffer[a]=0;

		ret=f_seek(file,8192*0,F_SEEK_SET);
		if (ret) return _f_result(__LINE__,ret);
		size=f_read(buffer,1,8192,file);
		if (size!=8192) return _f_result(__LINE__,size);
		if (buffer[0]!=0) return _f_result(__LINE__,buffer[0]);

		ret=f_seek(file,8192*1,F_SEEK_SET);
		if (ret) return _f_result(__LINE__,ret);
		size=f_read(buffer,1,8192,file);
		if (size!=8192) return _f_result(__LINE__,size);
		if (buffer[0]!=16) return _f_result(__LINE__,buffer[0]);

		ret=f_seek(file,8192*2,F_SEEK_SET);
		if (ret) return _f_result(__LINE__,ret);
		size=f_read(buffer,1,8192,file);
		if (size!=8192) return _f_result(__LINE__,size);
		if (buffer[0]!=32) return _f_result(__LINE__,buffer[0]);

		ret=f_seek(file,8192*3,F_SEEK_SET);
		if (ret) return _f_result(__LINE__,ret);
		size=f_read(buffer,1,8192,file);
		if (size!=8192) return _f_result(__LINE__,size);
		if (buffer[0]!=48) return _f_result(__LINE__,buffer[0]);

		ret=f_close(file);
		if (ret) return _f_result(__LINE__,ret);
	}


	ret=f_delete("32k.bin");
	if (ret) return _f_result(__LINE__,ret);


	_f_dump("passed...");
	return 0;
}

/****************************************************************************
 *
 * f_misc
 *
 * test case
 *
 * RETURNS
 *
 * error code or 0 if success
 *
 ***************************************************************************/

static int f_misc(void)
{
	int volumes[FN_MAXVOLUME];
	int ch,cnt;
	unsigned short ctime,cdate;
	unsigned char attr;
	char label[16];
	int ret;
	F_FILE *file;
	long size;
	F_STAT stat;

    _f_dump("f_misc");

	ret=f_initvolume(-1,0,0);
	if (ret!=F_ERR_INVALIDDRIVE) return _f_result(__LINE__,ret);

	ret=f_initvolume(FN_MAXVOLUME,0,0);
	if (ret!=F_ERR_INVALIDDRIVE) return _f_result(__LINE__,ret);

	ret=f_initvolume(curdrive,0,0);
	if (ret!=F_ERR_DRVALREADYMNT) return _f_result(__LINE__,ret);

	if (!curdrive)
	{
		ret=f_initvolume(1,0,0);
		if (ret!=F_ERR_INITFUNC) return _f_result(__LINE__,ret);
	}
	else
	{
		ret=f_initvolume(0,0,0);
		if (ret!=F_ERR_INITFUNC) return _f_result(__LINE__,ret);
	}

	ret=f_delvolume(-1);
	if (ret!=F_ERR_INVALIDDRIVE) return _f_result(__LINE__,ret);

	ret=f_delvolume(FN_MAXVOLUME);
	if (ret!=F_ERR_INVALIDDRIVE) return _f_result(__LINE__,ret);

	ret=f_getlabel(curdrive,label,0);
	if (ret) return _f_result(__LINE__,ret);

/* set a label*/
    ret=f_setlabel(curdrive,"{bracket}");
	if (ret) return _f_result(__LINE__,ret);

    ret=f_setlabel(curdrive,"TESTLABE");
	if (ret) return _f_result(__LINE__,ret);

    ret=f_getlabel(curdrive,label,sizeof(label));
	if (ret) return _f_result(__LINE__,ret);

	ret=STRCMP(label,"TESTLABE");
	if (ret) return _f_result(__LINE__,ret);


/* set a short label*/
    ret=f_setlabel(curdrive,"A");
	if (ret) return _f_result(__LINE__,ret);

    ret=f_getlabel(curdrive,label,sizeof(label));
	if (ret) return _f_result(__LINE__,ret);

	ret=STRCMP(label,"A");
	if (ret) return _f_result(__LINE__,ret);


/* set a long label*/
    ret=f_setlabel(curdrive,"1234567890123");
	if (ret) return _f_result(__LINE__,ret);

    ret=f_getlabel(curdrive,label,sizeof(label));
	if (ret) return _f_result(__LINE__,ret);

	ret=STRCMP(label,"12345678901"); /* 8+3 allowed! */
	if (ret) return _f_result(__LINE__,ret);

    ret=f_getlabel(curdrive,label,3);
	if (ret) return _f_result(__LINE__,ret);

	ret=STRCMP(label,"12");		      /* 2 char + terminator char */
	if (ret) return _f_result(__LINE__,ret);

    ret=f_getlabel(curdrive,label,1);
	if (ret) return _f_result(__LINE__,ret);

	ret=STRCMP(label,"");		      /* only terminator char */
	if (ret) return _f_result(__LINE__,ret);


/* set a capital an non capital label*/
    ret=f_setlabel(curdrive,"TestDrv");
	if (ret) return _f_result(__LINE__,ret);

    ret=f_getlabel(curdrive,label,sizeof(label));
	if (ret) return _f_result(__LINE__,ret);

	ret=STRCMP(label,"TESTDRV");
	if (ret) return _f_result(__LINE__,ret);

/* get/set attribute test */
	ret=f_mkdir("TestDir");
	if (ret) return _f_result(__LINE__,ret);

	file=f_open("TestDir","r");
	if (file) return _f_result(__LINE__,ret);

	file=f_open("TestDir","a");
	if (file) return _f_result(__LINE__,ret);

	file=f_open("TestDir","w");
	if (file) return _f_result(__LINE__,ret);

	ret=f_getattr("TestDira",&attr);
	if (!ret) return _f_result(__LINE__,ret);

	ret=f_getattr("",&attr);
	if (!ret) return _f_result(__LINE__,ret);

	ret=f_getattr("TestDir*",&attr);
	if (!ret) return _f_result(__LINE__,ret);

	ret=f_getattr("a/TestDir",&attr);
	if (!ret) return _f_result(__LINE__,ret);

	ret=f_getattr("TestDir",0);
	if (ret) return _f_result(__LINE__,ret);

	ret=f_getattr("TestDir",&attr);
	if (ret) return _f_result(__LINE__,ret);

	if (attr!=F_ATTR_DIR) return _f_result(__LINE__,attr);

	ret=f_setattr(" ",F_ATTR_READONLY);
	if (!ret) return _f_result(__LINE__,ret);

	ret=f_setattr("Te*Dir",F_ATTR_READONLY);
	if (!ret) return _f_result(__LINE__,ret);

	ret=f_setattr("a/TestDir",F_ATTR_READONLY);
	if (!ret) return _f_result(__LINE__,ret);

	ret=f_setattr("TeDir",F_ATTR_READONLY);
	if (!ret) return _f_result(__LINE__,ret);


	ret=f_setattr("TestDir",F_ATTR_READONLY);
	if (ret) return _f_result(__LINE__,ret);

	ret=f_getattr("TestDir",&attr);
	if (ret) return _f_result(__LINE__,ret);

	if (attr!=(F_ATTR_DIR|F_ATTR_READONLY)) return _f_result(__LINE__,attr);

	ret=f_setattr("TestDir",F_ATTR_READONLY);
	if (ret) return _f_result(__LINE__,ret);

	ret=f_getattr("TestDir",&attr);
	if (ret) return _f_result(__LINE__,ret);

	if (attr!=(F_ATTR_DIR|F_ATTR_READONLY)) return _f_result(__LINE__,attr);


	ret=f_settimedate(" ",0x1234,0x5678);
	if (!ret) return _f_result(__LINE__,ret);

	ret=f_settimedate("?",0x1234,0x5678);
	if (!ret) return _f_result(__LINE__,ret);

	ret=f_settimedate("a",0x1234,0x5678);
	if (!ret) return _f_result(__LINE__,ret);

	ret=f_settimedate("a/a",0x1234,0x5678);
	if (!ret) return _f_result(__LINE__,ret);


	ret=f_gettimedate(" ",&ctime,&cdate);
	if (!ret) return _f_result(__LINE__,ret);

	ret=f_gettimedate("*",&ctime,&cdate);
	if (!ret) return _f_result(__LINE__,ret);

	ret=f_gettimedate("a",&ctime,&cdate);
	if (!ret) return _f_result(__LINE__,ret);

	ret=f_gettimedate("a/a",&ctime,&cdate);
	if (!ret) return _f_result(__LINE__,ret);


	ret=f_gettimedate("TestDir",&ctime,0);
	if (ret) return _f_result(__LINE__,ret);

	ret=f_gettimedate("TestDir",0,&cdate);
	if (ret) return _f_result(__LINE__,ret);

	ret=f_gettimedate("TestDir",0,0);
	if (ret) return _f_result(__LINE__,ret);

	ret=f_gettimedate("TestDir",&ctime,&cdate);
	if (ret) return _f_result(__LINE__,ret);

	ret=f_settimedate("TestDir",0x1234,0x5678);
	if (ret) return _f_result(__LINE__,ret);

	ret=f_gettimedate("TestDir",&ctime,&cdate);
	if (ret) return _f_result(__LINE__,ret);

	if (ctime!=0x1234 || cdate!=0x5678) return _f_result(__LINE__,ret);

	ret=f_stat("TestDir",&stat);
	if (ret) return _f_result(__LINE__,ret);
	if (stat.modifiedtime!=0x1234 || stat.modifieddate!=0x5678) return _f_result(__LINE__,ret);

#ifdef HCC_UNICODE
	{
		wchar ts[10];
		(void)_towchar(ts,"TestDir");

		ret=f_wstat(ts,&stat);
		if (ret) return _f_result(__LINE__,ret);
	}
	if (stat.modifiedtime!=0x1234 || stat.modifieddate!=0x5678) return _f_result(__LINE__,ret);
#endif

	ret=f_rename("TestDir","Cannot");
	if (ret!=F_ERR_ACCESSDENIED) return _f_result(__LINE__,ret);

	ret=f_rmdir("TestDir");
	if (ret!=F_ERR_ACCESSDENIED) return _f_result(__LINE__,ret);

	ret=f_move("TestDir","Move");
	if (ret!=F_ERR_ACCESSDENIED) return _f_result(__LINE__,ret);


	ret=f_delete("TestDir");
	if (!ret) return _f_result(__LINE__,ret);

	file=f_truncate("TestDir",16380);
	if (file) return _f_result(__LINE__,0);

	ret=f_setattr("TestDir",0);	/* clear read only */
	if (ret) return _f_result(__LINE__,ret);

	ret=f_rename("TestDir","Cannot");
	if (ret) return _f_result(__LINE__,ret);

	ret=f_rmdir("Cannot");
	if (ret) return _f_result(__LINE__,ret);

	f_releaseFS(0x1234); /* try an non existed */
	f_releaseFS(fn_gettaskID());	 /* try an existed one */

	ret=f_rmdir("Cannot");
	if (ret!=F_ERR_TASKNOTFOUND) return _f_result(__LINE__,ret);

	ret=f_enterFS();
	if (ret) return _f_result(__LINE__,ret);

	ret=f_chdrive(curdrive); /* releaseFS removes the current drive of the user! */
	if (ret) return _f_result(__LINE__,ret);

/* put get c */
	ch=f_putc('A',(F_FILE*)-1);
	if (ch!=-1) return _f_result(__LINE__,ch);

	file=f_open("t.bin","w+");
	if (!file) return _f_result(__LINE__,ret);

	ch=f_putc('A',file);
	if (ch!='A') return _f_result(__LINE__,ch);

	ch=f_putc('B',file);
	if (ch!='B') return _f_result(__LINE__,ch);

	ch=f_putc('C',file);
	if (ch!='C') return _f_result(__LINE__,ch);

	ret=f_seek(file,-3,F_SEEK_CUR);
	if (ret) return _f_result(__LINE__,ret);

	ch=f_getc(file);
	if (ch!='A') return _f_result(__LINE__,ch);

	ch=f_getc(file);
	if (ch!='B') return _f_result(__LINE__,ch);

	ch=f_getc(file);
	if (ch!='C') return _f_result(__LINE__,ch);

	ch=f_getc(file); /* eof */
	if (ch!=-1) return _f_result(__LINE__,ch);

	ret=f_flush(file);
	if (ret) return _f_result(__LINE__,ret);

	ret=f_flush((F_FILE*)-1); /* invalid */
	if (ret!=F_ERR_NOTOPEN) return _f_result(__LINE__,ret);


	ret=f_seek(file,-2,F_SEEK_END);
	if (ret) return _f_result(__LINE__,ret);

	ch=f_getc(file);
	if (ch!='B') return _f_result(__LINE__,ch);

	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);


	ret=f_getattr("t.bin",&attr);
	if (ret) return _f_result(__LINE__,ret);

	if (attr!=F_ATTR_ARC) return _f_result(__LINE__,attr);

	ret=f_setattr("t.bin",F_ATTR_READONLY);
	if (ret) return _f_result(__LINE__,ret);

	ret=f_getattr("t.bin",&attr);
	if (ret) return _f_result(__LINE__,ret);

	if (attr!=F_ATTR_READONLY) return _f_result(__LINE__,attr);

	file=f_open("t.bin","r+");
	if (file) return _f_result(__LINE__,ret);

	file=f_open("t.bin","a");
	if (file) return _f_result(__LINE__,ret);

	file=f_open("t.bin","w");
	if (file) return _f_result(__LINE__,ret);

	ret=f_rename("t.bin","Cannot");
	if (ret!=F_ERR_ACCESSDENIED) return _f_result(__LINE__,ret);

	ret=f_delete("t.bin");
	if (ret!=F_ERR_ACCESSDENIED) return _f_result(__LINE__,ret);

	ret=f_setattr("t.bin",0);	/* clear read only */
	if (ret) return _f_result(__LINE__,ret);

	ret=f_rename("t.bin","Cannot");
	if (ret) return _f_result(__LINE__,ret);

	ret=f_delete("Cannot");
	if (ret) return _f_result(__LINE__,ret);

/* check volume */
	ret=f_checkvolume(curdrive);
	if (ret) return _f_result(__LINE__,ret);

	ret=f_checkvolume(5);
	if (!ret) return _f_result(__LINE__,ret);

	cnt=f_get_volume_count();
	if (cnt!=1) return _f_result(__LINE__,cnt); /* 1 valid volume */

	ret=f_get_volume_list(volumes);
	if (!ret) return _f_result(__LINE__,ret);

	if (volumes[0]!=curdrive) return _f_result(__LINE__,0); /* only 1 drive is valid and its letter is curdrive */

/* getdrive, chdrive*/
	ch=f_getdrive();
	if (ch!=curdrive) return _f_result(__LINE__,ret);

	ret=f_chdrive(curdrive);
	if (ret) return _f_result(__LINE__,ret);

	ch=f_getdrive();
	if (ch!=curdrive) return _f_result(__LINE__,ret);

	ret=f_chdrive(4);
	if (!ret) return _f_result(__LINE__,ret);

	ret=f_chdrive(5);
	if (!ret) return _f_result(__LINE__,ret);

	ret=f_chdrive(FN_MAXVOLUME);
	if (!ret) return _f_result(__LINE__,ret);

	ret=f_chdrive(100);
	if (!ret) return _f_result(__LINE__,ret);

	ret=f_chdrive(-1);
	if (!ret) return _f_result(__LINE__,ret);

	ch=f_getdrive();
	if (ch!=curdrive) return _f_result(__LINE__,ret);

	ret=f_chdrive(curdrive);
	if (ret) return _f_result(__LINE__,ret);

	ch=f_getdrive();
	if (ch!=curdrive) return _f_result(__LINE__,ret);


#if TEST_FULL_COVERAGE
 	testdrv_set(TESTDRV_STATE,2); /* signal 2 removed */

	ret=f_get_oem(curdrive,label,0);
	if (ret!=F_ERR_CARDREMOVED) return _f_result(__LINE__,ret);
#endif


	ret=f_get_oem(curdrive,label,0);
	if (ret) return _f_result(__LINE__,ret);

	ret=f_get_oem(curdrive,label,sizeof(label));
	if (ret) return _f_result(__LINE__,ret);

	ret=STRCMP(label,OEM_NAME); /* compare to original one*/
	if (ret) return _f_result(__LINE__,ret);

	ret=f_get_oem(curdrive,label,9);
	if (ret) return _f_result(__LINE__,ret);

	ret=STRCMP(label,OEM_NAME); /* compare to original one*/
	if (ret) return _f_result(__LINE__,ret);


/* filelength*/
	size=f_filelength(" ");
	if (size!=-1) return _f_result(__LINE__,size);

	size=f_filelength("?");
	if (size!=-1) return _f_result(__LINE__,size);

	size=f_filelength("aa/a");
	if (size!=-1) return _f_result(__LINE__,size);


#ifdef HCC_UNICODE
	ret=f_wgetdcwd(-1,cwd,F_MAXPATH);
#else
	ret=f_getdcwd(-1,cwd,F_MAXPATH);
#endif
	if (ret!=F_ERR_INVALIDDRIVE) return _f_result(__LINE__,ret);

#ifdef HCC_UNICODE
	ret=f_wgetdcwd(FN_MAXVOLUME,cwd,F_MAXPATH);
#else
	ret=f_getdcwd(FN_MAXVOLUME,cwd,F_MAXPATH);
#endif
	if (ret!=F_ERR_INVALIDDRIVE) return _f_result(__LINE__,ret);

#ifdef HCC_UNICODE
	ret=f_wgetdcwd(curdrive,cwd,0);
#else
	ret=f_getdcwd(curdrive,cwd,0);
#endif
	if (ret) return _f_result(__LINE__,ret);

#ifdef HCC_UNICODE
	ret=f_wgetdcwd(curdrive,cwd,1);
#else
	ret=f_getdcwd(curdrive,cwd,1);
#endif
	if (ret) return _f_result(__LINE__,ret);
	if (cwd[0]) return _f_result(__LINE__,ret);

#ifdef HCC_UNICODE
	ret=f_wgetcwd(cwd,0);
#else
	ret=f_getcwd(cwd,0);
#endif
	if (ret) return _f_result(__LINE__,ret);

#ifdef HCC_UNICODE
	ret=f_wgetcwd(cwd,1);
#else
	ret=f_getcwd(cwd,1);
#endif
	if (ret) return _f_result(__LINE__,ret);
	if (cwd[0]) return _f_result(__LINE__,ret);


/* create file and remove media when seeking */
	file=f_open("t.bin","w+");
	if (!file) return _f_result(__LINE__,0);
#if TEST_FULL_COVERAGE
	/* invalid seek */
 	testdrv_set(TESTDRV_STATE,2);	 /* signal 2 removed */
	ret=f_seek(file,0,F_SEEK_SET);   /* seek back */
	if (ret!=F_ERR_CARDREMOVED) return _f_result(__LINE__,ret);

	ret=f_close(file);
	if (ret!=F_ERR_NOTOPEN) return _f_result(__LINE__,ret);
#else
	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);
#endif

	ret=f_delete("t.bin");
	if (ret) return _f_result(__LINE__,ret);




	_f_dump("passed...");
	return 0;
}

/****************************************************************************
 *
 * f_renaming
 *
 * test case
 *
 * RETURNS
 *
 * error code or 0 if success
 *
 ***************************************************************************/

static int f_renaming(void)
{
	F_FILE *file;
	int ret;

    _f_dump("f_renaming");

/* directories */
	ret=f_mkdir("tT");
	if (ret) return _f_result(__LINE__,ret);

	ret=f_rename("tt/.","hello");
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);

	ret=f_rename("tt",".");
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);

	ret=f_rename("tt","..");
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);

	ret=f_rename("tt","...");
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);

	ret=f_move("tt","..");
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);

	ret=f_move("tt","...");
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);

	ret=f_move(".","a");
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);

	ret=f_move("tt",".");
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);

	ret=f_move("tt/.","a");
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);

	ret=f_move("ttt","Cannot");
	if (ret!=F_ERR_NOTFOUND) return _f_result(__LINE__,ret);

	ret=f_move("ttt/tt","Cannot");
	if (ret!=F_ERR_INVALIDDIR) return _f_result(__LINE__,ret);

	ret=f_move("","Cannot");
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);

	ret=f_move("tt","");
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);

	ret=f_move("tt","f:/tt");  /* can't move between drives !! */
	if (ret!=F_ERR_INVALIDDRIVE) return _f_result(__LINE__,ret);

	ret=f_move("tt","ttt/q");
	if (ret!=F_ERR_INVALIDDIR) return _f_result(__LINE__,ret);

	ret=f_move("tt",".q");
#if F_LONGFILENAME
	if (ret) return _f_result(__LINE__,ret);
	ret=f_move(".q","tt");
	if (ret) return _f_result(__LINE__,ret);
#else
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);
#endif

	ret=f_move("tt","...q");
#if F_LONGFILENAME
	if (ret) return _f_result(__LINE__,ret);
	ret=f_move("...q","tt");
	if (ret) return _f_result(__LINE__,ret);
#else
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);
#endif

	ret=f_move("tt","..");
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);

	ret=f_move("tt",".");
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);


	ret=f_rename("tt","  ");
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);

	ret=f_rename("tt","");
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);

	ret=f_rename("tt","a? ");
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);

	ret=f_rename("tt","tt");
	if (ret!=F_ERR_DUPLICATED) return _f_result(__LINE__,ret);


	ret=f_rename("  ","");
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);


	ret=f_rename("tt","tt/tt");
	if (ret!=F_ERR_INVALIDDIR) return _f_result(__LINE__,ret);

	ret=f_rename("tt","\\tt");
	if (ret!=F_ERR_DUPLICATED) return _f_result(__LINE__,ret);


	ret=f_mkdir("tT/a");
	if (ret) return _f_result(__LINE__,ret);

	ret=f_mkdir("tT/a/c");
	if (ret) return _f_result(__LINE__,ret);


	ret=f_rename("tt","tt/a");
	if (ret!=F_ERR_DUPLICATED) return _f_result(__LINE__,ret);

	ret=f_rename("tt","\\a");
	if (ret) return _f_result(__LINE__,ret);

	ret=f_rename("a/a","\\a");
	if (ret!=F_ERR_DUPLICATED) return _f_result(__LINE__,ret);

	ret=f_rename("a/a","a/.././b");
	if (ret) return _f_result(__LINE__,ret);


	ret=f_rename("a","Tt.tT");
	if (ret) return _f_result(__LINE__,ret);

	ret=f_rmdir("tT.Tt");
	if (ret) return _f_result(__LINE__,ret);

	ret=f_rmdir("\\b");
	if (ret!=F_ERR_NOTEMPTY) return _f_result(__LINE__,ret);

	ret=f_rmdir("\\b\\c");
	if (ret) return _f_result(__LINE__,ret);

	ret=f_rmdir("\\b");
	if (ret) return _f_result(__LINE__,ret);

/* files */

	ret=f_mkdir("a");
	if (ret) return _f_result(__LINE__,ret);

	ret=f_mkdir("a/b");
	if (ret) return _f_result(__LINE__,ret);

	ret=f_mkdir("a/c");
	if (ret) return _f_result(__LINE__,ret);


	file=f_open("a/b/test.bin","w+");
	if (!file) return _f_result(__LINE__,0);

	ret=f_move("a/b/test.bin","a/c/new.bin");
	if (ret!=F_ERR_LOCKED) return _f_result(__LINE__,ret);

	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);


	ret=f_move("a/b/test.bin","");
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);

	ret=f_move("a/b/test.bin","f:/tt");  /* can't move between drives !! */
	if (ret!=F_ERR_INVALIDDRIVE) return _f_result(__LINE__,ret);

	ret=f_move("a/b/test.bin","ttt/q");
	if (ret!=F_ERR_INVALIDDIR) return _f_result(__LINE__,ret);

	ret=f_move("a/b/test.bin",".q");
#if F_LONGFILENAME
	if (ret) return _f_result(__LINE__,ret);
	ret=f_move(".q","a/b/test.bin");
	if (ret) return _f_result(__LINE__,ret);
#else
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);
#endif

	ret=f_move("a/b/test.bin","...q");
#if F_LONGFILENAME
	if (ret) return _f_result(__LINE__,ret);
	ret=f_move("...q","a/b/test.bin");
	if (ret) return _f_result(__LINE__,ret);
#else
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);
#endif

	ret=f_move("a/b/test.bin","..");
	if (ret!=F_ERR_INVALIDNAME) return _f_result(__LINE__,ret);

	ret=f_move("a/b/test.bin","a/b/test.bin");
	if (ret!=F_ERR_DUPLICATED) return _f_result(__LINE__,ret);


	ret=f_move("a/b/test.bin","a/c/new.bin");
	if (ret) return _f_result(__LINE__,ret);

	_f_dump("passed...");
	return 0;
}

/****************************************************************************
 *
 * f_badblock
 *
 * test case
 *
 * RETURNS
 *
 * error code or 0 if success
 *
 ***************************************************************************/

#if	TEST_FULL_COVERAGE

static int f_badblock(void)
{
	F_FILE *file;
	int ret,badnum;
	long a,c,size;
	char *wbuff;

    _f_dump("f_badblock");

	/* simple bad block generation up to 16 write fails on all writes */
	for (badnum=0; badnum<16; badnum++)
	{
		/* set buffer content */
		for (a=0; a<32768; a++)
		{
			buffer[a]=(char)(a+badnum);
		}

		/* reset ptr */
		wbuff=buffer;

		/* writes it with write error */
		file=f_open("t.bin","w+");
		if (!file) return _f_result(__LINE__,0);

		for (a=0; a<50; a++)
		{
 			testdrv_set(TESTDRV_WR,badnum);

			size=f_write(wbuff,1,600,file);
			if (size!=600)
			{
				return _f_result(__LINE__,size);
			}

			wbuff+=600; /* fo to next area */
		}

		ret=f_close(file);
		if (ret) return _f_result(__LINE__,ret);

		/* reset buffer */
		(void)memset(buffer,0,32768);

		/* read back into buffer */
		file=f_open("t.bin","r");
		if (!file) return _f_result(__LINE__,0);

		size=f_read(buffer,1,600*50,file);
		if (size!=600*50) return _f_result(__LINE__,size);

		for (a=0; a<600*50; a++)
		{
			if (buffer[a]!=(char)(a+badnum))
			{
				return _f_result(__LINE__,a); /* content error */
			}
		}

		ret=f_close(file);
		if (ret) return _f_result(__LINE__,ret);

		ret=f_delete("t.bin");
		if (ret) return _f_result(__LINE__,ret);
	}

	/* test with reformatting upto 32 bad writes */
	for (badnum=0; badnum<32; badnum++)
	{
		/* set buffer content */
		for (a=0; a<32768; a++)
		{
			buffer[a]=(char)(a+badnum);
		}

		/* reset ptr */
		wbuff=buffer;

		/* reformat media (removing bad blocks */
		ret=f_format(curdrive,TEST_FAT_MEDIA);
		if (ret) return _f_result(__LINE__,ret);

		/* writes it with write error */
		file=f_open("t.bin","w+");
		if (!file) return _f_result(__LINE__,0);

		for (a=0; a<50; a++)
		{
 			testdrv_set(TESTDRV_WR,badnum);

			size=f_write(wbuff,1,600,file);
			if (size!=600)
			{
				return _f_result(__LINE__,size);
			}

			wbuff+=600; /* fo to next area */
		}

		ret=f_close(file);
		if (ret) return _f_result(__LINE__,ret);

		/* reset buffer */
		(void)memset(buffer,0,32768);

		/* read back into buffer */
		file=f_open("t.bin","r");
		if (!file) return _f_result(__LINE__,0);

		size=f_read(buffer,1,600*50,file);
		if (size!=600*50) return _f_result(__LINE__,size);

		for (a=0; a<600*50; a++)
		{
			if (buffer[a]!=(char)(a+badnum))
			{
				return _f_result(__LINE__,a); /* content error */
			}
		}

		ret=f_close(file);
		if (ret) return _f_result(__LINE__,ret);

		ret=f_delete("t.bin");
		if (ret) return _f_result(__LINE__,ret);
	}

	/* reset buffer */
	(void)memset(buffer,0xff,32768);

	/* creating empty test file */
	file=f_open("t.bin","w+");
	if (!file) return _f_result(__LINE__,0);

	size=f_write(buffer,1,600*50,file);
	if (size!=600*50) return _f_result(__LINE__,size);

	ret=f_close(file);
	if (ret) return _f_result(__LINE__,ret);

	/* test with inside seek bad write */

	for (badnum=0; badnum<12; badnum++)
	{
		/* set buffer content */
		for (a=0; a<32768; a++)
		{
			buffer[a]=(char)(0x80+a+badnum);
		}

		/* reset ptr */
		wbuff=buffer;

		DEBPR1("seekbad %d\n",badnum)

		for (a=0; a<50; a++)
		{
			/* writes part with write error */
			file=f_open("t.bin","r+");
			if (!file) return _f_result(__LINE__,0);

 			testdrv_set(TESTDRV_WR,badnum);

			ret=f_seek(file,a*600,F_SEEK_SET);
			if (ret) return _f_result(__LINE__,ret);

			if (a*600!=f_tell(file)) return _f_result(__LINE__,ret);

			size=f_write(wbuff,1,600,file);
			if (size!=600)
			{
				return _f_result(__LINE__,size);
			}

			ret=f_close(file);
			if (ret) return _f_result(__LINE__,ret);

			wbuff+=600; /* fo to next area */
		}

		/* reset buffer */
		(void)memset(buffer,0x2,32768);

		DEBPR1("seekbadchk %d\n",badnum)

		/* read back into buffer */
		file=f_open("t.bin","r");
		if (!file) return _f_result(__LINE__,0);

		size=f_read(buffer,1,600*50,file);
		if (size!=600*50) return _f_result(__LINE__,size);

		for (a=0; a<600*50; a++)
		{
			if (buffer[a]!=(char)(0x80+a+badnum))
			{
				return _f_result(__LINE__,a); /* content error */
			}
		}

		ret=f_close(file);
		if (ret) return _f_result(__LINE__,ret);

	}

	ret=f_delete("t.bin");
	if (ret) return _f_result(__LINE__,ret);

	/* reformat media (removing bad blocks */
	ret=f_format(curdrive,TEST_FAT_MEDIA);
	if (ret) return _f_result(__LINE__,ret);


	for (badnum=0; badnum<12; badnum++)
	{
		for (c=0; c<16; c++)
		{
			for (a=0; a<32768; a++)
			{
				buffer[a]=(char)(0x40+a/7+badnum+c);
			}

			file=f_open("t.bin","w+");
			if (!file) return _f_result(__LINE__,0);

			ret=f_seek(file,0,F_SEEK_SET);
			if (ret) return _f_result(__LINE__,ret);

			if (f_tell(file)) return _f_result(__LINE__,ret);

			size=f_write(buffer,1,512*c,file);
			if (size!=512*c) return _f_result(__LINE__,size);

			ret=f_close(file);
			if (ret) return _f_result(__LINE__,ret);


			file=f_open("t.bin","r+");
			if (!file) return _f_result(__LINE__,0);

			ret=f_seek(file,0,F_SEEK_END);
			if (ret) return _f_result(__LINE__,ret);

 			testdrv_set(TESTDRV_WR,badnum);

			size=f_write(buffer+512*c,1,512,file);
			if (size!=512) return _f_result(__LINE__,size);

			ret=f_seek(file,0,F_SEEK_SET);
			if (ret) return _f_result(__LINE__,ret);

			ret=f_seek(file,0,F_SEEK_END);
			if (ret) return _f_result(__LINE__,ret);

 			testdrv_set(TESTDRV_WR,badnum);

			size=f_write(buffer+512*(c+1),1,512,file);
			if (size!=512) return _f_result(__LINE__,size);


 			testdrv_set(TESTDRV_WR,badnum);

			ret=f_seek(file,0,F_SEEK_SET);
			if (ret) return _f_result(__LINE__,ret);

			ret=f_close(file);
			if (ret) return _f_result(__LINE__,ret);


			file=f_open("t.bin","r");
			if (!file) return _f_result(__LINE__,0);

			size=f_read(buffer,1,512*(c+2),file);
			if (size!=512*(c+2)) return _f_result(__LINE__,size);

			for (a=0; a<512*(c+2); a++)
			{
				if (buffer[a]!=(char)(0x40+a/7+badnum+c))
				{
					return _f_result(__LINE__,a); /* content error */
				}
			}

			ret=f_close(file);
			if (ret) return _f_result(__LINE__,ret);
		}
	}

 	testdrv_set(TESTDRV_STATE,2); /* signal 2 removed */

	ret=f_delete("t.bin"); /* can't be */
	if (ret!=F_ERR_CARDREMOVED) return _f_result(__LINE__,ret);

 	testdrv_set(TESTDRV_STATE,1); /* signal 1 removed */

	ret=f_delete("t.bin"); /* work */
	if (ret) return _f_result(__LINE__,ret);


	/* reformat media (removing bad blocks */
	ret=f_format(curdrive,TEST_FAT_MEDIA);
	if (ret) return _f_result(__LINE__,ret);


	_f_dump("passed...");
	return 0;
}

#endif

/****************************************************************************
 *
 * f_partition
 *
 * test case
 *
 * RETURNS
 *
 * error code or 0 if success
 *
 ***************************************************************************/

#if	TEST_FULL_COVERAGE

static F_PARTITION par1[1]=
{
	{1000, F_SYSIND_DOSFAT16UPTO32MB}
};

static F_PARTITION par2[2]=
{
	{1000, F_SYSIND_DOSFAT16UPTO32MB},
	{2000, F_SYSIND_DOSFAT16UPTO32MB}
};

static F_PARTITION par3[3]=
{
	{1000, F_SYSIND_DOSFAT16UPTO32MB},
	{2000, F_SYSIND_DOSFAT16UPTO32MB},
	{3000, F_SYSIND_DOSFAT16UPTO32MB}
};

static F_PARTITION par8[8]=
{
	{1000, F_SYSIND_DOSFAT16UPTO32MB},
	{2000, F_SYSIND_DOSFAT16UPTO32MB},
	{3000, F_SYSIND_DOSFAT16UPTO32MB},
	{4000, F_SYSIND_DOSFAT16UPTO32MB},

	{5000, F_SYSIND_DOSFAT16UPTO32MB},
	{6000, F_SYSIND_DOSFAT16UPTO32MB},
	{7000, F_SYSIND_DOSFAT16UPTO32MB},
	{8000, F_SYSIND_DOSFAT16UPTO32MB}
};

#define F_PARCHK 10
static F_PARTITION parchk[F_PARCHK];

static int f_partition(void)
{
	F_DRIVER *driver;
	int ret;
	int par;
	F_SPACE sp;

    _f_dump("f_partition");

	f_init();

	f_enterFS();

	ret=f_createdriver(&driver,f_testramdrvinit,3);
	if (ret) return	_f_result(__LINE__,ret);

	ret=f_createpartition(driver,1,par1);
	if (ret) return	_f_result(__LINE__,ret);

	ret=f_initvolumepartition(0,driver,0);
	if (ret!=F_ERR_NOTFORMATTED) return	_f_result(__LINE__,ret);

	ret=f_format(0,F_FAT12_MEDIA);
	if (ret) return	_f_result(__LINE__,ret);

	ret=f_delvolume(0);
	if (ret) return	_f_result(__LINE__,ret);


	ret=f_initvolumepartition(0,driver,0);
	if (ret) return	_f_result(__LINE__,ret);

	ret=f_delvolume(0);
	if (ret) return	_f_result(__LINE__,ret);

	ret=f_getpartition(driver,F_PARCHK,parchk);
	if (ret) return	_f_result(__LINE__,ret);

	for (par=0; par<1; par++)
	{
		if (par1[par].secnum != parchk[par].secnum)
			return _f_result(__LINE__,par);
		if (par1[par].system_indicator != parchk[par].system_indicator)
			return _f_result(__LINE__,par);
	}
	for (; par<F_PARCHK; par++)
	{
		if (parchk[par].secnum)	return _f_result(__LINE__,par);
		if (parchk[par].system_indicator) return _f_result(__LINE__,par);
	}
/**/
	ret=f_createpartition(driver,2,par2);
	if (ret) return	_f_result(__LINE__,ret);

	ret=f_initvolumepartition(0,driver,0);
	if (ret!=F_ERR_NOTFORMATTED) return	_f_result(__LINE__,ret);

	ret=f_format(0,F_FAT12_MEDIA);
	if (ret) return	_f_result(__LINE__,ret);

	ret=f_initvolumepartition(1,driver,1);
	if (ret!=F_ERR_NOTFORMATTED) return	_f_result(__LINE__,ret);

	ret=f_format(1,F_FAT12_MEDIA);
	if (ret) return	_f_result(__LINE__,ret);

	ret=f_delvolume(0);
	if (ret) return	_f_result(__LINE__,ret);

	ret=f_delvolume(1);
	if (ret) return	_f_result(__LINE__,ret);

	ret=f_initvolumepartition(0,driver,0);
	if (ret) return	_f_result(__LINE__,ret);

	ret=f_initvolumepartition(1,driver,1);
	if (ret) return	_f_result(__LINE__,ret);

	ret=f_delvolume(0);
	if (ret) return	_f_result(__LINE__,ret);

	ret=f_delvolume(1);
	if (ret) return	_f_result(__LINE__,ret);


	ret=f_getpartition(driver,F_PARCHK,parchk);
	if (ret) return	_f_result(__LINE__,ret);
	for (par=0; par<2; par++)
	{
		if (par2[par].secnum != parchk[par].secnum)
			return _f_result(__LINE__,par);
		if (par2[par].system_indicator != parchk[par].system_indicator)
			return _f_result(__LINE__,par);
	}
	for (; par<F_PARCHK; par++)
	{
		if (parchk[par].secnum)	return _f_result(__LINE__,par);
		if (parchk[par].system_indicator) return _f_result(__LINE__,par);
	}
/**/
	ret=f_createpartition(driver,3,par3);
	if (ret) return	_f_result(__LINE__,ret);

	ret=f_initvolumepartition(0,driver,0);
	if (ret!=F_ERR_NOTFORMATTED) return	_f_result(__LINE__,ret);

	ret=f_format(0,F_FAT12_MEDIA);
	if (ret) return	_f_result(__LINE__,ret);

	ret=f_initvolumepartition(1,driver,1);
	if (ret!=F_ERR_NOTFORMATTED) return	_f_result(__LINE__,ret);

	ret=f_format(1,F_FAT12_MEDIA);
	if (ret) return	_f_result(__LINE__,ret);

	ret=f_initvolumepartition(2,driver,2);
	if (ret!=F_ERR_NOTFORMATTED) return	_f_result(__LINE__,ret);

	ret=f_format(2,F_FAT12_MEDIA);
	if (ret) return	_f_result(__LINE__,ret);

	ret=f_delvolume(0);
	if (ret) return	_f_result(__LINE__,ret);

	ret=f_delvolume(1);
	if (ret) return	_f_result(__LINE__,ret);

	ret=f_delvolume(2);
	if (ret) return	_f_result(__LINE__,ret);

	ret=f_initvolumepartition(0,driver,0);
	if (ret) return	_f_result(__LINE__,ret);

	ret=f_initvolumepartition(1,driver,1);
	if (ret) return	_f_result(__LINE__,ret);

	ret=f_initvolumepartition(2,driver,2);
	if (ret) return	_f_result(__LINE__,ret);

	ret=f_delvolume(0);
	if (ret) return	_f_result(__LINE__,ret);

	ret=f_delvolume(1);
	if (ret) return	_f_result(__LINE__,ret);

	ret=f_delvolume(2);
	if (ret) return	_f_result(__LINE__,ret);

	ret=f_getpartition(driver,F_PARCHK,parchk);
	if (ret) return	_f_result(__LINE__,ret);
	for (par=0; par<3; par++)
	{
		if (par3[par].secnum != parchk[par].secnum)
			return _f_result(__LINE__,par);
		if (par3[par].system_indicator != parchk[par].system_indicator)
			return _f_result(__LINE__,par);
	}
	for (; par<F_PARCHK; par++)
	{
		if (parchk[par].secnum)	return _f_result(__LINE__,par);
		if (parchk[par].system_indicator) return _f_result(__LINE__,par);
	}
/**/
	ret=f_createpartition(driver,8,par8);
	if (ret) return	_f_result(__LINE__,ret);

	for (par=0; par<8 && par<FN_MAXVOLUME; par++)
	{
		ret=f_initvolumepartition(par,driver,par);
		if (ret!=F_ERR_NOTFORMATTED) return	_f_result(__LINE__,ret);

		ret=f_format(par,F_FAT12_MEDIA);
		if (ret) return	_f_result(__LINE__,ret);

		ret=f_delvolume(par);
		if (ret) return	_f_result(__LINE__,ret);

		ret=f_initvolumepartition(par,driver,par);
		if (ret) return	_f_result(__LINE__,ret);

		ret=f_getfreespace(par,&sp);
		if (ret) return	_f_result(__LINE__,ret);

		ret=f_delvolume(par);
		if (ret) return	_f_result(__LINE__,ret);
	}

	ret=f_getpartition(driver,F_PARCHK,parchk);
	if (ret) return	_f_result(__LINE__,ret);
	for (par=0; par<8; par++)
	{
		if (par8[par].secnum != parchk[par].secnum)
			return _f_result(__LINE__,par);
		if (par8[par].system_indicator != parchk[par].system_indicator)
			return _f_result(__LINE__,par);
	}
	for (; par<F_PARCHK; par++)
	{
		if (parchk[par].secnum)	return _f_result(__LINE__,par);
		if (parchk[par].system_indicator) return _f_result(__LINE__,par);
	}

	ret=f_releasedriver(driver);
	if (ret) return	_f_result(__LINE__,ret);

	_f_dump("passed...");
	return 0;
}
#endif

/****************************************************************************
 *
 * f_dotest
 *
 * main test call
 *
 ***************************************************************************/

unsigned int f_dotest(void)
{
	unsigned char error=0;

	_f_dump("File system test started...");

#if	!TEST_FULL_COVERAGE
	curdrive=0;
#else
	for (curdrive=0; curdrive<3; curdrive++)
	{
		if (curdrive==0) test_fat_media=F_FAT12_MEDIA;
		if (curdrive==1) test_fat_media=F_FAT16_MEDIA;
		if (curdrive==2) test_fat_media=F_FAT32_MEDIA;
#endif
		if (f_formatting()) error++;
		if (f_dirtest()) error++;
		if (f_findingtest()) error++;
		if (f_powerfail()) error++;
		if (f_seeking(128)) error++;
		if (f_seeking(256)) error++;
		if (f_seeking(512)) error++;
		if (f_seeking(1024)) error++;
		if (f_seeking(2048)) error++;
		if (f_seeking(4096)) error++;
		if (f_seeking(8192)) error++;
		if (f_seeking(16384)) error++;
		if (f_seeking(32768)) error++;
		if (f_opening()) error++;
		if (f_appending()) error++;
		if (f_writing()) error++;
		if (f_dots()) error++;
		if (f_rit()) error++;
		if (f_truncating()) error++;
		if (f_seekpos()) error++;
		if (f_misc()) error++;
		if (f_renaming()) error++;
#if TEST_CHKDSK_ENABLED
		if (f_chk()) error++;
#endif
#if	TEST_FULL_COVERAGE
		if (f_badblock()) error++;
#endif
		if (_f_poweroff()) error++;

#if	TEST_FULL_COVERAGE
		if (f_partition()) error++;
#endif

#if	TEST_FULL_COVERAGE
	}
#endif

	_f_dump("End of tests...");

    /*
	if (error) (void)_f_result(-1,error);
	else if (!error) _f_dump("No error found...");
	*/
	
	return error;
}

/****************************************************************************
 *
 * end of test_f.c
 *
 ***************************************************************************/

