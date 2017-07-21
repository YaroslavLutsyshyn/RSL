/*
Test for POD send functionality
*/

#include <cassert>
#include "rsl.h"

struct S {
	int x;
	float y;
};

struct BSTRUNCT {  // ugly name on purpose, the summary type name is hashed
	double x;
	float y;
	S s;
};


std::ostream & operator<<(std::ostream &os, const S& s) { return os << "S: " << s.x << " and " << s.y << " | ";  };  
std::ostream & operator<<(std::ostream &os, const BSTRUNCT& b) { return os << "B: " << b.x << " and " << b.y <<" and " << b.s << " | ";  };  

bool operator== (const S &s1, const S &s2) {
	return s1.x==s2.x && s1.y==s2.y;
};

bool operator== (const BSTRUNCT &b1, const BSTRUNCT &b2) {
	return b1.x==b2.x && b1.y==b2.y && b1.s==b2.s;
};


int main() {

	rsl::mpi_env mpi;
	rsl::mpi_communicator comm;  

	int xs,xd;
	double ys,yd;

	struct S ss, sd;
	struct BSTRUNCT bs,bd;

	std::vector<double> vs (12);
	std::vector<unsigned short> vus (5);
	std::vector<double> vd (12);
	std::vector<unsigned short> vud (5);

	if(comm.rank==2) {

		xs=12312435;
		ys=1.e-57;

		ss.x=500;
		ss.y=5.5;

		bs.x=12.2;
		bs.y=0.15;
		bs.s.x=165;
		bs.s.y=5.f;

		vs = std::vector<double> (12,11.2);
		vus = std::vector<unsigned short> (5,4);
	}

	if(comm.rank==3) {

		xd=12312435;
		yd=1.e-57;

		sd.x=500;
		sd.y=5.5;

		bd.x=12.2;
		bd.y=0.15;
		bd.s.x=165;
		bd.s.y=5.f;

		vd = std::vector<double> (12,11.2);
		vud = std::vector<unsigned short> (5,4);
	}


	rsl::mpi_pod_send<int,double,S,BSTRUNCT,std::vector<double>,
	    std::vector<unsigned short int>>
	    v(comm,2,3,0,xs,ys,ss,bs,vs,vus);

	v.use(xs,ys,ss,bs,vs,vus);

	if(comm.rank==3) {

		std::cout << xs << "\t" << xd << "\n";
		std::cout << bs << "\t" << bd << "\n";

		std::cout << "CHECK: via asserts \n";
		assert(xs==xd);
		assert(ys==yd);
		assert(ss==sd);
		assert(bs==bd);
		assert(vs==vd);
		assert(vus==vud);

		std::cout << "CHECK: OK if reached here \n";		
	}

}

