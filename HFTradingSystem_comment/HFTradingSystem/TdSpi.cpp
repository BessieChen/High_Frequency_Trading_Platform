#include "TdSpi.h"
#include<map>
#include<iostream>
#include<mutex>
#include"Strategy.h"
using namespace std;


extern std::map<std::string, std::string> accountConfig_map;//保存账户信息的map
//全局的互斥锁
extern std::mutex m_mutex;
extern Strategy* g_strategy;//策略类指针

extern int g_nRequestID;

//全局的持仓合约
extern std::vector<std::string> subscribe_inst_vec;

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
	m_InstId= accountConfig_map["contract"];
	m_nNextRequestID=0;
	m_QryOrder_Once = true;

	m_QryTrade_Once = true;
	m_QryDetail_Once = true;
	m_QryTradingAccount_Once = true;
	m_QryPosition_Once = true;
	m_QryInstrument_Once = true;
}

TdSpi::~TdSpi()
{
	Release();
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
	//strcpy(QryOrderField.InvestorID, "666666");
	strcpy(QryOrderField.InvestorID, m_UserId.c_str());
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
				

			}
		}

	}
	else//查询出错
	{
		m_QryOrder_Once = false;
		cerr << "查询报单出错，或没有成交，将首次查询成交" << endl;
		
	}
	if (bIsLast)
	{
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
				
			}
			

		}

	}
	else//查询出错
	{
		m_QryOrder_Once = false;
		cerr << "查询报单出错，或没有成交，将首次查询成交" << endl;
		
	}
	if (bIsLast)
	{
		//线程休眠3秒，让ctp柜台有充足的响应时间，然后再进行查询操作
		std::chrono::milliseconds sleepDuration(3 * 1000);
		std::this_thread::sleep_for(sleepDuration);
		ReqQryInvestorPositionDetail();
	}

	
}
void TdSpi::ReqQryInvestorPositionDetail()
{
	CThostFtdcQryInvestorPositionDetailField pdField;//创建
	memset(&pdField, 0, sizeof(pdField));//初始化为0
	strcpy(pdField.BrokerID, m_BrokerId.c_str());
	//strcpy(pdField.BrokerID, "0000");
	//strcpy(pdField.InstrumentID, m_InstId.c_str());


	strcpy(pdField.InvestorID, m_UserId.c_str());

	//strcpy(pdField.InvestorID, "0000");
	//调用交易api的ReqQryInvestorPositionDetail
	m_pUserTDApi_trade->ReqQryInvestorPositionDetail(&pdField,GetNextRequestID());
}

void TdSpi::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField* pField, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "请求查询投资者持仓明细回报响应：OnRspQryInvestorPositionDetail" << " pInvestorPositionDetail " << pField << endl;
	if (!IsErrorRspInfo(pRspInfo) && pField)
	{
		//所有合约
		if (m_QryDetail_Once == true)
		{
			//对于所有合约，只保存未平仓的，不保存已经平仓的
			//将程序启动前的持仓记录保存到未平仓容器tradeList_NotClosed_Long和tradeList_NotClosed_Short
			//使用结构体CThostFtdcTradeField，因为它有时间字段，而CThostFtdcInvestorPositionDetailField没有时间字段
			CThostFtdcTradeField* trade = new CThostFtdcTradeField();//创建CThostFtdcTradeField *
			
			strcpy(trade->InvestorID, pField->InvestorID);///投资者代码
			strcpy(trade->InstrumentID, pField->InstrumentID);///合约代码
			strcpy(trade->ExchangeID, pField->ExchangeID);///交易所代码
			trade->Direction = pField->Direction;//买卖方向
			trade->Price = pField->OpenPrice;//价格
			trade->Volume = pField->Volume;//数量
			strcpy(trade->TradeDate, pField->OpenDate);//成交日期
			strcpy(trade->TradeID, pField->TradeID);//*********成交编号********
			if (pField->Volume > 0)//筛选未平仓合约
			{
				if (trade->Direction == '0')//买入方向
					tradeList_NotClosed_Long.push_back(trade);
				else if (trade->Direction == '1')//卖出方向
					tradeList_NotClosed_Short.push_back(trade);
			}
			//收集持仓合约的代码
			bool find_instId = false;
			for (unsigned int i = 0; i < subscribe_inst_vec.size(); i++)
			{
				if (strcmp(subscribe_inst_vec[i].c_str(), trade->InstrumentID) == 0)//合约已存在，已订阅
				{
					find_instId = true;
					break;
				}
			}
			if (!find_instId)//合约未订阅过
			{
				cerr << "---------------------------------------该持仓合约未订阅过，加入订阅列表" << endl;
				subscribe_inst_vec.push_back(trade->InstrumentID);
			}

		}
		//输出所有合约的持仓明细，要在这边进行下一步的查询ReqQryTradingAccount()
		if (bIsLast)
		{
			m_QryDetail_Once = false;
			//持仓的合约
			string inst_holding;
			//
			for (unsigned int i = 0; i < subscribe_inst_vec.size(); i++)
				inst_holding = inst_holding + subscribe_inst_vec[i] + ",";
			//"IF2102,IF2103,"

			inst_holding = inst_holding.substr(0, inst_holding.length() - 1);//去掉最后的逗号，从位置0开始，选取length-1个字符
			//"IF2102,IF2103"

			cerr << "程序启动前的持仓列表:" << inst_holding << ",inst_holding.length()=" << inst_holding.length()
				<< ",subscribe_inst_vec.size()=" << subscribe_inst_vec.size() << endl;

			if (inst_holding.length() > 0)
				m_pUserMDSpi_trade->setInstIdList_Position_MD(inst_holding);//设置程序启动前的留仓，即需要订阅行情的合约

			//size代表笔数，而不是手数
			cerr << "账户所有合约未平仓单笔数（下单笔数，一笔可以对应多手）,多单:" << tradeList_NotClosed_Long.size()
				<< "空单：" << tradeList_NotClosed_Short.size() << endl;


			cerr << "-----------------------------------------未平仓多单明细打印start" << endl;
			for (vector<CThostFtdcTradeField*>::iterator iter = tradeList_NotClosed_Long.begin(); iter != tradeList_NotClosed_Long.end(); iter++)
			{
				cerr << "BrokerID:" << (*iter)->BrokerID << endl << "InvestorID:" << (*iter)->InvestorID << endl
					<< "InstrumentID:" << (*iter)->InstrumentID << endl << "Direction:" << (*iter)->Direction << endl
					<< "OpenPrice:" << (*iter)->Price << endl << "Volume:" << (*iter)->Volume << endl
					<< "TradeDate:" << (*iter)->TradeDate << endl << "TradeID:" << (*iter)->TradeID << endl;
			}

			cerr << "-----------------------------------------未平仓空单明细打印start" << endl;
			for (vector<CThostFtdcTradeField*>::iterator iter = tradeList_NotClosed_Short.begin(); iter != tradeList_NotClosed_Short.end(); iter++)
			{
				cerr << "BrokerID:" << (*iter)->BrokerID << endl << "InvestorID:" << (*iter)->InvestorID << endl
					<< "InstrumentID:" << (*iter)->InstrumentID << endl << "Direction:" << (*iter)->Direction << endl
					<< "OpenPrice:" << (*iter)->Price << endl << "Volume:" << (*iter)->Volume << endl
					<< "TradeDate:" << (*iter)->TradeDate << endl << "TradeID:" << (*iter)->TradeID << endl;
			}
			cerr << "---------------打印持仓明细完成---------------" << endl;
			cerr << "查询持仓明细正常，将首次查询账户资金信息" << endl;
		}
		
	}
	else
	{
		if (m_QryDetail_Once == true)
		{
			m_QryDetail_Once = false;
			cerr << "查询持仓明细出错，或没有持仓明细，将首次查询账户资金" << endl;
		}
	}
	if (bIsLast)
	{
		//线程休眠3秒，让ctp柜台有充足的响应时间，然后再进行查询操作
		std::chrono::milliseconds sleepDuration(3 * 1000);
		std::this_thread::sleep_for(sleepDuration);
		ReqQryTradingAccount();
	}
	
}

