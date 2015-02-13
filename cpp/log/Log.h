#ifndef _H_LOG_
#define _H_LOG_

/*
* iLOG3 - log function library written in c
* author	: calvin
* email	: calvinwilliams.c@gmail.com
* LastVersion	: v1.0.11
*
* Licensed under the LGPL v2.1, see the file LICENSE in base directory.
*/

#ifdef __cplusplus
extern "C" {
#endif

#if ( defined _WIN32 )
#ifndef _WINDLL_FUNC
#define _WINDLL_FUNC		_declspec(dllexport)
#endif
#elif ( defined __unix ) || ( defined _AIX ) || ( defined __linux__ ) || ( defined __hpux )
#ifndef _WINDLL_FUNC
#define _WINDLL_FUNC
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#if ( defined _WIN32 )
#include <windows.h>
#include <share.h>
#include <io.h>
#include <fcntl.h>
#elif ( defined __unix ) || ( defined _AIX ) || ( defined __linux__ ) || ( defined __hpux )
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <syslog.h>
#include <pthread.h>
#endif

#if ( defined __STDC_VERSION__ ) && ( __STDC_VERSION__ >= 199901 )
	struct tm *localtime_r(const time_t *timep, struct tm *result);
#endif


	/* ��־�ȼ��� */ 
#define LOG_LEVEL_DEBUG		0	/* ���Եȼ� */ 
#define LOG_LEVEL_INFO		1	/* ��ͨ��Ϣ�ȼ� */ 
#define LOG_LEVEL_WARN		2	/* ����ȼ� */ 
#define LOG_LEVEL_ERROR		3	/* ����ȼ� */ 
#define LOG_LEVEL_FATAL		4	/* ���ش���ȼ� */ 
#define LOG_LEVEL_NOLOG		5	/* ����Ҫ�����־ */ 


	/* ��־������� */ 
#define LOG_OUTPUT_NOSET	-1		/* ������ */
#define LOG_OUTPUT_STDOUT	1		/* ��׼��� */ 
#define LOG_OUTPUT_STDERR	2		/* ��׼������� */
#define LOG_OUTPUT_SYSLOG	3		/* UNIX&Linux��syslog �� Windows��WINDOWS EVENT */
#define LOG_OUTPUT_FILE		11		/* �ļ� */ 
#define LOG_OUTPUT_CALLBACK	21		/* ��ʹ���Զ�����־����ص������� */
#define LOG_LEVEL_DEFAULT	LOG_LEVEL_NOLOG


	/* ����־�����Ϻ� */ 
#define LOG_STYLE_DATE			1		/* ����"YYYY-MM-DD" */ 
#define LOG_STYLE_DATETIME		2		/* ����ʱ��"YYYY-MM-DD hh:mm:ss" */ 
#define LOG_STYLE_DATETIMEMS	4		/* ����ʱ�����"YYYY-MM-DD hh:mm:ss.6ms"������ʱ����껥�⣬�����Զ�ѡ����Ϣ�����ģ� */
#define LOG_STYLE_LOGLEVEL		8		/* ��־�ȼ� */ 
#define LOG_STYLE_PID			16		/* ����id */ 
#define LOG_STYLE_TID			32		/* �߳�id */ 
#define LOG_STYLE_SOURCE		64		/* "Դ�����ļ���:Դ�����к�" */ 
#define LOG_STYLE_FORMAT		128		/* Ӧ����־�� */ 
#define LOG_STYLE_NEWLINE		256		/* ���� */ 
#define LOG_STYLE_CUSTLABEL1	512		/* �Զ����ǩ1 */
#define LOG_STYLE_CUSTLABEL2	1024	/* �Զ����ǩ2 */ 
#define LOG_STYLE_CUSTLABEL3	2048	/* �Զ����ǩ3 */ 
#define LOG_STYLE_CALLBACK		4096	/* ��ʹ���Զ�������־���ص������� */ 
#define LOG_STYLES_DEFAULT		0


	/* ��������ֵ�� */ 
#define LOG_RETURN_ERROR_ALLOC		-11		/* �����ڴ�ʧ�� */ 
#define LOG_RETURN_ERROR_INTERNAL	-12		/* �ڲ����� */ 
#define LOG_RETURN_ERROR_ALLOC_MAX	-13		/* �ڴ�ʹ�ó��� */
#define LOG_RETURN_ERROR_PARAMETER	-14		/* �������� */ 
#define LOG_RETURN_ERROR_NOTSUPPORT	-17		/* �ݲ�֧�� */ 
#define LOG_RETURN_ERROR_CREATEFILE	-21		/* �����ļ�ʧ�� */
#define LOG_RETURN_ERROR_OPENFILE	-22		/* ���ļ�ʧ�� */
#define LOG_RETURN_ERROR_WRITEFILE	-23		/* д�ļ�ʧ�� */ 


#define LOG_NO_STYLEFUNC	NULL
#define LOG_NO_OUTPUTFUNC	NULL , NULL , NULL , NULL , NULL , NULL

#ifndef MAXLEN_FILENAME
#define MAXLEN_FILENAME		256
#endif


	typedef struct tagLOG LOG ;
	typedef struct tagLOGBUF LOGBUF ;


	/* ������־������� */ 
	_WINDLL_FUNC LOG *CreateLogHandle();
	_WINDLL_FUNC void DestroyLogHandle( LOG *g );

	/* �Զ���򿪡�������ر���־�������� */ 
	typedef int funcOpenLog( LOG *g , char *log_pathfilename , void **open_handle );
	typedef int funcWriteLog( LOG *g , void **open_handle , int log_level , char *buf , long len , long *writelen );
	typedef int funcChangeTest( LOG *g , void **test_handle );
	typedef int funcCloseLog( LOG *g , void **open_handle );
	typedef int funcLogStyle( LOG *g , LOGBUF *logbuf , char *c_filename , long c_fileline , int log_level , char *format , va_list valist );

	/* ����������ú��� */
	_WINDLL_FUNC int SetLogOutput( LOG *g , int output , char *log_pathfilename , funcOpenLog *pfuncOpenLogFirst , funcOpenLog *pfuncOpenLog , funcWriteLog *pfuncWriteLog , funcChangeTest *pfuncChangeTest , funcCloseLog *pfuncCloseLog , funcCloseLog *pfuncCloseLogFinally );
	_WINDLL_FUNC int SetLogOutput2( LOG *g , int output , funcOpenLog *pfuncOpenLogFirst , funcOpenLog *pfuncOpenLog , funcWriteLog *pfuncWriteLog , funcChangeTest *pfuncChangeTest , funcCloseLog *pfuncCloseLog , funcCloseLog *pfuncCloseLogFinally , char *log_pathfilename_format , ... );
	_WINDLL_FUNC int SetLogLevel( LOG *g , int log_level );
	_WINDLL_FUNC int SetLogStyles( LOG *g , long log_styles , funcLogStyle *pfuncLogStyle );


	/* д��־���� */ /* output log */
	_WINDLL_FUNC int WriteLog( LOG *g , char *c_filename , long c_fileline , int log_level , char *format , ... );
	_WINDLL_FUNC int DebugLog( LOG *g , char *c_filename , long c_fileline , char *format , ... );
	_WINDLL_FUNC int InfoLog( LOG *g , char *c_filename , long c_fileline , char *format , ... );
	_WINDLL_FUNC int WarnLog( LOG *g , char *c_filename , long c_fileline , char *format , ... );
	_WINDLL_FUNC int ErrorLog( LOG *g , char *c_filename , long c_fileline , char *format , ... );
	_WINDLL_FUNC int FatalLog( LOG *g , char *c_filename , long c_fileline , char *format , ... );


	/* д��־�����Ŀɱ������ */ /* output log macros */
#define LOGFAST( _g_ , _log_level_ , ... )	\
	if ((_log_level_) >= _g_->log_level) \
	WriteLog( _g_ , __FILE__ , __LINE__ , _log_level_ , __VA_ARGS__ );

#define WRITELOG( _g_ , _log_level_ , ... )	WriteLog( _g_ , __FILE__ , __LINE__ , _log_level_ , __VA_ARGS__ );
#define DEBUGLOG( _g_ , ... )			DebugLog( _g_ , __FILE__ , __LINE__ , __VA_ARGS__ );
#define INFOLOG( _g_ , ... )			InfoLog( _g_ , __FILE__ , __LINE__ , __VA_ARGS__ );
#define WARNLOG( _g_ , ... )			WarnLog( _g_ , __FILE__ , __LINE__ , __VA_ARGS__ );
#define ERRORLOG( _g_ , ... )			ErrorLog( _g_ , __FILE__ , __LINE__ , __VA_ARGS__ );
#define FATALLOG( _g_ , ... )			FatalLog( _g_ , __FILE__ , __LINE__ , __VA_ARGS__ );


	//�̰߳�ȫ����־
	_WINDLL_FUNC LOG *CreateLogHandleG();
	_WINDLL_FUNC void DestroyLogHandleG();

	_WINDLL_FUNC int SetLogOutputG( int output , char *log_pathfilename , funcOpenLog *pfuncOpenLogFirst , funcOpenLog *pfuncOpenLog , funcWriteLog *pfuncWriteLog , funcChangeTest *pfuncChangeTest , funcCloseLog *pfuncCloseLog , funcCloseLog *pfuncCloseLogFinally );
	_WINDLL_FUNC int SetLogLevelG( int log_level );
	_WINDLL_FUNC int SetLogStylesG( long log_styles , funcLogStyle *pfuncLogStyles );

#define SetLogOutput2G(_output_,_pfuncOpenLogFirst_,_pfuncOpenLog_,_pfuncWriteLog_,_pfuncChangeTest_,_pfuncCloseLog_,_pfuncCloseLogFinally_,_log_pathfilename_format_,...) \
	SetLogOutput2(_output_,_pfuncOpenLogFirst_,_pfuncOpenLog_,_pfuncWriteLog_,_pfuncChangeTest_,_pfuncCloseLog_,_pfuncCloseLogFinally_,_log_pathfilename_format_,__VA_ARGS__)


	/* д��־�����������̱߳��ش洢��ȱʡ��־����ĺ������ϰ汾�� */ 
	_WINDLL_FUNC int WriteLogG( char *c_filename , long c_fileline , int log_level , char *format , ... );
	_WINDLL_FUNC int DebugLogG( char *c_filename , long c_fileline , char *format , ... );
	_WINDLL_FUNC int InfoLogG( char *c_filename , long c_fileline , char *format , ... );
	_WINDLL_FUNC int WarnLogG( char *c_filename , long c_fileline , char *format , ... );
	_WINDLL_FUNC int ErrorLogG( char *c_filename , long c_fileline , char *format , ... );
	_WINDLL_FUNC int FatalLogG( char *c_filename , long c_fileline , char *format , ... );


	/* д��־�����Ŀɱ�����꣨�����̱߳��ش洢��ȱʡ��־����ĺ������ϰ汾�� */  
#define WRITELOGG( _log_level_ , ... )	WriteLogG( __FILE__ , __LINE__ , _log_level_ , __VA_ARGS__ );
#define DEBUGLOGG( ... )		DebugLogG( __FILE__ , __LINE__ , __VA_ARGS__ );
#define INFOLOGG( ... )			InfoLogG( __FILE__ , __LINE__ , __VA_ARGS__ );
#define WARNLOGG( ... )			WarnLogG( __FILE__ , __LINE__ , __VA_ARGS__ );
#define ERRORLOGG( ... )		ErrorLogG( __FILE__ , __LINE__ , __VA_ARGS__ );
#define FATALLOGG( ... )		FatalLogG( __FILE__ , __LINE__ , __VA_ARGS__ );



	/********************************************************/
	/* ����Ϊ�߼�����                                       */
	/********************************************************/

	/* �Զ�������־�ȼ��ص��������� */ 
	typedef int funcFilterLog( LOG *g , void **open_handle , int log_level , char *buf , long len );

	/* �Զ�����־ת��ǰ��ص��������� */ 
	typedef int funcBeforeRotateFile( LOG *g , char *rotate_log_pathfilename );
	typedef int funcAfterRotateFile( LOG *g , char *rotate_log_pathfilename );

	/* �Զ����ǩ���� */ 
#define LOG_MAXCNT_CUST_LABEL		3

	/* �Զ����ǩ��󳤶� */ 
#define LOG_MAXLEN_CUST_LABEL		32


	/* ��־ѡ�� */ 
#define LOG_OPTION_OPEN_AND_CLOSE			1	/* ÿ�ζ�����־��д��־���ر���־ */ 
#define LOG_OPTION_CHANGE_TEST				2	/* ����ļ��䶯   Ĭ��*/
#define LOG_OPTION_OPEN_ONCE				4	/* ��־��һ�β��ر� */ 
#define LOG_OPTION_SET_OUTPUT_BY_FILENAME	8	/* �Զ������ļ�������������� */

	/* "#stdout" -> LOG_OUTPUT_STDOUT */
	/* "#stderr" -> LOG_OUTPUT_STDERR */
	/* "#syslog" -> LOG_OUTPUT_SYSLOG */
#define LOG_OPTION_FILENAME_APPEND_DOT_LOG	16	/* ��־����ļ������Զ�����".log" */ 

#define LOG_OPTION_OPEN_DEFAULT			LOG_OPTION_CHANGE_TEST


	/* ��־ת��ģʽ */ 
#define LOG_ROTATEMODE_NONE			0	/* ��ת�� */ 
#define LOG_ROTATEMODE_SIZE			1	/* ����־�ļ���Сת�����ͺ���SetLogRotateSize���ʹ�ã�ת���ļ�����ʽ"ԭ��־�ļ���.���" */ 
#define LOG_ROTATEMODE_PER_DAY		2	/* ��ÿ��ת����ת���ļ�����ʽ"ԭ��־�ļ���.����������������" */
#define LOG_ROTATEMODE_PER_HOUR		3	/* ��Сʱת����ת���ļ�����ʽ"ԭ��־�ļ���.����������������_Сʱ" */ 

	/* ��־ת��ȱʡֵ */ 
#define LOG_ROTATE_SIZE_FILE_COUNT_DEFAULT			99999999
#define LOG_ROTATE_SIZE_PRESSURE_FACTOR_DEFAULT		0
#define LOG_FSYNC_PERIOD							10000


	/* ��������Сȱʡֵ */
#define LOG_BUFSIZE_DEFAULT				(1024)		/* ȱʡ����־��������С */
#define LOG_BUFSIZE_MAX					(16*1024)	/* �������־��������С */


	/* �߼�����������ú��� */
	_WINDLL_FUNC int SetLogOptions( LOG *g , int log_options );
	_WINDLL_FUNC int SetLogFileChangeTest( LOG *g , long interval );
	_WINDLL_FUNC int SetLogFsyncPeriod( LOG *g , long period );
	_WINDLL_FUNC int SetLogCustLabel( LOG *g , int index , char *cust_label );
	_WINDLL_FUNC int SetLogRotateMode( LOG *g , int rotate_mode );
	_WINDLL_FUNC int SetLogRotateSize( LOG *g , long log_rotate_size );
	_WINDLL_FUNC int SetLogRotatePressureFactor( LOG *g , long pressure_factor );
	_WINDLL_FUNC int SetLogRotateFileCount( LOG *g , long rotate_file_count );
	_WINDLL_FUNC int SetBeforeRotateFileFunc( LOG *g , funcAfterRotateFile *pfuncAfterRotateFile );
	_WINDLL_FUNC int SetAfterRotateFileFunc( LOG *g , funcAfterRotateFile *pfuncAfterRotateFile );
	_WINDLL_FUNC int SetFilterLogFunc( LOG *g , funcFilterLog *pfuncFilterLog );
	_WINDLL_FUNC int SetLogBufferSize( LOG *g , long log_bufsize , long max_log_bufsize );
	_WINDLL_FUNC int SetLogOutputFuncDirectly( LOG *g , funcOpenLog *pfuncOpenLogFirst , funcOpenLog *pfuncOpenLog , funcWriteLog *pfuncWriteLog , funcChangeTest *pfuncChangeTest , funcCloseLog *pfuncCloseLog , funcCloseLog *pfuncCloseLogFinally );
	_WINDLL_FUNC int SetLogStyleFuncDirectly( LOG *g , funcLogStyle *pfuncLogStyle );



	_WINDLL_FUNC int SetLogOptionsG( int log_options );
	_WINDLL_FUNC int SetLogFileChangeTestG( long interval );
	_WINDLL_FUNC int SetLogFsyncPeriodG( long period );
	_WINDLL_FUNC int SetLogCustLabelG( int index , char *cust_label );
	_WINDLL_FUNC int SetLogRotateModeG( int rotate_mode );
	_WINDLL_FUNC int SetLogRotateSizeG( long log_rotate_size );
	_WINDLL_FUNC int SetLogRotatePressureFactorG( long pressure_factor );
	_WINDLL_FUNC int SetLogRotateFileCountG( long rotate_file_count );
	_WINDLL_FUNC int SetBeforeRotateFileFuncG( funcAfterRotateFile *pfuncAfterRotateFile );
	_WINDLL_FUNC int SetAfterRotateFileFuncG( funcAfterRotateFile *pfuncAfterRotateFile );
	_WINDLL_FUNC int SetFilterLogFuncG( funcFilterLog *pfuncFilterLog );
	_WINDLL_FUNC int SetLogBufferSizeG( long log_bufsize , long max_log_bufsize );
	_WINDLL_FUNC int SetLogOutputFuncDirectlyG( funcOpenLog *pfuncOpenLogFirst , funcOpenLog *pfuncOpenLog , funcWriteLog *pfuncWriteLog , funcChangeTest *pfuncChangeTest , funcCloseLog *pfuncCloseLog , funcCloseLog *pfuncCloseLogFinally );
	_WINDLL_FUNC int SetLogStyleFuncDirectlyG( funcLogStyle *pfuncLogStyle );


#define TEST_LOGLEVEL_NOTENOUGH(_log_level_,_g_)	( (_log_level_) < (_g_)->log_level )
#define TEST_ATTRIBUTE(_param_,_attr_)			( ( _param_ & _attr_ ) == _attr_ )
#define TEST_NOT_ATTRIBUTE(_param_,_attr_)		( ( _param_ & _attr_ ) != _attr_ )
#define TEST_STDSTREAM(_fd_)				( (_fd_) != FD_NULL && ( (_fd_) == STDOUT_HANDLE || (_fd_) == STDERR_HANDLE ) )
#define TEST_FILEFD(_fd_)				( (_fd_) != FD_NULL && (_fd_) != STDOUT_HANDLE && (_fd_) != STDERR_HANDLE )


	/* �õ������̱߳��ش洢��ȱʡ��־����ĺ����汾 */ /* log handle get/set function for TLS */
	_WINDLL_FUNC LOG *GetGlobalLOG();
	_WINDLL_FUNC void SetGlobalLOG( LOG *g );


	/********************************************************/
	/* ����Ϊ���ӿ��� the following development for the hook*/
	/********************************************************/

	/* ƽ̨�� */ /* platfrom macros */
#if ( defined _WIN32 )
#define LOG_NEWLINE		"\r\n"
#define LOG_NEWLINE_LEN		2
#define SNPRINTF		_snprintf
#define VSNPRINTF		_vsnprintf
#define STDOUT_HANDLE		1
#define STDERR_HANDLE		2
#define FD_NULL			-1
#define WRITE			_write
#define STAT			_stat
#define ACCESS			_access
#define ACCESS_OK		00
#define RENAME			rename
#define UNLINK			unlink
#elif ( defined __unix ) || ( defined _AIX ) || ( defined __linux__ ) || ( defined __hpux )
#define LOG_NEWLINE		"\n"
#define LOG_NEWLINE_LEN		1
#define SNPRINTF		snprintf
#define VSNPRINTF		vsnprintf
#define STDOUT_HANDLE		STDOUT_FILENO
#define STDERR_HANDLE		STDERR_FILENO
#define FD_NULL			-1
#define WRITE			write
#define STAT			stat
#define ACCESS			access
#define ACCESS_OK		F_OK
#define RENAME			rename
#define UNLINK			unlink
#endif

	/* ����� */ 
#if ( defined _WIN32 )
#define SYSTEMTIME2TIMEVAL_USEC(_syst_,_tv_) \
	(_tv_).tv_usec = (_syst_).wMilliseconds * 1000 ;
#define SYSTEMTIME2TM(_syst_,_stime_) \
	(_stime_).tm_year = (_syst_).wYear - 1900 ; \
	(_stime_).tm_mon = (_syst_).wMonth - 1 ; \
	(_stime_).tm_mday = (_syst_).wDay ; \
	(_stime_).tm_hour = (_syst_).wHour ; \
	(_stime_).tm_min = (_syst_).wMinute ; \
	(_stime_).tm_sec = (_syst_).wSecond ;
#define LOCALTIME(_tt_,_stime_) \
	{ \
	SYSTEMTIME	stNow ; \
	GetLocalTime( & stNow ); \
	SYSTEMTIME2TM( stNow , (_stime_) ); \
	}
#elif ( defined __unix ) || ( defined _AIX ) || ( defined __linux__ ) || ( defined __hpux )
#define LOCALTIME(_tt_,_stime_) \
	localtime_r(&(_tt_),&(_stime_));
#endif

#ifndef STRCMP
#define STRCMP(_a_,_C_,_b_) ( strcmp(_a_,_b_) _C_ 0 )
#define STRNCMP(_a_,_C_,_b_,_n_) ( strncmp(_a_,_b_,_n_) _C_ 0 )
#endif

#ifndef STRICMP
#if ( defined _WIN32 )
#define STRICMP(_a_,_C_,_b_) ( stricmp(_a_,_b_) _C_ 0 )
#define STRNICMP(_a_,_C_,_b_,_n_) ( strnicmp(_a_,_b_,_n_) _C_ 0 )
#elif ( defined __unix ) || ( defined _AIX ) || ( defined __linux__ ) || ( defined __hpux )
#define STRICMP(_a_,_C_,_b_) ( strcasecmp(_a_,_b_) _C_ 0 )
#define STRNICMP(_a_,_C_,_b_,_n_) ( strncasecmp(_a_,_b_,_n_) _C_ 0 )
#endif
#endif

#define OFFSET_BUFPTR(_logbuf_,_offset_len_) \
	if( _offset_len_ != -1 ) \
	{ \
	(_logbuf_)->bufptr += _offset_len_ ; \
	(_logbuf_)->buf_remain_len -= _offset_len_ ; \
	}

	_WINDLL_FUNC int SetOpenFlag( LOG *g , char open_flag );
	_WINDLL_FUNC char IsLogOpened( LOG *g );

	_WINDLL_FUNC int GetLogLevel( LOG *g );

	_WINDLL_FUNC LOGBUF *GetLogBuffer( LOG *g );

	_WINDLL_FUNC long FormatLogBuffer( LOG *g , LOGBUF *logbuf , char *format , ... );
	_WINDLL_FUNC long FormatLogBufferV( LOG *g , LOGBUF *logbuf , char *format , va_list valist );
	_WINDLL_FUNC long MemcatLogBuffer( LOG *g , LOGBUF *logbuf , char *append , long len );


	/* ��־�������ṹ */ 
	struct tagLOGBUF
	{
		long			max_buf_size ;
		long			buf_size ;
		char			*bufbase ;
		char			*bufptr ;
		long			buf_remain_len ;
	} ;

	/* ��־����ṹ */ 
	struct tagLOG
	{
		/* ���� */
		int			output ;
		char			log_pathfilename[ MAXLEN_FILENAME + 1 ];
		funcOpenLog		*pfuncOpenLogFirst ;
		funcOpenLog		*pfuncOpenLog ;
		funcWriteLog		*pfuncWriteLog ;
		funcChangeTest		*pfuncChangeTest ;
		funcCloseLog		*pfuncCloseLog ;
		funcCloseLog		*pfuncCloseLogFinally ;
		void			*open_handle ;
		void			*test_handle ;

		int			fd ;
#if ( defined _WIN32 )
		HANDLE			hFile ;
#endif

		char			open_flag ;

		/* ��־�ȼ� */ 
		int			log_level ;

		funcFilterLog		*pfuncFilterLog ;

		/* ����־��� */
		long			log_styles ;
		funcLogStyle		*pfuncLogStyle ;
		/* �����и�ʽ��񣬹�����ʽ�������飬����д��־ʱ�����������־������ */
		funcLogStyle		*pfuncLogStyles[ 30 + 1 ] ;
		int			style_func_count ;

		/* ��־ѡ�� */
		int			log_options ;

		long			file_change_test_interval ;
		long			file_change_test_no ;
		struct STAT		file_change_stat ;
		long			fsync_period ;
		long			fsync_elapse ;

		/* �Զ����ǩ */ 
		char			cust_label[LOG_MAXCNT_CUST_LABEL][ LOG_MAXLEN_CUST_LABEL + 1 ] ;

		/* ��־ת�� */ 
		int			rotate_mode ;
		long			log_rotate_size ;
		long			rotate_file_no ;
		long			rotate_file_count ;
		long			pressure_factor ;

		long			skip_count ;

		funcAfterRotateFile	*pfuncAfterRotateFile ;
		funcBeforeRotateFile	*pfuncBeforeRotateFile ;

		/* ����־������ */
		LOGBUF			logbuf ;


		/* һ������ */ 
		struct timeval	cache1_tv ;
		struct tm		cache1_stime ;

		/* �������� */ 
		struct timeval	cache2_logstyle_tv ;
		struct tm		cache2_logstyle_stime ;
		char			cache2_logstyle_date_buf[ 10 + 1 ] ;
		long			cache2_logstyle_date_buf_len ;
		char			cache2_logstyle_datetime_buf[ 19 + 1 ] ;
		long			cache2_logstyle_datetime_buf_len ;

		unsigned long	cache2_logstyle_pid ;
		char			cache2_logstyle_pid_buf[ 20 + 1 ] ;
		long			cache2_logstyle_pid_buf_len ;

		unsigned long	cache2_logstyle_tid ;
		char			cache2_logstyle_tid_buf[ 20 + 1 ] ;
		long			cache2_logstyle_tid_buf_len ;

		struct timeval	cache2_rotate_tv ;
		struct tm		cache2_rotate_stime ;

		/* ת���ļ��� */
#if ( defined _WIN32 )
		HANDLE			rotate_lock ;
#elif ( defined __unix ) || ( defined _AIX ) || ( defined __linux__ ) || ( defined __hpux )
		int			rotate_lock ;
		struct flock		lock ;
#endif
	} ;

	/********************************************************/
	/* ����Ϊ�����ӿ�										*/
	/********************************************************/

	/* ���ø������� */ 
	_WINDLL_FUNC int ConvertLogOutput_atoi( char *output_desc , int *p_log_output );
	_WINDLL_FUNC int ConvertLogLevel_atoi( char *log_level_desc , int *p_log_level );
	_WINDLL_FUNC int ConvertLogLevel_itoa( int log_level , char **log_level_desc );
	_WINDLL_FUNC int ConvertLogStyle_atol( char *line_style_desc , long *p_line_style );
	_WINDLL_FUNC int ConvertLogOption_atol( char *log_option_desc , long *p_log_option );
	_WINDLL_FUNC int ConvertLogRotateMode_atoi( char *rotate_mode_desc , int *p_rotate_mode );
	_WINDLL_FUNC int ConvertBufferSize_atol( char *bufsize_desc , long *p_bufsize );


/* ��ȡ�����ļ���������־��� */ 
_WINDLL_FUNC LOG *CreateLogHandleFromConfig( char *config_filename , char *postfix );



/* ����滻�� */
#define create_log_handle	CreateLogHandle
#define destroy_log_handle	DestroyLogHandle
#define func_open_log		funcOpenLog
#define func_write_log		funcWriteLog
#define func_change_test	funcChangeTest
#define func_close_log		funcCloseLog
#define set_log_output		SetLogOutput
#define set_log_output2		SetLogOutput2
#define set_log_level		SetLogLevel
#define set_log_styles		SetLogStyles
#define write_log			WriteLog
#define debug_log			DebugLog
#define info_log			InfoLog
#define warn_log			WarnLog
#define error_log			ErrorLog
#define fatal_log			FatalLog
#define create_log_handle_g	CreateLogHandleG
#define destroy_log_handle_g	DestroyLogHandleG
#define set_log_output_g	SetLogOutputG
#define set_log_level_g		SetLogLevelG
#define set_log_styles_g	SetLogStylesG
#define set_log_output2_g	SetLogOutput2G
#define write_log_g			WriteLogG
#define debug_log_g			DebugLogG
#define info_log_g			InfoLogG
#define warn_log_g			WarnLogG
#define error_log_g			ErrorLogG
#define fatal_log_g			FatalLogG
#define convert_log_output_atoi		ConvertLogOutput_atoi
#define convert_log_level_atoi		ConvertLogLevel_atoi
#define convert_log_level_itoa		ConvertLogLevel_itoa
#define convert_log_style_atol		ConvertLogStyle_atol
#define cnovert_log_option_atol		ConvertLogOption_atol
#define convert_log_rotate_mode_atoi	ConvertLogRotateMode_atoi
#define convert_buffer_size_atol	ConvertBufferSize_atol
#define func_filter_log				funcFilterLog
#define func_before_rotate_file		funcBeforeRotateFile
#define func_after_rotate_file		funcAfterRotateFile
#define set_log_options				SetLogOptions
#define set_log_file_change_test	SetLogFileChangeTest
#define set_log_fsync_period		SetLogFsyncPeriod
#define set_log_cust_label			SetLogCustLabel
#define set_log_rotate_mode			SetLogRotateMode
#define set_log_rotate_size			SetLogRotateSize
#define set_log_rotate_pressure_factor	SetLogRotatePressureFactor
#define set_log_rotate_file_count	SetLogRotateFileCount
#define set_before_rotate_file_func	SetBeforeRotateFileFunc
#define set_after_rotate_file_func	SetAfterRotateFileFunc
#define set_filter_log_func			SetFilterLogFunc
#define set_log_buffer_size			SetLogBufferSize
#define set_log_output_func_directly	SetLogOutputFuncDirectly
#define set_log_stlye_func_directly	SetLogStyleFuncDirectly
#define set_log_options_g			SetLogOptionsG
#define set_log_file_change_test_g		SetLogFileChangeTestG
#define set_log_fsync_period_g			SetLogFsyncPeriodG
#define set_log_cust_label_g			SetLogCustLabelG
#define set_log_rotate_mode_g			SetLogRotateModeG
#define set_log_rotate_size_g			SetLogRotateSizeG
#define set_log_rotate_pressure_factor_g	SetLogRotatePressureFactorG
#define set_log_rotate_file_count_g		SetLogRotateFileCountG
#define set_before_rotate_file_func_g		SetBeforeRotateFileFuncG
#define set_after_rotate_file_func_g		SetAfterRotateFileFuncG
#define set_filter_log_func_g			SetFilterLogFuncG
#define set_log_buffer_size_g			SetLogBufferSizeG
#define set_log_output_func_directly_g		SetLogOutputFuncDirectlyG
#define set_log_stlye_func_directly_g		SetLogStyleFuncDirectlyG
#define get_global_log		GetGlobalLOG
#define set_global_log		SetGlobalLOG
#define set_open_flag		SetOpenFlag
#define is_log_opened		IsLogOpened
#define get_log_level		GetLogLevel
#define get_log_buffer		GetLogBuffer
#define format_log_buffer	FormatLogBuffer
#define format_log_buffer_v	FormatLogBufferV
#define memcat_log_buffer	MemcatLogBuffer
#define func_log_style		funcLogStyle
#define create_log_handle_from_config		CreateLogHandleFromConfig

#ifdef __cplusplus
}
#endif

#endif

