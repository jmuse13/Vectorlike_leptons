local_total++;

Double_t weight = *VLL_weight * *pileup_weight;
if(!include_fake_mc){
    weight*= *trig_global_sf;
}
Double_t gen_weight = *generator_weight;
Double_t xsecweight = 1;
 

// Kill very large Sherpa weights
if(fabs(*generator_weight) > 100.0 && std::binary_search(vll_helpers::sherpa_dids.begin(),vll_helpers::sherpa_dids.end(),*mc_channel_number)) gen_weight = 1.0;

weight *= gen_weight;
try {
    xsecweight *= vll_helpers::lumi_weights.at(*mc_channel_number).at(readsystematic);
} catch(std::out_of_range e) {
    try {
        xsecweight *= vll_helpers::lumi_weights.at(*mc_channel_number).at("NOSYS");
    } catch(std::out_of_range e) {
        std::cerr << "WARNING: Could not retrieve lumi weight for DSID " << *mc_channel_number << std::endl;
    }
}
weight*= xsecweight;

if(!is_weight_syst) {
    mutex.lock();
    h_cutflows.at(systematic)->Fill("initial",1.0);
    h_wcutflows.at(systematic)->Fill("initial",weight);
    mutex.unlock();
}

if(*mc_channel_number==410470) continue;
if(*mc_channel_number==410646) continue;
if(*mc_channel_number==410647) continue;

// Trigger
bool ee_trig_passed = false;
bool mm_trig_passed = false;
bool em_trig_passed = false;
for(auto &trig_passed : ee_trigs_passed)
    ee_trig_passed = ee_trig_passed || *trig_passed;
for(auto &trig_passed : mm_trigs_passed)
    mm_trig_passed = mm_trig_passed || *trig_passed;
for(auto &trig_passed : em_trigs_passed)
    em_trig_passed = em_trig_passed || *trig_passed;
if(!ee_trig_passed && !mm_trig_passed && !em_trig_passed) continue;

if(!is_weight_syst) {
    mutex.lock();
    h_cutflows.at(systematic)->Fill("trigger",1.0);
    h_wcutflows.at(systematic)->Fill("trigger",weight);
    mutex.unlock();
}

Double_t LT = 0.0;
Int_t N_ele = 0;
Int_t N_muon = 0;
Int_t N_tau = 0;

std::vector<struct vll_helpers::lep> light_leps;
std::vector<struct vll_helpers::jet> jet_vec;
std::vector<struct vll_helpers::tau> tau_vec;
std::vector<struct vll_helpers::missinget> met_vec;
std::vector<float> fakes;
std::vector<float> light_fakes;
std::vector<float> tau_fakes;
std::vector<float> htau_fakes;

met_vec.push_back({TLorentzVector()});
met_vec.back().v.SetPtEtaPhiM(MET[met_idx],0,METphi[met_idx],0);

// jet selection
Int_t N_bjets = 0;
Int_t N_jets = 0;
Float_t HT = 0;
TLorentzVector light_jet_sum;
TLorentzVector jet_sum;
TLorentzVector jet_tau_sum;
for(int i = 0; i < jet_pt.GetSize(); i++) {
    if(jet_pt[i] < 20e3) continue;
    if(fabs(jet_eta[i]) > 2.5) continue;
    N_jets++;
    HT += jet_pt[i];
    TLorentzVector vt; vt.SetPtEtaPhiM(jet_pt[i],jet_eta[i],jet_phi[i],jet_m[i]);
    light_jet_sum += vt;
    jet_sum += vt;
    jet_tau_sum += vt;
    jet_vec.push_back({TLorentzVector(),jet_pt[i],jet_eta[i],0});
    jet_vec.back().v.SetPtEtaPhiM(jet_pt[i],jet_eta[i],jet_phi[i],jet_m[i]);
    if(jet_btag[i]){
        N_bjets++;
    }
}
weight *= *jet_btag_eff;

