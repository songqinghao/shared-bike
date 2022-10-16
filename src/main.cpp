#include <iostream>
#include <unistd.h>
#include "bike.pb.h"
#include "ievent.h"
#include "events_def.h"
#include "user_event_handler.h"
#include "DispatchMsgService.h"
#include "NetworkInterface.h"
using namespace std;
int main()
{
    //创建一个消息请求
    tutorial::mobile_request msg;
    //对msg初始化
    msg.set_mobile("13559851513");
    //创建一个短信请求事件，
    iEvent *ie = new iEvent(EEVENTID_GET_MOBILE_CODE_REQ, 2);
    //创建短信请求事件，直接进行初始化，sn产生
    MobileCodeReqEv me("1359851511");
    //输出me的mobile
    me.dump(cout);
    cout << endl << msg.mobile() << endl;

    //创建短信回应事件，响应正确，并且返回验证码为666666
    MobileCodeRspEv mcre(200, 666666);
    mcre.dump(cout);

    UserEventHandler uehl;
    //uehl.handle(&me);
    DispatchMsgService*DMS = DispatchMsgService::getInstance();
    DMS->open();//单例模式open

    NetworkInterface* NTIF = new NetworkInterface();
    //设置端口号
    NTIF->start(8888);
    while(true)
    {
        NTIF->network_dispatch();
        //1s钟执行一次
        sleep(1);
        std::cout << "NetworkInterface_dispatch....." << std::endl;
    }
    //创建一个请求事件
    /*
    MobileCodeReqEv*pmcre = new MobileCodeReqEv("13559851513");
    //，投递进去
    DMS->enqueue(pmcre);
    */
    
    sleep(5);
    DMS->close();
    sleep(5);
    return 0;  
}



