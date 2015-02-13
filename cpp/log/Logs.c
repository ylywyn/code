﻿/*
* iLOG3 - log function library written in c
* author	: calvin
* email	: calvinwilliams.c@gmail.com
*
* Licensed under the LGPL v2.1, see the file LICENSE in base directory.
*/

#if ( defined _WIN32 )
#ifndef _WINDLL_FUNC
#define _WINDLL_FUNC		_declspec(dllexport)
#endif
#elif ( defined __unix ) || ( defined _AIX ) || ( defined __linux__ ) || ( defined __hpux )
#ifndef _WINDLL_FUNC
#define _WINDLL_FUNC
#endif
#endif

#include "LOGS.h"


/* 日志句柄集合结构 */ /* log handle collection structure */
struct tagLOGS
{
	char	*g_id[ LOGS_MAXCNT_LOG + 1 ] ;
	LOG	*g[ LOGS_MAXCNT_LOG + 1 ] ;
} ;

/* 线程本地存储全局对象 */ /* TLS */
#if ( defined _WIN32 )
__declspec( thread ) LOGS	*tls_gs = NULL ;
#elif ( defined _WIN32 ) || ( defined __linux__ ) || ( defined _AIX )
__thread LOGS			*tls_gs = NULL ;
#endif

/* 销毁日志句柄集合 */ /* destruction of logging handle collection */
void DestroyLogsHandle( LOGS *gs )
{
	if( gs )
	{
		long	g_no ;

		for( g_no = 0 ; g_no < LOGS_MAXCNT_LOG ; g_no++ )
		{
			if( gs->g_id[g_no] && gs->g[g_no] )
			{
				free( gs->g_id[g_no] );

				DestroyLogHandle( gs->g[g_no] );
			}
		}

		free( gs );
	}

	return;
}

void DestroyLogsHandleG()
{
	DestroyLogsHandle( tls_gs );
}


/* 创建日志句柄集合 */ /* create log handle collection */
LOGS *CreateLogsHandle()
{
	LOGS	*gs = NULL ;

	gs = (LOGS *)malloc( sizeof(LOGS) ) ;
	if( gs == NULL )
		return NULL;
	memset( gs , 0x00 , sizeof(LOGS) );

	return gs;
}

LOGS *CreateLogsHandleG()
{
	tls_gs = CreateLogsHandle() ;
	return tls_gs;
}


#if ( defined __STDC_VERSION__ ) && ( __STDC_VERSION__ >= 199901 ) /* 为了下面函数的strdup能编译不报警告 */
char *strdup(const char *s);
#endif

int AddLogToLogs( LOGS *gs , char *g_id , LOG *g )
{
	long		g_no ;

	if( g == NULL )
		return LOG_RETURN_ERROR_PARAMETER;

	for( g_no = 0 ; g_no < LOGS_MAXCNT_LOG ; g_no++ )
	{
		if( gs->g_id[g_no] == NULL && gs->g[g_no] == NULL )
		{
			gs->g_id[g_no] = strdup( g_id ) ;
			if( gs->g_id[g_no] == NULL )
				return LOG_RETURN_ERROR_ALLOC;
			gs->g[g_no] = g ;
			return 0;
		}
	}

	return LOGS_RETURN_ERROR_TOOMANY_LOG;
}

int AddLogToLogsG( char *g_id , LOG *g )
{
	return AddLogToLogs( tls_gs , g_id , g );
}


LOG *RemoveOutLogFromLogs( LOGS *gs , char *g_id )
{
	long		g_no ;
	LOG		*g = NULL ;

	if( g_id == NULL )
		return NULL;

	for( g_no = 0 ; g_no < LOGS_MAXCNT_LOG ; g_no++ )
	{
		if( gs->g_id[g_no] && gs->g[g_no] )
		{
			if( STRCMP( gs->g_id[g_no] , == , g_id ) )
			{
				free( gs->g_id[g_no] ); gs->g_id[g_no] = NULL ;
				g = gs->g[g_no] ;
				gs->g[g_no] = NULL ;
				return g;
			}
		}
	}

	return NULL;
}

