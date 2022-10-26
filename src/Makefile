INCLUDE=-I /home/sqh/qiniuLearning/shared-bike/src/third/include
INCLUDE_PATH_C=/home/sqh/qiniuLearning/shared-bike/src/third/include
##include /home/sqh/qiniuLearning/shared-bike/src/third/include/log4cpp/*.h
LIB_PATH_INI=-L/home/sqh/qiniuLearning/shared-bike/src/third/lib/iniparse/*
LIB_PATH_LOG=-L/home/sqh/qiniuLearning/shared-bike/src/third/lib/log4cpp/*
LIB_PATH_LOGCPP=-L/home/sqh/qiniuLearning/log4cpp/log4cpp/src/.libs
# LIB_PATH_LOG_TH=-L

# ini.o:
# 	gcc ${INCLUDE}/iniparser/iniparser.c ${INCLUDE}/iniparser/dictionary.c -llog4cpp ${LIB_PATH_INI} ${LIB_PATH_LOG} -c ini.o 

bike.exe:
	g++  -std=c++11   user_service.cpp BusProcessor.cpp sqlconnection.cpp ${INCLUDE_PATH_C}/iniparser/iniparser.c ${INCLUDE_PATH_C}/iniparser/dictionary.c Logger.cpp iniconfig.cpp NetworkInterface.cpp DispatchMsgService.cpp main.cpp ievent.cpp events_def.cpp eventtype.cpp user_event_handler.cpp bike.pb.cpp thread_pool.cpp thread_cond.cpp thread_mutex.cpp -llog4cpp -lprotobuf -pthread -levent -lmysqlclient ${INCLUDE} ${LIB_PATH_LOG} ${LIB_PATH_INI}   -o bike.exe
# bike.exe: ini.o
# 	g++  -std=c++11   NetworkInterface.cpp DispatchMsgService.cpp main.cpp ievent.cpp events_def.cpp eventtype.cpp user_event_handler.cpp bike.pb.cpp thread_pool.cpp thread_cond.cpp thread_mutex.cpp -lprotobuf -pthread -levent ini.o -o bike.exe
clear:
	rm -r bike.exe

