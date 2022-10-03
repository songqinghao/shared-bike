#include <iostream>
#include "bike.pb.h"
#include "event.h"
#include "events_def.h"
#include "user_event_handler.h"

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
    uehl.handle(&me);
    return 0;  
}



