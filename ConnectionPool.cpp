#include "ConnectionPool.h"


//线程安全的懒汉单例子
ConnectionPool* ConnectionPool::getConnectionPool()
{
	static ConnectionPool pool;
	return &pool;
}

shared_ptr<Connection>  ConnectionPool::getConnection()
{
	unique_lock<mutex> lock(_queueMutex);
	while (_connectionQue.empty())
	{
		if (cv_status::timeout == cv.wait_for(lock, chrono::milliseconds(_connectionTimeout)))
		{
			if (_connectionQue.empty())
			{
				LOG("获取空闲连接超时了... 获取连接失败");
				return nullptr;
			}
		}
		
	}
	//shared_ptr 智能指针在析构是，会把 connection 资源直接delete掉，
	//因此需要自定义shared_ptr资源释放的方式， 吧connection 直接归还到队列当中
	shared_ptr<Connection> sp(_connectionQue.front(), [&](Connection* pcon) {
		//这里是在服务器应用线程中调用的，所以一定要考虑队列的线程安全操作
		unique_lock<mutex> lock(_queueMutex);
		pcon->refreshAlivetime();
		_connectionQue.push(pcon);
		});

	_connectionQue.pop();
	/*if (_connectionQue.empty())
	{
		cv.notify_all(); //队列最后一个connection 通知生产者线程
	}*/
	//也可以不加
	cv.notify_all();  //生产者当队列不空时就等待
	return sp;
}

ConnectionPool::ConnectionPool()
{
	if (!loadConfigFile())
	{
		return;
	}

	//创建初始连接
	for (int i = 0; i < _initSize; ++i)
	{
		Connection* p = new Connection();
		p->connect(_ip, _port, _username, _password, _dbname);
		p->refreshAlivetime(); 
		_connectionQue.push(p);
		_connectionCnt++;
	}

	//启动一个新的线程作为连接的生产者
	thread produce(std::bind(&ConnectionPool::produceConnectionTask, this) );
	produce.detach();

	//启动一个新的定时线程，扫描超过maxIdleTime的空闲连接，进行多余的连接回收
	thread scanner(std::bind(&ConnectionPool::scannerConnectionTask, this));
	scanner.detach();
}

bool ConnectionPool::loadConfigFile()
{
	FILE* pf = fopen("mysql.ini", "r");
	if (pf == nullptr)
	{
		LOG("mysql.ini is not exist");
		return false;
	}

	while (!feof(pf))
	{
		char line[1024] = { 0 };
		fgets(line, 1024, pf);
		string str = line;
		int idx = str.find('=', 0);
		if (idx == -1)
		{
			continue;
		}
		int endidx = str.find('\n', idx);
		string key = str.substr(0, idx);
		string value = str.substr(idx + 1, endidx - idx - 1);

		if (key == "ip")
		{
			_ip = value;
		}
		else if (key == "port")
		{
			_port = atoi(value.c_str());
		}
		else if (key == "username")
		{
			_username = value;
		}
		else if (key == "password")
		{
			_password = value;
		}
		else if (key == "dbname")
		{
			_dbname = value;
		}
		else if (key == "initSize")
		{
			_initSize = atoi(value.c_str());
		}
		else if (key == "maxSize")
		{
			_maxSize = atoi(value.c_str());
		}
		else if (key == "maxIdleTime")
		{
			_maxIdleTime = atoi(value.c_str());
		}
		else if (key == "connectionTimeout")
		{
			_connectionTimeout = atoi(value.c_str());
		}
	}
}

void ConnectionPool::produceConnectionTask()
{
	while (true)
	{
		unique_lock<mutex> lock(_queueMutex);
		while (!_connectionQue.empty())
		{
			cv.wait(lock);  //队列不空，生产线程进入等待状态
		}

		//连接数量没有达到上限
		if (_connectionCnt < _maxSize)
		{
			Connection* p = new Connection();
			p->connect(_ip, _port, _username, _password, _dbname);
			p->refreshAlivetime();
			_connectionQue.push(p);
			_connectionCnt++;
		}

		//通知消费者线程可以连接
		cv.notify_all();
	}
}

void ConnectionPool::scannerConnectionTask()
{
	for (;;)
	{
		//通过sleep 模拟定时效果
		this_thread::sleep_for(chrono::seconds(_maxIdleTime));

		//扫描整个队列，释放多余的连接
		unique_lock<mutex> lock(_queueMutex);
		while (_connectionCnt > _initSize)
		{
			Connection* p = _connectionQue.front();  
			if (p->getAlivetime() >= (_maxIdleTime * 1000))
			{
				_connectionQue.pop();
				_connectionCnt--;
				delete p;
			}
			else   //队头都没有超过，等下一轮
			{
				break;
			}
			
		}
	}
}
