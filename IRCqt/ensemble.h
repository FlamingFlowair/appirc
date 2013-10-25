#ifndef ENSEMBLE_H
#define ENSEMBLE_H

#include <sys/select.h>

class Ensemble {
	private:
		fd_set unfd;
		int max;
	public:
		Ensemble();
		fd_set getfd_set() const;
		void setfd_set(fd_set unset);
		int getmax() const;
		void add(int fd);
		void clr(int fd);
		void raz();
		int recherchemax();
};

#endif // ENSEMBLE_H
