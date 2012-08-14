#ifndef _SIMULATE_FES_INIT_H_
#define _SIMULATE_FES_INIT_H_

#include "Simulate_Fes.hxx"

//MEASURE_SIMULATE_FES *measure_init_simulate_fes(void);
int SimFes_config(void);
int SimFes_init_link(void);
int SimFes_init_station(void);
int SimFes_init_multi_upana(void);
int SimFes_init_multi_uppoi(void);
int SimFes_sort_multi_upana_by_psid(void);
int SimFes_sort_multi_uppoi_by_psid(void);
int SimFes_init_upana(void);
int SimFes_init_uppoi(void);
int SimFes_sort_upana_by_psid(void);
int SimFes_sort_uppoi_by_psid(void);
int SimFes_uniq_upana_by_psid(void);
int SimFes_uniq_uppoi_by_psid(void);
void SimFes_split_ana_by_tab(void);
void SimFes_split_poi_by_tab(void);
void SimFes_sort_ana_per_tab(int i);
void SimFes_sort_poi_per_tab(int i);

#endif

