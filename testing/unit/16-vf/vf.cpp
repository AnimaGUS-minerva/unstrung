#include "base.h"
#include "derived.h"

int main()
{
	Derived d2;  	// Derived object
	Base* bp = &d2; // Base pointer to Derived object
	bp->f("five");	// Base f()
	bp->vf("six");	// which vf()?

	return 0;
}
