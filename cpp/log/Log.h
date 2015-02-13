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


	/* 日志等级宏 */ 
#define LOG_LEVEL_DEBUG		0	/* 调试等级 */ 
#define LOG_LEVEL_INFO		1	/* 普通信息等级 */ 
#define LOG_LEVEL_WARN		2	/* 警告等级 */ 
#define LOG_LEVEL_ERROR		3	/* 错误等级 */ 
#define LOG_LEVEL_FATAL		4	/* 严重错误等级 */ 
#define LOG_LEVEL_NOLOG		5	/* 不需要输出日志 */ 


	/* 日志输出类型 */ 
#define LOG_OUTPUT_NOSET	-1		/* 不设置 */
#define LOG_OUTPUT_STDOUT	1		/* 标准输出 */ 
#define LOG_OUTPUT_STDERR	2		/* 标准错误输出 */
#define LOG_OUTPUT_SYSLOG	3		/* UNIX&Linux的syslog 或 Windows的WINDOWS EVENT */
#define LOG_OUTPUT_FILE		11		/* 文件 */ 
#define LOG_OUTPUT_CALLBACK	21		/* （使用自定义日志输出回调函数） */
#define LOG_LEVEL_DEFAULT	LOG_LEVEL_NOLOG


	/* 行日志风格组合宏 */ 
#define LOG_STYLE_DATE			1		/* 日期"YYYY-MM-DD" */ 
#define LOG_STYLE_DATETIME		2		/* 日期时间"YYYY-MM-DD hh:mm:ss" */ 
#define LOG_STYLE_DATETIMEMS	4		/* 日期时间毫秒"YYYY-MM-DD hh:mm:ss.6ms"（日期时间类宏互斥，优先自动选用信息量最多的） */
#define LOG_STYLE_LOGLEVEL		8		/* 日志等级 */ 
#define LOG_STYLE_PID			16		/* 进程id */ 
#define LOG_STYLE_TID			32		/* 线程id */ 
#define LOG_STYLE_SOURCE		64		/* "源代码文件名:源代码行号" */ 
#define LOG_STYLE_FORMAT		128		/* 应用日志段 */ 
#define LOG_STYLE_NEWLINE		256		/* 换行 */ 
#define LOG_STYLE_CUSTLABEL1	512		/* 自定义标签1 */
#define LOG_STYLE_CUSTLABEL2	1024	/* 自定义标签2 */ 
#define LOG_STYLE_CUSTLABEL3	2048	/* 自定义标签3 */ 
#define LOG_STYLE_CALLBACK		4096	/* （使用自定义行日志风格回调函数） */ 
#define LOG_STYLES_DEFAULT		0


	/* 函数返回值宏 */ 
#define LOG_RETURN_ERROR_ALLOC		-11		/* 申请内存失败 */ 
#define LOG_RETURN_ERROR_INTERNAL	-12		/* 内部错误 */ 
#define LOG_RETURN_ERROR_ALLOC_MAX	-13		/* 内存使用超限 */
#define LOG_RETURN_ERROR_PARAMETER	-14		/* 参数错误 */ 
#define LOG_RETURN_ERROR_NOTSUPPORT	-17		/* 暂不支持 */ 
#define LOG_RETURN_ERROR_CREATEFILE	-21		/* 创建文件失败 */
#define LOG_RETURN_ERROR_OPENFILE	-22		/* 打开文件失败 */
#define LOG_RETURN_ERROR_WRITEFILE	-23		/* 写文件失败 */ 


#define LOG_NO_STYLEFUNC	NULL
#define LOG_NO_OUTPUTFUNC	NULL , NULL , NULL , NULL , NULL , NULL

