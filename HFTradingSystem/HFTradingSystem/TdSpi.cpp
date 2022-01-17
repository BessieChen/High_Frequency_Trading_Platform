#include "TdSpi.h"
#include<map>
#include<iostream>
#include<mutex>
using namespace std;


extern std::map<std::string, std::string> accountConfig_map;//保存账户信息的map
//全局的互斥锁
extern std::mutex m_mutex;


TdSpi::TdSpi(CThostFtdcTraderApi* tdapi, CThostFtdcMdApi* pUserApi_md, MdSpi* pUserSpi_md):
	m_pUserTDApi_trade(tdapi), m_pUserMDApi_trade(pUserApi_md), m_pUserMDSpi_trade(pUserSpi_md)

{
	//appid, simnow_client_test
	//	authcode, 0000000000000000
	//	product, simnow
	//	brokerId, 9999
	//	userId, 159599
	//	passwd, leon1222
	//	contract, ni2101, ta101
	//	MarketFront, tcp://180.168.146.187:10111
	//TradeFront, tcp ://180.168.146.187:10101
	
	//appid,authcode,userid,brokerid
	m_AppId= accountConfig_map["appid"];
	m_AuthCode= accountConfig_map["authcode"];
	m_BrokerId = accountConfig_map["brokerId"];
	m_UserId = accountConfig_map["userId"];
	m_Passwd= accountConfig_map["passwd"];
	m_nNextRequestID=0;
}

void TdSpi::OnFrontConnected()
{
	cerr << "OnFrontConnected ：" << endl;
	static const char* version = m_pUserTDApi_trade->GetApiVersion();
	cerr << "当前的CTP Api Version：" <<version<< endl;
	ReqAuthenticate();
}



int TdSpi::ReqAuthenticate()
{
	//virtual int ReqAuthenticate(CThostFtdcReqAuthenticateField * pReqAuthenticateField, int nRequestID) = 0;
	CThostFtdcReqAuthenticateField req;
	//初始化
	memset(&req, 0, sizeof(req));

	//m_AppId = accountConfig_map["appid"];
	//m_AuthCode = accountConfig_map["authcode"];
	//m_BrokerId = accountConfig_map["brokerId"];
	//m_UserId = accountConfig_map["userId"];
	//m_Passwd = accountConfig_map["passwd"];
	strcpy(req.AppID, m_AppId.c_str());
	strcpy(req.AuthCode, m_AuthCode.c_str());
	strcpy(req.BrokerID, m_BrokerId.c_str());
	strcpy(req.UserID, m_UserId.c_str());
	cerr << "请求认证的账户信息：" << endl << " appid: " << req.AppID << " authcode: " << req.AuthCode
		<< " brokerid: " << req.BrokerID << " userId: " << req.UserID << endl;
	
	return m_pUserTDApi_trade->ReqAuthenticate(&req,GetNextRequestID());
}
void TdSpi::OnRspAuthenticate(CThostFtdcRspAuthenticateField* pRspAuthenticateField, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo)
	{
		if (pRspInfo->ErrorID == 0)
		{
			cerr << "穿透测试验证成功！" << "ErrMsg:" << pRspInfo->ErrorMsg << endl;
			ReqUserLogin();
		}

		else
		{
			cerr << "穿透测试验证失败！" << " errorid:" << pRspInfo->ErrorID <<
				"ErrMsg:" << pRspInfo->ErrorMsg << endl;
		}
	}

}
int TdSpi::ReqUserLogin()
{
	cerr << "====ReqUserLogin====,用户登录中..." <<endl;
	//定义一个CThostFtdcReqUserLoginField
	CThostFtdcReqUserLoginField reqUserLogin;
	//初始化为0
	memset(&reqUserLogin, 0, sizeof(reqUserLogin));
	//复制brokerid,userid,passwd
	strcpy(reqUserLogin.BrokerID, m_BrokerId.c_str());
	strcpy(reqUserLogin.UserID, m_UserId.c_str());
	strcpy(reqUserLogin.Password, m_Passwd.c_str());
	
	//登录
	return m_pUserTDApi_trade->ReqUserLogin(&reqUserLogin, GetNextRequestID());

}

void TdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "登录请求回调OnRspUserLogin" << endl;
	if (!IsErrorRspInfo(pRspInfo) && pRspUserLogin)
	{
		m_nFrontID = pRspUserLogin->FrontID;
		m_nSessionID = pRspUserLogin->SessionID;
		int nextOrderRef = atoi(pRspUserLogin->MaxOrderRef);

		sprintf_s(orderRef, sizeof(orderRef), "%d", ++nextOrderRef);

		cout << "前置编号:" << pRspUserLogin->FrontID << endl
			<< "会话编号" << pRspUserLogin->SessionID << endl
			<< "最大报单引用:" << pRspUserLogin->MaxOrderRef << endl
			<< "上期所时间：" << pRspUserLogin->SHFETime << endl
			<< "大商所时间：" << pRspUserLogin->DCETime << endl
			<< "郑商所时间：" << pRspUserLogin->CZCETime << endl
			<< "中金所时间：" << pRspUserLogin->FFEXTime << endl
			<< "能源所时间：" << pRspUserLogin->INETime << endl
			<< "交易日：" << m_pUserTDApi_trade->GetTradingDay() << endl;
		strcpy(m_cTradingDay, m_pUserTDApi_trade->GetTradingDay());//设置交易日期
		
		cout << "--------------------------------------------" << endl << endl;
	}

	ReqSettlementInfoConfirm();

}


void TdSpi::OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{


}
void TdSpi::OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}
void TdSpi::ReqQryOrder()
{
}

void TdSpi::ReqQryTrade()
{
}

void TdSpi::ReqQryInvestorPositionDetail()
{
}

void TdSpi::ReqQryTradingAccount()
{
}

void TdSpi::ReqQryInvestorPosition_All()
{
}

void TdSpi::ReqQryInvestorPosition()
{
}

void TdSpi::ReqQryInstrumetAll()
{
}

void TdSpi::ReqQryInstrumet()
{
}

void TdSpi::ReqSettlementInfoConfirm()
{
	CThostFtdcSettlementInfoConfirmField req;//定义
	memset(&req, 0, sizeof(req));//初始化
	strcpy(req.BrokerID, m_BrokerId.c_str());
	strcpy(req.InvestorID, m_UserId.c_str());
	int iResult = m_pUserTDApi_trade->ReqSettlementInfoConfirm(&req, GetNextRequestID());
	cerr << "--->>> 投资者结算结果确认: " << ((iResult == 0) ? "成功" : "失败") << endl;
}


void TdSpi::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	
	if (bIsLast && !IsErrorRspInfo(pRspInfo) && pSettlementInfoConfirm)
	{
		cerr << "响应 | 结算单..." << pSettlementInfoConfirm->InvestorID
			<< "..." << pSettlementInfoConfirm->ConfirmDate << "," <<
			pSettlementInfoConfirm->ConfirmTime << "...确认" << endl << endl;

		cerr << "结算单确认正常，首次查询报单" << endl;
		//线程休眠3秒，让ctp柜台有充足的响应时间，然后再进行查询操作
		std::chrono::milliseconds sleepDuration(3 * 1000);
		std::this_thread::sleep_for(sleepDuration);
		//Sleep(1000);
		ReqQryOrder();
	}
}

bool TdSpi::IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo)
{
	// 如果ErrorID != 0, 说明收到了错误的响应
	bool bResult = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if (bResult)
		cerr << "--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << endl;
	return bResult;
	
}




void TdSpi::OnRspQrySettlementInfo(CThostFtdcSettlementInfoField* pSettlementInfo, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}


void TdSpi::OnRspUserPasswordUpdate(CThostFtdcUserPasswordUpdateField* pUserPasswordUpdate, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void TdSpi::OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void TdSpi::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField* pInvestorPosition, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void TdSpi::OnRspQryInstrument(CThostFtdcInstrumentField* pInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void TdSpi::OnRspQryOrder(CThostFtdcOrderField* pOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void TdSpi::OnRspQryTrade(CThostFtdcTradeField* pTrade, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void TdSpi::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField* pField, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void TdSpi::OnRspQryTradingAccount(CThostFtdcTradingAccountField* pTradingAccount, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void TdSpi::OnRtnOrder(CThostFtdcOrderField* pOrder)
{
}

void TdSpi::OnRtnTrade(CThostFtdcTradeField* pTrade)
{
}



void TdSpi::CancelOrder(CThostFtdcOrderField* pOrder)
{
}

void TdSpi::ShowPosition()
{
}

void TdSpi::ClosePosition()
{
}

void TdSpi::SetAllowOpen(bool isOk)
{
}

CThostFtdcOrderField* TdSpi::GetOrder(int nBrokerOrderSeq)
{
	return nullptr;
}

bool TdSpi::UpdateOrder(CThostFtdcOrderField* pOrder)
{
	return false;
}

int TdSpi::GetNextRequestID()
{
	//给m_nNextRequestID加上互斥锁
	/*m_mutex.lock();
	int nNextID = m_nNextRequestID++;
	m_mutex.unlock();*/
	//1原理，在构造函数里面使用m_mutex.lock();
	//在析构的时候使用解锁m_mutex.unlock();
std:lock_guard<mutex> m_lock(m_mutex);
	
	int nNextID = m_nNextRequestID++;

	return m_nNextRequestID;
}

void TdSpi::PlaceOrder(const char* pszCode, const char* ExchangeID, int nDirection, int nOpenClose, int nVolume, double fPrice)
{
}
