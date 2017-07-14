
#ifndef RSL_HEADER_INCUDED
#include "rsl.h"
#endif

#include <stdexcept>
#include <cassert>

using namespace rsl;

namespace rsl {

//===========================================================================80
// translation routines that convert calls into those returning a value
// should not be accessible to public, since standard ones already exist.

int mpi_env::mpi_status_was_initialized(void) {

	int flag;
	MPI_Initialized( &flag );

	return flag;
}

int mpi_env::mpi_status_was_finalized(void) {

	int flag;
	MPI_Finalized( &flag );

	return flag;
}

void mpi_env::mpi_init() {

    if( mpi_status_was_initialized() ) 
        throw std::runtime_error("MPI repeated initialization prohibited");

    int provided;
    MPI_Init_thread( nullptr , nullptr , MPI_THREAD_FUNNELED, &provided );

    if( provided!=MPI_THREAD_FUNNELED )
        throw std::runtime_error("Cannot start MPI with proper level "
            "of thread support");

    if( !mpi_status_was_initialized() )
        throw std::runtime_error("Failed to initialize MPI");
}

void mpi_env::mpi_finalize() {

    if( !mpi_status_was_initialized() ) 
        throw std::runtime_error("MPI must be initialized before finalizing");

    if( mpi_status_was_finalized() ) 
        throw std::runtime_error("MPI repeated finalization prohibited");

    MPI_Finalize();

    if( !mpi_status_was_finalized() ) 
        throw std::runtime_error("Failed to finalize MPI");

};

//===========================================================================80

int mpi_get_rank(MPI_Comm communicator) {

	int rank;
	MPI_Comm_rank( communicator, &rank );

	return rank;
}

int mpi_get_size(MPI_Comm communicator) {

	int size;
	MPI_Comm_size( communicator, &size );
	
	return size;
}

//===========================================================================80
// translation routines for built-in MPI datatypes

template <> MPI_Datatype mpi_type<short int>() {return MPI_SHORT;};  
template <> MPI_Datatype mpi_type<int>() {return MPI_INT;};  
template <> MPI_Datatype mpi_type<long int>() {return MPI_LONG;};  
template <> MPI_Datatype mpi_type<long long int>() {return MPI_LONG_LONG;};  
template <> MPI_Datatype mpi_type<unsigned char>() {
	return MPI_UNSIGNED_CHAR;
};  
template <> MPI_Datatype mpi_type<unsigned short int>()	{
	return MPI_UNSIGNED_SHORT;
};  
template <> MPI_Datatype mpi_type<unsigned int>() {return MPI_UNSIGNED;};  
template <> MPI_Datatype mpi_type<unsigned long int>() {
	return MPI_UNSIGNED_LONG;
};  
template <> MPI_Datatype mpi_type<unsigned long long int>() { 
	return MPI_UNSIGNED_LONG_LONG;
};  
template <> MPI_Datatype mpi_type<float>() {return MPI_FLOAT;};  
template <> MPI_Datatype mpi_type<double>() {return MPI_DOUBLE;};  
template <> MPI_Datatype mpi_type<long double>() {return MPI_LONG_DOUBLE;};  
template <> MPI_Datatype mpi_type<char>() {return MPI_BYTE;};  


//===========================================================================80


mpi_communicator::mpi_communicator(void) 
:
	communicator {MPI_COMM_WORLD},
	rank {mpi_get_rank(MPI_COMM_WORLD)}, 
	size {mpi_get_size(MPI_COMM_WORLD)} 
{
}


std::ostream & operator<<(std::ostream &os, const mpi_communicator &comm) { 
	os << "MPI Communicator: " << comm.get() << " "
	   << "rank/size " << comm.rank << " / " << comm.size;
    return os ;
};


void mpi_communicator::set_clocks(void) {

	using namespace std::chrono;
	using clock=std::chrono::high_resolution_clock;

	long long root_clock;

	if( rank==0 ) 
	{
		rank_time.start_time = clock::now();
		rank_time.delay = duration<long long,std::nano> {0};
		root_clock = time_point_cast<nanoseconds>(rank_time.start_time)
			.time_since_epoch().count();
	}

	for(int r=1; r<size; r++) {
		mpi_send<decltype(root_clock),1>(*this,&root_clock,0,r)
			.use(&root_clock);
	}

	if( rank!=0 ) 
	{
		auto d = duration<long long,std::nano> {root_clock};
		rank_time.start_time = clock::time_point{d};
	}


	for(int r=1; r<size; r++) {

		timing t(*this, r,0,1000,100);

		if( rank==r ) 
		{
			rank_time.delay = t.clockdelay();
		}
	}

	rank_time.flag = 1 ;

};

//===========================================================================80

std::string delayed_time::ns_time_stamp() {

	using namespace std::chrono;
	using clock=std::chrono::high_resolution_clock;

	if (flag==0) 
		return "";

	auto td = 
			duration_cast<nanoseconds>(
		      clock::now().time_since_epoch()-start_time.time_since_epoch()
		    )
			- delay;

	long long ns = td.count();
	long long chop_sec = ns / 1000000000l;
	long long chop_mks = ( ns % 1000000000l) / 1000l;
	long long chop_ns = ns % 1000;

	int max_sec = (max_walltime_hrs_for_timestamp*3600);
	int sec_pos = 1;

	while( max_sec /= 10 ) {
		++sec_pos;
	}

    std::ostringstream st;

    st  << "time: "
    	<< std::setw(sec_pos+9) << ns
    	<< " | "
    	<< std::setw(sec_pos) << chop_sec
    	<< " | "
    	<< std::setw(6) << chop_mks
    	<< " | "
    	<< std::setw(3) << chop_ns;

	//std::cout << st.str() <<"\n";

	return st.str();
};

//===========================================================================80

std::string error_code(int code) {

	//http://www.mcs.anl.gov/research/projects/mpi/mpi-standard
	// ... /mpi-report-1.1/node149.htm
	//has a list of all errors

	char err_string[MPI_MAX_ERROR_STRING];
	int len {0};
    int status = MPI_Error_string(code, err_string, &len);

    std::string msg;

    // Note that failure in MPI_Error_string() will in fact terminate 
    // the program

    if(status!=MPI_SUCCESS || len<=0) {
    	msg = "VERY ERROR, CANNOT EVEN INTERPRETE THE ERROR CODE: "
    		+std::to_string(code);
    	return msg;
    }

    msg=err_string;
    return msg;
};

//===========================================================================80

timing::timing(mpi_communicator comm,int _a,int _b,int _N,int _reps) 
:
	a {_a},
	b {_b},
	reps {_reps},
	N {_N},
	is_a {0},
	is_b {0}
{

	using namespace std::chrono;
	using clock=std::chrono::high_resolution_clock;
	
	clock::time_point a_sends;
	clock::time_point b_receives;
	clock::time_point b_sends;
	clock::time_point a_receives;

	void * mpi_buf;
	std::unique_ptr<int[]> u_buf;
	
	{
		int * tmp_buf = new int[N]; 
		mpi_buf = (void*) tmp_buf;
		u_buf = std::unique_ptr<int []>(tmp_buf);
	}

	for(int i=0; i<N; ++i) 
		u_buf[i]=( 1<<31 - i ) * i;

	if(comm.rank==a) 
		is_a=1;

	if(comm.rank==b) 
		is_b=1;

	for(int r=0; r<reps; ++r) {

		MPI_Barrier(comm());

		if(comm.rank==a) {
			a_sends=clock::now();
			MPI_Send(mpi_buf,N,mpi_type<int>(),b,0,comm());
		}
		else if(comm.rank==b) {
			MPI_Recv(mpi_buf,N,mpi_type<int>(),a,0,comm(),MPI_STATUS_IGNORE);
			b_receives=clock::now();
		}

		MPI_Barrier(comm());

		if(comm.rank==b) {
			b_sends=clock::now();
			MPI_Send(mpi_buf,N,mpi_type<int>(),a,0,comm());
		}
		else if(comm.rank==a) {
			MPI_Recv(mpi_buf,N,mpi_type<int>(),b,0,comm(),MPI_STATUS_IGNORE);
			a_receives=clock::now();
		}

		MPI_Barrier(comm());

		if(comm.rank==b) {

			long long b_times[2];

			b_times[0] = duration_cast<nanoseconds>
				(b_receives.time_since_epoch()).count();
			b_times[1] = duration_cast<nanoseconds>
				(b_sends.time_since_epoch()).count();

			MPI_Send(b_times,2,mpi_type<long long>(),a,0,comm());
		}
		else if(comm.rank==a) {

			long long b_times[2];
			MPI_Recv(b_times,2,mpi_type<long long>(),b,0,comm(),
				MPI_STATUS_IGNORE);

			long long a_times[2];
			a_times[0] = duration_cast<nanoseconds>
				(a_sends.time_since_epoch()).count();
			a_times[1] = duration_cast<nanoseconds>
				(a_receives.time_since_epoch()).count();

			a_to_b.push_back(b_times[0] - a_times[0]);
			b_to_a.push_back(b_times[1] - a_times[1]);

		}
	}

}



std::chrono::duration<long long,std::nano> timing::clockdelay() {

	if(!is_a) 
		return std::chrono::duration<long long,std::nano> {0};

	std::vector<long long> ab {a_to_b};
	std::vector<long long> ba {b_to_a};

	std::sort(ab.begin(),ab.end());
	std::sort(ba.begin(),ba.end());

	long long ab_sum = 0 , ba_sum = 0;

	int start = reps/3;
	int end = (2*reps)/3;
	int cnt = end-start;

	assert(end>start);

	for(int i = start; i < end; ++i) {

		ab_sum += ab[i];
		ba_sum += ba[i];
	}

    long long dt = (ab_sum+ba_sum)/cnt/2;
	long long delay = (ab_sum-ba_sum)/cnt/2;

	double to_noise = abs((double)dt) / sqrt( abs((double)delay) );

	if( to_noise < 3.0 ) {

		// the observed clock delay is most likely just coming from 
		// fluctuations in the send time.

		dt = 0;
	}

	return std::chrono::duration<long long,std::nano> {dt};
}

//===========================================================================80


//*******
};