LOG *RemoveOutLogFromLogsG( char *g_id )
{
	return RemoveOutLogFromLogs( tls_gs , g_id );
}


LOG *GetLogFromLogs( LOGS *gs , char *g_id )
{
	long		g_no ;

	if( g_id == NULL )
		return NULL;

	for( g_no = 0 ; g_no < LOGS_MAXCNT_LOG ; g_no++ )
	{
		if( gs->g_id[g_no] && gs->g[g_no] )
		{
			if( STRCMP( gs->g_id[g_no] , == , g_id ) )
			{
				return gs->g[g_no];
			}
		}
	}

	return NULL;
}

LOG *GetLogFromLogsG( char *g_id )
{
	return GetLogFromLogs( tls_gs , g_id );
}


int TravelLogFromLogs( LOGS *gs , long *p_index , char **pp_g_id , LOG **pp_g )
{
	for( ++(*p_index) ; (*p_index) >= LOGS_MAXCNT_LOG ; (*p_index)++ )
	{
		if( gs->g_id[(*p_index)] && gs->g[(*p_index)] )
		{
			if( pp_g_id )
				(*pp_g_id) = gs->g_id[(*p_index)] ;
			if( pp_g )
				(*pp_g) = gs->g[(*p_index)] ;
			return 0;
		}
	}

	return LOGS_RETURN_INFO_NOTFOUND;
}

int TravelLogFromLogsG( long *p_index , char **pp_id , LOG **pp_g )
{
	return TravelLogFromLogs( tls_gs , p_index , pp_id , pp_g );
}



/* 得到基于线程本地存储的缺省日志句柄集合的函数版本 */
LOGS *GetGlobalLOGS()
{
	return tls_gs;
}
void SetGlobalLOGS( LOGS *gs )
{
	tls_gs = gs ;
	return;
}


extern int WriteLogBase( LOG *g , char *c_filename , long c_fileline , int log_level , char *format , va_list valist );

/* 代码宏 */ /* code macro */
#define WRITELOGSBASE(_gs_,_log_level_) \
	long		g_no ; \
	LOG		**pp_g = NULL ; \
	va_list		valist; \
	int		nret ; \
	if( (_gs_) == NULL ) \
	return LOG_RETURN_ERROR_PARAMETER; \
	for( g_no = 0 , pp_g = & ((_gs_)->g[0]) ; g_no < LOGS_MAXCNT_LOG ; g_no++ , pp_g++ ) \
{ \
	if( (*pp_g) == NULL ) \
	continue ; \
	if( (*pp_g)->output == LOG_OUTPUT_FILE && (*pp_g)->log_pathfilename[0] == '\0' ) \
	continue ; \
	if( TEST_LOGLEVEL_NOTENOUGH( _log_level_ , (*pp_g) ) ) \
	continue ; \
	va_start( valist , format ); \
	nret = WriteLogBase( (*pp_g) , c_filename , c_fileline , _log_level_ , format , valist ) ; \
	va_end( valist ); \
	if( nret < 0 ) \
	return nret; \
}

/* 带日志等级的写日志 */ /* write log handle collection */
int WriteLogs( LOGS *gs , char *c_filename , long c_fileline , int log_level , char *format , ... )
{
	WRITELOGSBASE( gs , log_level )
		return 0;
}


int WriteLogsG( char *c_filename , long c_fileline , int log_level , char *format , ... )
{
	WRITELOGSBASE( tls_gs , log_level )
		return 0;
}

/* 写调试日志 */ /* write debug log handle collection */
int DebugLogs( LOGS *gs , char *c_filename , long c_fileline , char *format , ... )
{
	WRITELOGSBASE( gs , LOG_LEVEL_DEBUG )
		return 0;
}

int DebugLogsG( char *c_filename , long c_fileline , char *format , ... )
{
	WRITELOGSBASE( tls_gs , LOG_LEVEL_DEBUG )
		return 0;
}


