#include<fstream>
#include<map>
#include<iostream>
using namespace std;
std::map<std::string, std::string> accountConfig_map;//保存账户信息的map

int main()
{
	std::ifstream file1(".\\config\\config.txt", ios::in);
	string fieldKey;
	string fieldValve;
	char dataLine[256];
	if (!file1)
	{
		cout << "配置文件不存在" << endl;
		return -1;
	}
	else
	{
		while (file1.getline(dataLine, sizeof(dataLine),'\n'))
		{
			int length = strlen(dataLine);
			char tmp[128];
			int i = 0, j = 0;
			while(dataLine[i] != ',') i++;
			if(i > j) fieldKey = dataLine[j, i];
			i++; 
			j = i; 
			while(dataLine[i] != '\0') i++;
			if(i > j) fieldValve = dataLine[j, i];
			/* 
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
					cout <<"count: "<<count<< ",tmp :" << tmp << endl;
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
			}//for结束
			*/
			accountConfig_map.insert(make_pair(fieldKey, fieldValve));


		}
	}


	file1.close();
		return 0;
}