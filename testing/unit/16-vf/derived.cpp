#include "base.h"
#include "derived.h"

#include <iostream>
using namespace std;

void Derived::f(const char *f) {
	cout << "Derived f(" << f << ")" << endl;
}

void Derived::vf(const char *f) {
	cout << "Derived vf(" << f << ")" << endl;
}

