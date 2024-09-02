#include "Connection.h"
#include "ConnectionPool.h"

int main()
{
	//Connection conn;
	//char sql[1024];
	//sprintf(sql, "insert into user(name, age, sex) values ('%s','%d', '%s')",
	//	"zhang san", 20, "male");
	//conn.connect("127.0.0.1", 3306, "root", "Zqywq815.123", "chat");
	//conn.update(sql);
	//return 0;

	//ConnectionPool* cp = ConnectionPool::getConnectionPool();
	//cp->loadConfigFile();
	clock_t begin = clock();
	ConnectionPool* cp = ConnectionPool::getConnectionPool();
	for (int i = 0; i < 5000; ++i)
	{
		/*Connection conn;
		char sql[1024];
		sprintf(sql, "insert into user(name, age, sex) values ('%s','%d', '%s')",
			"zhang san", 20, "male");
		conn.connect("127.0.0.1", 3306, "root", "Zqywq815.123", "chat");
		conn.update(sql);*/
		char sql[1024];
		sprintf(sql, "insert into user(name, age, sex) values ('%s','%d', '%s')",
			"zhang san", 20x, "male");
		shared_ptr<Connection>sp = cp->getConnection();
		sp->update(sql);
	}
	clock_t end = clock();

	cout << (end - begin) << "ms" << endl;
}