std::string ele_syst = "NOSYS";
for(int i = 0; i < vll_helpers::elsyst.size(); i++){
    if(systematic.compare("el_"+vll_helpers::elsyst.at(i)) == 0){
	ele_syst = vll_helpers::elsyst.at(i);
    }
}
// Electron selection
float light_charge = 0;
for(int i = 0; i < el_pt.GetSize(); i++) {
    if(el_pt[i] < 20e3) continue;
    if(fabs(el_eta[i]) > 2.47) continue;
    if(fabs(el_eta[i]) > 1.37 && fabs(el_eta[i]) < 1.52) continue;

    bool quality = el_likelihood[i] && el_isIsolated[i];
    bool truthmatched = (el_truthClassification[i] == 2) || (el_truthClassification[i] == 3);

    if(!is_data && !truthmatched && !include_fake_mc) continue;  
    if(!quality && !include_fake_mc) {
	fakes.push_back(vll_fake_helpers::get_el_ff(el_pt[i]/1000,el_eta[i],ele_syst));
        light_fakes.push_back(vll_fake_helpers::get_el_ff(el_pt[i]/1000,el_eta[i],ele_syst));
        N_ele++;
        LT += el_pt[i];
        light_leps.push_back({TLorentzVector(),el_charge[i],11});
        light_leps.back().v.SetPtEtaPhiM(el_pt[i],el_eta[i],el_phi[i],0.511);
	weight *= el_effSF_reco[i] * el_effSF_PID[i] * el_effSF_isol[i];
	light_charge += el_charge[i];
    }
    if(quality) {
        N_ele++;
        LT += el_pt[i];
        light_leps.push_back({TLorentzVector(),el_charge[i],11});
        light_leps.back().v.SetPtEtaPhiM(el_pt[i],el_eta[i],el_phi[i],0.511);
        weight *= el_effSF_reco[i] * el_effSF_PID[i] * el_effSF_isol[i] * el_effSF_chflip[i];
	light_charge += el_charge[i];
    }
}
std::string mu_syst = "NOSYS";
for(int i = 0; i < vll_helpers::musyst.size(); i++){
     if(systematic.compare("mu_"+vll_helpers::musyst.at(i)) == 0){
        mu_syst = vll_helpers::musyst.at(i);
     }
}
// Muon selection
for(int i = 0; i < mu_pt.GetSize(); i++) {
    if(mu_pt[i] < 20e3) continue;
    if(fabs(mu_eta[i]) > 2.5) continue;

    bool quality = mu_isIsolated[i];
    bool truthmatched = (mu_truthClassification[i] == 4);

    if(!is_data && !truthmatched && !include_fake_mc) continue;
    if(!quality && !include_fake_mc) {
	fakes.push_back(vll_fake_helpers::get_mu_ff(mu_pt[i]/1000,mu_eta[i],mu_syst));
        light_fakes.push_back(vll_fake_helpers::get_mu_ff(mu_pt[i]/1000,mu_eta[i],mu_syst));
        N_muon++;
        LT += mu_pt[i];
        light_leps.push_back({TLorentzVector(),mu_charge[i],13});
        light_leps.back().v.SetPtEtaPhiM(mu_pt[i],mu_eta[i],mu_phi[i],105.7);
        weight *= mu_effSF_reco[i] * mu_effSF_TTVA[i] * (quality ? mu_effSF_isol[i] : 1.0);
	light_charge += mu_charge[i];
    }
    if(quality){
        N_muon++;
        LT += mu_pt[i];
        light_leps.push_back({TLorentzVector(),mu_charge[i],13});
        light_leps.back().v.SetPtEtaPhiM(mu_pt[i],mu_eta[i],mu_phi[i],105.7);
        weight *= mu_effSF_reco[i] * mu_effSF_TTVA[i] * (quality ? mu_effSF_isol[i] : 1.0);
	light_charge += mu_charge[i];
    }
}
if(N_ele+N_muon < 2) continue;

