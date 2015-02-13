﻿/*
* iLOG3 - log function library written in c
* author	: calvin
* email	: calvinwilliams.c@gmail.com
* LastVersion	: v1.0.11
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

#include "LOG.h"


/* 日志等级描述对照表 */ /* log level describe */
static char		sg_aszLogLevelDesc[][5+1] = { "DEBUG" , "INFO" , "WARN" , "ERROR" , "FATAL" , "NOLOG" } ;

/* 版本标识 */ /* version */
_WINDLL_FUNC int	_LOG_VERSION_1_0_11 = 0 ;

/* 线程本地存储全局对象 */ /* TLS */
#if ( defined _WIN32 )
__declspec( thread ) LOG	*tls_g = NULL ;
#elif ( defined __linux__ ) || ( defined _AIX )
__thread LOG			    *tls_g = NULL ;
#endif

/* 临界区 */ /* critical region */
#if ( defined __linux__ ) || ( defined __unix ) || ( defined _AIX )
pthread_mutex_t		g_pthread_mutex = PTHREAD_MUTEX_INITIALIZER ;
#endif

static int CreateMutexSection( LOG *g )
{
#if ( defined _WIN32 )
	char lock_pathfilename[ MAXLEN_FILENAME + 1 ] ;
	strcpy( lock_pathfilename , "Global\\iLOG3_ROTATELOCK" );
	g->rotate_lock = CreateMutex( NULL , FALSE , lock_pathfilename ) ;
	if( g->rotate_lock == NULL )
		return LOG_RETURN_ERROR_INTERNAL;
#elif ( defined __unix ) || ( defined _AIX ) || ( defined __linux__ ) || ( defined __hpux )
	char lock_pathfilename[ MAXLEN_FILENAME + 1 ] ;
	mode_t		m ;
	SNPRINTF( lock_pathfilename , sizeof(lock_pathfilename) , "/tmp/iLOG3.lock" );
	m=umask(0);
	g->rotate_lock = open( lock_pathfilename , O_CREAT|O_APPEND|O_WRONLY , S_IRWXU|S_IRWXG|S_IRWXO ) ;
	umask(m);
	if( g->rotate_lock == FD_NULL )
		return LOG_RETURN_ERROR_INTERNAL;
#endif
	return 0;
}

static int DestroyMutexSection( LOG *g )
{
#if ( defined _WIN32 )
	if( g->rotate_lock )
	{
		CloseHandle( g->rotate_lock );
	}
#elif ( defined __unix ) || ( defined _AIX ) || ( defined __linux__ ) || ( defined __hpux )
	if( g->rotate_lock != -1 )
	{
		close( g->rotate_lock );
	}
#endif
	return 0;
}

static int EnterMutexSection( LOG *g )
{
#if ( defined _WIN32 )
	DWORD	dw ;
	dw = WaitForSingleObject( g->rotate_lock , INFINITE ) ;
	if( dw != WAIT_OBJECT_0 )
		return LOG_RETURN_ERROR_INTERNAL;
#elif ( defined __unix ) || ( defined _AIX ) || ( defined __linux__ ) || ( defined __hpux )
	int	nret ;
	memset( & (g->lock) , 0x00 , sizeof(g->lock) );
	g->lock.l_type = F_WRLCK ;
	g->lock.l_whence = SEEK_SET ;
	g->lock.l_start = 0 ;
	g->lock.l_len = 0 ;
	nret = fcntl( g->rotate_lock , F_SETLKW , & (g->lock) ) ;
	if( nret == -1 )
		return LOG_RETURN_ERROR_INTERNAL;

	pthread_mutex_lock( & g_pthread_mutex );
#endif
	return 0;
}

static int LeaveMutexSection( LOG *g )
{
#if ( defined _WIN32 )
	BOOL	bret ;
	bret = ReleaseMutex( g->rotate_lock ) ;
	if( bret != TRUE )
		return LOG_RETURN_ERROR_INTERNAL;
#elif ( defined __unix ) || ( defined _AIX ) || ( defined __linux__ ) || ( defined __hpux )
	int	nret ;

	pthread_mutex_unlock( & g_pthread_mutex );

	memset( & (g->lock) , 0x00 , sizeof(g->lock) );
	g->lock.l_type = F_UNLCK ;
	g->lock.l_whence = SEEK_SET ;
	g->lock.l_start = 0 ;
	g->lock.l_len = 0 ;
	nret = fcntl( g->rotate_lock , F_SETLK , & (g->lock) ) ;
	if( nret == -1 )
		return LOG_RETURN_ERROR_INTERNAL;
#endif
	return 0;
}

/* 调整缓冲区大小 */ /* adjust buffer size */
static int SetBufferSize( LOG *g , LOGBUF *logbuf , long buf_size , long max_buf_size )
{
	int		nret = 0 ;

	if( g == NULL )
		return LOG_RETURN_ERROR_PARAMETER;
	if( logbuf == NULL )
		return LOG_RETURN_ERROR_PARAMETER;

	if( max_buf_size != -1 )
	{
		logbuf->max_buf_size = max_buf_size ;
	}

	if( buf_size != -1 )
	{
		if( logbuf->bufbase == NULL )
		{
			nret = EnterMutexSection( g ) ;
			if( nret )
				return nret;
			logbuf->bufbase = (char*)malloc( buf_size ) ;
			LeaveMutexSection( g );
			if( logbuf->bufbase == NULL )
				return LOG_RETURN_ERROR_ALLOC;

			logbuf->buf_size = buf_size ;
			memset( logbuf->bufbase , 0x00 , logbuf->buf_size );
		}
		else
		{
			char	*tmp = NULL ;

			nret = EnterMutexSection( g ) ;
			if( nret )
				return nret;
			tmp = (char*)realloc( logbuf->bufbase , buf_size ) ;
			LeaveMutexSection( g );
			if( tmp == NULL )
				return LOG_RETURN_ERROR_ALLOC;

			logbuf->buf_remain_len = logbuf->buf_remain_len + ( buf_size - logbuf->buf_size ) ;
			logbuf->bufptr = logbuf->bufptr - logbuf->bufbase + tmp ;
			logbuf->bufbase = tmp ;
			logbuf->buf_size = buf_size ;
		}
	}

	return 0;
}

/* 销毁日志句柄 */ /* destruction of log handle */
void DestroyLogHandle( LOG *g )
{
	if( g )
	{
		if( g->logbuf.bufbase )
		{
			free( g->logbuf.bufbase );
		}

		if( g->open_flag == 1 )
		{
			if( TEST_ATTRIBUTE( g->log_options , LOG_OPTION_OPEN_ONCE ) || TEST_ATTRIBUTE( g->log_options , LOG_OPTION_CHANGE_TEST ) )
			{
				if( g->pfuncCloseLogFinally )
				{
					g->pfuncCloseLogFinally( g , & (g->open_handle) );
					g->open_flag = 0 ;
				}
			}
		}

		DestroyMutexSection( g );

		memset( g , 0x00 , sizeof(LOG) );
		free(g);
	}

	return;
}
void DestroyLogHandleG()
{
	DestroyLogHandle( tls_g );
}


/* 创建日志句柄 */ /* create log handle */
LOG *CreateLogHandle()
{
	LOG		_g , *g = NULL ;
	int		nret ;

#if ( defined _WIN32 )
#elif ( defined __unix ) || ( defined _AIX ) || ( defined __linux__ ) || ( defined __hpux )
#else
	return LOG_RETURN_ERROR_NOTSUPPORT;
#endif

	memset( & _g , 0x00 , sizeof(LOG) );
	nret = CreateMutexSection( & _g ) ;
	if( nret )
		return NULL;

	nret = EnterMutexSection( & _g ) ;
	if( nret )
		return NULL;
	g = (LOG *)malloc( sizeof(LOG) ) ;
	LeaveMutexSection( & _g );
	if( g == NULL )
		return NULL;

	memcpy( g , & _g , sizeof(LOG) );

	SetLogLevel( g , LOG_LEVEL_DEFAULT );
	SetLogStyles( g , LOG_STYLES_DEFAULT , LOG_NO_STYLEFUNC );
	SetLogOptions( g , LOG_OPTION_CHANGE_TEST );

	g->rotate_mode = LOG_ROTATEMODE_NONE ;
	g->rotate_file_no = 1 ;
	g->rotate_file_count = LOG_ROTATE_SIZE_FILE_COUNT_DEFAULT ;
	g->pressure_factor = LOG_ROTATE_SIZE_PRESSURE_FACTOR_DEFAULT ;
	g->fsync_period = LOG_FSYNC_PERIOD ;
	g->fsync_elapse = g->fsync_period ;

	g->logbuf.buf_size = 0 ;
	g->logbuf.bufbase = NULL ;
	g->logbuf.bufptr = NULL ;
	g->logbuf.buf_remain_len = 0 ;
	nret = SetLogBufferSize( g , LOG_BUFSIZE_DEFAULT , LOG_BUFSIZE_MAX ) ;
	if( nret )
	{
		DestroyLogHandle( g );
		return NULL;
	}

	return g;
}
LOG *CreateLogHandleG()
{
	tls_g = CreateLogHandle() ;
	return tls_g;
}


/* 打开、输出、关闭日志函数 */ /* open , write , close log functions */
#if ( defined _WIN32 )

#define LOG_WINOPENFILE_TRYCOUNT	1000	/* windows上多线程打开同一文件会有互斥现象 */

static int OpenLog_OpenFile( LOG *g , char *log_pathfilename , void **open_handle )
{
	long	l ;

	if( log_pathfilename[0] == '\0' )
		return 0;
	if( g->open_flag == 1 )
		return 0;

	for( l = 0 ; l < LOG_WINOPENFILE_TRYCOUNT ; l++ )
	{
		g->hFile = CreateFileA( log_pathfilename , GENERIC_WRITE , FILE_SHARE_READ | FILE_SHARE_WRITE , NULL , OPEN_ALWAYS , FILE_ATTRIBUTE_NORMAL , NULL ) ;
		if( g->hFile != INVALID_HANDLE_VALUE )
			break;
	}
	if( l >= LOG_WINOPENFILE_TRYCOUNT )
		return LOG_RETURN_ERROR_OPENFILE;

	g->open_flag = 1 ;
	return 0;
}

static int WriteLog_WriteFile( LOG *g , void **open_handle , int log_level , char *buf , long len , long *writelen )
{
	BOOL	bret ;

	if( g->open_flag == 0 )
		return -1;

	SetFilePointer( g->hFile , 0 , NULL , FILE_END );
	bret = WriteFile( g->hFile , buf , len , writelen , NULL ) ;
	if( bret != TRUE )
		return LOG_RETURN_ERROR_WRITEFILE;

	return 0;
}