void TdSpi::ReqQryTradingAccount()
{
	CThostFtdcQryTradingAccountField req;//创建req的结构体对象
	memset(&req, 0, sizeof(req));//初始化
	//错误的brokerID
	//strcpy(req.BrokerID, "8888");


	strcpy(req.BrokerID, m_BrokerId.c_str());


	strcpy(req.InvestorID, m_UserId.c_str());
	//strcpy(req.InvestorID, "666666");
	//调用交易api的ReqQryTradingAccount
	int iResult = m_pUserTDApi_trade->ReqQryTradingAccount(&req, GetNextRequestID());
	cerr << "--->>> 请求查询资金账户: " << ((iResult == 0) ? "成功" : "失败") << endl;
}

void TdSpi::OnRspQryTradingAccount(CThostFtdcTradingAccountField* pTradingAccount, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "请求查询投资者资金账户回报响应：OnRspQryTradingAccount" << " pTradingAccount " << pTradingAccount << endl;
	if (!IsErrorRspInfo(pRspInfo) && pTradingAccount)
	{

		cerr << "投资者编号：" << pTradingAccount->AccountID
			<< "静态权益：期初权益" << pTradingAccount->PreBalance
			<< "动态权益：期货结算准备金" << pTradingAccount->Balance
			<< "可用资金：" << pTradingAccount->Available
			<< "可取资金：" << pTradingAccount->WithdrawQuota
			<< "当前保证金总额：" << pTradingAccount->CurrMargin
			<< "平仓盈亏：" << pTradingAccount->CloseProfit
			<< "持仓盈亏：" << pTradingAccount->PositionProfit
			<< "手续费：" << pTradingAccount->Commission
			<< "冻结保证金：" << pTradingAccount->FrozenCash
			<< endl;
		//所有合约
		if (m_QryTradingAccount_Once == true)
		{
			m_QryTradingAccount_Once = false;
		}

		cerr << "---------------打印资金账户明细完成---------------" << endl;
		cerr << "查询资金账户正常，将首次查询投资者持仓信息" << endl;
	}
	else
	{
		if (m_QryTradingAccount_Once == true)
		{
			m_QryTradingAccount_Once = false;
			cerr << "查询资金账户出错，将首次查询投资者持仓" << endl;
		}
	}
	//线程休眠3秒，让ctp柜台有充足的响应时间，然后再进行查询操作
	std::chrono::milliseconds sleepDuration(3 * 1000);
	std::this_thread::sleep_for(sleepDuration);
	ReqQryInvestorPosition_All();
}