int leading_light = 0;
float leading_light_pt = 0;
for(int k = 0; k < light_leps.size(); k++){
    if(light_leps.at(k).v.Pt() > leading_light_pt){
	leading_light = k;
	leading_light_pt = light_leps.at(k).v.Pt();
    }
}
int sub_leading_light = 0;
float sub_leading_light_pt = 0;
for(int k = 0; k < light_leps.size(); k++){
    if(k == leading_light) continue;
    if(light_leps.at(k).v.Pt() > sub_leading_light_pt){
        sub_leading_light = k;
        sub_leading_light_pt = light_leps.at(k).v.Pt();
    }
}

int leading_jet = 0;
float leading_jet_pt = 0;
int subleading_jet = 0;
float subleading_jet_pt = 0;
for(int k = 0; k < jet_vec.size(); k++){
    if(jet_vec.at(k).pt > leading_jet_pt){
        leading_jet_pt = jet_vec.at(k).pt;
        leading_jet = k;
    }
}
for(int k = 0; k < jet_vec.size(); k++){
    if(k == leading_jet) continue;
    if(jet_vec.at(k).pt > subleading_jet_pt){
        subleading_jet_pt = jet_vec.at(k).pt;
        subleading_jet = k;
    }
}
float delphi_jet_et = 0;
float delphi_jet_light = 0;
float delR_jet_et = 0;
float delR_jet_light = 0;
float jet_inv = 0;
if(jet_vec.size() != 0){
    jet_vec.at(leading_jet).leading = 1;
    delphi_jet_et = met_vec.at(0).v.DeltaPhi(jet_vec.at(leading_jet).v);
    delphi_jet_light = light_leps.at(leading_light).v.DeltaPhi(jet_vec.at(leading_jet).v);
    delR_jet_et = met_vec.at(0).v.DeltaR(jet_vec.at(leading_jet).v);
    delR_jet_light = light_leps.at(leading_light).v.DeltaR(jet_vec.at(leading_jet).v);
}
if(jet_vec.size() > 1){
   jet_inv = (jet_vec[leading_jet].v + jet_vec[subleading_jet].v).M();
}
// Tau selection
TLorentzVector light_tau_sum;
std::vector<float> taupt;
std::vector<float> taufakept;
std::string tau_syst = "NOSYS";
float tau_pre_pt = 0;
for(int i = 0; i < vll_helpers::tausyst.size(); i++){
     if(systematic.compare("tau_"+vll_helpers::tausyst.at(i)) == 0){
        tau_syst = vll_helpers::tausyst.at(i);
    }
}
std::string htau_syst = "NOSYS";
for(int i = 0; i < vll_helpers::htausyst.size(); i++){
     if(systematic.compare("htau_"+vll_helpers::htausyst.at(i)) == 0){
        htau_syst = vll_helpers::htausyst.at(i);
    }
}
for(int i = 0; i < tau_pt.GetSize(); i++){
    if(tau_EleBDT[i] == 0) continue;
    if(tau_pt[i] < 20e3) continue;
    if(tau_passEleOLR[i] == 0) continue;
    if(fabs(tau_eta[i]) > 2.47) continue;
    if(fabs(tau_charge[i]) != 1) continue;
    if(tau_RNNJetScoreSigTrans[i] < 0.01) continue;
    if(fabs(tau_eta[i]) > 1.37 && fabs(tau_eta[i]) < 1.52) continue;

    bool quality = tau_isMedium[i];
    bool truthmatched = (tau_truthClassification[i] == 0) || (tau_truthClassification[i] == 4) || (tau_truthClassification[i] == 5) || (tau_truthClassification[i] == 7);

    if(!is_data && !truthmatched && !include_fake_mc) continue;
    if(!quality && !include_fake_mc) {
        fakes.push_back(vll_fake_helpers::get_tau_ff(tau_nTracks[i],tau_pt[i]/1000,tau_syst));
        tau_fakes.push_back(vll_fake_helpers::get_tau_ff(tau_nTracks[i],tau_pt[i]/1000,tau_syst));
        htau_fakes.push_back(vll_fake_helpers::get_htau_ff(tau_nTracks[i],tau_pt[i]/1000,htau_syst));
        taupt.push_back(tau_pt[i]/1000);
        taufakept.push_back(tau_pt[i]/1000);
        TLorentzVector vt; vt.SetPtEtaPhiM(tau_pt[i],tau_eta[i],tau_phi[i],1776.86);
        tau_vec.push_back({TLorentzVector(),tau_pt[i],tau_eta[i],0});
        tau_vec.back().v.SetPtEtaPhiM(tau_pt[i],tau_eta[i],tau_phi[i],1776.86);
        light_tau_sum += vt;
        jet_tau_sum += vt;
        N_tau++;
        weight *= quality ? tau_effSF_Medium[i] : tau_effSF_Base[i];
   }
    if(quality){
        taupt.push_back(tau_pt[i]/1000);
        TLorentzVector vt; vt.SetPtEtaPhiM(tau_pt[i],tau_eta[i],tau_phi[i],1776.86);
        light_tau_sum += vt;
        jet_tau_sum += vt;
        tau_vec.push_back({TLorentzVector(),tau_pt[i],tau_eta[i],0});
        tau_vec.back().v.SetPtEtaPhiM(tau_pt[i],tau_eta[i],tau_phi[i],1776.86);
        N_tau++;
        weight *= quality ? tau_effSF_Medium[i] : tau_effSF_Base[i];
    }
}
int leading_tau = 0;
float leading_tau_pt = 0;
float delphi_tau_et = 0;
float delphi_tau_light = 0;
float delphi_tau_jet = 0;
float delR_tau_et = 0;
float delR_tau_light = 0;
float delR_tau_jet = 0;
for(int k = 0; k < tau_vec.size(); k++){
    if(tau_vec.at(k).pt > leading_tau_pt){
	leading_tau_pt = tau_vec.at(k).pt;
	leading_tau = k;
    }
}