static int CloseLog_CloseHandle( LOG *g , void **open_handle )
{
	if( g->open_flag == 0 )
		return 0;

	CloseHandle( g->hFile );

	g->open_flag = 0 ;
	return 0;
}

static int OpenLog_RegisterEventSource( LOG *g , char *log_pathfilename , void **open_handle )
{
	if( log_pathfilename[0] == '\0' )
		return 0;
	if( g->open_flag == 1 )
		return 0;

	g->hFile = RegisterEventSource( NULL , g->log_pathfilename ) ;
	if( g->hFile == NULL )
		return LOG_RETURN_ERROR_OPENFILE;

	g->open_flag = 1 ;
	return 0;
}

static int WriteLog_ReportEvent( LOG *g , void **open_handle , int log_level , char *buf , long len , long *writelen )
{
	unsigned short	event_log_level ;
	char		*ptr = NULL ;

	if( g->open_flag == 0 )
		return -1;

	if( log_level == LOG_LEVEL_DEBUG )
		event_log_level = EVENTLOG_INFORMATION_TYPE ;
	else if( log_level == LOG_LEVEL_INFO )
		event_log_level = EVENTLOG_INFORMATION_TYPE ;
	else if( log_level == LOG_LEVEL_WARN )
		event_log_level = EVENTLOG_WARNING_TYPE ;
	else if( log_level == LOG_LEVEL_ERROR )
		event_log_level = EVENTLOG_ERROR_TYPE ;
	else if( log_level == LOG_LEVEL_FATAL )
		event_log_level = EVENTLOG_ERROR_TYPE ;
	else
		event_log_level = EVENTLOG_ERROR_TYPE ;
	ptr = buf ;
	ReportEvent( g->hFile , event_log_level , 0 , 0 , NULL , 1 , 0 , & ptr , NULL );

	return 0;
}

static long CloseLog_DeregisterEventSource( LOG *g , void **open_handle )
{
	if( g->open_flag == 0 )
		return 0;

	DeregisterEventSource( g->hFile );

	g->open_flag = 0 ;
	return 0;
}

#elif ( defined __unix ) || ( defined _AIX ) || ( defined __linux__ ) || ( defined __hpux )

static int OpenLog_open( LOG *g , char *log_pathfilename , void **open_handle )
{
	if( log_pathfilename[0] == '\0' )
		return 0;
	if( g->open_flag == 1 )
		return 0;

	g->fd = open( log_pathfilename , O_CREAT|O_APPEND|O_WRONLY , S_IRWXU|S_IRWXG|S_IRWXO ) ;
	if( g->fd == -1 )
		return LOG_RETURN_ERROR_OPENFILE;

	g->open_flag = 1 ;
	return 0;
}

static int CloseLog_close( LOG *g , void **open_handle )
{
	if( g->open_flag == 0 )
		return 0;

	close( g->fd );

	g->open_flag = 0 ;
	return 0;
}

static int OpenLog_openlog( LOG *g , char *log_pathfilename , void **open_handle )
{
	if( log_pathfilename[0] == '\0' )
		return 0;
	if( g->open_flag == 1 )
		return 0;

	openlog( g->log_pathfilename , LOG_PID , LOG_USER );

	g->open_flag = 1 ;
	return 0;
}

static int WriteLog_syslog( LOG *g , void **open_handle , int log_level , char *buf , long len , long *writelen )
{
	int	syslog_log_level ;

	if( g->open_flag == 0 )
		return -1;

	if( log_level == LOG_LEVEL_DEBUG )
		syslog_log_level = LOG_DEBUG ;
	else if( log_level == LOG_LEVEL_INFO )
		syslog_log_level = LOG_INFO ;
	else if( log_level == LOG_LEVEL_WARN )
		syslog_log_level = LOG_WARNING ;
	else if( log_level == LOG_LEVEL_ERROR )
		syslog_log_level = LOG_ERR ;
	else if( log_level == LOG_LEVEL_FATAL )
		syslog_log_level = LOG_EMERG ;
	else
		syslog_log_level = LOG_ALERT ;
	syslog( syslog_log_level , buf );
	(*writelen) = len ;

	return 0;
}

static int CloseLog_closelog( LOG *g , void **open_handle )
{
	if( g->open_flag == 0 )
		return 0;

	closelog();

	g->open_flag = 0 ;
	return 0;
}
#endif

static int WriteLog_write( LOG *g , void **open_handle , int log_level , char *buf , long len , long *writelen )
{
	if( g->open_flag == 0 )
		return -1;

	(*writelen) = WRITE( g->fd , buf , len ) ;
	if( (*writelen) == -1 )
		return LOG_RETURN_ERROR_WRITEFILE;

	return 0;
}

static int ChangeTest_interval( LOG *g , void **test_handle )
{
	int		nret ;

	g->file_change_test_no--;
	if( g->file_change_test_no < 1 )
	{
		struct STAT	file_change_stat ;
		nret = STAT( g->log_pathfilename , & file_change_stat ) ;
		if( ( nret == -1 || file_change_stat.st_size < g->file_change_stat.st_size ) && g->pfuncCloseLogFinally )
		{
			nret = g->pfuncCloseLogFinally( g , & (g->open_handle) ) ;
			if( nret )
				return nret;
		}
		g->file_change_test_no = g->file_change_test_interval ;
	}

	return 0;
}

int ExpandPathFilename( char *pathfilename , long pathfilename_bufsize )
{
	long		pathfilename_len ;

	char		*p1 = NULL , *p2 = NULL ;
	char		env_key[ MAXLEN_FILENAME + 1 ] ;
	long		env_key_len ;
	char		*env_val = NULL ;
	long		env_val_len ;

	pathfilename_len = strlen(pathfilename) ;

	p1 = strchr( pathfilename , '$' );
	while( p1 )
	{
		/* 展开环境变量 */ /* expand environment variable */
		p2 = strchr( p1 + 1 , '$' ) ;
		if( p2 == NULL )
			return LOG_RETURN_ERROR_PARAMETER;

		memset( env_key , 0x00 , sizeof(env_key) );
		env_key_len = p2 - p1 + 1 ;
		strncpy( env_key , p1 + 1 , env_key_len - 2 );
		env_val = getenv( env_key ) ;
		if( env_val == NULL )
			return LOG_RETURN_ERROR_PARAMETER;

		env_val_len = strlen(env_val) ;
		if( pathfilename_len + ( env_val_len - env_key_len ) > pathfilename_bufsize-1 )
			return LOG_RETURN_ERROR_PARAMETER;

		memmove( p2+1 + ( env_val_len - env_key_len ) , p2+1 , strlen(p2+1) + 1 );
		memcpy( p1 , env_val , env_val_len );
		pathfilename_len += env_val_len - env_key_len ;

		p1 = strchr( p1 + ( env_val_len - env_key_len ) , '$' );
	}

	return 0;
}