void TdSpi::ReqQryInvestorPosition_All()
{
	CThostFtdcQryInvestorPositionField req;//创建req
	memset(&req, 0, sizeof(req));//初始化为0

	//strcpy(req.BrokerID, "8888");
	strcpy(req.BrokerID, m_BrokerId.c_str());
	strcpy(req.InvestorID, m_UserId.c_str());
	//strcpy(req.InvestorID, "0000");
	//合约为空，则代表查询所有合约的持仓，这个和req为空是一样的
	strcpy(req.InstrumentID, m_InstId.c_str());
	//调用交易api的ReqQryInvestorPosition
	int iResult = m_pUserTDApi_trade->ReqQryInvestorPosition(&req, GetNextRequestID());//req为空，代表查询所有合约的持仓
	cerr << "--->>> 请求查询投资者持仓: " << ((iResult == 0) ? "成功" : "失败") << endl;
}
void TdSpi::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField* pInvestorPosition,
	CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
	//cerr << "请求查询持仓响应：OnRspQryInvestorPosition " << ",pInvestorPosition  " << pInvestorPosition << endl;
	if (!IsErrorRspInfo(pRspInfo) && pInvestorPosition)
	{

		//账户下所有合约
		if (m_QryPosition_Once == true)
		{
			cerr << "请求查询持仓响应：OnRspQryInvestorPosition " << " pInvestorPosition:  "
				<< pInvestorPosition << endl;//会包括已经平仓没有持仓的记录
			cerr << "响应  | 合约 " << pInvestorPosition->InstrumentID << endl
				<< " 持仓多空方向 " << pInvestorPosition->PosiDirection << endl//2多3空
			   // << " 映射后的方向 " << MapDirection(pInvestorPosition->PosiDirection-2,false) << endl
				<< " 总持仓 " << pInvestorPosition->Position << endl
				<< " 今日持仓 " << pInvestorPosition->TodayPosition << endl
				<< " 上日持仓 " << pInvestorPosition->YdPosition << endl
				<< " 保证金 " << pInvestorPosition->UseMargin << endl
				<< " 持仓成本 " << pInvestorPosition->PositionCost << endl
				<< " 开仓量 " << pInvestorPosition->OpenVolume << endl
				<< " 平仓量 " << pInvestorPosition->CloseVolume << endl
				<< " 持仓日期 " << pInvestorPosition->TradingDay << endl
				<< " 平仓盈亏（按昨结） " << pInvestorPosition->CloseProfitByDate << endl
				<< " 持仓盈亏 " << pInvestorPosition->PositionProfit << endl
				<< " 逐日盯市平仓盈亏（按昨结） " << pInvestorPosition->CloseProfitByDate << endl//快期中显示的是这个值
				<< " 逐笔对冲平仓盈亏（按开平合约） " << pInvestorPosition->CloseProfitByTrade << endl//在交易中比较有意义
				<< endl;


			//构造合约对应持仓明细信息的结构体map
			bool  find_trade_message_map = false;
			for (map<string, position_field*>::iterator iter = m_position_field_map.begin(); iter != m_position_field_map.end(); iter++)
			{
				if (strcmp((iter->first).c_str(), pInvestorPosition->InstrumentID) == 0)//合约已存在
				{
					find_trade_message_map = true;
					break;
				}
			}
			if (!find_trade_message_map)//合约不存在
			{
				cerr << "-----------------------没有这个合约，需要构造交易信息结构体" << endl;
				position_field* p_trade_message = new position_field();
				p_trade_message->instId = pInvestorPosition->InstrumentID;
				//构造持仓合约的string
				m_Inst_Postion += pInvestorPosition->InstrumentID ;
				m_Inst_Postion += ",";
				m_position_field_map.insert(pair<string, position_field*>(pInvestorPosition->InstrumentID, p_trade_message));
			}
			if (pInvestorPosition->PosiDirection == '2')//多单
			{
				//昨仓和今仓一次返回
				//获取该合约的持仓明细信息结构体 second; m_map[键]
				position_field* p_tdm = m_position_field_map[pInvestorPosition->InstrumentID];
				p_tdm->LongPosition = p_tdm->LongPosition + pInvestorPosition->Position;
				p_tdm->TodayLongPosition = p_tdm->TodayLongPosition + pInvestorPosition->TodayPosition;
				p_tdm->YdLongPosition = p_tdm->LongPosition - p_tdm->TodayLongPosition;
				p_tdm->LongCloseProfit = p_tdm->LongCloseProfit + pInvestorPosition->CloseProfit;
				p_tdm->LongPositionProfit = p_tdm->LongPositionProfit + pInvestorPosition->PositionProfit;
			}
			else if (pInvestorPosition->PosiDirection == '3')//空单
			{
				//昨仓和今仓一次返回

				position_field* p_tdm = m_position_field_map[pInvestorPosition->InstrumentID];
				p_tdm->ShortPosition = p_tdm->ShortPosition + pInvestorPosition->Position;
				p_tdm->TodayShortPosition = p_tdm->TodayShortPosition + pInvestorPosition->TodayPosition;
				p_tdm->YdShortPosition = p_tdm->ShortPosition - p_tdm->TodayShortPosition;
				p_tdm->ShortCloseProfit = p_tdm->ShortCloseProfit + pInvestorPosition->CloseProfit;
				p_tdm->ShortPositionProfit = p_tdm->ShortPositionProfit + pInvestorPosition->PositionProfit;
			}

			if (bIsLast)
			{
				m_QryPosition_Once = false;
				m_Inst_Postion = m_Inst_Postion.substr(0, m_Inst_Postion.length() - 1);
				
				//m_pUserMDSpi_trade->setInstIdList_Position_MD(m_Inst_Postion);
				
				for (map<string, position_field*>::iterator iter = m_position_field_map.begin(); iter != m_position_field_map.end(); iter++)
				{
					cerr << "合约代码：" << iter->second->instId << endl
						<< "多单持仓量：" << iter->second->LongPosition << endl
						<< "空单持仓量：" << iter->second->ShortPosition << endl
						<< "多单今日持仓：" << iter->second->TodayLongPosition << endl
						<< "多单上日持仓：" << iter->second->YdLongPosition << endl
						<< "空单今日持仓：" << iter->second->TodayShortPosition << endl
						<< "空单上日持仓：" << iter->second->YdShortPosition << endl
						<< "多单浮动盈亏：" << iter->second->LongPositionProfit << endl
						<< "多单平仓盈亏：" << iter->second->LongCloseProfit << endl
						<< "空单浮动盈亏：" << iter->second->ShortPositionProfit << endl
						<< "空单平仓盈亏：" << iter->second->ShortCloseProfit << endl;

					//账户平仓盈亏
					m_CloseProfit = m_CloseProfit + iter->second->LongCloseProfit + iter->second->ShortCloseProfit;
					//账户浮动盈亏
					m_OpenProfit = m_OpenProfit + iter->second->LongPositionProfit + iter->second->ShortPositionProfit;
				}

				cerr << "账户浮动盈亏 " << m_OpenProfit << endl;
				cerr << "账户平仓盈亏 " << m_CloseProfit << endl;
			}//bisLast


		}
		cerr << "---------------查询投资者持仓完成---------------" << endl;
		cerr << "查询持仓正常，首次查询所有合约代码" << endl;
	}
	else
	{
		if (m_QryPosition_Once == true)
			m_QryPosition_Once = false;
		cerr << "查询投资者持仓出错，或没有持仓，首次查询所有合约" << endl;
	}
	if (bIsLast)
	{
		//线程休眠3秒，让ctp柜台有充足的响应时间，然后再进行查询操作
		std::chrono::milliseconds sleepDuration(10 * 1000);
		std::this_thread::sleep_for(sleepDuration);
		ReqQryInstrumetAll();
	}
	
}

