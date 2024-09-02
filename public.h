#pragma once
#include <string>
#include <iostream>
#include "mysql.h"
#include <queue>
#include <mutex>
using namespace std;
#define LOG(str) \
	cout << __FILE__ << ":"<< __LINE__ << " " << \
	 __TIMESTAMP__ << " : " << str << endl