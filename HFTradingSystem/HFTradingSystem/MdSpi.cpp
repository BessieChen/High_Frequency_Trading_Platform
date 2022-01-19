#include"MdSpi.h"
#include<iostream>
#include<mutex>
#include<vector>


extern std::map<std::string, std::string> accountConfig_map;//保存账户信息的map


//全局的互斥锁
extern std::mutex m_mutex;

//全局的requestId
extern int g_nRequestID;

//全局的持仓合约
extern std::vector<std::string> subscribe_inst_vec;


using namespace std;

MdSpi::MdSpi(CThostFtdcMdApi* mdapi):mdapi(mdapi)
{
	//期货公司代码
	m_BrokerId = accountConfig_map["brokerId"];
	//期货账户
	m_UserId = accountConfig_map["userId"];
	//密码
	m_Passwd = accountConfig_map["passwd"];
	//策略需要交易的合约
	memset(m_InstId, 0, sizeof(m_InstId));
	strcpy(m_InstId, accountConfig_map["contract"].c_str());

}

MdSpi::~MdSpi()
{
	if(m_InstIdList_all)
	delete[] m_InstIdList_all;

	if (m_InstIdList_Position_MD)
	delete[] m_InstIdList_Position_MD;

	if (loginField)
		delete loginField;
	

}

void MdSpi::OnFrontConnected()
{
	cerr << "行情前置已连接上，请求登录" << endl;

	ReqUserLogin(m_BrokerId, m_UserId, m_Passwd);
	
}

void MdSpi::ReqUserLogin(std::string brokerId, std::string userId, std::string passwd)
{
	loginField = new CThostFtdcReqUserLoginField();
	//strcpy(loginField->BrokerID, brokerId.c_str());
	/*strcpy(loginField->UserID, userId.c_str());
	strcpy(loginField->Password, passwd.c_str());*/
	//错误的brokerid
	strcpy(loginField->BrokerID, "0000");
	strcpy(loginField->UserID, "");
	strcpy(loginField->Password, "");
	int nResult = mdapi->ReqUserLogin(loginField, GetNextRequestID());
	cerr << "请求登录行情：" << ((nResult == 0) ? "成功" : "失败") << endl;
}

void MdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "OnRspUserLogin 行情登陆成功! " << endl;;
	cerr << "交易日：" << mdapi->GetTradingDay() << endl;
	if (pRspInfo->ErrorID == 0) {
		cerr << "请求的登陆成功," << "请求ID为" << nRequestID << endl;
		/***************************************************************/
		cerr << "尝试订阅行情" << endl;

		////插入合约到订阅列表
		InsertInstToSubVec(m_InstId);
		
		//订阅行情
		//if (subscribe_inst_vec.size() > 0)
		//{
		//	//"IF2012,IF2101,IF2103"
		//	cerr << "订阅行情的合约数量：" << subscribe_inst_vec.size() << endl;
		//	cerr << "有持仓，订阅行情  " << endl;
		//	SubscribeMarketData(subscribe_inst_vec);
		//
		//}

		
		//SubscribeMarketData("IF2012,IF2101,IF2103");

		//订阅“量化策略交易的合约”行情
		//SubscribeMarketData(m_InstId);

		////订阅“持仓合约”的行情
		//if (m_InstIdList_Position_MD)
		//{
		//	//"IF2012,IF2101,IF2103"
		//	cerr << "m_InstIdList_Position_MD 大小：" << strlen(m_InstIdList_Position_MD) << "," << m_InstIdList_Position_MD << endl;
		//	cerr << "有持仓，订阅行情  " << endl;
		//	SubscribeMarketData(m_InstIdList_Position_MD);//订阅行的流控6笔/秒，没有持仓就不用订阅行情

		//	cerr << "有持仓，订阅行情  " << endl;
		//	
		//	
		//												  
		//}


		//else
		//{
		//	cerr << "当前没有持仓" << endl;
		//}

		//SubscribeMarketData_All();
		SubscribeMarketData("IF2101");
		/*SubscribeMarketData("IF2101");
		SubscribeMarketData("IF2101");
		SubscribeMarketData("IF2101");
		SubscribeMarketData("IF2101");
		SubscribeMarketData("IF2101");
		SubscribeMarketData("IF2101");*/
		//策略启动后默认禁止开仓
		cerr << endl << endl << endl << "策略默认禁止开仓，如果允许交易，请输入指令（允许开仓：yxkc,禁止开仓：jzkc）" << endl;
		

	}
}

void MdSpi::OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void MdSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "OnRspSubMarketData : Instrument:" <<pSpecificInstrument->InstrumentID<< endl;

	if (pRspInfo)
		cerr << "errorid"<<pRspInfo->ErrorID <<"ErrorMsg:"<< pRspInfo->ErrorMsg << endl;

	
}