/// <summary>
/// 查询单个期货合约
/// </summary>
void TdSpi::ReqQryInvestorPosition(char * pInstrument)
{
	CThostFtdcQryInvestorPositionField req;//创建req
	memset(&req, 0, sizeof(req));//初始化为0

	
	strcpy(req.BrokerID, m_BrokerId.c_str());
	strcpy(req.InvestorID, m_UserId.c_str());
	
	//合约填写具体的合约代码
	strcpy(req.InstrumentID, pInstrument);
	//调用交易api的ReqQryInvestorPosition
	int iResult = m_pUserTDApi_trade->ReqQryInvestorPosition(&req, GetNextRequestID());//req为空，代表查询所有合约的持仓
	cerr << "--->>> 请求查询投资者持仓: " << ((iResult == 0) ? "成功" : "失败") << endl;
}

void TdSpi::ReqQryInstrumetAll()
{
	CThostFtdcQryInstrumentField req;//创建req
	memset(&req, 0, sizeof(req));//初始化为0


	//调用交易api的ReqQryInstrument
	int iResult = m_pUserTDApi_trade->ReqQryInstrument(&req, GetNextRequestID());//req结构体为0，查询所有合约
	cerr << "--->>> 请求查询合约: " << ((iResult == 0) ? "成功" : "失败") << endl;
}

void TdSpi::ReqQryInstrumet(char * pInstrument)
{
	CThostFtdcQryInstrumentField req;//创建req
	memset(&req, 0, sizeof(req));//初始化为0
	strcpy(req.InstrumentID, pInstrument);//合约填写具体的代码，表示查询该合约的信息
	//调用交易api的ReqQryInstrument
	int iResult = m_pUserTDApi_trade->ReqQryInstrument(&req, GetNextRequestID());//
	cerr << "--->>> 请求查询合约: " << ((iResult == 0) ? "成功" : "失败") << endl;
}



