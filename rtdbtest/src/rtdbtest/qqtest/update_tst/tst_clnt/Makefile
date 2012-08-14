CXXFLAGS = -I/home/d5000/ems/src/include/ -I/home/d5000/ems/db_def/ \
		   -L/home/d5000/ems/lib -lrtdb_api -lpthread
clnt_tst:main.c
	$(CXX) $(CXXFLAGS) -o $@ $^
clean:
	rm -f clnt_tst
