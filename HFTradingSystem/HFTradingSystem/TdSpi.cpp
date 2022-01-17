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
	m_QryOrder_Once = true;

	m_QryTrade_Once = true;
	m_QryDetail_Once = true;
	m_QryTradingAccount_Once = true;
	m_QryPosition_Once = true;
	m_QryInstrument_Once = true;
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


	strcpy(req.AppID, m_AppId.c_str());
	//strcpy(req.AppID, "eiruoejladkfj");
	strcpy(req.AuthCode, m_AuthCode.c_str());
	//strcpy(req.AuthCode, "eiruoejladkfj");
	strcpy(req.BrokerID, m_BrokerId.c_str());
	//strcpy(req.BrokerID,"0000");
	//strcpy(req.UserID, m_UserId.c_str());
	strcpy(req.UserID, "");
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

			//<error id = "AUTH_FAILED" value = "63" prompt = "CTP:客户端认证失败" / >
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
	//strcpy(reqUserLogin.BrokerID, "0000");
	//errorid:3，不合法的登录
	//strcpy(reqUserLogin.UserID, "00000000");
	strcpy(reqUserLogin.UserID, m_UserId.c_str());
	strcpy(reqUserLogin.Password, m_Passwd.c_str());

	//strcpy(reqUserLogin.Password, "00000000");
	
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
	CThostFtdcQryOrderField  QryOrderField;//定义
	memset(&QryOrderField, 0, sizeof(CThostFtdcQryOrderField));//初始化为0
	//brokerid有误
	//strcpy(QryOrderField.BrokerID, "0000");
	strcpy(QryOrderField.BrokerID, m_BrokerId.c_str());
	//InvestorID有误
	strcpy(QryOrderField.InvestorID, "666666");
	//strcpy(QryOrderField.InvestorID, m_UserId.c_str());
	//调用api的ReqQryOrder
	m_pUserTDApi_trade->ReqQryOrder(&QryOrderField, GetNextRequestID());

}
void TdSpi::OnRspQryOrder(CThostFtdcOrderField* pOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "请求查询报单响应：OnRspQryOrder" <<",pOrder  "<<pOrder<< endl;
	if (!IsErrorRspInfo(pRspInfo) && pOrder)
	{
		cerr << "请求查询报单响应：前置编号FrontID：" << pOrder->FrontID << ",会话编号:" << pOrder->SessionID
			<< ",报单引用:  " << pOrder->OrderRef << endl;
			//所有合约
		if (m_QryOrder_Once == true)
		{
			CThostFtdcOrderField* order = new CThostFtdcOrderField();
			memcpy(order, pOrder, sizeof(CThostFtdcOrderField));
			orderList.push_back(order);

			//bIsLast是否是最后一笔回报
			if (bIsLast)
			{
				m_QryOrder_Once = false;
				cerr << "所有合约的报单次数" << orderList.size() << endl;
				cerr << "---------------打印报单开始---------------" << endl;
				for (vector<CThostFtdcOrderField*>::iterator iter = orderList.begin(); iter != orderList.end(); iter++)
				{
					cerr << "经纪公司代码：" << (*iter)->BrokerID << endl << "投资者代码：" << (*iter)->InvestorID << endl
						<< "用户代码：" << (*iter)->UserID << endl << "合约代码：" << (*iter)->InstrumentID << endl
						<< "买卖方向：" << (*iter)->Direction << endl << "组合开平标志：" << (*iter)->CombOffsetFlag << endl
						<< "价格：" << (*iter)->LimitPrice << endl << "数量：" << (*iter)->VolumeTotalOriginal << endl
						<< "报单引用：" << (*iter)->OrderRef << endl << "客户代码：" << (*iter)->ClientID << endl
						<< "报单状态：" << (*iter)->OrderStatus << endl << "委托时间：" << (*iter)->InsertTime << endl
						<< "报单编号：" << (*iter)->OrderStatus << endl << "交易日：" << (*iter)->TradingDay << endl
						<< "报单日期：" << (*iter)->InsertDate << endl;

				}
				cerr << "---------------打印报单完成---------------" << endl;
				cerr << "查询报单正常，将首次查询成交" << endl;
				//线程休眠3秒，让ctp柜台有充足的响应时间，然后再进行查询操作
				std::chrono::milliseconds sleepDuration(3 * 1000);
				std::this_thread::sleep_for(sleepDuration);
				ReqQryTrade();

			}
		}

	}
	else//查询出错
	{
		m_QryOrder_Once = false;
		cerr << "查询报单出错，或没有成交，将首次查询成交" << endl;
		//线程休眠3秒，让ctp柜台有充足的响应时间，然后再进行查询操作
		std::chrono::milliseconds sleepDuration(3 * 1000);
		std::this_thread::sleep_for(sleepDuration);
		ReqQryTrade();
	}
	
}


