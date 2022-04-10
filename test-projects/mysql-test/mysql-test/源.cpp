#include<mysql.h>
#include<string.h>
#include<stdio.h>

int main(void)
{
	MYSQL* con = mysql_init((MYSQL*)0);
	MYSQL_RES* res;
	MYSQL_ROW row;
	mysql_real_connect(con, "152.136.253.115", "root", "000815", "voip_groups", 3306, NULL, 0);
	mysql_select_db(con, "voip_groups");
	char query1[] = "select ip from registed";
	char query2[] = "select port from registed";
	mysql_real_query(con, query1, strlen(query1));
	res = mysql_store_result(con);
	int len = mysql_num_fields(res);
	while (row = mysql_fetch_row(res))
	{
		for (int i = 0; i < len; ++i)
			printf("%s\n", row[i]);
	}
	mysql_real_query(con, query2, strlen(query2));
	res = mysql_store_result(con);
	len = mysql_num_fields(res);
	while (row = mysql_fetch_row(res))
	{
		for (int i = 0; i < len; ++i)
			printf("%s\n", row[i]);
	}
	mysql_close(con);
	return 0;
}
