#include "base.h"
#include <iostream>
using namespace std;

void Base::f(const char *f) {
	cout << "Base f(" << f << ")" << endl;
        this->vf(f);
}

void Base::vf(const char *f) {
	cout << "Base vf(" << f << ")" << endl;
}