#ifndef MAXLEN_FILENAME
#define MAXLEN_FILENAME		256
#endif


	typedef struct tagLOG LOG ;
	typedef struct tagLOGBUF LOGBUF ;


	/* 管理日志句柄函数 */ 
	_WINDLL_FUNC LOG *CreateLogHandle();
	_WINDLL_FUNC void DestroyLogHandle( LOG *g );

	/* 自定义打开、输出、关闭日志函数类型 */ 
	typedef int funcOpenLog( LOG *g , char *log_pathfilename , void **open_handle );
	typedef int funcWriteLog( LOG *g , void **open_handle , int log_level , char *buf , long len , long *writelen );
	typedef int funcChangeTest( LOG *g , void **test_handle );
	typedef int funcCloseLog( LOG *g , void **open_handle );
	typedef int funcLogStyle( LOG *g , LOGBUF *logbuf , char *c_filename , long c_fileline , int log_level , char *format , va_list valist );

	/* 句柄环境设置函数 */
	_WINDLL_FUNC int SetLogOutput( LOG *g , int output , char *log_pathfilename , funcOpenLog *pfuncOpenLogFirst , funcOpenLog *pfuncOpenLog , funcWriteLog *pfuncWriteLog , funcChangeTest *pfuncChangeTest , funcCloseLog *pfuncCloseLog , funcCloseLog *pfuncCloseLogFinally );
	_WINDLL_FUNC int SetLogOutput2( LOG *g , int output , funcOpenLog *pfuncOpenLogFirst , funcOpenLog *pfuncOpenLog , funcWriteLog *pfuncWriteLog , funcChangeTest *pfuncChangeTest , funcCloseLog *pfuncCloseLog , funcCloseLog *pfuncCloseLogFinally , char *log_pathfilename_format , ... );
	_WINDLL_FUNC int SetLogLevel( LOG *g , int log_level );
	_WINDLL_FUNC int SetLogStyles( LOG *g , long log_styles , funcLogStyle *pfuncLogStyle );


	/* 写日志函数 */ /* output log */
	_WINDLL_FUNC int WriteLog( LOG *g , char *c_filename , long c_fileline , int log_level , char *format , ... );
	_WINDLL_FUNC int DebugLog( LOG *g , char *c_filename , long c_fileline , char *format , ... );
	_WINDLL_FUNC int InfoLog( LOG *g , char *c_filename , long c_fileline , char *format , ... );
	_WINDLL_FUNC int WarnLog( LOG *g , char *c_filename , long c_fileline , char *format , ... );
	_WINDLL_FUNC int ErrorLog( LOG *g , char *c_filename , long c_fileline , char *format , ... );
	_WINDLL_FUNC int FatalLog( LOG *g , char *c_filename , long c_fileline , char *format , ... );


	/* 写日志函数的可变参数宏 */ /* output log macros */
#define LOGFAST( _g_ , _log_level_ , ... )	\
	if ((_log_level_) >= _g_->log_level) \
	WriteLog( _g_ , __FILE__ , __LINE__ , _log_level_ , __VA_ARGS__ );

#define WRITELOG( _g_ , _log_level_ , ... )	WriteLog( _g_ , __FILE__ , __LINE__ , _log_level_ , __VA_ARGS__ );
#define DEBUGLOG( _g_ , ... )			DebugLog( _g_ , __FILE__ , __LINE__ , __VA_ARGS__ );
#define INFOLOG( _g_ , ... )			InfoLog( _g_ , __FILE__ , __LINE__ , __VA_ARGS__ );
#define WARNLOG( _g_ , ... )			WarnLog( _g_ , __FILE__ , __LINE__ , __VA_ARGS__ );
#define ERRORLOG( _g_ , ... )			ErrorLog( _g_ , __FILE__ , __LINE__ , __VA_ARGS__ );
#define FATALLOG( _g_ , ... )			FatalLog( _g_ , __FILE__ , __LINE__ , __VA_ARGS__ );


	//线程安全的日志
	_WINDLL_FUNC LOG *CreateLogHandleG();
	_WINDLL_FUNC void DestroyLogHandleG();

	_WINDLL_FUNC int SetLogOutputG( int output , char *log_pathfilename , funcOpenLog *pfuncOpenLogFirst , funcOpenLog *pfuncOpenLog , funcWriteLog *pfuncWriteLog , funcChangeTest *pfuncChangeTest , funcCloseLog *pfuncCloseLog , funcCloseLog *pfuncCloseLogFinally );
	_WINDLL_FUNC int SetLogLevelG( int log_level );
	_WINDLL_FUNC int SetLogStylesG( long log_styles , funcLogStyle *pfuncLogStyles );

