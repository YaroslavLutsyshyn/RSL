
// include file with mpi_send templates


//===========================================================================80

namespace rsl {

template<typename T, int N>
mpi_send<T,N>::mpi_send(mpi_communicator comm,T* data,const int sender,
	                    const int receiver,const int tag)
:
	num_elem {N},
	is_sender {comm.rank==sender},
	is_receiver {comm.rank==receiver},
	request_cleared {0}
{
	std::copy(data,data+num_elem,buf);

	if( sender==receiver ) 
	{
		request_cleared=1;
		return;
	}

	if(is_sender) 
		MPI_Isend(buf,N,mpi_type<T>(),receiver,tag,comm(),&request);

	if(is_receiver) 
		MPI_Irecv(buf,N,mpi_type<T>(),sender,tag,comm(),&request);
}

template<typename T, int N>
mpi_send<T,N>::~mpi_send()
{
	clear();
}

template<typename T, int N>
void mpi_send<T,N>::clear_sender(void)
{
	if(is_sender && !request_cleared) { 
		MPI_Wait(&request,MPI_STATUS_IGNORE);
		request_cleared=1;
	}
}

template<typename T, int N>
void mpi_send<T,N>::clear_receiver(void)
{
	if(is_receiver && !request_cleared) {
		MPI_Wait(&request,MPI_STATUS_IGNORE);	
		request_cleared=1;
	}
}

template<typename T, int N>
void mpi_send<T,N>::clear(void)
{
	clear_receiver();
	clear_sender();
}

template<typename T, int N>
void mpi_send<T,N>::use(T* data)
{
	if(is_receiver)
	{
		clear_receiver();
		std::copy(buf,buf+num_elem,data);
	}
}

} // close rsl namespace 