if(tau_vec.size() != 0){
    tau_vec.at(leading_tau).leading = 1;
    delphi_tau_et = met_vec.at(0).v.DeltaPhi(tau_vec.at(leading_tau).v);
    delphi_tau_light = light_leps.at(leading_light).v.DeltaPhi(tau_vec.at(leading_tau).v);             
    delR_tau_et = met_vec.at(0).v.DeltaR(tau_vec.at(leading_tau).v);
    delR_tau_light = light_leps.at(leading_light).v.DeltaR(tau_vec.at(leading_tau).v);
    if(jet_vec.size() != 0){
        delphi_tau_jet = jet_vec.at(leading_jet).v.DeltaPhi(tau_vec.at(leading_tau).v);
        delR_tau_jet = jet_vec.at(leading_jet).v.DeltaR(tau_vec.at(leading_tau).v);
    }
}

// Fakes
std::vector<float> FF;
std::vector<float> nominal_percent_ff;
std::vector<float> up_percent_ff;
std::vector<float> down_percent_ff;

nominal_percent_ff.push_back(0.48);
nominal_percent_ff.push_back(0.58);
nominal_percent_ff.push_back(0.23);
nominal_percent_ff.push_back(0.39);
nominal_percent_ff.push_back(0.51);
nominal_percent_ff.push_back(0.43);
nominal_percent_ff.push_back(0.50);

nominal_percent_ff.push_back(1);
nominal_percent_ff.push_back(1);
nominal_percent_ff.push_back(1);
nominal_percent_ff.push_back(1);
nominal_percent_ff.push_back(1);

down_percent_ff.push_back(0);
down_percent_ff.push_back(0);
down_percent_ff.push_back(0);
down_percent_ff.push_back(0);
down_percent_ff.push_back(0);
down_percent_ff.push_back(0);
down_percent_ff.push_back(0);
down_percent_ff.push_back(1);
down_percent_ff.push_back(1);
down_percent_ff.push_back(1);
down_percent_ff.push_back(1);
down_percent_ff.push_back(1);

up_percent_ff.push_back(1);
up_percent_ff.push_back(1);
up_percent_ff.push_back(1);
up_percent_ff.push_back(1);
up_percent_ff.push_back(1);
up_percent_ff.push_back(1);
up_percent_ff.push_back(1);
up_percent_ff.push_back(1);
up_percent_ff.push_back(1);
up_percent_ff.push_back(1);
up_percent_ff.push_back(1);
up_percent_ff.push_back(1);

