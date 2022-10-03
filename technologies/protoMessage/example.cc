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
    	mobile_request mr;//定义一个类
	mr.set_mobile("13559866666");

	mr.SerializeToString(&data);//将mobile的数据格式化成二进制流到data
	cout<<"序列化以后:"<<data<<endl;
	//客户端发送data send(socked, data.c_str(), data.length());
	
    }

    //服务端接收请求
    {
	//receive(socket, data, ....)
    	mobile_request mr;
	mr.ParseFromString(data);//解析data
	cout<<"客户端电话号码:"<<mr.mobile()<<endl;
    }
	

    return 0;
}