/* 设置日志输出 */ /* set log output */
int SetLogOutput( LOG *g , int output , char *log_pathfilename , funcOpenLog *pfuncOpenLogFirst , funcOpenLog *pfuncOpenLog , funcWriteLog *pfuncWriteLog , funcChangeTest *pfuncChangeTest , funcCloseLog *pfuncCloseLog , funcCloseLog *pfuncCloseLogFinally )
{
	char		pathfilename[ MAXLEN_FILENAME + 1 ] ;

	int		nret = 0 ;

	if( g == NULL )
		return LOG_RETURN_ERROR_PARAMETER;
	if( log_pathfilename == NULL || log_pathfilename[0] == '\0' )
	{
		memset( pathfilename , 0x00 , sizeof(pathfilename) );
	}
	else
	{
		memset( pathfilename , 0x00 , sizeof(pathfilename) );
		strncpy( pathfilename , log_pathfilename , sizeof(pathfilename)-1 );

		if( output != LOG_OUTPUT_CALLBACK )
		{
			nret = ExpandPathFilename( pathfilename , sizeof(pathfilename) ) ;
			if( nret )
				return nret;
		}
	}

	if( TEST_ATTRIBUTE( g->log_options , LOG_OPTION_SET_OUTPUT_BY_FILENAME ) )
	{
		if( strncmp( pathfilename , "#stdout#" , 8 ) == 0 )
		{
			memmove( pathfilename , pathfilename+8 , strlen(pathfilename+8)+1 );
			output = LOG_OUTPUT_STDOUT ;
		}
		else if( strncmp( pathfilename , "#stderr#" , 8 ) == 0 )
		{
			memmove( pathfilename , pathfilename+8 , strlen(pathfilename+8)+1 );
			output = LOG_OUTPUT_STDERR ;
		}
		else if( strncmp( pathfilename , "#syslog#" , 8 ) == 0 )
		{
			memmove( pathfilename , pathfilename+8 , strlen(pathfilename+8)+1 );
			output = LOG_OUTPUT_SYSLOG ;
		}
	}

	if( output == LOG_OUTPUT_FILE && TEST_ATTRIBUTE( g->log_options , LOG_OPTION_FILENAME_APPEND_DOT_LOG ) )
	{
		if( strlen(pathfilename) + 4 > sizeof(pathfilename) - 1 )
			return LOG_RETURN_ERROR_PARAMETER;

		strcat( pathfilename , ".log" );
	}

	if( g->open_flag == 1 )
	{
		if( TEST_ATTRIBUTE( g->log_options , LOG_OPTION_OPEN_ONCE ) || TEST_ATTRIBUTE( g->log_options , LOG_OPTION_CHANGE_TEST ) )
		{
			if( g->pfuncCloseLogFinally )
			{
				nret = g->pfuncCloseLogFinally( g , & (g->open_handle) ) ;
			}
		}
	}

	if( TEST_NOT_ATTRIBUTE( g->log_options , LOG_OPTION_OPEN_AND_CLOSE )
		&& TEST_NOT_ATTRIBUTE( g->log_options , LOG_OPTION_CHANGE_TEST )
		&& TEST_NOT_ATTRIBUTE( g->log_options , LOG_OPTION_OPEN_ONCE ) )
	{
		g->log_options |= LOG_OPTION_OPEN_DEFAULT ;
	}

	memset( g->log_pathfilename , 0x00 , sizeof(g->log_pathfilename) );
	strncpy( g->log_pathfilename , pathfilename , sizeof(g->log_pathfilename)-1 );

	if( output != LOG_OUTPUT_NOSET )
	{
		g->output = output ;
		if( g->output == LOG_OUTPUT_STDOUT )
		{
			g->fd = STDOUT_HANDLE ;
			g->pfuncOpenLogFirst = NULL ;
			g->pfuncOpenLog = NULL ;
			g->pfuncWriteLog = & WriteLog_write ;
			g->pfuncChangeTest = NULL ;
			g->pfuncCloseLog = NULL ;
			g->pfuncCloseLogFinally = NULL ;

			g->log_options &= ~LOG_OPTION_OPEN_AND_CLOSE ;
			g->log_options &= ~LOG_OPTION_CHANGE_TEST ;
			g->log_options |= LOG_OPTION_OPEN_ONCE ;

			g->open_flag = 1 ;
		}
		else if( output == LOG_OUTPUT_STDERR )
		{
			g->fd = STDERR_HANDLE ;
			g->pfuncOpenLogFirst = NULL ;
			g->pfuncOpenLog = NULL ;
			g->pfuncWriteLog = & WriteLog_write ;
			g->pfuncChangeTest = NULL ;
			g->pfuncCloseLog = NULL ;
			g->pfuncCloseLogFinally = NULL ;

			g->log_options &= ~LOG_OPTION_OPEN_AND_CLOSE ;
			g->log_options &= ~LOG_OPTION_CHANGE_TEST ;
			g->log_options |= LOG_OPTION_OPEN_ONCE ;

			g->open_flag = 1 ;
		}
		else if( output == LOG_OUTPUT_SYSLOG )
		{
#if ( defined _WIN32 )
			g->pfuncOpenLogFirst = & OpenLog_RegisterEventSource ;
			g->pfuncOpenLog = NULL ;
			g->pfuncWriteLog = & WriteLog_ReportEvent ;
			g->pfuncChangeTest = NULL ;
			g->pfuncCloseLog = NULL ;
			g->pfuncCloseLogFinally = & CloseLog_DeregisterEventSource ;
#elif ( defined __unix ) || ( defined _AIX ) || ( defined __linux__ ) || ( defined __hpux )
			g->pfuncOpenLogFirst = & OpenLog_openlog ;
			g->pfuncOpenLog = NULL ;
			g->pfuncWriteLog = & WriteLog_syslog ;
			g->pfuncChangeTest = NULL ;
			g->pfuncCloseLog = NULL ;
			g->pfuncCloseLogFinally = & CloseLog_closelog ;
#endif

			g->log_options &= ~LOG_OPTION_OPEN_AND_CLOSE ;
			g->log_options &= ~LOG_OPTION_CHANGE_TEST ;
			g->log_options |= LOG_OPTION_OPEN_ONCE ;

			g->open_flag = 0 ;
		}
		else if( output == LOG_OUTPUT_FILE )
		{
#if ( defined _WIN32 )
			if( TEST_ATTRIBUTE( g->log_options , LOG_OPTION_OPEN_ONCE ) || TEST_ATTRIBUTE( g->log_options , LOG_OPTION_CHANGE_TEST ) )
			{
				g->pfuncOpenLogFirst = & OpenLog_OpenFile ;
				g->pfuncOpenLog = NULL ;
				g->pfuncWriteLog = & WriteLog_WriteFile ;
				g->pfuncChangeTest = & ChangeTest_interval ;
				g->pfuncCloseLog = NULL ;
				g->pfuncCloseLogFinally = & CloseLog_CloseHandle ;
			}
			else
			{
				g->pfuncOpenLogFirst = NULL ;
				g->pfuncOpenLog = & OpenLog_OpenFile ;
				g->pfuncWriteLog = & WriteLog_WriteFile ;
				g->pfuncChangeTest = NULL ;
				g->pfuncCloseLog = & CloseLog_CloseHandle ;
				g->pfuncCloseLogFinally = NULL ;
			}
#elif ( defined __unix ) || ( defined _AIX ) || ( defined __linux__ ) || ( defined __hpux )
			if( TEST_ATTRIBUTE( g->log_options , LOG_OPTION_OPEN_ONCE ) || TEST_ATTRIBUTE( g->log_options , LOG_OPTION_CHANGE_TEST ) )
			{
				g->pfuncOpenLogFirst = & OpenLog_open ;
				g->pfuncOpenLog = NULL ;
				g->pfuncWriteLog = & WriteLog_write ;
				g->pfuncChangeTest = & ChangeTest_interval ;
				g->pfuncCloseLog = NULL ;
				g->pfuncCloseLogFinally = & CloseLog_close ;
			}
			else
			{
				g->pfuncOpenLogFirst = NULL ;
				g->pfuncOpenLog = & OpenLog_open ;
				g->pfuncWriteLog = & WriteLog_write ;
				g->pfuncChangeTest = NULL ;
				g->pfuncCloseLog = & CloseLog_close ;
				g->pfuncCloseLogFinally = NULL ;
			}
#endif
			g->open_flag = 0 ;
		}
		else if( output == LOG_OUTPUT_CALLBACK )
		{
			SetLogOutputFuncDirectly( g , pfuncOpenLogFirst , pfuncOpenLog , pfuncWriteLog , pfuncChangeTest , pfuncCloseLog , pfuncCloseLogFinally );
			g->open_flag = 0 ;
		}
	}

	if( TEST_ATTRIBUTE( g->log_options , LOG_OPTION_OPEN_ONCE ) || TEST_ATTRIBUTE( g->log_options , LOG_OPTION_CHANGE_TEST ) )
	{
		if( g->pfuncOpenLogFirst )
		{
			nret = g->pfuncOpenLogFirst( g , g->log_pathfilename , & (g->open_handle) ) ;
			if( nret )
				return nret;
		}
	}

	return 0;
}

int SetLogOutput2( LOG *g , int output , funcOpenLog *pfuncOpenLogFirst , funcOpenLog *pfuncOpenLog , funcWriteLog *pfuncWriteLog , funcChangeTest *pfuncChangeTest , funcCloseLog *pfuncCloseLog , funcCloseLog *pfuncCloseLogFinally , char *log_pathfilename_format , ... )
{
	va_list		valist ;
	char		log_pathfilename[ MAXLEN_FILENAME + 1 ] ;

	memset( log_pathfilename , 0x00 , sizeof(log_pathfilename) );
	va_start( valist , log_pathfilename_format );
	VSNPRINTF( log_pathfilename , sizeof(log_pathfilename)-1 , log_pathfilename_format , valist );
	va_end( valist );

	return SetLogOutput( g , output , log_pathfilename , pfuncOpenLogFirst , pfuncOpenLog , pfuncWriteLog , pfuncChangeTest , pfuncCloseLog , pfuncCloseLogFinally );
}

int SetLogOutputG( int output , char *log_pathfilename , funcOpenLog *pfuncOpenLogFirst , funcOpenLog *pfuncOpenLog , funcWriteLog *pfuncWriteLog , funcChangeTest *pfuncChangeTest , funcCloseLog *pfuncCloseLog , funcCloseLog *pfuncCloseLogFinally )
{
	return SetLogOutput( tls_g , output , log_pathfilename , pfuncOpenLogFirst , pfuncOpenLog , pfuncWriteLog , pfuncChangeTest , pfuncCloseLog , pfuncCloseLogFinally );
}


/* 设置日志等级 */ /* set log level */
int SetLogLevel( LOG *g , int log_level )
{
	if( g == NULL )
		return LOG_RETURN_ERROR_PARAMETER;
	g->log_level = log_level ;
	return 0;
}
int SetLogLevelG( int log_level )
{
	return SetLogLevel( tls_g , log_level );
}


/* 行格式函数集合 */ /* log style functions */
static int LogStyle_SEPARATOR( LOG *g , LOGBUF *logbuf , char *c_filename , long c_fileline , int log_level , char *format , va_list valist )
{
	MemcatLogBuffer( g , logbuf , " | " , 3 );
	return 0;
}

static int LogStyle_SEPARATOR2( LOG *g , LOGBUF *logbuf , char *c_filename , long c_fileline , int log_level , char *format , va_list valist )
{
	MemcatLogBuffer( g , logbuf , "| " , 2 );
	return 0;
}

static int LogStyle_DATE( LOG *g , LOGBUF *logbuf , char *c_filename , long c_fileline , int log_level , char *format , va_list valist )
{
	if( g->cache1_tv.tv_sec == 0 )
		g->cache1_tv.tv_sec = time( NULL ) ;
	if( g->cache1_tv.tv_sec != g->cache2_logstyle_tv.tv_sec )
	{
		LOCALTIME( g->cache1_tv.tv_sec , g->cache1_stime )
			FormatLogBuffer( g , logbuf , "%04d-%02d-%02d" , g->cache1_stime.tm_year+1900 , g->cache1_stime.tm_mon+1 , g->cache1_stime.tm_mday );
		g->cache2_logstyle_tv.tv_sec = g->cache1_tv.tv_sec ;
		g->cache2_logstyle_tv.tv_usec = g->cache1_tv.tv_usec ;
		g->cache2_logstyle_date_buf_len = 10 ;
		memcpy( g->cache2_logstyle_date_buf , logbuf->bufptr - g->cache2_logstyle_date_buf_len , g->cache2_logstyle_date_buf_len );
		g->cache2_logstyle_date_buf[g->cache2_logstyle_date_buf_len] = '\0' ;
	}
	else
	{
		MemcatLogBuffer( g , logbuf , g->cache2_logstyle_date_buf , g->cache2_logstyle_date_buf_len );
	}
	return 0;
}

static int LogStyle_DATETIME( LOG *g , LOGBUF *logbuf , char *c_filename , long c_fileline , int log_level , char *format , va_list valist )
{
	if( g->cache1_tv.tv_sec == 0 )
		g->cache1_tv.tv_sec = time( NULL ) ;
	if( g->cache1_tv.tv_sec != g->cache2_logstyle_tv.tv_sec )
	{
		LOCALTIME( g->cache1_tv.tv_sec , g->cache1_stime )
			FormatLogBuffer( g , logbuf , "%04d-%02d-%02d %02d:%02d:%02d" , g->cache1_stime.tm_year+1900 , g->cache1_stime.tm_mon+1 , g->cache1_stime.tm_mday , g->cache1_stime.tm_hour , g->cache1_stime.tm_min , g->cache1_stime.tm_sec ) ;
		g->cache2_logstyle_tv.tv_sec = g->cache1_tv.tv_sec ;
		g->cache2_logstyle_datetime_buf_len = 19 ;
		memcpy( g->cache2_logstyle_datetime_buf , logbuf->bufptr - g->cache2_logstyle_datetime_buf_len , g->cache2_logstyle_datetime_buf_len ); g->cache2_logstyle_datetime_buf[g->cache2_logstyle_datetime_buf_len] = '\0' ;
	}
	else
	{
		MemcatLogBuffer( g , logbuf , g->cache2_logstyle_datetime_buf , g->cache2_logstyle_datetime_buf_len ) ;
	}
	return 0;
}

