###本文件夹用于讲解proto序列化相关例子和知识点###
1.protobuf安装
wget  https://github.com/protocolbuffers/protobuf/releases/download/v3.7.1/protobuf-cpp-3.7.1.tar.gz
tar -zxvf protobuf-all-3.19.4.tar.gz
cd protobuf-3.19.4
./antogen.sh
./configure
make
make check
sudo make install
