/*
 * =====================================================================================
 *
 *       Filename:  sql.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年11月12日 15时49分08秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jianxi sun (jianxi), ycsunjane@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __SQL_H__
#define __SQL_H__
#include <mysql.h>
#include <my_global.h>

#define SQL 		MYSQL
#define DBNAME 		"ac"
#define RESOURCE 	"resource"

#define GETRES "SELECT * FROM resource LIMIT 1"
#define pr_sqlerr()  \
sys_err("Error %u: %s\n", mysql_errno(sql), mysql_error(sql));

extern MYSQL sql;

#define COLMAX 	(128)
struct col_name_t {
	char name[128];
};

struct tbl_dsc_t {
	unsigned int col_num;
	struct col_name_t *head;
};

struct tbl_col_t {
	struct tbl_dsc_t res;
};

int sql_init(SQL *sql);
void sql_query_res(SQL *sql);
#endif /* __SQL_H__ */
