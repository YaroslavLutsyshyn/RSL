
// include file with mpi_pod_send templates

//===========================================================================80

namespace rsl {

template<typename...A>
mpi_pod_send<A...>::mpi_pod_send(mpi_communicator comm,const int sender,
	                    const int receiver, const int tag, const A&... data)
:
	is_sender {comm.rank==sender},
	is_receiver {comm.rank==receiver},
	request_cleared {0}
{

	if ( !is_sender && !is_receiver )
		return;

	argstack = 
		std::unique_ptr<serializer::podstack<A...>>
		(
			new serializer::podstack<A...>(data...)
		);

	if( is_sender)
		argstack->copyin(data...);

	if( sender==receiver ) 	{
		request_cleared=1;
		return;
	}

	int hashedtag = argstack->hash(tag);

	if(hashedtag<0) 
		throw std::runtime_error("negative hashed tag"); 

	if(is_sender) 
		MPI_Isend(
			argstack->getbuf(),
			argstack->getcount(),
			mpi_type<typename serializer::podstack<A...>::buffer_type>(),
			receiver,
			hashedtag, 
			comm(),
			&request);

	if(is_receiver) 
		MPI_Irecv(
			argstack->getbuf(),
			argstack->getcount(),
			mpi_type<typename serializer::podstack<A...>::buffer_type>(),
			sender,
			hashedtag,
			comm(),
			&request);
}


template<typename...A>
mpi_pod_send<A...>::~mpi_pod_send() {
	clear();
}

template<typename...A>
void mpi_pod_send<A...>::clear_sender(void) {

	if(is_sender && !request_cleared) { 
		MPI_Wait(&request,MPI_STATUS_IGNORE);
		request_cleared=1;
	}
}

template<typename...A>
void mpi_pod_send<A...>::clear_receiver(void) {

	if(is_receiver && !request_cleared) {
		MPI_Wait(&request,MPI_STATUS_IGNORE);	
		request_cleared=1;
	}
}

template<typename...A>
void mpi_pod_send<A...>::clear(void) {
	clear_receiver();
	clear_sender();
}

template<typename...A>
void mpi_pod_send<A...>::use(A&... data) {
	if(is_receiver) {
		clear_receiver();
		argstack->copyout(data...);
	}
}

} // close rsl namespace 