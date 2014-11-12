/*
 * ============================================================================
 *
 *       Filename:  sql.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年11月12日 15时49分05秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jianxi sun (jianxi), ycsunjane@gmail.com
 *   Organization:  
 *
 * ============================================================================
 */
#include <stdio.h>
#include <stdint.h>
#include <mysqld_error.h>
#include "sql.h"
#include "log.h"

MYSQL sql;
struct tbl_col_t tables;

int sql_query(SQL *sql, char *cmd, MYSQL_RES **res, my_ulonglong *rows)
{
	int ret;
	ret = mysql_query(sql, cmd);
	if(ret) {
		pr_sqlerr();
		return -1;
	}

	*res = mysql_store_result(sql);
	if(!res) {
		pr_sqlerr();
		return -1;
	}

	my_ulonglong row = mysql_num_rows(*res);
	if(row <= 0) {
		sys_warn("Have no resource in database\n");
		mysql_free_result(*res);
		return -1;
	}
	if(rows) *rows = row;

	return 0;
}

void sql_query_res(SQL *sql)
{
	MYSQL_RES *res;
	if(sql_query(sql, GETRES, &res, NULL) < 0)
		return;

	int i;
	MYSQL_ROW row;
	unsigned long *lengths;
	struct col_name_t *col = tables.res.head;

	while((row = mysql_fetch_row(res))) {
		lengths = mysql_fetch_lengths(res);

		for(i = 0; i < tables.res.col_num; i++) {
			printf("len:%lu, %s:%s\n", 
				lengths[i], 
				col[i].name, 
				row[i]);
		}
	}
}

void sql_close(SQL *sql)
{
	mysql_close(sql);
}

void _sql_tbl_col(SQL *sql, char *table, struct tbl_dsc_t *dsc)
{
	MYSQL_RES *res;
	res = mysql_list_fields(sql, table, NULL);
	if(!res) {
		pr_sqlerr();
		exit(-1);
	}
	dsc->col_num = mysql_num_fields(res);
     	dsc->head = 
		malloc(sizeof(struct col_name_t) * dsc->col_num);
	if(!dsc->head) {
		sys_err("Malloc failed: %s(%d)\n", 
			strerror(errno), errno);
		exit(-1);
	}

	int i;
	MYSQL_FIELD *col;
	for(i = 0; i < dsc->col_num; i++) {
		col = mysql_fetch_field_direct(res, i);
		assert(strlen(col->name) < (COLMAX - 1));
		strcpy(dsc->head[i].name, col->name);
	}

	mysql_free_result(res);
}

void sql_tbl_col(SQL *sql)
{
	_sql_tbl_col(sql, RESOURCE, &tables.res);
}

int sql_init(SQL *sql)
{
	int ret;
	void *state;

	printf("Mysql version:%s\n", mysql_get_client_info());

	sql = mysql_init(sql);
	if(sql == NULL) {
		pr_sqlerr();
		exit(-1);
	}

	mysql_options(sql, MYSQL_READ_DEFAULT_GROUP, "acctl");
	state = mysql_real_connect(sql, NULL, NULL, 
		NULL, NULL, 0, NULL, 0);
	if(state == NULL) {
		pr_sqlerr();
		exit(-1);
	}
	
	ret = mysql_select_db(sql, DBNAME);
	if(ret) {
		pr_sqlerr();
		exit(-1);
	}

	sql_tbl_col(sql);
	return 0;
}

