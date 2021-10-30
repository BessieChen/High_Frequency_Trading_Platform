#pragma once
#include <string>
#include "main.h"
#include "ThostFtdcMdApi.h"

class MarketDataSpi : public CThostFtdcMdApi {
public:
	MarketDataSpi(CThostFtdcMdApi* mdapi);
	~MarketDataSpi();

	//��������ʱ����
	void OnFrontConnected();

	void ReqUserLogin(std::string brokerId, std::string userId, std::string passwd);
	///��¼������Ӧ
	void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo,
		int nRequestID, bool bIsLast);
	///�ǳ�������Ӧ
	void OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo,
		int nRequestID, bool bIsLast);
	///��������Ӧ��
	void OnRspSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo,
		int nRequestID, bool bIsLast);
	///ȡ����������Ӧ��
	void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo,
		int nRequestID, bool bIsLast);

	//�����г�����
	void SubscribeMarketData(char* instIdList);
	void SubscribeMarketData(std::string instIdList);

	///�������֪ͨ
	void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData);

	//����Ϊ�Ǽ̳еĽӿ�
	//�������еĺ�Լ 
	void SubscribeAllMarketData();

	//���ĳֲֺ�Լ����,��stringת��Ϊchar����
	void setHoldingInstIdList(std::string& instHolding);
	
	//����ȫ���ĺ�Լ 
	void setAllInstIdList(std::string& instAll);

private:
	CThostFtdcMdApi* mdapi;
	CThostFtdcReqUserLoginField* loginField;
	std::string m_BrokerId;
	std::string m_UserId;
	std::string m_Passwd;

	char m_InstId[32];		//������, ��Ҫ���׵ĺ�Լ 
	char* m_HoldingInstId;	//�ֲֵĺ�Լ: holding
	char* m_AllInstId;		//���еĺ�Լ: all
};


