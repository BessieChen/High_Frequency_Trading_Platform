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
	//ͨ����Լ�ַ�������������
	void SubscribeMarketData(char* instIdList);

	void SubscribeMarketData(std::string instIdList);
	//ͨ����Լvector������
	void SubscribeMarketData(std::vector<std::string> &subscribeVec);
	

	//�������еĺ�Լ
	void SubscribeMarketData_All();
	
	
	///�������֪ͨ
	void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData);

	//���ĳֲֺ�Լ����,��stringת��Ϊchar����
	void setInstIdList_Position_MD(std::string& inst_holding);

	//���ĳֲֺ�Լ����,��stringת��Ϊchar����
	void InsertInstToSubVec(char * Inst);
	//"IF2101,IF2102,IF2103" 
	//����ȫ����Լ
	void set_InstIdList_All(std::string& inst_all);
	int GetNextRequestID();

private:
	CThostFtdcMdApi* mdapi;
	CThostFtdcReqUserLoginField* loginField;
	std::string m_BrokerId;
	std::string m_UserId;
	std::string m_Passwd;
	
	//����������Ҫ���׵ĺ�Լ
	char m_InstId[31];

	//�ֲֵĺ�Լ
	char* m_InstIdList_Position_MD;

	//���еĺ�Լ
	char* m_InstIdList_all;
};