void MdSpi::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}
//char instIdList[]="IF2012,IF2101,IF2103"
void MdSpi::SubscribeMarketData(char* instIdList)
{
	//char*的vetor
	vector<char*>list;
	/*   strtok()用来将字符串分割成一个个片段。参数s指向欲分割的字符串，参数delim则为分割字符串中包含的所有字符。
		当strtok()在参数s的字符串中发现参数delim中包含的分割字符时, 则会将该字符改为\0 字符。
		在第一次调用时，strtok()必需给予参数s字符串，往后的调用则将参数s设置成NULL。
		每次调用成功则返回指向被分割出片段的指针。*/
	char* token = strtok(instIdList, ",");
	while (token != NULL)
	{
		list.push_back(token);
		token = strtok(NULL, ",");
	}
	unsigned int len = list.size();
	char** ppInstrument = new char* [len];
	for (unsigned i = 0; i < len; i++)
	{
		ppInstrument[i] = list[i];//指针赋值，没有新分配内存空间
	}
	//调用行情api的SubscribeMarketData
	int nRet = mdapi->SubscribeMarketData(ppInstrument, len);
	cerr << "请求订阅行情：" << ((nRet == 0) ? "成功" : "失败") << endl;

	
	delete[] ppInstrument;
}
//string instIdList="IF2012,IF2101,IF2103"
void MdSpi::SubscribeMarketData(std::string instIdList)
{
	
	int len = instIdList.size();
	//分配len+1个char的空间，否则会报错
	char* pInst = new char[len+1];
	strcpy(pInst, instIdList.c_str());
	//不要忘记加上结尾标志
	pInst[len] = '\0';
	
	
	//SubscribeMarketData(char* instIdList)
	SubscribeMarketData(pInst);
	delete[]pInst;
}

void MdSpi::SubscribeMarketData(std::vector<std::string>& subscribeVec)
{

	int nLen = subscribeVec.size();

	if (nLen > 0)
	{
		char  * * ppInst  = new char * [nLen];
		for ( int i = 0; i<nLen; i++)
		{
			ppInst[i] = new char[31]{ 0 };
			//memcpy(ppInst[i], subscribeVec[i].c_str(), 31);
			strcpy(ppInst[i], subscribeVec[i].c_str());
			
		}

		int nResult=mdapi->SubscribeMarketData(ppInst, nLen);
		cerr << "订阅行情 " << (nResult == 0 ? ("成功") : ("失败")) << endl;

		for (int i = 0; i < nLen; i++)
			delete[] ppInst[i];

		delete[] ppInst;
	}
		
}
//char* m_InstIdList_all;
void MdSpi::SubscribeMarketData_All()
{
	SubscribeMarketData(m_InstIdList_all);
}

void MdSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData)
{
	/*cout << "===========================================" << endl;
	cout << "深度行情" << endl;
	cout << " 交易日:" << pDepthMarketData->TradingDay << endl
		<< "合约代码:" << pDepthMarketData->InstrumentID << endl
		<< "最新价:" << pDepthMarketData->LastPrice << endl
		<< "上次结算价:" << pDepthMarketData->PreSettlementPrice << endl
		<< "昨收盘:" << pDepthMarketData->PreClosePrice << endl
		<< "数量:" << pDepthMarketData->Volume << endl
		<< "昨持仓量:" << pDepthMarketData->PreOpenInterest << endl
		<< "最后更新时间" << pDepthMarketData->UpdateTime << endl
		<< "最后更新毫秒" << pDepthMarketData->UpdateMillisec << endl
	<< "申买价一：" << pDepthMarketData->BidPrice1 << endl
	<< "申买量一:" << pDepthMarketData->BidVolume1 << endl
	<< "申卖价一:" << pDepthMarketData->AskPrice1 << endl
	<< "申卖量一:" << pDepthMarketData->AskVolume1 << endl
	<< "今收盘价:" << pDepthMarketData->ClosePrice << endl
	<< "当日均价:" << pDepthMarketData->AveragePrice << endl
	<< "本次结算价格:" << pDepthMarketData->SettlementPrice << endl
	<< "成交金额:" << pDepthMarketData->Turnover << endl
	<< "持仓量:" << pDepthMarketData->OpenInterest << endl;*/
}

//"IF2102,IF2103",string类型转换为你char *
void MdSpi::setInstIdList_Position_MD(std::string& inst_holding)
{
	int len = inst_holding.length();
	m_InstIdList_Position_MD = new char[len+1];

	memcpy(m_InstIdList_Position_MD, inst_holding.c_str(), len);
	m_InstIdList_Position_MD[len] = '\0';
}

void MdSpi::InsertInstToSubVec(char* Inst)
{
	//将数据插入到vector里面  subscribe_inst_vec;
	bool findflag = false;
	int len = subscribe_inst_vec.size();
	for (int i = 0; i < len; i++)
	{
		if (strcmp(subscribe_inst_vec[i].c_str(), Inst) == 0)
		{
			findflag = true;
			break;
		}
	}
	//如果没有找到就插入订阅合约的vector中
	if (!findflag)
		subscribe_inst_vec.push_back(Inst);


}
/// <summary>
/// 将string类型的所有合约转换为char *类型
/// </summary>
/// <param name="inst_all"></param>
///string inst_all="IF2012,IF2101,IF2103";
void MdSpi::set_InstIdList_All(std::string& inst_all)
{
	//字符串的长度
	int nLen = inst_all.size();
	//
	m_InstIdList_all = new char[nLen + 1];
	strcpy(m_InstIdList_all, inst_all.c_str());
	//千万记住加上结束标志，重要，否则会出错
	m_InstIdList_all[nLen] = '\0';
	//delete [] m_InstIdList_all;写到类的析构函数中
}

int MdSpi::GetNextRequestID()
{
	//给g_nRequestID加上互斥锁
		/*m_mutex.lock();
		int nNextID = g_nRequestID++;
		m_mutex.unlock();*/
		//1原理，在构造函数里面使用m_mutex.lock();
		//在析构的时候使用解锁m_mutex.unlock();
std:lock_guard<mutex> m_lock(m_mutex);

	int nNextID = g_nRequestID++;

	return g_nRequestID;
}


