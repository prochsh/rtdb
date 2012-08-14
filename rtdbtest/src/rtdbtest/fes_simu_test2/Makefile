.SUFFIXES: .o .C .cxx

include $(APPHOMEDIR)/platform.conf

VPATH=$(MAKEPATH)/src

C++FLAGS=`$(APPHOMEDIR)/platform.mak c++flags`
#C++FLAGS+= -DSIMFES_DEBUG

INCDIR=`$(APPHOMEDIR)/platform.mak incdir` -I$(MAKEPATH)/include

PROCLIBS=-L$(LIBDIR) $(LIBS) $(SHMLIB) $(CDBLIB) $(MSGLIB) $(SOCKLIB) $(IOLIB) $(MDFLIB) $(ALMLIB)

ifdef Wall
	C++FLAGS+=-Wall
endif

.cxx.o:
	@echo $(CC) $(C++FLAGS) $(INCDIR) -c $<
	@     $(CC) $(C++FLAGS) $(INCDIR) -c $<


#make fes_simu
FES_SIMULATE_CXX=Simulate_Fes_init.cxx Simulate_Fes_main.cxx Simulate_Fes_trash.cxx Simulate_Fes_ana.cxx  Simulate_Fes_poi.cxx Simulate_Fes_queue.cxx Simulate_Fes_send_ana.cxx Simulate_Fes_send_ana.cxx
FES_SIMULATE_OBJ=$(patsubst %.cxx,%.o,$(FES_SIMULATE_CXX))
fes_simu_test:$(FES_SIMULATE_OBJ)
	@echo $(CC) -o $(SRCBINDIR)/$@ $^ $(PROCLIBS)
	@     $(CC) -o $(SRCBINDIR)/$@ $^ $(PROCLIBS)

#make fes_simu_test_clean
fes_simu_clean:
	$(RM) -f $(FES_SIMULATE_OBJ)