static int LogStyle_DATETIMEMS( LOG *g , LOGBUF *logbuf , char *c_filename , long c_fileline , int log_level , char *format , va_list valist )
{
#if ( defined _WIN32 )
	SYSTEMTIME	stNow ;
	GetLocalTime( & stNow );
	SYSTEMTIME2TIMEVAL_USEC( stNow , g->cache1_tv );
#elif ( defined __unix ) || ( defined _AIX ) || ( defined __linux__ ) || ( defined __hpux )
	gettimeofday( & (g->cache1_tv) , NULL );
#endif	
	LogStyle_DATETIME( g , logbuf , c_filename , c_fileline , log_level , format , valist );
	FormatLogBuffer( g , logbuf , ".%06ld" , g->cache1_tv.tv_usec ) ;
	return 0;
}

static int LogStyle_LOGLEVEL( LOG *g , LOGBUF *logbuf , char *c_filename , long c_fileline , int log_level , char *format , va_list valist )
{
	FormatLogBuffer( g , logbuf , "%-5s" , sg_aszLogLevelDesc[log_level] );
	return 0;
}

static int LogStyle_CUSTLABEL1( LOG *g , LOGBUF *logbuf , char *c_filename , long c_fileline , int log_level , char *format , va_list valist )
{
	FormatLogBuffer( g , logbuf , "%s" , g->cust_label[1-1] );
	return 0;
}

static int LogStyle_CUSTLABEL2( LOG *g , LOGBUF *logbuf , char *c_filename , long c_fileline , int log_level , char *format , va_list valist )
{
	FormatLogBuffer( g , logbuf , "%s" , g->cust_label[2-1] );
	return 0;
}

static int LogStyle_CUSTLABEL3( LOG *g , LOGBUF *logbuf , char *c_filename , long c_fileline , int log_level , char *format , va_list valist )
{
	FormatLogBuffer( g , logbuf , "%s" , g->cust_label[3-1] );
	return 0;
}

static int LogStyle_SPACE( LOG *g , LOGBUF *logbuf , char *c_filename , long c_fileline , int log_level , char *format , va_list valist )
{
	MemcatLogBuffer( g , logbuf , " " , 1 );
	return 0;
}

static int LogStyle_PID( LOG *g , LOGBUF *logbuf , char *c_filename , long c_fileline , int log_level , char *format , va_list valist )
{
	unsigned long	pid ;
#if ( defined _WIN32 )
	HWND		hd ;
	DWORD		dw ;
	hd = GetForegroundWindow() ;
	GetWindowThreadProcessId( hd , & dw );
	pid = (long)dw ;
#elif ( defined __unix ) || ( defined _AIX ) || ( defined __linux__ ) || ( defined __hpux )
	pid = (long)getpid() ;
#endif
	if( pid != g->cache2_logstyle_pid )
	{
		g->cache2_logstyle_pid_buf_len = FormatLogBuffer( g , logbuf , "%u:" , pid ) ;

		memcpy( g->cache2_logstyle_pid_buf , logbuf->bufptr - g->cache2_logstyle_pid_buf_len , g->cache2_logstyle_pid_buf_len ); g->cache2_logstyle_pid_buf[g->cache2_logstyle_pid_buf_len] = '\0' ;

		g->cache2_logstyle_pid = pid ;
	}
	else
	{
		MemcatLogBuffer( g , logbuf , g->cache2_logstyle_pid_buf , g->cache2_logstyle_pid_buf_len ) ;
	}
	return 0;
}

static int LogStyle_TID( LOG *g , LOGBUF *logbuf , char *c_filename , long c_fileline , int log_level , char *format , va_list valist )
{
	unsigned long	tid ;
#if ( defined _WIN32 )
	tid = (unsigned long)GetCurrentThreadId() ;
#elif ( defined __unix ) || ( defined _AIX ) || ( defined __linux__ ) || ( defined __hpux )
#if ( defined _PTHREAD_H )
	tid = (unsigned long)pthread_self() ;
#else
#endif
#endif
	if( tid != g->cache2_logstyle_tid )
	{
		g->cache2_logstyle_tid_buf_len = FormatLogBuffer( g , logbuf , "%u:" , tid ) ;

		memcpy( g->cache2_logstyle_tid_buf , logbuf->bufptr - g->cache2_logstyle_tid_buf_len , g->cache2_logstyle_tid_buf_len ); g->cache2_logstyle_tid_buf[g->cache2_logstyle_tid_buf_len] = '\0' ;

		g->cache2_logstyle_tid = tid ;
	}
	else
	{
		MemcatLogBuffer( g , logbuf , g->cache2_logstyle_tid_buf , g->cache2_logstyle_tid_buf_len ) ;
	}
	return 0;
}

static int LogStyle_SOURCE( LOG *g , LOGBUF *logbuf , char *c_filename , long c_fileline , int log_level , char *format , va_list valist )
{
#if ( defined _WIN32 )
	char		*pfilename = NULL ;
	pfilename = strrchr( c_filename , '\\' ) ;
	if( pfilename )
	{
		pfilename++;
	}
	else
	{
		pfilename = strrchr( c_filename , '/' ) ;
		if( pfilename )
			pfilename++;
		else
			pfilename = c_filename ;
	}
	FormatLogBuffer( g , logbuf , "%s:%ld" , pfilename , c_fileline ) ;
#elif ( defined __unix ) || ( defined _AIX ) || ( defined __linux__ ) || ( defined __hpux )
	FormatLogBuffer( g , logbuf , "%s:%ld" , c_filename , c_fileline ) ;
#endif
	return 0;
}

static int LogStyle_FORMAT( LOG *g , LOGBUF *logbuf , char *c_filename , long c_fileline , int log_level , char *format , va_list valist )
{
	FormatLogBufferV( g , logbuf , format , valist );
	return 0;
}

static int LogStyle_NEWLINE( LOG *g , LOGBUF *logbuf , char *c_filename , long c_fileline , int log_level , char *format , va_list valist )
{
	MemcatLogBuffer( g , logbuf , LOG_NEWLINE , LOG_NEWLINE_LEN );
	return 0;
}

/* 填充行日志段 */ /* fill log segments */
static int LogStyle_FuncArray( LOG *g , LOGBUF *logbuf , char *c_filename , long c_fileline , int log_level , char *format , va_list valist )
{
	int		format_func_index ;
	funcLogStyle	**ppfuncLogStyle = NULL ;
	int		nret ;
	/* 遍历格式函数数组 */ /* travel all log style functions */
	for( format_func_index = 0 , ppfuncLogStyle = g->pfuncLogStyles ; format_func_index < g->style_func_count ; format_func_index++ , ppfuncLogStyle++ )
	{
		nret = (*ppfuncLogStyle)( g , logbuf , c_filename , c_fileline , log_level , format , valist ) ;
		if( nret )
			return nret;
	}

	return 0;
}

/* 设置行日志风格 */ /* set log styles */
int SetLogStyles( LOG *g , long log_styles , funcLogStyle *pfuncLogStyle )
{
	if( g == NULL )
		return LOG_RETURN_ERROR_PARAMETER;

	g->log_styles = log_styles ;
	if( g->log_styles == LOG_STYLE_CALLBACK )
	{
		SetLogStyleFuncDirectly( g , pfuncLogStyle );
		return 0;
	}

	/* 构造行风格函数数组 */
	g->style_func_count = 0 ;

	if( TEST_ATTRIBUTE( g->log_styles , LOG_STYLE_DATETIMEMS ) )
	{
		g->pfuncLogStyles[ g->style_func_count++ ] = LogStyle_DATETIMEMS ;
		g->pfuncLogStyles[ g->style_func_count++ ] = LogStyle_SEPARATOR ;
	}
	else if( TEST_ATTRIBUTE( g->log_styles , LOG_STYLE_DATETIME ) )
	{
		g->pfuncLogStyles[ g->style_func_count++ ] = LogStyle_DATETIME ;
		g->pfuncLogStyles[ g->style_func_count++ ] = LogStyle_SEPARATOR ;
	}
	else if( TEST_ATTRIBUTE( g->log_styles , LOG_STYLE_DATE ) )
	{
		g->pfuncLogStyles[ g->style_func_count++ ] = LogStyle_DATE ;
		g->pfuncLogStyles[ g->style_func_count++ ] = LogStyle_SEPARATOR ;
	}
	if( TEST_ATTRIBUTE( g->log_styles , LOG_STYLE_LOGLEVEL ) )
	{
		g->pfuncLogStyles[ g->style_func_count++ ] = LogStyle_LOGLEVEL ;
		g->pfuncLogStyles[ g->style_func_count++ ] = LogStyle_SEPARATOR ;
	}
	if( TEST_ATTRIBUTE( g->log_styles , LOG_STYLE_CUSTLABEL1 )
		|| TEST_ATTRIBUTE( g->log_styles , LOG_STYLE_CUSTLABEL2 )
		|| TEST_ATTRIBUTE( g->log_styles , LOG_STYLE_CUSTLABEL3 ) )
	{
		if( TEST_ATTRIBUTE( g->log_styles , LOG_STYLE_CUSTLABEL1 ) )
		{
			g->pfuncLogStyles[ g->style_func_count++ ] = LogStyle_CUSTLABEL1 ;
			g->pfuncLogStyles[ g->style_func_count++ ] = LogStyle_SPACE ;
		}
		if( TEST_ATTRIBUTE( g->log_styles , LOG_STYLE_CUSTLABEL2 ) )
		{
			g->pfuncLogStyles[ g->style_func_count++ ] = LogStyle_CUSTLABEL2 ;
			g->pfuncLogStyles[ g->style_func_count++ ] = LogStyle_SPACE ;
		}
		if( TEST_ATTRIBUTE( g->log_styles , LOG_STYLE_CUSTLABEL3 ) )
		{
			g->pfuncLogStyles[ g->style_func_count++ ] = LogStyle_CUSTLABEL3 ;
			g->pfuncLogStyles[ g->style_func_count++ ] = LogStyle_SPACE ;
		}
		g->pfuncLogStyles[ g->style_func_count++ ] = LogStyle_SEPARATOR2 ;
	}
	if( TEST_ATTRIBUTE( g->log_styles , LOG_STYLE_PID )
		|| TEST_ATTRIBUTE( g->log_styles , LOG_STYLE_TID )
		|| TEST_ATTRIBUTE( g->log_styles , LOG_STYLE_SOURCE ) )
	{
		if( TEST_ATTRIBUTE( g->log_styles , LOG_STYLE_PID ) )
		{
			g->pfuncLogStyles[ g->style_func_count++ ] = LogStyle_PID ;
		}
		if( TEST_ATTRIBUTE( g->log_styles , LOG_STYLE_TID ) )
		{
			g->pfuncLogStyles[ g->style_func_count++ ] = LogStyle_TID ;
		}
		if( TEST_ATTRIBUTE( g->log_styles , LOG_STYLE_SOURCE ) )
		{
			g->pfuncLogStyles[ g->style_func_count++ ] = LogStyle_SOURCE ;
		}
		g->pfuncLogStyles[ g->style_func_count++ ] = LogStyle_SEPARATOR ;
	}
	if( TEST_ATTRIBUTE( g->log_styles , LOG_STYLE_FORMAT ) )
	{
		g->pfuncLogStyles[ g->style_func_count++ ] = LogStyle_FORMAT ;
	}
	if( TEST_ATTRIBUTE( g->log_styles , LOG_STYLE_NEWLINE ) )
	{
		g->pfuncLogStyles[ g->style_func_count++ ] = LogStyle_NEWLINE ;
	}

	g->pfuncLogStyle = & LogStyle_FuncArray ;

	return 0;
}
int SetLogStylesG( long log_styles , funcLogStyle *pfuncLogStyles )
{
	return SetLogStyles( tls_g , log_styles , pfuncLogStyles );
}


