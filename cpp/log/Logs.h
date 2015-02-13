#ifndef _H_LOGS_
#define _H_LOGS_

/*
* iLOG3 - log function library written in c
* author	: calvin
* email	: calvinwilliams.c@gmail.com
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

#include "LOG.h"

	/* 函数返回值宏 */ 
#define LOGS_RETURN_ERROR_TOOMANY_LOG	-31	/* 太多日志句柄了 */ 
#define LOGS_RETURN_ERROR_NOTFOUND		-32	/* 没有该日志句柄 */ 
#define LOGS_RETURN_INFO_NOTFOUND		32	/* 没有该日志句柄了 */ 

	/* 其它宏 */ 
#define LOGS_MAXCNT_LOG			10	/* 日志句柄集合中最大日志句柄数量 */ 

	typedef struct tagLOGS LOGS ;

	/* 管理日志句柄集合函数 */ 
	_WINDLL_FUNC LOGS *CreateLogsHandle();
	_WINDLL_FUNC void DestroyLogsHandle( LOGS *gs );

	_WINDLL_FUNC int AddLogToLogs( LOGS *gs , char *g_id , LOG *g );
	_WINDLL_FUNC LOG *RemoveOutLogFromLogs( LOGS *gs , char *g_id );
	_WINDLL_FUNC LOG *GetLogFromLogs( LOGS *gs , char *g_id );
	_WINDLL_FUNC int TravelLogFromLogs( LOGS *gs , long *p_index , char **pp_g_id , LOG **pp_g );

	/* 写日志集合函数 */ 
	_WINDLL_FUNC int WriteLogs( LOGS *g , char *c_filename , long c_fileline , int log_level , char *format , ... );
	_WINDLL_FUNC int DebugLogs( LOGS *g , char *c_filename , long c_fileline , char *format , ... );
	_WINDLL_FUNC int InfoLogs( LOGS *g , char *c_filename , long c_fileline , char *format , ... );
	_WINDLL_FUNC int WarnLogs( LOGS *g , char *c_filename , long c_fileline , char *format , ... );
	_WINDLL_FUNC int ErrorLogs( LOGS *g , char *c_filename , long c_fileline , char *format , ... );
	_WINDLL_FUNC int FatalLogs( LOGS *g , char *c_filename , long c_fileline , char *format , ... );


	/* 写日志集合函数的可变参数宏 */ 
#define WRITELOGS( _g_ , _log_level_ , ... )	WriteLogs( _g_ , __FILE__ , __LINE__ , _log_level_ , __VA_ARGS__ );
#define DEBUGLOGS( _g_ , ... )			DebugLogs( _g_ , __FILE__ , __LINE__ , __VA_ARGS__ );
#define INFOLOGS( _g_ , ... )			InfoLogs( _g_ , __FILE__ , __LINE__ , __VA_ARGS__ );
#define WARNLOGS( _g_ , ... )			WarnLogs( _g_ , __FILE__ , __LINE__ , __VA_ARGS__ );
#define ERRORLOGS( _g_ , ... )			ErrorLogs( _g_ , __FILE__ , __LINE__ , __VA_ARGS__ );
#define FATALLOGS( _g_ , ... )			FatalLogs( _g_ , __FILE__ , __LINE__ , __VA_ARGS__ );


	/* 风格替换宏 */
#define create_logs_handle		CreateLogsHandle
#define destroy_logs_handle		DestroyLogsHandle

#define add_log_to_logs			AddLogToLogs
#define remove_out_log_from_logs	RemoveOutLogFromLogs
#define get_log_from_logs		GetLogFromLogs
#define travel_log_from_logs		TravelLogFromLogs

#define write_logs			WriteLogs
#define debug_logs			DebugLogs
#define info_logs			InfoLogs
#define warn_logs			WarnLogs
#define error_logs			ErrorLogs
#define fatal_logs			FatalLogs

#define LOG_TRAVELLOG_INDEX_INIT	-1



	_WINDLL_FUNC LOGS *CreateLogsHandleG();
	_WINDLL_FUNC void DestroyLogsHandleG();

	_WINDLL_FUNC int AddLogToLogsG( char *g_id , LOG *g );
	_WINDLL_FUNC LOG *RemoveOutLogFromLogsG( char *g_id );
	_WINDLL_FUNC LOG *GetLogFromLogsG( char *g_id );
	_WINDLL_FUNC int TravelLogFromLogsG( long *p_index , char **pp_g_id , LOG **pp_g );

	/* 写日志集合函数（基于线程本地存储的缺省日志句柄的函数集合版本） */ 
	_WINDLL_FUNC int WriteLogsG( char *c_filename , long c_fileline , int log_level , char *format , ... );
	_WINDLL_FUNC int DebugLogsG( char *c_filename , long c_fileline , char *format , ... );
	_WINDLL_FUNC int InfoLogsG( char *c_filename , long c_fileline , char *format , ... );
	_WINDLL_FUNC int WarnLogsG( char *c_filename , long c_fileline , char *format , ... );
	_WINDLL_FUNC int ErrorLogsG( char *c_filename , long c_fileline , char *format , ... );
	_WINDLL_FUNC int FatalLogsG( char *c_filename , long c_fileline , char *format , ... );


	/* 写日志集合函数的可变参数宏（基于线程本地存储的缺省日志句柄的函数集合版本） */
#define WRITELOGSG( _log_level_ , ... )	WriteLogsG( __FILE__ , __LINE__ , _log_level_ , __VA_ARGS__ );
#define DEBUGLOGSG( ... )		DebugLogsG( __FILE__ , __LINE__ , __VA_ARGS__ );
#define INFOLOGSG( ... )		InfoLogsG( __FILE__ , __LINE__ , __VA_ARGS__ );
#define WARNLOGSG( ... )		WarnLogsG( __FILE__ , __LINE__ , __VA_ARGS__ );
#define ERRORLOGSG( ... )		ErrorLogsG( __FILE__ , __LINE__ , __VA_ARGS__ );
#define FATALLOGSG( ... )		FatalLogsG( __FILE__ , __LINE__ , __VA_ARGS__ );


	/* 风格替换宏 */
#define create_logs_handle_g		CreateLogsHandleG
#define destroy_logs_handle_g		DestroyLogsHandleG

#define add_log_to_logs_g			AddLogToLogsG
#define remove_out_log_from_logs_g	RemoveOutLogFromLogsG
#define get_log_from_logs_g			GetLogFromLogsG
#define travel_log_from_logs_g		TravelLogFromLogsG

#define write_logs_g			WriteLogsG
#define debug_logs_g			DebugLogsG
#define info_logs_g				InfoLogsG
#define warn_logs_g				WarnLogsG
#define error_logs_g			ErrorLogsG
#define fatal_logs_g			FatalLogsG


	/* 得到基于线程本地存储的缺省日志句柄集合的函数版本 */ 
	_WINDLL_FUNC LOGS *GetGlobalLOGS();
	_WINDLL_FUNC void SetGlobalLOGS( LOGS *gs );
#define get_global_logs		GetGlobalLOGS
#define set_global_logs		SetGlobalLOGS


	/* 读取配置文件，创建日志句柄集合 */ 
	_WINDLL_FUNC LOGS *CreateLogsHandleFromConfig( char *config_filename , char *postfix );
#define create_logs_handle_from_config		CreateLogsHandleFromConfig

#ifdef __cplusplus
}
#endif

#endif
