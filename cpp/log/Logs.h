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
#define LOGS_MAXCNT_LOG			        10	/* 日志句柄集合中最大日志句柄数量 */ 

	typedef struct tagLOGS LOGS;

	/* 管理日志句柄集合函数 */
	_WINDLL_FUNC LOGS *CreateLogsHandle();
	_WINDLL_FUNC void DestroyLogsHandle(LOGS *gs);

	_WINDLL_FUNC int AddLogToLogs(LOGS *gs, char *g_id, LOG *g);
	_WINDLL_FUNC LOG *RemoveOutLogFromLogs(LOGS *gs, char *g_id);
	_WINDLL_FUNC LOG *GetLogFromLogs(LOGS *gs, char *g_id);
	_WINDLL_FUNC int TravelLogFromLogs(LOGS *gs, long *p_index, char **pp_g_id, LOG **pp_g);

	/* 写日志集合函数 */
	_WINDLL_FUNC int WriteLogs(LOGS *g, char *c_filename, long c_fileline, int log_level, char *format, ...);
	_WINDLL_FUNC int DebugLogs(LOGS *g, char *c_filename, long c_fileline, char *format, ...);
	_WINDLL_FUNC int InfoLogs(LOGS *g, char *c_filename, long c_fileline, char *format, ...);
	_WINDLL_FUNC int WarnLogs(LOGS *g, char *c_filename, long c_fileline, char *format, ...);
	_WINDLL_FUNC int ErrorLogs(LOGS *g, char *c_filename, long c_fileline, char *format, ...);
	_WINDLL_FUNC int FatalLogs(LOGS *g, char *c_filename, long c_fileline, char *format, ...);


	/* 写日志集合函数的可变参数宏 */
#define WRITELOGS( _g_ , _log_level_ , ... )	WriteLogs( _g_ , __FILE__ , __LINE__ , _log_level_ , __VA_ARGS__ );
#define DEBUGLOGS( _g_ , ... )			DebugLogs( _g_ , __FILE__ , __LINE__ , __VA_ARGS__ );
#define INFOLOGS( _g_ , ... )			InfoLogs( _g_ , __FILE__ , __LINE__ , __VA_ARGS__ );
#define WARNLOGS( _g_ , ... )			WarnLogs( _g_ , __FILE__ , __LINE__ , __VA_ARGS__ );
#define ERRORLOGS( _g_ , ... )			ErrorLogs( _g_ , __FILE__ , __LINE__ , __VA_ARGS__ );
#define FATALLOGS( _g_ , ... )			FatalLogs( _g_ , __FILE__ , __LINE__ , __VA_ARGS__ );

#define LOG_TRAVELLOG_INDEX_INIT	-1

	/* 读取配置文件，创建日志句柄集合 */
	_WINDLL_FUNC LOGS *CreateLogsHandleFromConfig(char *config_filename, char *postfix);

	/* 风格替换宏 */
#define create_logs_handle		CreateLogsHandle
#define destroy_logs_handle		DestroyLogsHandle
#define add_log_to_logs				AddLogToLogs
#define remove_out_log_from_logs	RemoveOutLogFromLogs
#define get_log_from_logs			GetLogFromLogs
#define travel_log_from_logs		TravelLogFromLogs
#define write_logs			WriteLogs
#define debug_logs			DebugLogs
#define info_logs			InfoLogs
#define warn_logs			WarnLogs
#define error_logs			ErrorLogs
#define fatal_logs			FatalLogs
#define create_logs_handle_from_config		CreateLogsHandleFromConfig

#ifdef __cplusplus
}
#endif

#endif