/* 转档日志文件 */ /* rotate log file */
static int RotateLogFileSize( LOG *g , long step )
{
	char		rotate_log_pathfilename[ MAXLEN_FILENAME + 1 ] ;
	char		rotate_log_pathfilename_access[ MAXLEN_FILENAME + 1 ] ;
	int		nret ;

	if( g->open_flag == 1 )
	{
		if( TEST_ATTRIBUTE( g->log_options , LOG_OPTION_OPEN_ONCE ) || TEST_ATTRIBUTE( g->log_options , LOG_OPTION_CHANGE_TEST ) )
		{
			if( g->pfuncCloseLogFinally )
			{
				nret = g->pfuncCloseLogFinally( g , & (g->open_handle) ) ;
				if( nret )
					return nret;
			}
		}
	}

	nret = EnterMutexSection( g ) ;
	if( nret )
		return nret;

	nret = STAT( g->log_pathfilename , & (g->file_change_stat) ) ;
	if( nret == -1 )
	{
		LeaveMutexSection( g );
		return LOG_RETURN_ERROR_INTERNAL;
	}

	if( g->file_change_stat.st_size >= g->log_rotate_size )
	{
		for( g->rotate_file_no = 1 ; g->rotate_file_no <= g->rotate_file_count ; g->rotate_file_no++ )
		{
			SNPRINTF( rotate_log_pathfilename , sizeof(rotate_log_pathfilename) , "%s.%ld" , g->log_pathfilename , g->rotate_file_no );
			strcpy( rotate_log_pathfilename_access , rotate_log_pathfilename );
			if( g->pfuncBeforeRotateFile )
				g->pfuncBeforeRotateFile( g , rotate_log_pathfilename_access );
			nret = ACCESS( rotate_log_pathfilename_access , ACCESS_OK ) ;
			if( nret == -1 )
				break;
		}
		if( g->rotate_file_no > g->rotate_file_count )
		{
			g->rotate_file_no = g->rotate_file_count ;
			SNPRINTF( rotate_log_pathfilename , sizeof(rotate_log_pathfilename) , "%s.%ld" , g->log_pathfilename , g->rotate_file_count );
		}

		nret = RENAME( g->log_pathfilename , rotate_log_pathfilename ) ;
		if( nret )
		{
			UNLINK( rotate_log_pathfilename );
			RENAME( g->log_pathfilename , rotate_log_pathfilename ) ;
		}

		if( g->pfuncAfterRotateFile )
			g->pfuncAfterRotateFile( g , rotate_log_pathfilename );
		g->skip_count = 1 ;
	}
	else
	{
		if( step == 0 )
			step = 1 ;
		if( g->pressure_factor == 0 )
			g->skip_count = 1 ;
		else
			g->skip_count = ( g->log_rotate_size - g->file_change_stat.st_size ) / step / g->pressure_factor + 1 ;
	}

	LeaveMutexSection( g );

	return 0;
}

static int RotateLogFilePerDate( LOG *g )
{
	char		rotate_log_pathfilename[ MAXLEN_FILENAME + 1 ] ;
	int		nret ;

	if( g->open_flag == 1 )
	{
		if( TEST_ATTRIBUTE( g->log_options , LOG_OPTION_OPEN_ONCE ) || TEST_ATTRIBUTE( g->log_options , LOG_OPTION_CHANGE_TEST ) )
		{
			if( g->pfuncCloseLogFinally )
			{
				nret = g->pfuncCloseLogFinally( g , & (g->open_handle) ) ;
				if( nret )
					return nret;
				g->open_flag = 0 ;
			}
		}
	}

	nret = EnterMutexSection( g ) ;
	if( nret )
		return nret;

	if( g->cache1_tv.tv_sec == 0 )
		g->cache1_tv.tv_sec = time( NULL ) ;
	if( g->cache1_stime.tm_mday == 0 )
		LOCALTIME( g->cache1_tv.tv_sec , g->cache1_stime )

		if( g->cache2_rotate_tv.tv_sec == 0 || g->cache2_rotate_stime.tm_mday == 0 )
		{
			memcpy( & (g->cache2_rotate_tv) , & (g->cache1_tv) , sizeof(struct timeval) );
			memcpy( & (g->cache2_rotate_stime) , & (g->cache1_stime) , sizeof(struct tm) );
		}
		else if
			(
			g->cache1_tv.tv_sec == g->cache2_rotate_tv.tv_sec
			||
			(
			g->cache1_stime.tm_year == g->cache2_rotate_stime.tm_year
			&&
			g->cache1_stime.tm_mon == g->cache2_rotate_stime.tm_mon
			&&
			g->cache1_stime.tm_mday == g->cache2_rotate_stime.tm_mday
			)
			)
		{
		}
		else
		{
			SNPRINTF( rotate_log_pathfilename , sizeof(rotate_log_pathfilename) , "%s.%04d%02d%02d" , g->log_pathfilename , g->cache2_rotate_stime.tm_year+1900 , g->cache2_rotate_stime.tm_mon+1 , g->cache2_rotate_stime.tm_mday );
			RENAME( g->log_pathfilename , rotate_log_pathfilename );
			if( g->pfuncAfterRotateFile )
				g->pfuncAfterRotateFile( g , rotate_log_pathfilename );

			memcpy( & (g->cache2_rotate_tv) , & (g->cache1_tv) , sizeof(struct timeval) );
			memcpy( & (g->cache2_rotate_stime) , & (g->cache1_stime) , sizeof(struct tm) );
		}

		LeaveMutexSection( g );

		return 0;
}

static int RotateLogFilePerHour( LOG *g )
{
	char		rotate_log_pathfilename[ MAXLEN_FILENAME + 1 ] ;
	int		nret ;

	if( g->open_flag == 1 )
	{
		if( TEST_ATTRIBUTE( g->log_options , LOG_OPTION_OPEN_ONCE ) || TEST_ATTRIBUTE( g->log_options , LOG_OPTION_CHANGE_TEST ) )
		{
			if( g->pfuncCloseLogFinally )
			{
				nret = g->pfuncCloseLogFinally( g , & (g->open_handle) ) ;
				if( nret )
					return nret;
				g->open_flag = 0 ;
			}
		}
	}

	nret = EnterMutexSection( g ) ;
	if( nret )
		return nret;

	if( g->cache1_tv.tv_sec == 0 )
		g->cache1_tv.tv_sec = time( NULL ) ;
	if( g->cache1_stime.tm_mday == 0 )
		LOCALTIME( g->cache1_tv.tv_sec , g->cache1_stime )

		if( g->cache2_rotate_tv.tv_sec == 0 || g->cache2_rotate_stime.tm_mday == 0 )
		{
			memcpy( & (g->cache2_rotate_tv) , & (g->cache1_tv) , sizeof(struct timeval) );
			memcpy( & (g->cache2_rotate_stime) , & (g->cache1_stime) , sizeof(struct tm) );
		}
		else if
			(
			g->cache2_rotate_tv.tv_sec == g->cache1_tv.tv_sec
			||
			(
			g->cache2_rotate_stime.tm_year == g->cache1_stime.tm_year
			&&
			g->cache2_rotate_stime.tm_mon == g->cache1_stime.tm_mon
			&&
			g->cache2_rotate_stime.tm_mday == g->cache1_stime.tm_mday
			&&
			g->cache2_rotate_stime.tm_hour == g->cache1_stime.tm_hour
			)
			)
		{
		}
		else
		{
			SNPRINTF( rotate_log_pathfilename , sizeof(rotate_log_pathfilename) , "%s.%04d%02d%02d_%02d" , g->log_pathfilename , g->cache2_rotate_stime.tm_year+1900 , g->cache2_rotate_stime.tm_mon+1 , g->cache2_rotate_stime.tm_mday , g->cache2_rotate_stime.tm_hour );
			RENAME( g->log_pathfilename , rotate_log_pathfilename );
			if( g->pfuncAfterRotateFile )
				g->pfuncAfterRotateFile( g , rotate_log_pathfilename );

			memcpy( & (g->cache2_rotate_tv) , & (g->cache1_tv) , sizeof(struct timeval) );
			memcpy( & (g->cache2_rotate_stime) , & (g->cache1_stime) , sizeof(struct tm) );
		}

		LeaveMutexSection( g );

		return 0;
}