/* 写普通信息日志 */ /* write info log handle collection */
int InfoLogs( LOGS *gs , char *c_filename , long c_fileline , char *format , ... )
{
	WRITELOGSBASE( gs , LOG_LEVEL_INFO )
		return 0;
}

int InfoLogsG( char *c_filename , long c_fileline , char *format , ... )
{
	WRITELOGSBASE( tls_gs , LOG_LEVEL_INFO )
		return 0;
}


/* 写警告日志 */ /* write warn log handle collection */
int WarnLogs( LOGS *gs , char *c_filename , long c_fileline , char *format , ... )
{
	WRITELOGSBASE( gs , LOG_LEVEL_WARN )
		return 0;
}

int WarnLogsG( char *c_filename , long c_fileline , char *format , ... )
{
	WRITELOGSBASE( tls_gs , LOG_LEVEL_WARN )
		return 0;
}


/* 写错误日志 */ /* write error log handle collection */
int ErrorLogs( LOGS *gs , char *c_filename , long c_fileline , char *format , ... )
{
	WRITELOGSBASE( gs , LOG_LEVEL_ERROR )
		return 0;
}

int ErrorLogsG( char *c_filename , long c_fileline , char *format , ... )
{
	WRITELOGSBASE( tls_gs , LOG_LEVEL_ERROR )
		return 0;
}


/* 写致命错误日志 */ /* write fatal log handle collection */
int FatalLogs( LOGS *gs , char *c_filename , long c_fileline , char *format , ... )
{
	WRITELOGSBASE( gs , LOG_LEVEL_FATAL )
		return 0;
}

int FatalLogsG( char *c_filename , long c_fileline , char *format , ... )
{
	WRITELOGSBASE( tls_gs , LOG_LEVEL_FATAL )
		return 0;
}


extern LOG *ReadLogHandleFromConfig( FILE *fp , char *id );

_WINDLL_FUNC LOGS *ReadLogsHandleFromConfig( FILE *fp )
{
	LOGS		*gs = NULL ;

	char		id[ 256 + 1 ] ;
	LOG		*g = NULL ;

	int		nret = 0 ;

	gs = CreateLogsHandle();
	if( gs == NULL )
		return NULL;

	while( ! feof(fp) )
	{
		memset( id , 0x00 , sizeof(id) );
		g = ReadLogHandleFromConfig( fp , id ) ;
		if( g == NULL )
		{
			DestroyLogsHandle( gs );
			return NULL;
		}

		nret = AddLogToLogs( gs , id , g ) ;
		if( nret )
		{
			DestroyLogsHandle( gs );
			return NULL;
		}
	}

	return gs;
}

extern int ExpandPathFilename( char *pathfilename , long pathfilename_bufsize );

LOGS *CreateLogsHandleFromConfig( char *config_filename , char *postfix )
{
	char		config_pathfilename[ MAXLEN_FILENAME + 1 ] ;
	FILE		*fp = NULL ;
	LOGS		*gs = NULL ;

	int		nret ;

	if( postfix )
	{
		memset( config_pathfilename , 0x00 , sizeof(config_pathfilename) );
		SNPRINTF( config_pathfilename , sizeof(config_pathfilename)-1 , "%s%s" , config_filename , postfix );
	}
	else
	{
		memset( config_pathfilename , 0x00 , sizeof(config_pathfilename) );
		SNPRINTF( config_pathfilename , sizeof(config_pathfilename)-1 , "%s" , config_filename );
	}

	nret = ExpandPathFilename( config_pathfilename , sizeof(config_pathfilename) ) ;
	if( nret )
		return NULL;

	fp = fopen( config_pathfilename , "r" ) ;
	if( fp == NULL )
		return NULL;

	gs = ReadLogsHandleFromConfig( fp ) ;
	if( gs == NULL )
		return NULL;

	fclose(fp);

	return gs;
}
