#include"main.h"
#include"TdSpi.h"
#include"Strategy.h"
#include<mutex>
#include<iostream>
#include<fstream>




using namespace std;


std::map<std::string, std::string> accountConfig_map;//±£´æÕË»§ÐÅÏ¢µÄmap

Strategy* g_strategy;//²ßÂÔÀàÖ¸Õë

TdSpi* g_pUserTDSpi_AI;//È«¾ÖµÄTD»Øµ÷´¦ÀíÀà¶ÔÏó£¬AI½»»¥º¯Êý»áÓÃµ½

void  AIThread();//AIÏß³Ìº¯Êý


//È«¾ÖµÄ»¥³âËø
std::mutex m_mutex;

void ReadConfigMap(map<std::string, std::string> &accountmap);

int main()

{
	cerr << "---------------------------------------------" << endl;
	cerr << "---------------------------------------------" << endl;
	cerr << "------------QD¸ßÆµ½»Ò×ÏµÍ³Æô¶¯-----------" << endl;
	cerr << "---------------------------------------------" << endl;
	cerr << "---------------------------------------------" << endl;
	
	
	//-----------------1¡¢¶ÁÈ¡ÕË»§ÐÅÏ¢-------------------
	ReadConfigMap(accountConfig_map);

	//-----------------2¡¢´´½¨ÐÐÇéapiºÍ»Øµ÷ÀàÊµÀý------------------------
	CThostFtdcMdApi* pUserApi_market = CThostFtdcMdApi::CreateFtdcMdApi("./Temp/Marketflow/");
	MdSpi* pUserSpi_market = new MdSpi(pUserApi_market);
	pUserApi_market->RegisterSpi(pUserSpi_market);

	char mdFront[50];
	strcpy(mdFront, accountConfig_map["MarketFront"].c_str());
	pUserApi_market->RegisterFront(mdFront);


	//-----------------3¡¢´´½¨½»Ò×apiºÍ»Øµ÷ÀàÊµÀý------------------------
	CThostFtdcTraderApi* pUserApi_trade = CThostFtdcTraderApi::CreateFtdcTraderApi("./Temp/Tradeflow/");
	TdSpi * pUserSpi_trade = new TdSpi(pUserApi_trade, pUserApi_market, pUserSpi_market);
	pUserApi_trade->RegisterSpi(pUserSpi_trade);//api×¢²á»Øµ÷Àà

	pUserApi_trade->SubscribePublicTopic(THOST_TERT_RESTART);//¶©ÔÄ¹«ÓÐÁ÷
	pUserApi_trade->SubscribePrivateTopic(THOST_TERT_QUICK);//¶©ÔÄË½ÓÐÁ÷


	char tdFront[50];
	strcpy(tdFront, accountConfig_map["TradeFront"].c_str());
	pUserApi_trade->RegisterFront(tdFront);

	//-----------------4¡¢´´½¨²ßÂÔÀàÊµÀý-----------------------
	
	g_strategy = new Strategy(pUserSpi_trade);


	//-----------------5¡¢Æô¶¯½»Ò×Ïß³Ì-----------------------
	pUserApi_trade->Init();//Æô¶¯Ïß³Ì


	//-----------------6¡¢´´½¨AIÏß³Ì-----------------------
std:thread th1(AIThread);



	//-----------------7¡¢µÈ´ýÏß³ÌÍË³ö-----------------------
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
		cout << "ÅäÖÃÎÄ¼þ²»´æÔÚ" << endl;
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
			}//for½áÊø
			accountConfig_map.insert(make_pair(fieldKey, fieldValve));


		}
	}


	file1.close();
}
