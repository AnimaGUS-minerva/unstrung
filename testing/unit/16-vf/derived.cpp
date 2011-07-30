#include "base.h"
#include "derived.h"

#include <iostream>
using namespace std;

void Derived::vf(const char *f) {
	cout << "Derived vf(" << f << ")" << endl;
}

