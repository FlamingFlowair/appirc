#include "ensemble.h"

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
	FD_CLR(fd, &unfd);
	if ((fd+1) == max) {
		max=recherchemax()+1;
	}
}

void Ensemble::raz() {
	FD_ZERO(&unfd);
}

int Ensemble::recherchemax() {
	int max;
	for(int i=0;i<65535; ++i) {
		if (FD_ISSET(i, &unfd)) {
			max=i;
		}
	}
	return max;
}
