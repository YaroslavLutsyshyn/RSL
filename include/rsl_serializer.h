
namespace rsl {
namespace serializer {
 
template<bool C, typename T, typename F>
using Conditional = typename std::conditional<C,T,F>::type;


template<typename T>
constexpr bool Is_pod() { return std::is_pod<T>::value; }


template<typename T>
struct container_size_string_of_pod {
	constexpr std::string operator()(const T& t) { 
		return "1"; 
	}
};


template<typename T>
struct container_size_string_of_pod_container {
	constexpr std::string operator()(const T& t) { 
		return std::to_string(t.size()); 
	}
};


template<typename T>
std::string unwind_stack_container_size_string(const T& t) {

	return Conditional<Is_pod<T>(),
		               container_size_string_of_pod<T>,
		               container_size_string_of_pod_container<T>>{}(t);
}


template<typename T, typename... A>
std::string unwind_stack_container_size_string(const T& t, const A&... a) {

	std::string s = unwind_stack_container_size_string(t);
	return s + unwind_stack_container_size_string(a...);
};


template<typename T>
struct size_of_pod {
	constexpr size_t operator()(const T& t) { 
		return sizeof(T); 
	}
};


template<typename T>
struct size_of_pod_container {
	constexpr size_t operator()(const T& t)	{ 
		return sizeof(typename T::value_type) * t.size(); 
	}
};


template<typename T>
constexpr size_t unwind_stack_get_size(const T& t) {

	return Conditional<Is_pod<T>(),
		               size_of_pod<T>,
		               size_of_pod_container<T>>{}(t);
};


template<typename T, typename... A>
size_t unwind_stack_get_size(const T& t, const A&... a) {

	size_t s = unwind_stack_get_size(t);
	return s + unwind_stack_get_size(a...);
};


template<typename T>
struct copyin_pod {
	void operator()(char * p, const T& t, const size_t s) { 
		memcpy( (void *) p, (void *) &t, s );
	}
};


template<typename T>
struct copyin_pod_container {

	using member_type = typename T::value_type;

	void operator()(char * p, const T& t, const size_t s) {

		const size_t z = sizeof(member_type);

		for(auto & iter : t) {
			memcpy( (void *) p, (void *) &iter, z );
			p+=z;
		};
	}
};


template<typename T>
struct copyout_pod {

	void operator()(char * p, T& t, const size_t s) { 
		memcpy( (void *) &t, (void *) p, s );
	}
};


template<typename T>
struct copyout_pod_container {

	using member_type = typename T::value_type;

	void operator()(char * p, T& t, const size_t s) { 
		const size_t z = sizeof(member_type);

		for(auto & iter : t) {
			memcpy( (void *) &iter, (void *) p, z );
			p+=z;
		};
	}
};


template<typename T>
char * unwind_stack_copyin(char *p, const T& t) {

	size_t s = unwind_stack_get_size(t);

	Conditional<Is_pod<T>(),
		               copyin_pod<T>,
		               copyin_pod_container<T>>{}(p,t,s);

	return p+s;
};


template<typename T, typename... A>
char * unwind_stack_copyin(char *p, const T& t, const A&... a) {

	p = unwind_stack_copyin(p,t);
	return unwind_stack_copyin(p,a...);
};

template<typename T>
char * unwind_stack_copyout(char *p, T& t) {

	size_t s = unwind_stack_get_size(t);

	Conditional<Is_pod<T>(),
		               copyout_pod<T>,
		               copyout_pod_container<T>>{}(p,t,s);

	return p+s;
};


template<typename T, typename... A>
char * unwind_stack_copyout(char *p, T& t, A&... a) {

	p = unwind_stack_copyout(p,t);
	return unwind_stack_copyout(p,a...);
};


template<typename T>
constexpr int make_int_hash (const T& s) {
	return abs(
			static_cast<int>(
				std::hash<T>{}(s)
			)
		) 
		& 0xafffffff;
}


template<typename... A>
podstack<A...>::podstack(const A&... a)
:
	size {unwind_stack_get_size(a...)},	

	small_type_hash {
		make_int_hash<std::string>(typeid(this).name()
			+unwind_stack_container_size_string(a...))
	},

	buf_heap {nullptr}

{ 
	if( size < rsl::on_stack_send_buffer_size ) {

		buf = buf_stack;
	} 
	else {

		buf_heap = new char[size];
		buf = buf_heap;
	}

};


template<typename... A>
void podstack<A...>::copyin(const A&... a) { 
	unwind_stack_copyin(buf,a...);
}


template<typename... A>
void podstack<A...>::copyout(A&... a) { 
	unwind_stack_copyout(buf,a...);
}


template<typename... A>
constexpr void * podstack<A...>::getbuf() const { 
	return (void * ) buf;
};


template<typename... A>
constexpr int podstack<A...>::getcount() const { 
	return static_cast<int>(size);
};


template<typename... A>
constexpr int podstack<A...>::hash(const int tag) { 		
	return tag + small_type_hash;
};


} // end of namespace serializer
} // end of namespace rsl