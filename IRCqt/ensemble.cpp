#include "ensemble.h"
#include <iostream>

using namespace std;


Ensemble::Ensemble(){
	max=0;
	FD_ZERO(&unfd);
}

fd_set Ensemble::getfd_set() const {
	return unfd;
}

void Ensemble::setfd_set(fd_set unset) {
	unfd=unset;
}

int Ensemble::getmax() const {
	return max;
}

void Ensemble::add(int fd) {
	FD_SET(fd, &unfd);
	if (fd >= max) {
		max=fd+1;
	}
}

void Ensemble::clr(int fd) {
	cout << "I" << endl;
	FD_CLR(fd, &unfd);
	cout << "II" << endl;
	if ((fd+1) == max) {
		cout << "III" << endl;
		max=recherchemax()+1;
		cout << "IV" << endl;
	}
	cout << "V" << endl;
}

void Ensemble::raz() {
	FD_ZERO(&unfd);
}

int Ensemble::recherchemax() {
	int max;
	cout << "01" << endl;
	for(int i=0;i<65535; ++i) {
		if (FD_ISSET(i, &unfd) != 0) {
			max=i;
			cout << "0newmax : " << max << endl;
		}
	}
	cout << "0a" << endl;
	return max;
}
