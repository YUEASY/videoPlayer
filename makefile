http_sever:http_sever.cc
	g++  $^ -o $@ -L/usr/lib64/mysql -ljsoncpp -lmysqlclient -lpthread -std=c++11
.PHONY:clean
clean:
	rm -f http_sever