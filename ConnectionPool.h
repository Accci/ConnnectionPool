#pragma once
#include "public.h"
#include "Connection.h"
#include <atomic>
#include <memory>
#include <condition_variable>
#include <thread>
#include <functional>
class ConnectionPool
{
public:
	static ConnectionPool* getConnectionPool();
	shared_ptr<Connection> getConnection();
private:
	//单列 构造函数私有化
	ConnectionPool();

	//从配置文件中加载配置项
	bool loadConfigFile();

	//运行在独立的线程中，专门负责生成新连接
	void produceConnectionTask();

	//扫描超过maxIdleTime的空闲连接,进行多余的连接回收
	void scannerConnectionTask();

	string _ip;				//mysql的ip地址
	unsigned short _port;	//mysql的端口号
	string _username;		//mysql登录名
	string _password;		//mysql登录密码
	string _dbname;
	int _initSize;			//连接池的初始连接量
	int _maxSize;			//连接池的最大连接量
	int _maxIdleTime;		//连接池的最大空闲时间
	int _connectionTimeout;	//连接池获取连接的超时时间

	queue<Connection*> _connectionQue;	//存储mysql连接的队列

	mutex _queueMutex;		//维护连接队列线程安全的互斥锁

	atomic_int _connectionCnt;	//记录连接锁创建的connection连接的总数量

	condition_variable cv;		//设置条件变量，用于连接生产线程和消费线程的通信


}; 

