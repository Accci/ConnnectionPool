#pragma once
#include "public.h"
#include <ctime>

class Connection
{
public:
	Connection();
	~Connection();
	bool connect(string ip,
		unsigned short port,
		string username,
		string password,
		string dbname);
	bool update(string sql);
	MYSQL_RES* query(string sql);

	//刷新一下连接的起始的空闲时间点
	void refreshAlivetime() { _alivetime = clock(); }

	//返回存活时间
	clock_t getAlivetime() const{ return clock() - _alivetime; }
private:
	MYSQL* _conn;
	clock_t _alivetime; //记录进入空闲状态后的起始存活时间
};

