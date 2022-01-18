#include"MdSpi.h"
#include<iostream>
#include<mutex>
#include<vector>


extern std::map<std::string, std::string> accountConfig_map;//保存账户信息的map


//全局的互斥锁
extern std::mutex m_mutex;

//全局的requestId
extern int g_nRequestID;

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
	
}

void MdSpi::OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void MdSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void MdSpi::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}
//char instIdList[]="IF2012,IF2101,IF2103"
void MdSpi::SubscribeMarketData(char* instIdList)
{

}
//string instIdList="IF2012,IF2101,IF2103"
void MdSpi::SubscribeMarketData(std::string instIdList)
{
	
}
//char* m_InstIdList_all;
void MdSpi::SubscribeMarketData_All()
{

}

void MdSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData)
{
}

//"IF2102,IF2103",string类型转换为你char *
void MdSpi::setInstIdList_Position_MD(std::string& inst_holding)
{
	m_InstIdList_Position_MD = new char[inst_holding.length()+1];

	memcpy(m_InstIdList_Position_MD, inst_holding.c_str(), inst_holding.length());
	m_InstIdList_Position_MD[inst_holding.length()] = '\0';
}

void MdSpi::set_InstIdList_All(std::string& inst_all)
{
	m_InstIdList_all = new char[inst_all.size() + 1];
	//m_InstIdList_all = new char[inst_all.length()];
	strcpy(m_InstIdList_all, inst_all.c_str());
	m_InstIdList_all[inst_all.length()] = '\0';
}

int MdSpi::GetNextRequestID()
{
	//给m_nNextRequestID加上互斥锁
		/*m_mutex.lock();
		int nNextID = m_nNextRequestID++;
		m_mutex.unlock();*/
		//1原理，在构造函数里面使用m_mutex.lock();
		//在析构的时候使用解锁m_mutex.unlock();
std:lock_guard<mutex> m_lock(m_mutex);

	int nNextID = g_nRequestID++;

	return g_nRequestID;
}