/* 写日志基函数 */ /* output log */
int WriteLogBase( LOG *g , char *c_filename , long c_fileline , int log_level , char *format , va_list valist )
{
	long    writelen ;
	int		nret ;

	if( format == NULL )
		return 0;

	/* 初始化行日志缓冲区 */ /* initialize log buffer  */
	g->logbuf.buf_remain_len = g->logbuf.buf_size - 1 - 1 ;
	g->logbuf.bufptr = g->logbuf.bufbase ;

	/* 填充行日志缓冲区 */ /* fill log buffer */
	if( g->pfuncLogStyle )
	{
		nret = g->pfuncLogStyle( g , & (g->logbuf) , c_filename , c_fileline , log_level , format , valist ) ;
		if( nret )
			return nret;
	}

	/* 自定义过滤日志 */ /* filter log */
	if( g->pfuncFilterLog )
	{
		nret = g->pfuncFilterLog( g , & (g->open_handle) , log_level , g->logbuf.bufbase , g->logbuf.buf_size-1-1 - g->logbuf.buf_remain_len ) ;
		if( nret )
			return nret;
	}

	/* 打开文件 */ /* open log */
	if( g->open_flag == 0 )
	{
		if( TEST_ATTRIBUTE( g->log_options , LOG_OPTION_CHANGE_TEST ) || TEST_ATTRIBUTE( g->log_options , LOG_OPTION_OPEN_ONCE ) )
		{
			/* 打开日志文件 */
			if( g->pfuncOpenLogFirst )
			{
				nret = g->pfuncOpenLogFirst( g , g->log_pathfilename , & (g->open_handle) ) ;
				if( nret )
					return nret;
			}
		}
		else if( TEST_ATTRIBUTE( g->log_options , LOG_OPTION_OPEN_AND_CLOSE ) )
		{
			/* 打开日志文件 */
			if( g->pfuncOpenLog )
			{
				nret = g->pfuncOpenLog( g , g->log_pathfilename , & (g->open_handle) ) ;
				if( nret )
					return nret;
			}
		}
	}

	/* 导出日志缓冲区 */ /* output log */
	if( g->pfuncWriteLog )
	{
		nret = g->pfuncWriteLog( g , & (g->open_handle) , log_level , g->logbuf.bufbase , g->logbuf.buf_size-1-1 - g->logbuf.buf_remain_len , & writelen ) ;
		if( nret )
			return nret;
	}

#if ( defined __unix ) || ( defined _AIX ) || ( defined __linux__ ) || ( defined __hpux )
	if( g->output == LOG_OUTPUT_FILE )
	{
		g->fsync_elapse--;
		if( g->fsync_elapse < 1 )
		{
			fsync( g->fd );
			g->fsync_elapse = g->fsync_period ;
		}
	}
#endif

	/* 关闭日志 */ /* close log */
	if( g->open_flag == 1 )
	{
		if( g->output == LOG_OUTPUT_FILE || g->output == LOG_OUTPUT_CALLBACK )
		{
			if( TEST_ATTRIBUTE( g->log_options , LOG_OPTION_CHANGE_TEST ) )
			{
				/* 测试日志文件 */
				if( g->pfuncChangeTest )
				{
					nret = g->pfuncChangeTest( g , & (g->test_handle) ) ;
					if( nret )
						return nret;
				}
			}
			else if( TEST_ATTRIBUTE( g->log_options , LOG_OPTION_OPEN_AND_CLOSE ) )
			{
				/* 关闭日志文件 */
				if( g->pfuncCloseLog )
				{
					nret = g->pfuncCloseLog( g , & (g->open_handle) ) ;
					if( nret )
						return nret;
				}
			}
		}
	}

	/* 如果输出到文件 */
	if( g->output == LOG_OUTPUT_FILE )
	{
		/* 日志转档侦测 */ /* rotate log file */
		if( g->rotate_mode == LOG_ROTATEMODE_NONE )
		{
		}
		else if( g->rotate_mode == LOG_ROTATEMODE_SIZE && g->log_rotate_size > 0 )
		{
			g->skip_count--;
			if( g->skip_count < 1 )
			{
				RotateLogFileSize( g , writelen );
			}
		}
		else if( g->rotate_mode == LOG_ROTATEMODE_PER_DAY )
		{
			RotateLogFilePerDate( g );
		}
		else if( g->rotate_mode == LOG_ROTATEMODE_PER_HOUR )
		{
			RotateLogFilePerHour( g );
		}
	}

	/* 清空一级缓存 */ /* clean level 1 cache */
	g->cache1_tv.tv_sec = 0 ;
	g->cache1_stime.tm_mday = 0 ;

	return 0;
}

/* 代码宏 */ /* code macros */
#define WRITELOGBASE(_g_,_log_level_) \
	va_list		valist; \
	int		nret ; \
	if( (_g_) == NULL ) \
	return LOG_RETURN_ERROR_PARAMETER; \
	if( (_g_)->output == LOG_OUTPUT_FILE && (_g_)->log_pathfilename[0] == '\0' ) \
	return 0; \
	if( TEST_LOGLEVEL_NOTENOUGH( _log_level_ , (_g_) ) ) \
	return 0; \
	va_start( valist , format ); \
	nret = WriteLogBase( (_g_) , c_filename , c_fileline , _log_level_ , format , valist ) ; \
	va_end( valist ); \
	if( nret < 0 ) \
	return nret;

/* 带日志等级的写日志 */ /* write log */
int WriteLog( LOG *g , char *c_filename , long c_fileline , int log_level , char *format , ... )
{
	WRITELOGBASE( g , log_level )
		return 0;
}
int WriteLogG( char *c_filename , long c_fileline , int log_level , char *format , ... )
{
	WRITELOGBASE( tls_g , log_level )
		return 0;
}


/* 写调试日志 */ /* write debug log */
int DebugLog( LOG *g , char *c_filename , long c_fileline , char *format , ... )
{
	WRITELOGBASE( g , LOG_LEVEL_DEBUG )
		return 0;
}
int DebugLogG( char *c_filename , long c_fileline , char *format , ... )
{
	WRITELOGBASE( tls_g , LOG_LEVEL_DEBUG )
		return 0;
}

/* 写普通信息日志 */ /* write info log */
int InfoLog( LOG *g , char *c_filename , long c_fileline , char *format , ... )
{
	WRITELOGBASE( g , LOG_LEVEL_INFO )
		return 0;
}
int InfoLogG( char *c_filename , long c_fileline , char *format , ... )
{
	WRITELOGBASE( tls_g , LOG_LEVEL_INFO )
		return 0;
}


/* 写警告日志 */ /* write warn log */
int WarnLog( LOG *g , char *c_filename , long c_fileline , char *format , ... )
{
	WRITELOGBASE( g , LOG_LEVEL_WARN )
		return 0;
}
int WarnLogG( char *c_filename , long c_fileline , char *format , ... )
{
	WRITELOGBASE( tls_g , LOG_LEVEL_WARN )
		return 0;
}


/* 写错误日志 */ /* write error log */
int ErrorLog( LOG *g , char *c_filename , long c_fileline , char *format , ... )
{
	WRITELOGBASE( g , LOG_LEVEL_ERROR )
		return 0;
}
int ErrorLogG( char *c_filename , long c_fileline , char *format , ... )
{
	WRITELOGBASE( tls_g , LOG_LEVEL_ERROR )
		return 0;
}


/* 写致命错误日志 */ /* write fatal log */
int FatalLog( LOG *g , char *c_filename , long c_fileline , char *format , ... )
{
	WRITELOGBASE( g , LOG_LEVEL_FATAL )
		return 0;
}
int FatalLogG( char *c_filename , long c_fileline , char *format , ... )
{
	WRITELOGBASE( tls_g , LOG_LEVEL_FATAL )
		return 0;
}



/* 文件变动测试间隔 */
#define LOG_FILECHANGETEST_INTERVAL_ON_OPEN_ONCE_DEFAULT	10

/* 设置日志选项 */ /* set log options */
int SetLogOptions( LOG *g , int log_options )
{
	if( g == NULL )
		return LOG_RETURN_ERROR_PARAMETER;

	g->log_options = log_options ;

	SetLogFileChangeTest( g , LOG_FILECHANGETEST_INTERVAL_ON_OPEN_ONCE_DEFAULT );

	return 0;
}
int SetLogOptionsG( int log_options )
{
	return SetLogOptions( tls_g , log_options );
}


/* 设置文件改变测试间隔 */
int SetLogFileChangeTest( LOG *g , long interval )
{
	if( g == NULL )
		return LOG_RETURN_ERROR_PARAMETER;

	g->file_change_test_interval = interval ;
	g->file_change_test_no = g->file_change_test_interval ;
	memset( & (g->file_change_stat) , 0x00 , sizeof(g->file_change_stat) );

	return 0;
}
int SetLogFileChangeTestG( long interval )
{
	return SetLogFileChangeTest( tls_g , interval );
}


/* 刷存储IO周期 */
int SetLogFsyncPeriod( LOG *g , long period )
{
	g->fsync_period = period ;
	g->fsync_elapse = g->fsync_period ;

	return 0;
}
int SetLogFsyncPeriodG( long period )
{
	return SetLogFsyncPeriod( tls_g , period );
}


/* 设置日志自定义标签 */ /* set log custom labels */
int SetLogCustLabel( LOG *g , int index , char *cust_label )
{
	if( g == NULL )
		return LOG_RETURN_ERROR_PARAMETER;
	if( 1 <= index && index <= LOG_MAXCNT_CUST_LABEL )
	{
		memset( g->cust_label[index-1] , 0x00 , sizeof(g->cust_label[index-1]) );
		if( cust_label && cust_label[0] != '\0' )
			strncpy( g->cust_label[index-1] , cust_label , sizeof(g->cust_label[index-1])-1 );
		return 0;
	}
	else
	{
		return LOG_RETURN_ERROR_PARAMETER;
	}
}
int SetLogCustLabelG( int index , char *cust_label )
{
	return SetLogCustLabel( tls_g , index , cust_label );
}


/* 设置日志转档模式 */ /* set log rotate mode */
int SetLogRotateMode( LOG *g , int rotate_mode )
{
	if( g == NULL )
		return LOG_RETURN_ERROR_PARAMETER;
	if( rotate_mode == LOG_ROTATEMODE_SIZE
		|| rotate_mode == LOG_ROTATEMODE_PER_DAY
		|| rotate_mode == LOG_ROTATEMODE_PER_HOUR )
	{
		g->rotate_mode = rotate_mode ;
		return 0;
	}
	else
	{
		return LOG_RETURN_ERROR_PARAMETER;
	}
}
int SetLogRotateModeG( int rotate_mode )
{
	return SetLogRotateMode( tls_g , rotate_mode );
}


/* 设置日志转档大小 */ /* set rotate size */
int SetLogRotateSize( LOG *g , long log_rotate_size )
{
	if( g == NULL )
		return LOG_RETURN_ERROR_PARAMETER;
	if( log_rotate_size <= 0 )
		return LOG_RETURN_ERROR_PARAMETER;
	g->log_rotate_size = log_rotate_size ;
	return 0;
}
int SetLogRotateSizeG( long log_rotate_size )
{
	return SetLogRotateSize( tls_g , log_rotate_size );
}


