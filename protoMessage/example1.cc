#include "bike.pb.h"
#include <string>
#include <iostream>

using namespace std;
using namespace tutorial;
int main(void)
{
    string data; //存储序列化的消息	
    //客户端发送请求
    {
    	list_account_records_response larr;//定义一个类
	larr.set_code(200);
	larr.set_desc("OK");

	for(int i = 0; i < 5; i++)
	{
	    list_account_records_response_account_record* ar = larr.add_records();
	    ar->set_type(0);
	    ar->set_limit(i*100);
	    ar->set_timestamp(time(NULL));//当前时间戳
	}
	cout<<"records size:"<<larr.records_size()<<endl;
	larr.SerializeToString(&data);//将mobile的数据格式化成二进制流到data
	//cout<<"序列化以后:"<<data<<endl;
	//客户端发送data send(socked, data.c_str(), data.length());
	
    }

    //服务端接收请求
    {
	//receive(socket, data, ....)
    	//mobile_request mr;
	//mr.ParseFromString(data);//解析data
	//cout<<"客户端电话号码:"<<mr.mobile()<<endl;
	list_account_records_response larr;
	larr.ParseFromString(data);
	cout<<"records size:"<<larr.records_size()<<endl;
    	cout<<"code:"<<larr.code()<<endl;
	for(int i = 0; i < larr.records_size(); i++)
	{
	    const list_account_records_response_account_record& ar = larr.records(i);
	    cout<<"limit:"<<ar.limit()<<endl;
	}
    }
	

    return 0;
}