void TdSpi::OnRspQryInstrument(CThostFtdcInstrumentField* pInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	//cerr << "请求查询合约响应：OnRspQryInstrument" << ",pInstrument   " << pInstrument->InstrumentID << endl;
	if (!IsErrorRspInfo(pRspInfo) && pInstrument)
	{

		//账户下所有合约
		if (m_QryInstrument_Once == true)
		{
			m_Instrument_All = m_Instrument_All + pInstrument->InstrumentID + ",";

			//保存所有合约信息到map
			CThostFtdcInstrumentField* pInstField = new CThostFtdcInstrumentField();
			memcpy(pInstField, pInstrument, sizeof(CThostFtdcInstrumentField));
			m_inst_field_map.insert(pair<string, CThostFtdcInstrumentField*>(pInstrument->InstrumentID, pInstField));

			//策略交易的合约
			if (strcmp(m_InstId.c_str(), pInstrument->InstrumentID) == 0)
			{
				cerr << "响应 | 合约：" << pInstrument->InstrumentID
					<< "合约名称：" << pInstrument->InstrumentName
					<< " 合约在交易所代码：" << pInstrument->ExchangeInstID
					<< " 产品代码：" << pInstrument->ProductID
					<< " 产品类型：" << pInstrument->ProductClass
					<< " 多头保证金率：" << pInstrument->LongMarginRatio
					<< " 空头保证金率：" << pInstrument->ShortMarginRatio
					<< " 合约数量乘数：" << pInstrument->VolumeMultiple
					<< " 最小变动价位：" << pInstrument->PriceTick
					<< " 交易所代码：" << pInstrument->ExchangeID
					<< " 交割年份：" << pInstrument->DeliveryYear
					<< " 交割月：" << pInstrument->DeliveryMonth
					<< " 创建日：" << pInstrument->CreateDate
					<< " 到期日：" << pInstrument->ExpireDate
					<< " 上市日：" << pInstrument->OpenDate
					<< " 开始交割日：" << pInstrument->StartDelivDate
					<< " 结束交割日：" << pInstrument->EndDelivDate
					<< " 合约生命周期状态：" << pInstrument->InstLifePhase
					<< " 当前是否交易：" << pInstrument->IsTrading << endl;
			}

			if (bIsLast)
			{
				m_QryInstrument_Once = false;
				m_Instrument_All = m_Instrument_All.substr(0, m_Instrument_All.length() - 1);
				cerr << "m_Instrument_All的大小：" << m_Instrument_All.size() << endl;
				cerr << "map的大小（合约数量）：" << m_inst_field_map.size() << endl;

				//将持仓合约信息设置到mdspi
				//m_pUserMDSpi_trade->setInstIdList_Position_MD(m_Inst_Postion);


				//将合约信息结构体的map复制到策略类
				g_strategy->set_instPostion_map_stgy(m_inst_field_map);
				cerr << "--------------------------输出合约信息map的内容-----------------------" << endl;
				//ShowInstMessage();
				//保存全市场合约，在TD进行，需要订阅全市场合约行情时再运行
				m_pUserMDSpi_trade->set_InstIdList_All(m_Instrument_All);
				cerr << "TD初始化完成，启动MD" << endl;
				m_pUserMDApi_trade->Init();
			}
		}
	}
	else
	{
		m_QryInstrument_Once = false;
		cerr << "查询合约失败" << endl;
	}
	

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








void TdSpi::OnRtnOrder(CThostFtdcOrderField* pOrder)
{

	//判断是否本程序发出的报单；

	if (pOrder->FrontID != m_nFrontID || pOrder->SessionID != m_nSessionID) {

		CThostFtdcOrderField* pOld = GetOrder(pOrder->BrokerOrderSeq);
		if (pOld == NULL) {
			return;
		}
	}
	
	char* pszStatus = new char[13];
	switch (pOrder->OrderStatus) {
	case THOST_FTDC_OST_AllTraded:
		strcpy(pszStatus, "全部成交");
		break;
	case THOST_FTDC_OST_PartTradedQueueing:
		strcpy(pszStatus, "部分成交");
		break;
	case THOST_FTDC_OST_NoTradeQueueing:
		strcpy(pszStatus, "未成交");
		break;
	case THOST_FTDC_OST_Canceled:
		strcpy(pszStatus, "已撤单");
		break;
	case THOST_FTDC_OST_Unknown:
		strcpy(pszStatus, "未知");
		break;
	case THOST_FTDC_OST_NotTouched:
		strcpy(pszStatus, "未触发");
		break;
	case THOST_FTDC_OST_Touched:

		strcpy(pszStatus, "已触发");
		break;
	default:
		strcpy(pszStatus, "");
		break;
	}

	/*printf("order returned,ins: %s, vol: %d, price:%f, orderref:%s,requestid:%d,traded vol: %d,ExchID:%s, OrderSysID:%s,status: %s,statusmsg:%s\n"
		, pOrder->InstrumentID, pOrder->VolumeTotalOriginal, pOrder->LimitPrice, pOrder->OrderRef, pOrder->RequestID
		, pOrder->VolumeTraded, pOrder->ExchangeID, pOrder->OrderSysID, pszStatus, pOrder->StatusMsg);*/
	//保存并更新报单的状态
	UpdateOrder(pOrder);
	cerr <<"BrokerOrderSeq:"<< pOrder->BrokerOrderSeq<< "  ,OrderRef;" <<pOrder->OrderRef<<" ,报单状态  " << pszStatus << endl;

	if (pOrder->OrderStatus == '3'|| pOrder->OrderStatus == '1')
	{
		CThostFtdcOrderField* pOld = GetOrder(pOrder->BrokerOrderSeq);
		if (pOld && pOld->OrderStatus != '6')
		{
			cerr << "onRtnOrder 准备撤单了:" << endl;
			CancelOrder(pOrder);
		}
		
		
	}
	
}

void TdSpi::OnRtnTrade(CThostFtdcTradeField* pTrade)
{
	
	//需要判断是否是断线重连
	//1、编写一个FindTrade函数
	bool bFind = false;

	bFind=FindTrade(pTrade);

	//2、如果没有记录，则插入成交数据，编写一个插入成交的函数
	if (!bFind)
	{
		InsertTrade(pTrade);
		ShowTradeList();
	}
		

	
		
	
	
	
	
	//判断是否断线重传；
	//如果程序断线重连以后，成交会再次刷新一次
	//set<string>::iterator it = m_TradeIDs.find(pTrade->TradeID);
	////成交已存在
	//if (it != m_TradeIDs.end()) {
	//	return;
	//}
	//成交id它不在我们的set集合里面
	
	//判断是否本程序发出的报单；
	//CThostFtdcOrderField* pOrder = GetOrder(pTrade->BrokerOrderSeq);
	//if (pOrder != NULL) {
	//	//只处理本程序发出的报单；

	//	//插入成交到set
	//	{
	//		std::lock_guard<std::mutex> m_lock(m_mutex);//加锁，保证这个set数据的安全
	//		m_TradeIDs.insert(pOrder->TraderID);
	//	}
	//	
	//	printf("trade returned,ins: %s, trade vol: %d, trade price:%f, ExchID:%s, OrderSysID:%s\n"
	//		, pTrade->InstrumentID, pTrade->Volume, pTrade->Price, pTrade->ExchangeID, pTrade->OrderSysID);

	//	//g_strategy->OnRtnTrade(pTrade);

	//}
}



void TdSpi::CancelOrder(CThostFtdcOrderField* pOrder)
{
	CThostFtdcInputOrderActionField oa;//创建一个撤单的结构体对象
	memset(&oa, 0, sizeof(CThostFtdcInputOrderActionField));//初始化，字段清零
	
	oa.ActionFlag = THOST_FTDC_AF_Delete;//撤单
	//下面这三个字段，能确定我们的报单
	oa.FrontID = pOrder->FrontID;//前置编号
	oa.SessionID = pOrder->SessionID;//会话
	strcpy(oa.OrderRef, pOrder->OrderRef);//报单引用

	if (pOrder->ExchangeID[0] != '\0') {
		strcpy(oa.ExchangeID, pOrder->ExchangeID);
	}
	if (pOrder->OrderSysID[0] != '\0') {
		strcpy(oa.OrderSysID, pOrder->OrderSysID);
	}

	strcpy(oa.BrokerID, pOrder->BrokerID);
	strcpy(oa.UserID, pOrder->UserID);
	strcpy(oa.InstrumentID, pOrder->InstrumentID);
	strcpy(oa.InvestorID, pOrder->InvestorID);

	//oa.RequestID = pOrder->RequestID;
	oa.RequestID = GetNextRequestID();
	//调用交易api的撤单函数
	int nRetCode = m_pUserTDApi_trade->ReqOrderAction(&oa, oa.RequestID);

	char* pszStatus = new char[13];
	switch (pOrder->OrderStatus) {
	case THOST_FTDC_OST_AllTraded:
		strcpy(pszStatus, "全部成交");
		break;
	case THOST_FTDC_OST_PartTradedQueueing:
		strcpy(pszStatus, "部分成交");
		break;
	case THOST_FTDC_OST_NoTradeQueueing:
		strcpy(pszStatus, "未成交");
		break;
	case THOST_FTDC_OST_Canceled:
		strcpy(pszStatus, "已撤单");
		break;
	case THOST_FTDC_OST_Unknown:
		strcpy(pszStatus, "未知");
		break;
	case THOST_FTDC_OST_NotTouched:
		strcpy(pszStatus, "未触发");
		break;
	case THOST_FTDC_OST_Touched:

		strcpy(pszStatus, "已触发");
		break;
	default:
		strcpy(pszStatus, "");
		break;
	}
	/*printf("撤单ing,ins: %s, vol: %d, price:%f, orderref:%s,requestid:%d,traded vol: %d,ExchID:%s, OrderSysID:%s,status: %s,statusmsg:%s\n"
		, pOrder->InstrumentID, pOrder->VolumeTotalOriginal, pOrder->LimitPrice, pOrder->OrderRef, pOrder->RequestID
		, pOrder->VolumeTraded, pOrder->ExchangeID, pOrder->OrderSysID, pszStatus, pOrder->StatusMsg);*/
	//cerr << "TdSpi::CancelOrder 撤单ing" << pszStatus << endl;
	if (nRetCode != 0) {
		printf("cancel order failed.\n");
	}
	else
	{
		pOrder->OrderStatus = '6';//‘6’表示撤单途中
		cerr << "TdSpi::CancelOrder 撤单ing" << pszStatus << endl;
		cerr << "TdSpi::CancelOrder 状态改为 pOrder->OrderStatus :" << pOrder->OrderStatus << endl;
	}

	UpdateOrder(pOrder);
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
	CThostFtdcOrderField* pOrder = NULL;//创建一个报单结构体指针
	std::lock_guard<std::mutex> m_lock(m_mutex);//加锁
	map<int, CThostFtdcOrderField*>::iterator it = m_Orders.find(nBrokerOrderSeq);//
	//找到了
	if (it != m_Orders.end()) {
		pOrder = it->second;
	}

	return pOrder;
}

bool TdSpi::UpdateOrder(CThostFtdcOrderField* pOrder)
{
	//经纪公司的下单序列号,大于0表示已经接受报单
	if (pOrder->BrokerOrderSeq > 0) 
	{
		std::lock_guard<std::mutex> m_lock(m_mutex);//加锁，保证这个映射数据的安全
		//迭代器，查找是否有这个报单
		map<int, CThostFtdcOrderField*>::iterator it = m_Orders.find(pOrder->BrokerOrderSeq);
		//如果存在，我们需要更新它的状态
		if (it != m_Orders.end()) 
		{
			CThostFtdcOrderField* pOld = it->second;//把结构体的指针赋值给pOld
			//原报单已经关闭；
			char cOldStatus = pOld->OrderStatus;
			switch (cOldStatus) 
			{
			case THOST_FTDC_OST_AllTraded://全部成交
			case THOST_FTDC_OST_Canceled://已撤单
			case '6'://canceling//自己定义的，本程序已经发送了撤单请求，还在途中
			case THOST_FTDC_OST_Touched://已经触发
				return false;
			}
			memcpy(pOld, pOrder, sizeof(CThostFtdcOrderField));//更新报单的状态
			cerr << "TdSpi::UpdateOrder pOrder->OrderStatus :" << (it->second)->OrderStatus << endl;
			
			
		}
		//如果不存在，我们需要插入这个报单信息
		else 
		{
			CThostFtdcOrderField* pNew = new CThostFtdcOrderField();
			memcpy(pNew, pOrder, sizeof(CThostFtdcOrderField));
			m_Orders.insert(pair<int, CThostFtdcOrderField*>(pNew->BrokerOrderSeq, pNew));
		}
		return true;
	}
	//否则的话就不用加入到映射
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
	
	int nNextID = g_nRequestID++;

	return g_nRequestID;
}

void TdSpi::ShowInstMessage()
{
	//std::map<std::string, CThostFtdcInstrumentField*> m_inst_field_map;

	for (std::map<std::string, CThostFtdcInstrumentField*>::iterator iter = m_inst_field_map.begin(); iter != m_inst_field_map.end(); iter++)
	{
		CThostFtdcInstrumentField* pInstrument = iter->second;
		
		cerr << "响应 | 合约：" << pInstrument->InstrumentID
			<< "合约名称：" << pInstrument->InstrumentName
			<< " 合约在交易所代码：" << pInstrument->ExchangeInstID
			<< " 产品代码：" << pInstrument->ProductID
			<< " 产品类型：" << pInstrument->ProductClass
			<< " 多头保证金率：" << pInstrument->LongMarginRatio
			<< " 空头保证金率：" << pInstrument->ShortMarginRatio
			<< " 合约数量乘数：" << pInstrument->VolumeMultiple
			<< " 最小变动价位：" << pInstrument->PriceTick
			<< " 交易所代码：" << pInstrument->ExchangeID
			<< " 交割年份：" << pInstrument->DeliveryYear
			<< " 交割月：" << pInstrument->DeliveryMonth
			<< " 创建日：" << pInstrument->CreateDate
			<< " 到期日：" << pInstrument->ExpireDate
			<< " 上市日：" << pInstrument->OpenDate
			<< " 开始交割日：" << pInstrument->StartDelivDate
			<< " 结束交割日：" << pInstrument->EndDelivDate
			<< " 合约生命周期状态：" << pInstrument->InstLifePhase
			<< " 当前是否交易：" << pInstrument->IsTrading << endl;
	}
}

bool TdSpi::FindTrade(CThostFtdcTradeField* pTrade)
{
	//ExchangeID //交易所代码
	//	TradeID   //成交编号
	//	Direction  //买卖方向
	std::lock_guard<std::mutex> m_lock(m_mutex);//加锁，保证这个set数据的安全
	for (auto it = tradeList.begin(); it != tradeList.end(); it++)
	{
		//判断是否已经存在
		if (strcmp((*it)->ExchangeID, pTrade->ExchangeID) ==0&& 
			strcmp((*it)->TradeID, pTrade->TradeID) == 0&& (*it)->Direction== pTrade->Direction)
			return true;
	}

	return false;
}

void TdSpi::InsertTrade(CThostFtdcTradeField* pTrade)
{
	std::lock_guard<std::mutex> m_lock(m_mutex);//加锁，保证这个set数据的安全
	CThostFtdcTradeField* trade = new CThostFtdcTradeField();//创建trade，分配堆空间，记得在析构函数里面要delete
	memcpy(trade, pTrade, sizeof(CThostFtdcTradeField));//pTrade复制给trade
	tradeList.push_back(trade);//输入录入
}

void TdSpi::ShowTradeList()
{
	std::lock_guard<std::mutex> m_lock(m_mutex);//加锁，保证这个set数据的安全
	cerr << endl << endl;
	cerr << "---------------打印成交明细-------------" << endl;
	for (auto iter = tradeList.begin(); iter != tradeList.end(); iter++)
	{
		cerr << endl << "投资者代码：" << (*iter)->InvestorID << "  "
			<< "用户代码：" << (*iter)->UserID << "  " << "成交编号：" << (*iter)->TradeID << "  "
			<< "合约代码：" << (*iter)->InstrumentID << "  " << "买卖方向：" << (*iter)->Direction << "  "
			<< "开平：" << (*iter)->OffsetFlag << "  " << "投机/套保" << (*iter)->HedgeFlag << "  "
			<< "价格：" << (*iter)->Price << "  " << "数量：" << (*iter)->Volume << "  "
			<< "报单引用：" << (*iter)->OrderRef << "  " << "本地报单编号：" << (*iter)->OrderLocalID << "  "
			<< "成交时间：" << (*iter)->TradeTime << "  " << "业务单元：" << (*iter)->BusinessUnit << "  "
			<< "序号：" << (*iter)->SequenceNo << "  " << "经纪公司下单序号：" << (*iter)->BrokerOrderSeq << "  "
			<< "交易日：" << (*iter)->TradingDay << endl;
	}

	cerr << endl << endl;
}

void TdSpi::Release()
{
	//当天的所有成交
	//std::vector<CThostFtdcTradeField*> tradeList;
	for (auto it = tradeList.begin(); it != tradeList.end(); it++)
	{
		delete (*it);
		*it = nullptr;
	}
	tradeList.clear();
}

void TdSpi::PlaceOrder(const char* pszCode, const char* pszExchangeID, int nDirection, int nOpenClose, int nVolume, double fPrice)
{
	//创建报单结构体对象
	CThostFtdcInputOrderField orderField;
	//初始化清零
	memset(&orderField, 0, sizeof(CThostFtdcInputOrderField));

	//fill the broker and user fields;
	
	strcpy(orderField.BrokerID, m_BrokerId.c_str());//复制我们的brokerid到结构体对象
	strcpy(orderField.InvestorID, m_UserId.c_str());//复制我们的InvestorID到结构体对象

	//set the Symbol code;
	strcpy(orderField.InstrumentID, pszCode);//设置下单的期货合约
	CThostFtdcInstrumentField* instField = m_inst_field_map.find(pszCode)->second;
	/*if (instField)
	{
		const char* ExID = instField->ExchangeID;
		strcpy(orderField.ExchangeID, ExID);
	}*/
	strcpy(orderField.ExchangeID, pszExchangeID);//交易所代码，"SHFE","CFFEX","DCE","CZCE","INE"
	if (nDirection == 0) {
		orderField.Direction = THOST_FTDC_D_Buy;
	}
	else {
		orderField.Direction = THOST_FTDC_D_Sell;
	}

	orderField.LimitPrice = fPrice;//价格

	orderField.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;//投机还是套利，套保，做市等

	
	
	orderField.VolumeTotalOriginal = nVolume;//下单手数

	//--------------1、价格条件----------------
		//限价单；
	orderField.OrderPriceType = THOST_FTDC_OPT_LimitPrice;//报单的价格类型条件

	//市价单；
	//orderField.OrderPriceType=THOST_FTDC_OPT_AnyPrice;
	//中金所市价
	//orderField.OrderPriceType = THOST_FTDC_OPT_FiveLevelPrice;

	//--------------2、触发条件----------------
	//	///立即
//#define THOST_FTDC_CC_Immediately '1'
/////止损
//#define THOST_FTDC_CC_Touch '2'
/////止赢
//#define THOST_FTDC_CC_TouchProfit '3'
/////预埋单
//#define THOST_FTDC_CC_ParkedOrder '4'
/////最新价大于条件价
//#define THOST_FTDC_CC_LastPriceGreaterThanStopPrice '5'
/////最新价大于等于条件价
//#define THOST_FTDC_CC_LastPriceGreaterEqualStopPrice '6'
/////最新价小于条件价
//#define THOST_FTDC_CC_LastPriceLesserThanStopPrice '7'
/////最新价小于等于条件价
//#define THOST_FTDC_CC_LastPriceLesserEqualStopPrice '8'
/////卖一价大于条件价
//#define THOST_FTDC_CC_AskPriceGreaterThanStopPrice '9'
/////卖一价大于等于条件价
//#define THOST_FTDC_CC_AskPriceGreaterEqualStopPrice 'A'
/////卖一价小于条件价
//#define THOST_FTDC_CC_AskPriceLesserThanStopPrice 'B'
/////卖一价小于等于条件价
//#define THOST_FTDC_CC_AskPriceLesserEqualStopPrice 'C'
/////买一价大于条件价
//#define THOST_FTDC_CC_BidPriceGreaterThanStopPrice 'D'
/////买一价大于等于条件价
//#define THOST_FTDC_CC_BidPriceGreaterEqualStopPrice 'E'
/////买一价小于条件价
//#define THOST_FTDC_CC_BidPriceLesserThanStopPrice 'F'
/////买一价小于等于条件价
//#define THOST_FTDC_CC_BidPriceLesserEqualStopPrice 'H'

	orderField.ContingentCondition = THOST_FTDC_CC_Immediately;//报单的触发条件
	//orderField.ContingentCondition = THOST_FTDC_CC_LastPriceGreaterThanStopPrice;//报单的触发条件
	//orderField.StopPrice = 5035.0;//



	//--------------3、时间条件----------------
	//orderField.TimeCondition = THOST_FTDC_TC_IOC;

//	///立即完成，否则撤销
//#define THOST_FTDC_TC_IOC '1'
/////本节有效
//#define THOST_FTDC_TC_GFS '2'
/////当日有效
//#define THOST_FTDC_TC_GFD '3'
/////指定日期前有效
//#define THOST_FTDC_TC_GTD '4'
/////撤销前有效
//#define THOST_FTDC_TC_GTC '5'
/////集合竞价有效
//#define THOST_FTDC_TC_GFA '6'


	//orderField.TimeCondition = THOST_FTDC_TC_GFS;//时间条件,本节有效,------没法用，错单，上期所、中金所：不被支持的报单类型
	//orderField.TimeCondition = THOST_FTDC_TC_GTD;//时间条件,指定日期有效,错单，不被支持的报单类型
	//orderField.TimeCondition = THOST_FTDC_TC_GTC;//时间条件,撤销前有效,错单，不被支持的报单类型
	//orderField.TimeCondition = THOST_FTDC_TC_GFA;//时间条件,集合竞价有效,错单，不被支持的报单类型


	//orderField.TimeCondition = THOST_FTDC_TC_IOC;//时间条件，立即完成，否则撤销
	orderField.TimeCondition = THOST_FTDC_TC_GFD;//时间条件,当日有效


	//--------------4、数量条件----------------
	orderField.VolumeCondition = THOST_FTDC_VC_AV;//任意数量

	//orderField.VolumeCondition = THOST_FTDC_VC_MV;//最小数量
	//orderField.VolumeCondition = THOST_FTDC_VC_CV;//全部数量


	

	strcpy(orderField.GTDDate, m_cTradingDay);//下单的日期



	orderField.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;//强平原因

	switch (nOpenClose) {
	case 0:
		orderField.CombOffsetFlag[0] = THOST_FTDC_OF_Open;//开仓
		break;
	case 1:
		orderField.CombOffsetFlag[0] = THOST_FTDC_OF_Close;//平仓
		break;
	case 2:
		orderField.CombOffsetFlag[0] = THOST_FTDC_OF_CloseToday;//平今仓
		break;
	case 3:
		orderField.CombOffsetFlag[0] = THOST_FTDC_OF_CloseYesterday;//平昨仓
		break;
	}


	//调用交易的api的ReqOrderInsert
	int retCode = m_pUserTDApi_trade->ReqOrderInsert(&orderField, GetNextRequestID());

	if (retCode != 0) {
		printf("failed to insert order,instrument: %s, volume: %d, price: %f\n", pszCode, nVolume, fPrice);
	}
}
