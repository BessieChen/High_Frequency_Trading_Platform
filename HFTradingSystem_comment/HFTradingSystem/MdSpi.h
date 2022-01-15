#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include<string>
#include"main.h"
#include "ThostFtdcMdApi.h"
#include<map>
#include<vector>
//using namespace std;
class MdSpi :public CThostFtdcMdSpi {
public:
	MdSpi(CThostFtdcMdApi* mdapi);
	~MdSpi();
	//建立连接时触发
	void OnFrontConnected();

	void ReqUserLogin(std::string brokerId, std::string userId, std::string passwd);
	///登录请求响应
	void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo,
		int nRequestID, bool bIsLast);
	///登出请求响应
	void OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo,
		int nRequestID, bool bIsLast);
	///订阅行情应答
	void OnRspSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo,
		int nRequestID, bool bIsLast);
	///取消订阅行情应答
	void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo,
		int nRequestID, bool bIsLast);
	//通过合约字符串来订阅行情
	void SubscribeMarketData(char* instIdList);

	void SubscribeMarketData(std::string instIdList);
	//通过合约vector来订阅
	void SubscribeMarketData(std::vector<std::string> &subscribeVec);
	

	//订阅所有的合约
	void SubscribeMarketData_All();
	
	
	///深度行情通知
	void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData);

	//订阅持仓合约行情,将string转换为char数组
	void setInstIdList_Position_MD(std::string& inst_holding);

	//订阅持仓合约行情,将string转换为char数组
	void InsertInstToSubVec(char * Inst);
	//"IF2101,IF2102,IF2103" 
	//订阅全部合约
	void set_InstIdList_All(std::string& inst_all);
	int GetNextRequestID();

private:
	CThostFtdcMdApi* mdapi;
	CThostFtdcReqUserLoginField* loginField;
	std::string m_BrokerId;
	std::string m_UserId;
	std::string m_Passwd;
	
	//策略里面需要交易的合约
	char m_InstId[31];

	//持仓的合约
	char* m_InstIdList_Position_MD;

	//所有的合约
	char* m_InstIdList_all;
};