if((config != fake_lep) && (fakes.size() != 0)) continue;
if((config == fake_lep) && (fakes.size() == 0)) continue;
if(fakes.size() != 0){
    for(int k = 0; k < nominal_percent_ff.size(); k++){
        float temp_ff = vll_fake_helpers::get_event_ff(light_fakes,tau_fakes,htau_fakes,down_percent_ff.at(k),up_percent_ff.at(k),nominal_percent_ff.at(k),taufakept,tau_syst);
        if((config == fake_lep) && (!is_data) && (fakes.size() != 0)){
            temp_ff *= -1;
        }
        FF.push_back(temp_ff);
    }
}
   
// Sort leptons in descending order of pT
std::sort(light_leps.rbegin(),light_leps.rend(),[](const vll_helpers::lep &lhs,const vll_helpers::lep &rhs) {return lhs.v.Pt() < rhs.v.Pt();});

// mOSSF and mLightLeps calculation


Double_t closest_mass_so_far = 1e99;
const Double_t MZ = 91188;
struct vll_helpers::lep *l1 = NULL;
struct vll_helpers::lep *l2 = NULL;
bool SS = false;
bool SF = false;
TLorentzVector light_sum;
for(auto lep1 = light_leps.begin(); lep1 != light_leps.end(); lep1++) {
    light_sum += lep1->v;
    for(auto lep2 = std::next(lep1); lep2 != light_leps.end(); lep2++) {
        if(lep1->charge == lep2->charge) SS = true;
        if(lep1->absPdgId == lep2->absPdgId) SF = true;
        if(!SS && SF) {
            Double_t inv_m = (lep1->v + lep2->v).M();
            if(fabs(inv_m - MZ) < fabs(closest_mass_so_far-MZ)) {
                closest_mass_so_far = inv_m;
                l1 = &*lep1;
                l2 = &*lep2;
            }
        }
    }
}
Double_t mOSSF = (l1 && l2 ? (l1->v + l2->v).M() : -1.0);
light_tau_sum += light_sum;
light_jet_sum += light_sum;
std::vector<struct vll_helpers::lep *> other_leps;
if(N_ele+N_muon == 4) {
    std::vector<struct vll_helpers::lep *> other_leps;
    for(auto lep = light_leps.begin(); lep != light_leps.end(); lep++) {
        struct vll_helpers::lep *l = &*lep;
        if(l != l1 && l != l2)
            other_leps.push_back(l);
    }
}

if(config == fake_lep){
    //four_emu
    if(N_ele+N_muon >= 4){
        weight *= FF.at(0);
    }
    //three_emu_one_ta
    if((N_ele+N_muon == 3) && (N_tau >= 1)){
        weight *= FF.at(1);
    }
    //two_emuOSOF_one_ta
    if((N_ele == 1) && (N_muon == 1) && (N_tau == 1) && (SS == 0)){
        weight *= FF.at(2);
    }
    //two_emuOSSF_one_ta
    if((N_ele+N_muon == 2) && (N_tau == 1) && (SS == 0) && (SF != 0)){
        weight *= FF.at(3);
    }
    //two_emuSSSF_one_ta
    if((N_ele+N_muon == 2) && (N_tau == 1) && (SS != 0) && (SF != 0)){
        weight *= FF.at(4);
    }
    //two_emuSSOF_one_ta
    if((N_ele == 1) && (N_muon == 1) && (N_tau == 1) && (SS != 0)){
        weight *= FF.at(5);
    }
    //two_emu_two_ta
    if((N_ele+N_muon == 2) && (N_tau >= 2)){
        weight *= FF.at(6);
    }
    if((N_ele+N_muon == 3) && (N_tau == 0)){
        weight *= FF.at(7);
    }
    if((N_ele+N_muon == 2) && (N_tau == 0) && (mOSSF >= 0) && (SS == 0) && (SF != 0)){
        weight *= FF.at(8);
    }
    if((N_ele == 1) && (N_muon == 1) && (N_tau == 0) && (SS == 0)){
        weight *= FF.at(9);
    }
    if((N_ele+N_muon == 2) && (N_tau == 0) && (mOSSF < 0) && (SS != 0) && (SF != 0)){
        weight *= FF.at(10);
    }
    if((N_ele == 1) && (N_muon == 1) && (N_tau == 0) && (SS != 0)){
        weight *= FF.at(11);
    }
}