/* 设置日志转档紧迫系数 */ /* set rotate pressure fator */
int SetLogRotatePressureFactor( LOG *g , long pressure_factor )
{
	if( g == NULL )
		return LOG_RETURN_ERROR_PARAMETER;
	if( pressure_factor < 0 )
		return LOG_RETURN_ERROR_PARAMETER;
	g->pressure_factor = pressure_factor ;
	return 0;
}
int SetLogRotatePressureFactorG( long pressure_factor )
{
	return SetLogRotatePressureFactor( tls_g , pressure_factor );
}


/* 设置日志转档最大后缀序号 */
int SetLogRotateFileCount( LOG *g , long rotate_file_count )
{
	if( g == NULL )
		return LOG_RETURN_ERROR_PARAMETER;
	if( rotate_file_count <= 0 )
		return LOG_RETURN_ERROR_PARAMETER;
	g->rotate_file_count = rotate_file_count ;
	return 0;
}
int SetLogRotateFileCountG( long rotate_file_count )
{
	return SetLogRotateFileCount( tls_g , rotate_file_count );
}


/* 设置自定义日志转档前回调函数 */ /* set custom callback function before rotate log */
int SetBeforeRotateFileFunc( LOG *g , funcBeforeRotateFile *pfuncBeforeRotateFile )
{
	g->pfuncBeforeRotateFile = pfuncBeforeRotateFile ;
	return 0;
}
int SetBeforeRotateFileFuncG( funcBeforeRotateFile *pfuncBeforeRotateFile )
{
	return SetBeforeRotateFileFunc( tls_g , pfuncBeforeRotateFile );
}


/* 设置自定义日志转档后回调函数 */ /* set custom callback function after rotate log */
int SetAfterRotateFileFunc( LOG *g , funcAfterRotateFile *pfuncAfterRotateFile )
{
	g->pfuncAfterRotateFile = pfuncAfterRotateFile ;
	return 0;
}
int SetAfterRotateFileFuncG( funcAfterRotateFile *pfuncAfterRotateFile )
{
	return SetAfterRotateFileFunc( tls_g , pfuncAfterRotateFile );
}


/* 设置自定义检查日志等级回调函数类型 */ /* set custom filter callback function */
int SetFilterLogFunc( LOG *g , funcFilterLog *pfuncFilterLog )
{
	g->pfuncFilterLog = pfuncFilterLog ;
	return 0;
}
int SetFilterLogFuncG( funcFilterLog *pfuncFilterLog )
{
	return SetFilterLogFunc( tls_g , pfuncFilterLog );
}


/* 设置行日志缓冲区大小 */ /* set log buffer size */
int SetLogBufferSize( LOG *g , long log_bufsize , long max_log_bufsize )
{
	return SetBufferSize( g , & (g->logbuf) , log_bufsize , max_log_bufsize );
}
int SetLogBufferSizeG( long log_bufsize , long max_log_bufsize )
{
	return SetLogBufferSize( tls_g , log_bufsize , max_log_bufsize );
}


/* 直接设置日志输出回调函数 */
int SetLogOutputFuncDirectly( LOG *g , funcOpenLog *pfuncOpenLogFirst , funcOpenLog *pfuncOpenLog , funcWriteLog *pfuncWriteLog , funcChangeTest *pfuncChangeTest , funcCloseLog *pfuncCloseLog , funcCloseLog *pfuncCloseLogFinally )
{
	g->pfuncOpenLogFirst = pfuncOpenLogFirst ;
	g->pfuncOpenLog = pfuncOpenLog ;
	g->pfuncWriteLog = pfuncWriteLog ;
	g->pfuncChangeTest = pfuncChangeTest ;
	g->pfuncCloseLog = pfuncCloseLog ;
	g->pfuncCloseLogFinally = pfuncCloseLogFinally ;
	return 0;
}
int SetLogOutputFuncDirectlyG( funcOpenLog *pfuncOpenLogFirst , funcOpenLog *pfuncOpenLog , funcWriteLog *pfuncWriteLog , funcChangeTest *pfuncChangeTest , funcCloseLog *pfuncCloseLog , funcCloseLog *pfuncCloseLogFinally )
{
	return SetLogOutputFuncDirectly( tls_g , pfuncOpenLogFirst , pfuncOpenLog , pfuncWriteLog , pfuncChangeTest , pfuncCloseLog , pfuncCloseLogFinally );
}


/* 直接设置行日志风格回调函数 */
int SetLogStyleFuncDirectly( LOG *g , funcLogStyle *pfuncLogStyle )
{
	g->pfuncLogStyle = pfuncLogStyle ;

	return 0;
}
int SetLogStyleFuncDirectlyG( funcLogStyle *pfuncLogStyle )
{
	return SetLogStyleFuncDirectly( tls_g , pfuncLogStyle );
}


/* 得到基于线程本地存储的缺省日志句柄的函数版本 */
LOG *GetGlobalLOG()
{
	return tls_g;
}
void SetGlobalLOG( LOG *g )
{
	tls_g = g ;
	return;
}


int SetOpenFlag( LOG *g , char open_flag )
{
	g->open_flag = open_flag ;
	return 0;
}

char IsLogOpened( LOG *g )
{
	return g->open_flag ;
}

int GetLogLevel( LOG *g )
{
	return g->log_level;
}

LOGBUF *GetLogBuffer( LOG *g )
{
	return & (g->logbuf);
}



long FormatLogBuffer( LOG *g , LOGBUF *logbuf , char *format , ... )
{
	va_list		valist ;
	long		len ;
_REDO :
	va_start( valist , format );
	len = VSNPRINTF( logbuf->bufptr , logbuf->buf_remain_len , format , valist ) ;
	va_end( valist );
#if ( defined _WIN32 )
	if( len == -1 )
#elif ( defined __linux__ ) || ( defined _AIX )
	if( len >= logbuf->buf_remain_len )
#endif
	{
		long		new_buf_size ;
		int		nret ;
		if( logbuf->buf_size == logbuf->max_buf_size )
		{
			logbuf->bufptr[0] = '\0' ;
			return LOG_RETURN_ERROR_ALLOC_MAX;
		}
		new_buf_size = logbuf->buf_size * 2 ;
		if( new_buf_size > logbuf->max_buf_size )
			new_buf_size = logbuf->max_buf_size ;
		nret = SetBufferSize( g , logbuf , new_buf_size , -1 );
		if( nret )
			return nret;
		goto _REDO;
	}
	OFFSET_BUFPTR( logbuf , len )
		return len;
}

long FormatLogBufferV( LOG *g , LOGBUF *logbuf , char *format , va_list valist )
{
	long		len ;
_REDO :
	len = VSNPRINTF( logbuf->bufptr , logbuf->buf_remain_len , format , valist ) ;
#if ( defined _WIN32 )
	if( len == -1 )
#elif ( defined __linux__ ) || ( defined _AIX )
	if( len >= logbuf->buf_remain_len )
#endif
	{
		long		new_buf_size ;
		int		nret ;
		if( logbuf->buf_size == logbuf->max_buf_size )
		{
			logbuf->bufptr[0] = '\0' ;
			return LOG_RETURN_ERROR_ALLOC_MAX;
		}
		new_buf_size = logbuf->buf_size * 2 ;
		if( new_buf_size > logbuf->max_buf_size )
			new_buf_size = logbuf->max_buf_size ;
		nret = SetBufferSize( g , logbuf , new_buf_size , -1 );
		if( nret )
			return nret;
		goto _REDO;
	}
	OFFSET_BUFPTR( logbuf , len )
		return len;
}

long MemcatLogBuffer( LOG *g , LOGBUF *logbuf , char *append , long len )
{
_REDO :
	if( len > logbuf->buf_remain_len )
	{
		long		new_buf_size ;
		int		nret ;
		if( logbuf->buf_size == logbuf->max_buf_size )
			return LOG_RETURN_ERROR_ALLOC_MAX;
		new_buf_size = logbuf->buf_size * 2 ;
		if( new_buf_size > logbuf->max_buf_size )
			new_buf_size = logbuf->max_buf_size ;
		nret = SetBufferSize( g , logbuf , new_buf_size , -1 );
		if( nret )
			return nret;
		goto _REDO;
	}
	memcpy( logbuf->bufptr , append , len );
	OFFSET_BUFPTR( logbuf , len )
		logbuf->bufptr[0] = '\0' ;
	return len;
}

int ConvertLogOutput_atoi( char *output_desc , int *p_log_output )
{
	if( strcmp( output_desc , "STDOUT" ) == 0 )
		(*p_log_output) = LOG_OUTPUT_STDOUT ;
	else if( strcmp( output_desc , "STDERR" ) == 0 )
		(*p_log_output) = LOG_OUTPUT_STDERR ;
	else if( strcmp( output_desc , "SYSLOG" ) == 0 )
		(*p_log_output) = LOG_OUTPUT_SYSLOG ;
	else if( strcmp( output_desc , "FILE" ) == 0 )
		(*p_log_output) = LOG_OUTPUT_FILE ;
	else if( strcmp( output_desc , "CALLBACK" ) == 0 )
		(*p_log_output) = LOG_OUTPUT_CALLBACK ;
	else
		return LOG_RETURN_ERROR_PARAMETER;

	return 0;
}

int ConvertLogLevel_atoi( char *log_level_desc , int *p_log_level )
{
	if( strcmp( log_level_desc , "DEBUG" ) == 0 )
		(*p_log_level) = LOG_LEVEL_DEBUG ;
	else if( strcmp( log_level_desc , "INFO" ) == 0 )
		(*p_log_level) = LOG_LEVEL_INFO ;
	else if( strcmp( log_level_desc , "WARN" ) == 0 )
		(*p_log_level) = LOG_LEVEL_WARN ;
	else if( strcmp( log_level_desc , "ERROR" ) == 0 )
		(*p_log_level) = LOG_LEVEL_ERROR ;
	else if( strcmp( log_level_desc , "FATAL" ) == 0 )
		(*p_log_level) = LOG_LEVEL_FATAL ;
	else if( strcmp( log_level_desc , "NOLOG" ) == 0 )
		(*p_log_level) = LOG_LEVEL_NOLOG ;
	else
		return LOG_RETURN_ERROR_PARAMETER;

	return 0;
}

int ConvertLogLevel_itoa( int log_level , char **log_level_desc )
{
	(*log_level_desc) = sg_aszLogLevelDesc[log_level] ;
	return 0;
}

