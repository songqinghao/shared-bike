#include <iostream>
#include <unistd.h>
#include "bike.pb.h"
#include "ievent.h"
#include "events_def.h"
#include "user_event_handler.h"
#include "DispatchMsgService.h"
using namespace std;
int main()
{
    tutorial::mobile_request msg;
    msg.set_mobile("13559851513");
    iEvent *ie = new iEvent(EEVENTID_GET_MOBILE_CODE_REQ, 2);

    MobileCodeReqEv me("1359851511");
    me.dump(cout);
    cout << endl << msg.mobile() << endl;


    MobileCodeRspEv mcre(200, 666666);
    mcre.dump(cout);

    UserEventHandler uehl;
    //uehl.handle(&me);
    DispatchMsgService*DMS = DispatchMsgService::getInstance();
    DMS->open();//单例模式open
    //创建一个请求事件
    MobileCodeReqEv*pmcre = new MobileCodeReqEv("13559851513");
    //投递进去
    DMS->enqueue(pmcre);
    sleep(5);
    DMS->close();
    sleep(5);
    return 0;  
}



