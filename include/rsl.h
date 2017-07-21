
#define RSL_HEADER_INCUDED
#include <mpi.h>
#include <iostream>
#include <string>
#include <exception>
#include <stdexcept>
#include <string.h>   // for memcpy()
#include <chrono>     // for timing
#include <memory>
#include <vector>
#include <algorithm>    
#include <sstream>
#include <iomanip>    // std::setw and other formatting
#include <functional> // for simple hashing
#include <typeinfo>
 

namespace rsl {

const int max_walltime_hrs_for_timestamp = 2; // 24*3
const int on_stack_send_buffer_size = 10000;

//===========================================================================80

class mpi_env {

private:

	void mpi_init(void);
	void mpi_finalize(void);

	int mpi_status_was_initialized(void);
	int mpi_status_was_finalized(void);

public:

	mpi_env() {
		mpi_init();
	}

	~mpi_env() {
		mpi_finalize();
	}
};

//===========================================================================80

class delayed_time {

public:

	int flag;

	std::chrono::high_resolution_clock::time_point start_time;
	std::chrono::duration<long long,std::nano> delay; 

	delayed_time()
	: 
		flag {0} 
	{};

	std::string ns_time_stamp(void);

};

//===========================================================================80

template <typename T> 
MPI_Datatype mpi_type(void);

//===========================================================================80

class mpi_communicator {

private:

	const MPI_Comm communicator;

public:

	delayed_time rank_time;

public:

	const int size;
	const int rank;

	inline MPI_Comm get(void) const {return communicator;}
	inline MPI_Comm operator()(void) const {return get();}

	void set_clocks(void);

	mpi_communicator(void);
};

std::ostream & operator<<(std::ostream &os, const mpi_communicator 
	& communicator);  

//===========================================================================80

// special kudos to a comment by user 
// David Rodríguez - dribeas
// https://stackoverflow.com/users/36565/david-rodr%C3%ADguez-dribeas
// https://stackoverflow.com/questions/15033827/multiple-threads-writing-to-stdcout-or-stdcerr

class tag {
    
private:

    std::ostringstream st;
    const std::string open_bracket = "[ ";
    const std::string close_bracket = " ***] ";

 public:
    template <typename T> 
    tag& operator<<(T const& t) {
       st << t;
       return *this;
    }

    tag(mpi_communicator comm) : 
    	tag(std::to_string(comm.rank),comm.rank_time.ns_time_stamp()) 
    {};

    tag() : tag("- ") {};

    tag(std::string rank) {
    	st << open_bracket << "rank: " << rank << close_bracket;
    };

    tag(std::string rank, std::string timestamp) {
    	st << open_bracket << "rank: " << rank << " " << timestamp 
    		<< close_bracket;
    };

	~tag() {
		std::string s = st.str();
		std::cout << s;
	}
};

//===========================================================================80

class timing {

private:

	int is_a;
	int is_b;

	int a,b;
	int reps;
	int N;
	std::vector <long long> a_to_b, b_to_a;

public:

	timing(mpi_communicator,int,int,int,int);
	std::chrono::duration<long long, std::nano> clockdelay();

};

//===========================================================================80

namespace serializer {

template<typename...A>
class podstack { 

public:

	using buffer_type = char;

private:

	// buffer_type is defined 
	// for external use of podstack<> only
	// char is hard-wired in podstack because
	// sizeof() is guaranteed to be in units 
	// of sizeof(char).

	char * buf;
	char   buf_stack [rsl::on_stack_send_buffer_size];
	char * buf_heap; 

public:

	constexpr void * getbuf() const;
	constexpr int getcount() const;

	const size_t size;
	const int small_type_hash;

	podstack(const A&...);
	void copyin(const A&...);
	void copyout(A&...);
	constexpr int hash(const int /*tag*/);
	~podstack() { if(buf_heap) delete buf_heap; }
};

} // close namespace serializer

//===========================================================================80


template<typename...A>
class mpi_pod_send {

private:

	const int is_sender;
	const int is_receiver;

	std::unique_ptr<serializer::podstack<A...>> argstack;

	MPI_Request request;

	int request_cleared;

public:

	mpi_pod_send(mpi_communicator,const int,const int,const int,const A&...);
    mpi_pod_send(const mpi_pod_send&)=delete; // copy constructor is removed
	~mpi_pod_send();
	void clear(void);
	void clear_sender(void);
	void clear_receiver(void);
	void use(A&...);
};

//===========================================================================80


template<typename T, int N>
class mpi_send {

private:

	const int num_elem;

	T buf[N];

	const int is_sender;
	const int is_receiver;

	MPI_Request request;

	int request_cleared;

public:

	mpi_send(mpi_communicator,T*,const int,const int,const int=0);
    mpi_send(const mpi_send&)=delete; // copy constructor is removed
	~mpi_send();
	void clear(void);
	void clear_sender(void);
	void clear_receiver(void);
	void use(T*);
};
 
//===========================================================================80

} // end of namespace

#include "rsl_mpi_pod_send.h"
#include "rsl_mpi_send.h"
#include "rsl_serializer.h"