int fold = 0;
if(N_ele+N_muon >= 4){
    if(fold_map.at(systematic).at(0) < 6){
	fold = fold_map.at(systematic).at(0);
	fold_map.at(systematic).at(0) = fold_map.at(systematic).at(0) + 1;
    }
    if(fold_map.at(systematic).at(0) > 5){
        fold_map.at(systematic).at(0) = 1;
    }
}
if((N_ele+N_muon == 3) && (N_tau >= 1)){
    if(fold_map.at(systematic).at(1) < 6){
        fold = fold_map.at(systematic).at(1);
        fold_map.at(systematic).at(1) = fold_map.at(systematic).at(1) + 1;
    }
    if(fold_map.at(systematic).at(1) > 5){
        fold_map.at(systematic).at(1) = 1;
    }
}
if((N_ele == 1) && (N_muon == 1) && (N_tau == 1) && (SS == 0)){
    if(fold_map.at(systematic).at(2) < 6){
        fold = fold_map.at(systematic).at(2);
        fold_map.at(systematic).at(2) = fold_map.at(systematic).at(2) + 1;
    }
    if(fold_map.at(systematic).at(2) > 5){
        fold_map.at(systematic).at(2) = 1;
    }
}
if((N_ele+N_muon == 2) && (N_tau == 1) && (SS == 0) && (SF != 0)){
    if(fold_map.at(systematic).at(3) < 6){
        fold = fold_map.at(systematic).at(3);
        fold_map.at(systematic).at(3) = fold_map.at(systematic).at(3) + 1;
    }
    if(fold_map.at(systematic).at(3) > 5){
        fold_map.at(systematic).at(3) = 1;
    }
}
if((N_ele+N_muon == 2) && (N_tau == 1) && (SS != 0) && (SF != 0)){
    if(fold_map.at(systematic).at(4) < 6){
        fold = fold_map.at(systematic).at(4);
        fold_map.at(systematic).at(4) = fold_map.at(systematic).at(4) + 1;
    }
    if(fold_map.at(systematic).at(4) > 5){
        fold_map.at(systematic).at(4) = 1;
    }
}
if((N_ele == 1) && (N_muon == 1) && (N_tau == 1) && (SS != 0)){
    if(fold_map.at(systematic).at(5) < 6){
        fold = fold_map.at(systematic).at(5);
        fold_map.at(systematic).at(5) = fold_map.at(systematic).at(5) + 1;
    }
    if(fold_map.at(systematic).at(5) > 5){
        fold_map.at(systematic).at(5) = 1;
    }
}
if((N_ele+N_muon == 2) && (N_tau >= 2)){
    if(fold_map.at(systematic).at(6) < 6){
        fold = fold_map.at(systematic).at(6);
        fold_map.at(systematic).at(6) = fold_map.at(systematic).at(6) + 1;
    }
    if(fold_map.at(systematic).at(6) > 5){
        fold_map.at(systematic).at(6) = 1;
    }
}
if((N_ele+N_muon == 3) && (N_tau == 0)){
    if(fold_map.at(systematic).at(7) < 6){
        fold = fold_map.at(systematic).at(7);
        fold_map.at(systematic).at(7) = fold_map.at(systematic).at(7) + 1;
    }
    if(fold_map.at(systematic).at(7) > 5){
        fold_map.at(systematic).at(7) = 1;
    }
}
if((N_ele+N_muon == 2) && (N_tau == 0) && (mOSSF >= 0) && (SS == 0) && (SF != 0)){
    if(fold_map.at(systematic).at(8) < 6){
        fold = fold_map.at(systematic).at(8);
        fold_map.at(systematic).at(8) = fold_map.at(systematic).at(8) + 1;
    }
    if(fold_map.at(systematic).at(8) > 5){
        fold_map.at(systematic).at(8) = 1;
    }
}
if((N_ele == 1) && (N_muon == 1) && (N_tau == 0) && (SS == 0)){
    if(fold_map.at(systematic).at(9) < 6){
        fold = fold_map.at(systematic).at(9);
        fold_map.at(systematic).at(9) = fold_map.at(systematic).at(9) + 1;
    }
    if(fold_map.at(systematic).at(9) > 5){
        fold_map.at(systematic).at(9) = 1;
    }
}
if((N_ele+N_muon == 2) && (N_tau == 0) && (mOSSF < 0) && (SS != 0) && (SF != 0)){
    if(fold_map.at(systematic).at(10) < 6){
        fold = fold_map.at(systematic).at(10);
        fold_map.at(systematic).at(10) = fold_map.at(systematic).at(10) + 1;
    }
    if(fold_map.at(systematic).at(10) > 5){
        fold_map.at(systematic).at(10) = 1;
    }
}
if((N_ele == 1) && (N_muon == 1) && (N_tau == 0) && (SS != 0)){
    if(fold_map.at(systematic).at(11) < 6){
        fold = fold_map.at(systematic).at(11);
        fold_map.at(systematic).at(11) = fold_map.at(systematic).at(11) + 1;
    }
    if(fold_map.at(systematic).at(11) > 5){
        fold_map.at(systematic).at(11) = 1;
    }
}

