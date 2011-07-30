class Base {
public:
	void f(const char *f);
	virtual void vf(const char *f);
};

class Derived : public Base {
public:
	void f(const char *f);
	void vf(const char *f);
};

#include <iostream>
using namespace std;

void Base::f(const char *f) {
	cout << "Base f(" << f << ")" << endl;
        this->vf(f);
}

void Base::vf(const char *f) {
	cout << "Base vf(" << f << ")" << endl;
}

void Derived::f(const char *f) {
	cout << "Derived f(" << f << ")" << endl;
}

void Derived::vf(const char *f) {
	cout << "Derived vf(" << f << ")" << endl;
}

int main()
{
	Derived d2;  	// Derived object
	Base* bp = &d2; // Base pointer to Derived object
	bp->f("five");	// Base f()
	bp->vf("six");	// which vf()?

	return 0;
}
