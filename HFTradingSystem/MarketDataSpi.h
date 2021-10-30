#pragma once
#include <string>
#include "main.h"
#include "ThostFtdcMdApi.h"

class MarketDataSpi : public CThostFtdcMdApi {
public:
	MarketDataSpi(CThostFtdcMdApi* mdapi);
	~MarketDataSpi();

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

	//订阅市场数据
	void SubscribeMarketData(char* instIdList);
	void SubscribeMarketData(std::string instIdList);

	///深度行情通知
	void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData);

	//以下为非继承的接口
	//订阅所有的合约 
	void SubscribeAllMarketData();

	//订阅持仓合约行情,将string转换为char数组
	void setHoldingInstIdList(std::string& instHolding);
	
	//订阅全部的合约 
	void setAllInstIdList(std::string& instAll);

private:
	CThostFtdcMdApi* mdapi;
	CThostFtdcReqUserLoginField* loginField;
	std::string m_BrokerId;
	std::string m_UserId;
	std::string m_Passwd;

	char m_InstId[32];		//策略中, 需要交易的合约 
	char* m_HoldingInstId;	//持仓的合约: holding
	char* m_AllInstId;		//所有的合约: all
};


