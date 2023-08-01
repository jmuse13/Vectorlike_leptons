std::unordered_map<std::string,Cut> final_states_nocuts;
final_states_nocuts["Two_emuOSSF_One_ta"] = Cuts::Nlight == 2 && Cuts::Ntau == 1 && Cuts::SS == 0 && Cuts::SF != 0;
final_states_nocuts["Two_emuOSOF_One_ta"] = Cuts::Nele == 1 && Cuts::Nmuon == 1 && Cuts::Ntau == 1 && Cuts::SS == 0;
final_states_nocuts["Two_emuSSSF_One_ta"] = Cuts::Nlight == 2 && Cuts::Ntau == 1 && Cuts::SS != 0 && Cuts::SF != 0;
final_states_nocuts["Two_emuSSOF_One_ta"] = Cuts::Nele == 1 && Cuts::Nmuon == 1 && Cuts::Ntau == 1 && Cuts::SS != 0;
final_states_nocuts["Two_emu_Two_ta"] = Cuts::Nlight == 2 && Cuts::Ntau >= 2;
final_states_nocuts["Three_emu_Zero_ta"] = Cuts::Nlight == 3 && Cuts::Ntau == 0;
final_states_nocuts["Three_emu_One_ta"] = Cuts::Nlight == 3 && Cuts::Ntau >= 1;
final_states_nocuts["Four_emu"] = Cuts::Nlight >= 4;

for(auto&& it : final_states_nocuts)
    final_states.push_back(new FinalState(it.first,std::move(it.second)));