void TdSpi::ReqQryTrade()
{
	CThostFtdcQryTradeField tdField;//定义
	memset(&tdField, 0, sizeof(tdField));//初始化

	strcpy(tdField.BrokerID, m_BrokerId.c_str());
	//strcpy(tdField.BrokerID,"0000");
	strcpy(tdField.InvestorID, m_UserId.c_str());
	//strcpy(tdField.InvestorID, "888888");
	//调用交易api的ReqQryTrade
	m_pUserTDApi_trade->ReqQryTrade(&tdField, GetNextRequestID());
}
void TdSpi::OnRspQryTrade(CThostFtdcTradeField* pTrade, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{

	cerr << "请求查询成交回报响应：OnRspQryTrade" << " pTrade " << pTrade << endl;

	if (!IsErrorRspInfo(pRspInfo) && pTrade)
	{
		//所有合约
		if (m_QryTrade_Once == true)
		{
			CThostFtdcTradeField* trade = new CThostFtdcTradeField();//创建trade
			memcpy(trade, pTrade, sizeof(CThostFtdcTradeField));//pTrade复制给trade
			tradeList.push_back(trade);

			if (bIsLast)
			{
				m_QryTrade_Once = false;
				cerr << "所有合约的成交次数" << tradeList.size() << endl;
				cerr << "---------------打印成交开始---------------" << endl;
				for (vector<CThostFtdcTradeField*>::iterator iter = tradeList.begin(); iter != tradeList.end(); iter++)
				{
					cerr << "经纪公司代码：" << (*iter)->BrokerID << endl << "投资者代码：" << (*iter)->InvestorID << endl
						<< "用户代码：" << (*iter)->UserID << endl << "成交编号：" << (*iter)->TradeID << endl
						<< "合约代码：" << (*iter)->InstrumentID << endl << "买卖方向：" << (*iter)->Direction << endl
						<< "组合开平标志：" << (*iter)->OffsetFlag << endl << "投机套保标志：" << (*iter)->HedgeFlag << endl
						<< "价格：" << (*iter)->Price << endl << "数量：" << (*iter)->Volume << endl
						<< "报单引用：" << (*iter)->OrderRef << endl << "本地报单编号：" << (*iter)->OrderLocalID << endl
						<< "成交时间：" << (*iter)->TradeTime << endl << "业务单元：" << (*iter)->BusinessUnit << endl
						<< "序号：" << (*iter)->SequenceNo << endl << "经纪公司下单序号：" << (*iter)->BrokerOrderSeq << endl
						<< "交易日：" << (*iter)->TradingDay << endl;

				}
				cerr << "---------------打印成交完成---------------" << endl;
				cerr << "查询报单正常，将首次查询持仓明细" << endl;
				//线程休眠3秒，让ctp柜台有充足的响应时间，然后再进行查询操作
				std::chrono::milliseconds sleepDuration(3 * 1000);
				std::this_thread::sleep_for(sleepDuration);
				ReqQryInvestorPositionDetail();
			}
			

		}

	}
	else//查询出错
	{
		m_QryOrder_Once = false;
		cerr << "查询报单出错，或没有成交，将首次查询成交" << endl;
		//线程休眠3秒，让ctp柜台有充足的响应时间，然后再进行查询操作
		std::chrono::milliseconds sleepDuration(3 * 1000);
		std::this_thread::sleep_for(sleepDuration);
		ReqQryInvestorPositionDetail();
	}

	
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
	//strcpy(req.BrokerID, "0000");
	strcpy(req.InvestorID, m_UserId.c_str());
	//strcpy(req.InvestorID, "000000");
	
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
