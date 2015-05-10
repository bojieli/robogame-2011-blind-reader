#include <boost/asio.hpp>
#include <unistd.h>
boost::asio::io_service *iosev = 0;
boost::asio::ip::tcp::socket *sock = 0;

void read_data(unsigned char *buf, int size){
	boost::asio::read(*sock, boost::asio::buffer(buf, size));
}
void write_data(unsigned char *buf, int size){
	boost::asio::write(*sock, boost::asio::buffer(buf, size));
}
void init_connection() {
	iosev = new boost::asio::io_service;
	sock = new boost::asio::ip::tcp::socket(*iosev);
	boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address_v4::from_string("127.0.0.1"), 40000);
	boost::system::error_code ec;
	sock->connect(ep, ec);
	if (ec){
		throw 1;
	}else{
		pid_t pid = getpid();
		write_data((unsigned char *)&pid,sizeof(pid_t));
	}
}
int main(){
	init_connection();
	pid_t pid = 9999;
	write_data((unsigned char *)&pid,sizeof(pid_t));
	sleep(999999);
}