//nominal
double WZ_weights [] = {0.83473614301,0.672136380002,0.679806720062,0.626402690358,0.774038298949};
//up
double WZ_weights_u [] = {0.822176905645,0.656862893153,0.660370520536,0.59654330274,0.774038298949};
//down
double WZ_weights_d [] = {0.847295380376,0.687409866852,0.699242919588,0.656262077976,0.774038298949};

if(is_WZ){
    if(N_jets == 1){
	weight *= WZ_weights[0];
    }
    if(N_jets == 2){           
        weight *= WZ_weights[1];
    }
    if(N_jets == 3){           
        weight *= WZ_weights[2];
    }
    if(N_jets == 4){           
        weight *= WZ_weights[3];
    }
    if(N_jets > 4){
        weight *= WZ_weights[4];
    }
}
if(is_WZ_u){
    if(N_jets == 1){
        weight *= WZ_weights_u[0];
    }
    if(N_jets == 2){           
        weight *= WZ_weights_u[1];
    }
    if(N_jets == 3){           
        weight *= WZ_weights_u[2];
    }
    if(N_jets == 4){           
        weight *= WZ_weights_u[3];
    }
    if(N_jets > 4){
        weight *= WZ_weights_u[4];
    }
}
if(is_WZ_d){
    if(N_jets == 1){
        weight *= WZ_weights_d[0];
    }
    if(N_jets == 2){           
        weight *= WZ_weights_d[1];
    }
    if(N_jets == 3){           
        weight *= WZ_weights_d[2];
    }
    if(N_jets == 4){           
        weight *= WZ_weights_d[3];
    }
    if(N_jets > 4){
        weight *= WZ_weights_d[4];
    }
}
// Fill histograms
std::pair<Double_t,Double_t> top2_ele_pt = {-1,-1};
std::pair<Double_t,Double_t> top2_mu_pt = {-1,-1};
for(const auto &lep : light_leps) {
    if(lep.absPdgId == 11 && lep.v.Pt() > top2_ele_pt.first) {
        top2_ele_pt.second = top2_ele_pt.first;
        top2_ele_pt.first = lep.v.Pt();
    } else if(lep.absPdgId == 13 && lep.v.Pt() > top2_mu_pt.first) {
        top2_mu_pt.second = top2_mu_pt.first;
        top2_mu_pt.first = lep.v.Pt();
    } else if(lep.absPdgId == 11 && lep.v.Pt() > top2_ele_pt.second)
        top2_ele_pt.second = lep.v.Pt();
    else if(lep.absPdgId == 13 && lep.v.Pt() > top2_mu_pt.second)
        top2_mu_pt.second = lep.v.Pt();
}

