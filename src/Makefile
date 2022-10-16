bike.exe:
	g++  -std=c++11   NetworkInterface.cpp DispatchMsgService.cpp main.cpp ievent.cpp events_def.cpp eventtype.cpp user_event_handler.cpp bike.pb.cpp thread_pool.cpp thread_cond.cpp thread_mutex.cpp -lprotobuf -pthread -levent -o bike.exe
clear:
	rm -r bike.exe