int ConvertLogStyle_atol( char *line_style_desc , long *p_line_style )
{
	if( strcmp( line_style_desc , "DATE" ) == 0 )
		(*p_line_style) = LOG_STYLE_DATE ;
	else if( strcmp( line_style_desc , "DATETIME" ) == 0 )
		(*p_line_style) = LOG_STYLE_DATETIME ;
	else if( strcmp( line_style_desc , "DATETIMEMS" ) == 0 )
		(*p_line_style) = LOG_STYLE_DATETIMEMS ;
	else if( strcmp( line_style_desc , "LOGLEVEL" ) == 0 )
		(*p_line_style) = LOG_STYLE_LOGLEVEL ;
	else if( strcmp( line_style_desc , "PID" ) == 0 )
		(*p_line_style) = LOG_STYLE_PID ;
	else if( strcmp( line_style_desc , "TID" ) == 0 )
		(*p_line_style) = LOG_STYLE_TID ;
	else if( strcmp( line_style_desc , "SOURCE" ) == 0 )
		(*p_line_style) = LOG_STYLE_SOURCE ;
	else if( strcmp( line_style_desc , "FORMAT" ) == 0 )
		(*p_line_style) = LOG_STYLE_FORMAT ;
	else if( strcmp( line_style_desc , "NEWLINE" ) == 0 )
		(*p_line_style) = LOG_STYLE_NEWLINE ;
	else if( strcmp( line_style_desc , "CUSTLABEL1" ) == 0 )
		(*p_line_style) = LOG_STYLE_CUSTLABEL1 ;
	else if( strcmp( line_style_desc , "CUSTLABEL2" ) == 0 )
		(*p_line_style) = LOG_STYLE_CUSTLABEL2 ;
	else if( strcmp( line_style_desc , "CUSTLABEL3" ) == 0 )
		(*p_line_style) = LOG_STYLE_CUSTLABEL3 ;
	else
		return LOG_RETURN_ERROR_PARAMETER;

	return 0;
}

int ConvertLogOption_atol( char *log_option_desc , long *p_log_option )
{
	if( strcmp( log_option_desc , "OPEN_AND_CLOSE" ) == 0 )
		(*p_log_option) = LOG_OPTION_OPEN_AND_CLOSE ;
	else if( strcmp( log_option_desc , "CHANGE_TEST" ) == 0 )
		(*p_log_option) = LOG_OPTION_CHANGE_TEST ;
	else if( strcmp( log_option_desc , "OPEN_ONCE" ) == 0 )
		(*p_log_option) = LOG_OPTION_OPEN_ONCE ;
	else if( strcmp( log_option_desc , "SET_OUTPUT_BY_FILENAME" ) == 0 )
		(*p_log_option) = LOG_OPTION_SET_OUTPUT_BY_FILENAME ;
	else if( strcmp( log_option_desc , "FILENAME_APPEND_DOT_LOG" ) == 0 )
		(*p_log_option) = LOG_OPTION_FILENAME_APPEND_DOT_LOG ;
	else
		return LOG_RETURN_ERROR_PARAMETER;

	return 0;
}

int ConvertLogRotateMode_atoi( char *rotate_mode_desc , int *p_rotate_mode )
{
	if( strcmp( rotate_mode_desc , "NONE" ) == 0 )
		(*p_rotate_mode) = LOG_ROTATEMODE_NONE ;
	else if( strcmp( rotate_mode_desc , "SIZE" ) == 0 )
		(*p_rotate_mode) = LOG_ROTATEMODE_SIZE ;
	else if( strcmp( rotate_mode_desc , "PER_DAY" ) == 0 )
		(*p_rotate_mode) = LOG_ROTATEMODE_PER_DAY ;
	else if( strcmp( rotate_mode_desc , "PER_HOUR" ) == 0 )
		(*p_rotate_mode) = LOG_ROTATEMODE_PER_HOUR ;
	else
		return LOG_RETURN_ERROR_PARAMETER;

	return 0;
}

int ConvertBufferSize_atol( char *bufsize_desc , long *p_bufsize )
{
	long	bufsize_desc_len ;

	bufsize_desc_len = strlen(bufsize_desc) ;

	if( bufsize_desc_len > 2 && strcmp( bufsize_desc + bufsize_desc_len - 2 , "GB" ) == 0 )
		(*p_bufsize) = atol(bufsize_desc) * 1024 * 1024 * 1024 ;
	else if( bufsize_desc_len > 2 && strcmp( bufsize_desc + bufsize_desc_len - 2 , "MB" ) == 0 )
		(*p_bufsize) = atol(bufsize_desc) * 1024 * 1024 ;
	else if( bufsize_desc_len > 2 && strcmp( bufsize_desc + bufsize_desc_len - 2 , "KB" ) == 0 )
		(*p_bufsize) = atol(bufsize_desc) * 1024 ;
	else if( bufsize_desc_len > 1 && strcmp( bufsize_desc + bufsize_desc_len - 1 , "B" ) == 0 )
		(*p_bufsize) = atol(bufsize_desc) ;
	else
		(*p_bufsize) = atol(bufsize_desc) ;

	if( (*p_bufsize) == 0 )
		return LOG_RETURN_ERROR_PARAMETER;

	return 0;
}


_WINDLL_FUNC LOG *ReadLogHandleFromConfig( FILE *fp , char *id )
{
	LOG		*g = NULL ;

	char		filebuffer[ 1024 + 1 ] ;
	char		key[ 64 + 1 ] ;
	char		value[ 512 + 1 ] ;
	char		value2[ 512 + 1 ] ;

	long		option , options ;
	int		output ;
	int		level ;
	long		style , styles ;
	int		log_rotate_mode ;
	long		log_rotate_size ;
	long		log_bufsize , max_log_bufsize ;

	char		*pc = NULL ;

	int		nret ;

	g = CreateLogHandle() ;
	if( g == NULL )
		return NULL;

	while(1)
	{
		/* 读入一行并分解之 */ /* read a row and decomposition */
		memset( filebuffer , 0x00 , sizeof(filebuffer) );
		if( fgets( filebuffer , sizeof(filebuffer)-1 , fp ) == NULL )
			break;

		memset( key , 0x00 , sizeof(key) );
		memset( value , 0x00 , sizeof(value) );
		memset( value2 , 0x00 , sizeof(value2) );
		sscanf( filebuffer , "%s %s %[^\n]" , key , value , value2 );
		if( key[0] == '\0' )
			break;

		if( value2[0] == '\"' )
		{
			long	len2 ;
			len2 = strlen(value2) ;
			if( value2[len2-1] == '\"' ) /* 如果最后为引号，去掉之 */ /* by found */
				value2[len2-1] = '\0' ;
			memmove( value2 , value2 + 1 , len2 - 1 );
		}

		if( STRCMP( key , == , "id" ) )
		{
			if( id )
				strcpy( id , value );
		}
		else if( STRCMP( key , == , "options" ) )
		{
			options = 0 ;
			pc = strtok( value , "|" ) ;
			while( pc )
			{
				nret = ConvertLogOption_atol( pc , & option ) ;
				if( nret )
					goto ERR;

				options |= option ;

				pc = strtok( NULL , "|" ) ;
			}

			nret = SetLogOptions( g , options ) ;
			if( nret )
				goto ERR;
		}
		else if( STRCMP( key , == , "output" ) )
		{
			nret = ConvertLogOutput_atoi( value , & output ) ;
			if( nret )
				goto ERR;

			nret = SetLogOutput( g , output , value2 , LOG_NO_OUTPUTFUNC ) ;
			if( nret )
				goto ERR;
		}
		else if( STRCMP( key , == , "level" ) )
		{
			nret = ConvertLogLevel_atoi( value , & level ) ;
			if( nret )
				goto ERR;

			nret = SetLogLevel( g , level ) ;
			if( nret )
				goto ERR;
		}
		else if( STRCMP( key , == , "styles" ) )
		{
			styles = 0 ;
			pc = strtok( value , "|" ) ;
			while( pc )
			{
				nret = ConvertLogStyle_atol( pc , & style ) ;
				if( nret )
					goto ERR;

				styles |= style ;

				pc = strtok( NULL , "|" ) ;
			}

			nret = SetLogStyles( g , styles , LOG_NO_STYLEFUNC ) ;
			if( nret )
				goto ERR;
		}
		else if( STRCMP( key , == , "custlabel1" ) )
		{
			nret = SetLogCustLabel( g , 1 , value ) ;
			if( nret )
				goto ERR;
		}
		else if( STRCMP( key , == , "custlabel2" ) )
		{
			nret = SetLogCustLabel( g , 2 , value ) ;
			if( nret )
				goto ERR;
		}
		else if( STRCMP( key , == , "custlabel3" ) )
		{
			nret = SetLogCustLabel( g , 3 , value ) ;
			if( nret )
				goto ERR;
		}
		else if( STRCMP( key , == , "rotate_mode" ) )
		{
			nret = ConvertLogRotateMode_atoi( value , & log_rotate_mode ) ;
			if( nret )
				goto ERR;

			nret = SetLogRotateMode( g , log_rotate_mode ) ;
			if( nret )
				goto ERR;
		}
		else if( STRCMP( key , == , "rotate_size" ) )
		{
			nret = ConvertBufferSize_atol( value , & log_rotate_size ) ;
			if( nret )
				goto ERR;

			nret = SetLogRotateSize( g , log_rotate_size ) ;
			if( nret )
				goto ERR;
		}
		else if( STRCMP( key , == , "rotate_pressure_factor" ) )
		{
			nret = SetLogRotatePressureFactor( g , atol(value) ) ;
			if( nret )
				goto ERR;
		}
		else if( STRCMP( key , == , "log_bufsize" ) )
		{
			if( STRCMP( value , != , "" ) )
			{
				nret = ConvertBufferSize_atol( value , & log_bufsize ) ;
				if( nret )
					goto ERR;
			}
			else
			{
				log_bufsize = -1 ;
			}

			if( STRCMP( value2 , != , "" ) )
			{
				nret = ConvertBufferSize_atol( value2 , & max_log_bufsize ) ;
				if( nret )
					goto ERR;
			}
			else
			{
				max_log_bufsize = -1 ;
			}

			nret = SetLogBufferSize( g , log_bufsize , max_log_bufsize ) ;
			if( nret )
				goto ERR;
		}
	}

	return g;

ERR :

	DestroyLogHandle( g );
	return NULL;
}

extern int ExpandPathFilename( char *pathfilename , long pathfilename_bufsize );

LOG *CreateLogHandleFromConfig( char *config_filename , char *postfix )
{
	char    config_pathfilename[ MAXLEN_FILENAME + 1 ] ;
	FILE	*fp = NULL ;
	LOG		*g = NULL ;

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

	g = ReadLogHandleFromConfig( fp , NULL ) ;
	if( g == NULL )
		return NULL;

	fclose(fp);

	return g;
}
