#include"main.h"
#include"TdSpi.h"
#include"Strategy.h"
#include<mutex>
#include<iostream>
#include<fstream>




using namespace std;


std::map<std::string, std::string> accountConfig_map;//保存账户信息的map

Strategy* g_strategy;//策略类指针

TdSpi* g_pUserTDSpi_AI;//全局的TD回调处理类对象，AI交互函数会用到

int g_nRequestID=0;

void  AIThread();//AI线程函数


//全局的互斥锁
std::mutex m_mutex;

void ReadConfigMap(map<std::string, std::string> &accountmap);

int main()

{
	cerr << "---------------------------------------------" << endl;
	cerr << "---------------------------------------------" << endl;
	cerr << "------------QD高频交易系统启动-----------" << endl;
	cerr << "---------------------------------------------" << endl;
	cerr << "---------------------------------------------" << endl;
	
	
	//-----------------1、读取账户信息-------------------
	ReadConfigMap(accountConfig_map);

	//-----------------2、创建行情api和回调类实例------------------------
	CThostFtdcMdApi* pUserApi_market = CThostFtdcMdApi::CreateFtdcMdApi("./Temp/Marketflow/");
	MdSpi* pUserSpi_market = new MdSpi(pUserApi_market);
	pUserApi_market->RegisterSpi(pUserSpi_market);

	char mdFront[50];
	strcpy(mdFront, accountConfig_map["MarketFront"].c_str());
	pUserApi_market->RegisterFront(mdFront);


	//-----------------3、创建交易api和回调类实例------------------------
	CThostFtdcTraderApi* pUserApi_trade = CThostFtdcTraderApi::CreateFtdcTraderApi("./Temp/Tradeflow/");
	TdSpi * pUserSpi_trade = new TdSpi(pUserApi_trade, pUserApi_market, pUserSpi_market);
	pUserApi_trade->RegisterSpi(pUserSpi_trade);//api注册回调类

	pUserApi_trade->SubscribePublicTopic(THOST_TERT_RESTART);//订阅公有流
	pUserApi_trade->SubscribePrivateTopic(THOST_TERT_QUICK);//订阅私有流


	char tdFront[50];
	strcpy(tdFront, accountConfig_map["TradeFront"].c_str());
	pUserApi_trade->RegisterFront(tdFront);

	//-----------------4、创建策略类实例-----------------------
	
	g_strategy = new Strategy(pUserSpi_trade);


	//-----------------5、启动交易线程-----------------------
	pUserApi_trade->Init();//启动线程


	//-----------------6、创建AI线程-----------------------
std:thread th1(AIThread);



	//-----------------7、等待线程退出-----------------------
	pUserApi_market->Join();
	pUserApi_trade->Join();
	th1.join();


	pUserApi_market->Release();
	pUserApi_trade->Release();
	return 0;
}

void AIThread()
{


}

void ReadConfigMap(map<std::string, std::string>& accountmap)
{
	std::ifstream file1(".\\config\\config.txt", ios::in);
	string fieldKey;
	string fieldValve;
	char dataLine[256];
	if (!file1)
	{
		cout << "配置文件不存在" << endl;
		return ;
	}
	else
	{
		while (file1.getline(dataLine, sizeof(dataLine), '\n'))
		{
			int length = strlen(dataLine);
			char tmp[128];
			for (int i = 0, j = 0, count = 0; i < length + 1; i++)
			{
				if (dataLine[i] != ',' && dataLine[i] != '\0')
					tmp[j++] = dataLine[i];
				else
				{
					//appid, simnow_client_test
					//authcode, 0000000000000000
					tmp[j] = '\0';

					count++;
					//cout << "count: " << count << ",tmp :" << tmp << endl;
					j = 0;
					switch (count)
					{
					case 1:
						fieldKey = tmp;
						break;
					case 2:
						fieldValve = tmp;
					default:
						break;
					}
				}
			}//for结束
			accountConfig_map.insert(make_pair(fieldKey, fieldValve));


		}
	}


	file1.close();
}