float lt_tauin = LT;
for(int v = 0; v < taupt.size(); v++){
    lt_tauin += taupt.at(v);
}
// Now that we're done computing the event weight and event selection, we have all we need if we're currently looking at a weight systematic
if(is_weight_syst) {
    if(weight_systematics.size() <= local_cuts_passed)
        weight_systematics.push_back(std::unordered_map<std::string,Double_t>());
    weight_systematics[local_cuts_passed][systematic] = weight;
    local_cuts_passed++;
    continue;
}
local_cuts_passed++;

float pre_weight = weight/gen_weight;
// Fill TTrees
event_info[Cuts::Nele] = N_ele;
event_info[Cuts::Nmuon] = N_muon;
event_info[Cuts::SS] = SS;
event_info[Cuts::SF] = SF;
event_info[Cuts::pre_weight] = pre_weight/xsecweight;
event_info[Cuts::LT] = LT;
event_info[Cuts::MET] = MET[met_idx];
event_info[Cuts::METsig] = METsig[met_idx];
event_info[Cuts::mOSSF] = mOSSF;
event_info[Cuts::FOLD] = fold;
event_info[Cuts::Minv_light] = light_sum.M();
event_info[Cuts::Minv_light_tau] = light_tau_sum.M();
event_info[Cuts::Nlight] = N_ele+N_muon;
event_info[Cuts::Ntau] = N_tau;
event_info[Cuts::NBJet] = N_bjets;
event_info[Cuts::LTMET] = LT+MET[met_idx];
event_info[Cuts::pT_leadingLep] = leading_light_pt;
event_info[Cuts::pT_SubleadingLep] = sub_leading_light_pt;
event_info[Cuts::NJet] = N_jets;
event_info[Cuts::Delphi_light_et] = light_leps.at(leading_light).v.DeltaPhi(met_vec.at(0).v);
event_info[Cuts::DelR_light_et] = light_leps.at(leading_light).v.DeltaR(met_vec.at(0).v);
event_info[Cuts::Delphi_light_light] = light_leps.at(leading_light).v.DeltaPhi(light_leps.at(sub_leading_light).v);
event_info[Cuts::DelR_light_light] = light_leps.at(leading_light).v.DeltaR(light_leps.at(sub_leading_light).v);
event_info[Cuts::pT_leadingTau] = leading_tau_pt;
event_info[Cuts::Delphi_tau_et] = delphi_tau_et;
event_info[Cuts::Delphi_tau_light] = delphi_tau_light;
event_info[Cuts::DelR_tau_et] = delR_tau_et;
event_info[Cuts::DelR_tau_light] = delR_tau_light;
event_info[Cuts::LT_tau] = lt_tauin;
event_info[Cuts::MT] = (met_vec.at(0).v+light_leps.at(leading_light).v).Mt();
event_info[Cuts::HT] = HT ;
event_info[Cuts::Minv_light_jet] = light_jet_sum.M();
event_info[Cuts::Delphi_jet_et] = delphi_jet_et;
event_info[Cuts::DelR_jet_et] = delR_jet_et;
event_info[Cuts::pT_leadingJet] = leading_jet_pt;
event_info[Cuts::Delphi_jet_light] = delphi_jet_light;
event_info[Cuts::DelR_jet_light] = delR_jet_light;
event_info[Cuts::Minv_tau_jet] = jet_tau_sum.M();
event_info[Cuts::Delphi_tau_jet] = delphi_tau_jet;
event_info[Cuts::DelR_tau_jet] = delR_tau_jet;
event_info[Cuts::Minv_jetjet] = jet_inv;
event_info[Cuts::LTHT] = LT + HT;
for(auto &fs : final_states){
    if(fs->PassesCuts(event_info)){
        if(do_systematics && weight_systematics.size() != 0){
            fs->Fill(event_info,weight,systematic,weight_systematics[local_cuts_passed-1]);
	}
	else if(do_fake_systematics && weight_systematics.size() != 0){
            fs->Fill(event_info,weight,systematic,weight_systematics[local_cuts_passed-1]);
        }
        else{
            fs->Fill(event_info,weight,systematic);
	}
    }
}