#define SetLogOutput2G(_output_,_pfuncOpenLogFirst_,_pfuncOpenLog_,_pfuncWriteLog_,_pfuncChangeTest_,_pfuncCloseLog_,_pfuncCloseLogFinally_,_log_pathfilename_format_,...) \
	SetLogOutput2(_output_,_pfuncOpenLogFirst_,_pfuncOpenLog_,_pfuncWriteLog_,_pfuncChangeTest_,_pfuncCloseLog_,_pfuncCloseLogFinally_,_log_pathfilename_format_,__VA_ARGS__)


	/* 写日志函数（基于线程本地存储的缺省日志句柄的函数集合版本） */ 
	_WINDLL_FUNC int WriteLogG( char *c_filename , long c_fileline , int log_level , char *format , ... );
	_WINDLL_FUNC int DebugLogG( char *c_filename , long c_fileline , char *format , ... );
	_WINDLL_FUNC int InfoLogG( char *c_filename , long c_fileline , char *format , ... );
	_WINDLL_FUNC int WarnLogG( char *c_filename , long c_fileline , char *format , ... );
	_WINDLL_FUNC int ErrorLogG( char *c_filename , long c_fileline , char *format , ... );
	_WINDLL_FUNC int FatalLogG( char *c_filename , long c_fileline , char *format , ... );


	/* 写日志函数的可变参数宏（基于线程本地存储的缺省日志句柄的函数集合版本） */  
#define WRITELOGG( _log_level_ , ... )	WriteLogG( __FILE__ , __LINE__ , _log_level_ , __VA_ARGS__ );
#define DEBUGLOGG( ... )		DebugLogG( __FILE__ , __LINE__ , __VA_ARGS__ );
#define INFOLOGG( ... )			InfoLogG( __FILE__ , __LINE__ , __VA_ARGS__ );
#define WARNLOGG( ... )			WarnLogG( __FILE__ , __LINE__ , __VA_ARGS__ );
#define ERRORLOGG( ... )		ErrorLogG( __FILE__ , __LINE__ , __VA_ARGS__ );
#define FATALLOGG( ... )		FatalLogG( __FILE__ , __LINE__ , __VA_ARGS__ );



	/********************************************************/
	/* 以下为高级特性                                       */
	/********************************************************/

	/* 自定义检查日志等级回调函数类型 */ 
	typedef int funcFilterLog( LOG *g , void **open_handle , int log_level , char *buf , long len );

	/* 自定义日志转档前后回调函数类型 */ 
	typedef int funcBeforeRotateFile( LOG *g , char *rotate_log_pathfilename );
	typedef int funcAfterRotateFile( LOG *g , char *rotate_log_pathfilename );

	/* 自定义标签数量 */ 
#define LOG_MAXCNT_CUST_LABEL		3

	/* 自定义标签最大长度 */ 
#define LOG_MAXLEN_CUST_LABEL		32


	/* 日志选项 */ 
#define LOG_OPTION_OPEN_AND_CLOSE			1	/* 每次都打开日志、写日志、关闭日志 */ 
#define LOG_OPTION_CHANGE_TEST				2	/* 侦测文件变动   默认*/
#define LOG_OPTION_OPEN_ONCE				4	/* 日志打开一次不关闭 */ 
#define LOG_OPTION_SET_OUTPUT_BY_FILENAME	8	/* 自动根据文件名重置输出类型 */

	/* "#stdout" -> LOG_OUTPUT_STDOUT */
	/* "#stderr" -> LOG_OUTPUT_STDERR */
	/* "#syslog" -> LOG_OUTPUT_SYSLOG */
#define LOG_OPTION_FILENAME_APPEND_DOT_LOG	16	/* 日志输出文件名后自动加上".log" */ 

#define LOG_OPTION_OPEN_DEFAULT			LOG_OPTION_CHANGE_TEST


	/* 日志转档模式 */ 
#define LOG_ROTATEMODE_NONE			0	/* 不转档 */ 
#define LOG_ROTATEMODE_SIZE			1	/* 按日志文件大小转档，和函数SetLogRotateSize配合使用；转档文件名格式"原日志文件名.序号" */ 
#define LOG_ROTATEMODE_PER_DAY		2	/* 按每天转档；转档文件名格式"原日志文件名.年年年年月月日日" */
#define LOG_ROTATEMODE_PER_HOUR		3	/* 按小时转档；转档文件名格式"原日志文件名.年年年年月月日日_小时" */ 

	/* 日志转档缺省值 */ 
#define LOG_ROTATE_SIZE_FILE_COUNT_DEFAULT			99999999
#define LOG_ROTATE_SIZE_PRESSURE_FACTOR_DEFAULT		0
#define LOG_FSYNC_PERIOD							10000


	/* 缓冲区大小缺省值 */
#define LOG_BUFSIZE_DEFAULT				(1024)		/* 缺省行日志缓冲区大小 */
#define LOG_BUFSIZE_MAX					(16*1024)	/* 最大行日志缓冲区大小 */


	/* 高级句柄环境设置函数 */
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


	/* 得到基于线程本地存储的缺省日志句柄的函数版本 */ /* log handle get/set function for TLS */
	_WINDLL_FUNC LOG *GetGlobalLOG();
	_WINDLL_FUNC void SetGlobalLOG( LOG *g );


	/********************************************************/
	/* 以下为钩子开发 the following development for the hook*/
	/********************************************************/

	/* 平台宏 */ /* platfrom macros */
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

	/* 代码宏 */ 
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


	/* 日志缓冲区结构 */ 
	struct tagLOGBUF
	{
		long			max_buf_size ;
		long			buf_size ;
		char			*bufbase ;
		char			*bufptr ;
		long			buf_remain_len ;
	} ;

	/* 日志句柄结构 */ 
	struct tagLOG
	{
		/* 基本 */
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

		/* 日志等级 */ 
		int			log_level ;

		funcFilterLog		*pfuncFilterLog ;

		/* 行日志风格 */
		long			log_styles ;
		funcLogStyle		*pfuncLogStyle ;
		/* 分析行格式风格，构建格式函数数组，便于写日志时快速填充行日志缓冲区 */
		funcLogStyle		*pfuncLogStyles[ 30 + 1 ] ;
		int			style_func_count ;

		/* 日志选项 */
		int			log_options ;

		long			file_change_test_interval ;
		long			file_change_test_no ;
		struct STAT		file_change_stat ;
		long			fsync_period ;
		long			fsync_elapse ;

		/* 自定义标签 */ 
		char			cust_label[LOG_MAXCNT_CUST_LABEL][ LOG_MAXLEN_CUST_LABEL + 1 ] ;

		/* 日志转档 */ 
		int			rotate_mode ;
		long			log_rotate_size ;
		long			rotate_file_no ;
		long			rotate_file_count ;
		long			pressure_factor ;

		long			skip_count ;

		funcAfterRotateFile	*pfuncAfterRotateFile ;
		funcBeforeRotateFile	*pfuncBeforeRotateFile ;

		/* 行日志缓冲区 */
		LOGBUF			logbuf ;


		/* 一级缓存 */ 
		struct timeval	cache1_tv ;
		struct tm		cache1_stime ;

		/* 二级缓存 */ 
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

		/* 转档文件锁 */
#if ( defined _WIN32 )
		HANDLE			rotate_lock ;
#elif ( defined __unix ) || ( defined _AIX ) || ( defined __linux__ ) || ( defined __hpux )
		int			rotate_lock ;
		struct flock		lock ;
#endif
	} ;

	/********************************************************/
	/* 以下为辅助接口										*/
	/********************************************************/

	/* 配置辅助函数 */ 
	_WINDLL_FUNC int ConvertLogOutput_atoi( char *output_desc , int *p_log_output );
	_WINDLL_FUNC int ConvertLogLevel_atoi( char *log_level_desc , int *p_log_level );
	_WINDLL_FUNC int ConvertLogLevel_itoa( int log_level , char **log_level_desc );
	_WINDLL_FUNC int ConvertLogStyle_atol( char *line_style_desc , long *p_line_style );
	_WINDLL_FUNC int ConvertLogOption_atol( char *log_option_desc , long *p_log_option );
	_WINDLL_FUNC int ConvertLogRotateMode_atoi( char *rotate_mode_desc , int *p_rotate_mode );
	_WINDLL_FUNC int ConvertBufferSize_atol( char *bufsize_desc , long *p_bufsize );


/* 读取配置文件，创建日志句柄 */ 
_WINDLL_FUNC LOG *CreateLogHandleFromConfig( char *config_filename , char *postfix );



/* 风格替换宏 */
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

