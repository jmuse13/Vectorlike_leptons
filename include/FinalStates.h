#include "vll_helpers.h"

#include "TEfficiency.h"
#include "TH1.h"
#include "TTree.h"
#include "TLorentzVector.h"
#include "ROOT/TThreadedObject.hxx"

#include <iostream>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <string>


namespace Cuts {
    //    enum ParticleQuantity { pT, eta, phi, m, charge, absPdgId };
    enum EventQuantity {TRUE,Nele,Nmuon,SS,SF,FOLD,pre_weight,Minv_jetjet,MT,LT,MET,METsig,mOSSF,Minv_light,Minv_light_tau,Nlight,Ntau,NBJet,LTMET,pT_leadingLep,pT_SubleadingLep,NJet,HT,LTHT,Minv_light_jet,Delphi_jet_et,DelR_jet_et,pT_leadingJet,DelR_light_et,DelR_light_light,DelR_jet_light,Delphi_light_et,Delphi_light_light,Delphi_jet_light,Minv_tau_jet,eta_leadingJet,pT_leadingTau,DelR_tau_et,DelR_tau_light,DelR_tau_jet,Delphi_tau_et,Delphi_tau_light,Delphi_tau_jet,LT_tau};
    enum Operation { lt, gt, leq, geq, eq, neq };
};

class _Cut;
typedef std::unique_ptr<_Cut> Cut;
class _Cut {
    public:
        _Cut(Cuts::EventQuantity qq, Double_t dd, Cuts::Operation oo) : q(qq),d(dd),o(oo),and_with(NULL) {}
        _Cut(const _Cut &c);
        ~_Cut() { delete and_with; }

        _Cut &operator&=(const _Cut &rhs);
        Cut operator&&(const _Cut &rhs);
        void print();
        bool apply(const std::unordered_map<Cuts::EventQuantity, Double_t> &event_info);

    private:
        const Cuts::EventQuantity q;
        const Double_t d;
        const Cuts::Operation o;
        _Cut *and_with;
};

_Cut::_Cut(const _Cut &c) : q(c.q),d(c.d),o(c.o),and_with(NULL) {
    if(c.and_with) and_with = new _Cut(*(c.and_with));
}

_Cut &_Cut::operator&=(const _Cut &rhs) {
    if(!and_with) and_with = new _Cut(rhs);
    else *and_with &= rhs;
    return *this;
}

Cut _Cut::operator&&(const _Cut &rhs) {
    Cut ret = std::make_unique<_Cut>(*this);
    *ret &= rhs;
    return ret;
}

Cut operator&&(const Cut &lhs, Cuts::EventQuantity q) {
    if(q == Cuts::TRUE) return std::make_unique<_Cut>(*lhs);
    else return nullptr;
}
Cut operator&=(const Cut &lhs, const Cut &rhs) { return std::make_unique<_Cut>(*lhs &= *rhs); }
Cut operator&&(const Cut &lhs, const Cut &rhs) { return *lhs && *rhs; }
Cut operator<(Cuts::EventQuantity q, Double_t d) { if(q != Cuts::TRUE) return std::make_unique<_Cut>(q,d,Cuts::lt); else return nullptr; }
Cut operator>(Cuts::EventQuantity q, Double_t d) { if(q != Cuts::TRUE) return std::make_unique<_Cut>(q,d,Cuts::gt); else return nullptr; }
Cut operator<=(Cuts::EventQuantity q, Double_t d) { if(q != Cuts::TRUE) return std::make_unique<_Cut>(q,d,Cuts::leq); else return nullptr; }
Cut operator>=(Cuts::EventQuantity q, Double_t d) { if(q != Cuts::TRUE) return std::make_unique<_Cut>(q,d,Cuts::geq); else return nullptr; }
Cut operator==(Cuts::EventQuantity q, Double_t d) { if(q != Cuts::TRUE) return std::make_unique<_Cut>(q,d,Cuts::eq); else return nullptr; }
Cut operator!=(Cuts::EventQuantity q, Double_t d) { if(q != Cuts::TRUE) return std::make_unique<_Cut>(q,d,Cuts::neq); else return nullptr; }
inline Cut operator<(Cuts::EventQuantity q, int i) { return q < double(i); }
inline Cut operator>(Cuts::EventQuantity q, int i) { return q > double(i); }
inline Cut operator<=(Cuts::EventQuantity q, int i) { return q <= double(i); }
inline Cut operator>=(Cuts::EventQuantity q, int i) { return q >= double(i); }
inline Cut operator==(Cuts::EventQuantity q, int i) { return q == double(i); }
inline Cut operator!=(Cuts::EventQuantity q, int i) { return q != double(i); }

void _Cut::print() {
    std::cout << "q:" << q;
    switch(o) {
        case Cuts::lt:
            std::cout << " < ";
            break;
        case Cuts::gt:
            std::cout << " > ";
            break;
        case Cuts::leq:
            std::cout << " <= ";
            break;
        case Cuts::geq:
            std::cout << " >= ";
            break;
        case Cuts::eq:
            std::cout << " == ";
            break;
        case Cuts::neq:
            std::cout << " != ";
            break;
        default:
            break;
    }
    std::cout << "d:" << d;
    if(and_with) {
        std::cout << " && ";
        and_with->print();
    } else
        std::cout << std::endl;
}

bool _Cut::apply(const std::unordered_map<Cuts::EventQuantity, Double_t> &event_info) {
    bool ret;
    switch(o) {
        case Cuts::lt:
            ret = event_info.at(q) < d;
            break;
        case Cuts::gt:
            ret = event_info.at(q) > d;
            break;
        case Cuts::leq:
            ret = event_info.at(q) <= d;
            break;
        case Cuts::geq:
            ret = event_info.at(q) >= d;
            break;
		case Cuts::eq:
		    ret = (event_info.at(q) == d);
		    break;
		case Cuts::neq:
		    ret = event_info.at(q) != d;
		    break;
		default:
		    ret = false;
	    }

	    if(and_with) ret = ret && and_with->apply(event_info);
	    return ret;
	}

class FinalState {
    public:
	FinalState(std::string fs_name, Cut c);
	~FinalState();

	bool PassesCuts(const std::unordered_map<Cuts::EventQuantity,Double_t> &event_info);
	void AddTree(const std::string &systematic);
	void Fill(const std::unordered_map<Cuts::EventQuantity,Double_t> &event_info, const Double_t weight, const std::string &systematic, const std::unordered_map<std::string,Double_t> &systematic_weights);
	void Write();

	std::string GetName() { return m_name; }

	private:
	    std::string m_name;
	    Cut m_cut;
	    std::mutex m_mutex;
	    TTree *m_nominal_tree;
	    std::unordered_map<std::string,TTree *> m_trees;
	    std::unordered_map<std::string,TH1 *> m_histograms;
	    std::unordered_map<std::string,TEfficiency *> m_efficiencies;
	    TDirectory *m_hist_dir;

	    // Branches
	    std::unordered_map<std::string,Float_t> m_syst_weights;
            Float_t m_Nele;
            Float_t m_Nmuon;
            Float_t m_SS;
            Float_t m_SF;
	    Float_t m_fold;
	    Float_t m_LT;
	    Float_t m_MET;
	    Float_t m_MT;
	    Float_t m_METsig;
	    Float_t m_mOSSF;
	    Float_t m_Minv_light;
	    Float_t m_Minv_light_tau;
	    Float_t m_Nlight;
	    Float_t m_Ntau;
 	    Float_t m_pre_weight;
	    Float_t m_NBJet;
	    Float_t m_LTMET;
	    Float_t m_pT_leadingLep;
	    Float_t m_pT_SubleadingLep;
	    Float_t m_NJet;
	    Float_t m_HT;
	    Float_t m_LTHT;
	    Float_t m_Minv_light_jet;
	    Float_t m_Delphi_jet_et;
	    Float_t m_DelR_jet_et;
	    Float_t m_pT_leadingJet;
	    Float_t m_Delphi_light_et;
	    Float_t m_Delphi_light_light;
	    Float_t m_Delphi_jet_light;
	    Float_t m_DelR_light_et;
	    Float_t m_DelR_light_light;
	    Float_t m_DelR_jet_light;
	    Float_t m_Minv_tau_jet;
	    Float_t m_pT_leadingTau;
	    Float_t m_eta_leadingJet;
	    Float_t m_eta_leadingTau;
	    Float_t m_Delphi_tau_et;
	    Float_t m_Delphi_tau_light;
	    Float_t m_Delphi_tau_jet;
	    Float_t m_DelR_tau_et;
	    Float_t m_DelR_tau_light;
	    Float_t m_DelR_tau_jet;
	    Float_t m_LT_tau;
	    Float_t m_Minv_jetjet;
	    Float_t m_weight;
};

FinalState::FinalState(std::string fs_name, Cut c) : m_name(fs_name),m_cut(std::make_unique<_Cut>(*c)),m_trees(),m_hist_dir(NULL) {
    std::lock_guard<std::mutex> lg(m_mutex);
    m_nominal_tree = new TTree(fs_name.c_str(),fs_name.c_str());
    m_nominal_tree->Branch("weight",&m_weight);
    m_nominal_tree->Branch("Nele",&m_Nele);
    m_nominal_tree->Branch("Nmuon",&m_Nmuon);
    m_nominal_tree->Branch("SS",&m_SS);
    m_nominal_tree->Branch("SF",&m_SF);
    m_nominal_tree->Branch("LT",&m_LT);
    m_nominal_tree->Branch("MET",&m_MET);
    m_nominal_tree->Branch("METsig",&m_METsig);
    m_nominal_tree->Branch("mOSSF",&m_mOSSF);
    m_nominal_tree->Branch("Minv_light",&m_Minv_light);
    m_nominal_tree->Branch("Minv_light_tau",&m_Minv_light_tau);
    m_nominal_tree->Branch("Nlight",&m_Nlight);
    m_nominal_tree->Branch("Ntau",&m_Ntau);
    m_nominal_tree->Branch("NBJet",&m_NBJet);
    m_nominal_tree->Branch("LTMET",&m_LTMET);
    m_nominal_tree->Branch("pT_leadingLep",&m_pT_leadingLep);
    m_nominal_tree->Branch("pT_SubleadingLep",&m_pT_SubleadingLep);
    m_nominal_tree->Branch("NJet",&m_NJet);
    m_nominal_tree->Branch("HT",&m_HT);
    m_nominal_tree->Branch("MT",&m_MT);
    m_nominal_tree->Branch("pre_weight",&m_pre_weight);
    m_nominal_tree->Branch("LTHT",&m_LTHT);
    m_nominal_tree->Branch("FOLD",&m_fold);
    m_nominal_tree->Branch("Minv_light_jet",&m_Minv_light_jet);
    m_nominal_tree->Branch("Delphi_jet_et",&m_Delphi_jet_et);
    m_nominal_tree->Branch("DelR_jet_et",&m_DelR_jet_et);
    m_nominal_tree->Branch("pT_leadingJet",&m_pT_leadingJet);
    m_nominal_tree->Branch("Delphi_light_et",&m_Delphi_light_et);
    m_nominal_tree->Branch("Delphi_light_light",&m_Delphi_light_light);
    m_nominal_tree->Branch("Delphi_jet_light",&m_Delphi_jet_light);
    m_nominal_tree->Branch("DelR_light_et",&m_DelR_light_et);
    m_nominal_tree->Branch("DelR_light_light",&m_DelR_light_light);
    m_nominal_tree->Branch("DelR_jet_light",&m_DelR_jet_light);
    m_nominal_tree->Branch("Minv_tau_jet",&m_Minv_tau_jet);
    m_nominal_tree->Branch("pT_leadingTau",&m_pT_leadingTau);
    m_nominal_tree->Branch("Delphi_tau_light",&m_Delphi_tau_light);
    m_nominal_tree->Branch("Delphi_tau_et",&m_Delphi_tau_et);
    m_nominal_tree->Branch("Delphi_tau_jet",&m_Delphi_tau_jet);
    m_nominal_tree->Branch("DelR_tau_light",&m_DelR_tau_light);
    m_nominal_tree->Branch("DelR_tau_et",&m_DelR_tau_et);
    m_nominal_tree->Branch("DelR_tau_jet",&m_DelR_tau_jet);
    m_nominal_tree->Branch("LT_tau",&m_LT_tau);
    m_nominal_tree->Branch("Minv_jetjet",&m_Minv_jetjet);

    std::vector<TH1 *> hists;
    std::vector<TEfficiency *> effs;
    TDirectory *pwd = TDirectory::CurrentDirectory();
    m_hist_dir = pwd->mkdir((std::string("hist_")+fs_name).c_str());
    m_hist_dir->cd();
    hists.push_back(new TH1D("LT",(fs_name+"_LT;LT").c_str(),200,0,1000e3));
    hists.push_back(new TH1D("MET",(fs_name+"_MET;MET").c_str(),200,0,600e3));
    hists.push_back(new TH1D("Mlightleps",(fs_name+"_Mlightleps;Mlightleps").c_str(),200,0,600e3));
    hists.push_back(new TH1D("mOSSF",(fs_name+"_mOSSF;mOSSF").c_str(),200,0,600e3));
    hists.push_back(new TH1D("NBJet",(fs_name+"_NBJet;NBJet").c_str(),5,0,5));
    effs.push_back(new TEfficiency("ee_e1_trig_eff","ee_trig_eff;leading_ele_pt;#varepsilon",200,0,500e3));
    effs.push_back(new TEfficiency("ee_e2_trig_eff","ee_trig_eff;subleading_ele_pt;#varepsilon",200,0,500e3));
    effs.push_back(new TEfficiency("mm_m1_trig_eff","mm_trig_eff;leading_muon_pt;#varepsilon",200,0,500e3));
    effs.push_back(new TEfficiency("mm_m2_trig_eff","mm_trig_eff;subleading_muon_pt;#varepsilon",200,0,500e3));
    effs.push_back(new TEfficiency("em_e1_trig_eff","em_trig_eff;leading_ele_pt;#varepsilon",200,0,500e3));
    effs.push_back(new TEfficiency("em_m1_trig_eff","em_trig_eff;leading_muon_pt;#varepsilon",200,0,500e3));

    for(auto h : hists)
	m_histograms.insert(std::make_pair(h->GetName(),h));
    for(auto e : effs)
	m_efficiencies.insert(std::make_pair(e->GetName(),e));
    pwd->cd();
}

FinalState::~FinalState() {
    delete m_nominal_tree;
    for(auto t : m_trees)
	delete t.second;
    for(auto h : m_histograms)
	delete h.second;
    for(auto e : m_efficiencies)
	delete e.second;
}

bool FinalState::PassesCuts(const std::unordered_map<Cuts::EventQuantity,Double_t> &event_info) {
    return m_cut->apply(event_info);
}

// This function takes the name of a systematic variation and either makes a new branch in the nominal tree in the case of a weight systematic or makes a new tree for a non-weight systematic.
void FinalState::AddTree(const std::string &systematic) {
    if(systematic.compare("NOSYS") == 0) return;
    if(vll_helpers::systematics.at(systematic)) {
	std::lock_guard<std::mutex> lg(m_mutex);
	std::string weight_name = "weight_";
	weight_name += systematic;
	auto branches = *(m_nominal_tree->GetListOfBranches());
	for(int i = 0; i < m_nominal_tree->GetNbranches(); i++)
	    if(weight_name.compare(((TBranch *)(branches[i]))->GetName()) == 0)
		return;
	m_nominal_tree->Branch(weight_name.c_str(),&(m_syst_weights[systematic]));
    } else {
	std::lock_guard<std::mutex> lg(m_mutex);
	if(m_trees.find(systematic) != m_trees.end()) return;
	TTree *tree = new TTree((m_name+"___"+systematic).c_str(),(m_name+"_"+systematic).c_str());
	tree->Branch("weight",&m_weight);
        tree->Branch("Nele",&m_Nele);
        tree->Branch("Nmuon",&m_Nmuon);
        tree->Branch("SS",&m_SS);
        tree->Branch("SF",&m_SF);
	tree->Branch("LT",&m_LT);
	tree->Branch("MET",&m_MET);
	tree->Branch("METsig",&m_METsig);
	tree->Branch("mOSSF",&m_mOSSF);
	tree->Branch("Minv_light",&m_Minv_light);
	tree->Branch("Minv_light_tau",&m_Minv_light_tau);
	tree->Branch("Nlight",&m_Nlight);
	tree->Branch("MT",&m_MT);
	tree->Branch("Ntau",&m_Ntau);
	tree->Branch("NBJet",&m_NBJet);
	tree->Branch("LTMET",&m_LTMET);
        tree->Branch("pre_weight",&m_pre_weight);
	tree->Branch("pT_leadingLep",&m_pT_leadingLep);
	tree->Branch("pT_SubleadingLep",&m_pT_SubleadingLep);
	tree->Branch("NJet",&m_NJet);
	tree->Branch("HT",&m_HT);
	tree->Branch("LTHT",&m_LTHT);
   	tree->Branch("FOLD",&m_fold);
	tree->Branch("Minv_light_jet",&m_Minv_light_jet);
	tree->Branch("Delphi_jet_et",&m_Delphi_jet_et);
	tree->Branch("DelR_jet_et",&m_DelR_jet_et);
	tree->Branch("pT_leadingJet",&m_pT_leadingJet);
	tree->Branch("Delphi_light_et",&m_Delphi_light_et);
	tree->Branch("Delphi_light_light",&m_Delphi_light_light);
	tree->Branch("Delphi_jet_light",&m_Delphi_jet_light);
	tree->Branch("DelR_light_et",&m_DelR_light_et);
	tree->Branch("DelR_light_light",&m_DelR_light_light);
	tree->Branch("DelR_jet_light",&m_DelR_jet_light);
	tree->Branch("Minv_tau_jet",&m_Minv_tau_jet);
	tree->Branch("pT_leadingTau",&m_pT_leadingTau);
	tree->Branch("Delphi_tau_light",&m_Delphi_tau_light);
	tree->Branch("Delphi_tau_et",&m_Delphi_tau_et);
	tree->Branch("Delphi_tau_jet",&m_Delphi_tau_jet);
	tree->Branch("DelR_tau_light",&m_DelR_tau_light);
	tree->Branch("DelR_tau_et",&m_DelR_tau_et);
	tree->Branch("DelR_tau_jet",&m_DelR_tau_jet);
	tree->Branch("LT_tau",&m_LT_tau);
	tree->Branch("Minv_jetjet",&m_Minv_jetjet);
       	m_trees.insert(std::pair<std::string,TTree *>(systematic,tree));
    }
}

void FinalState::Fill(const std::unordered_map<Cuts::EventQuantity,Double_t> &event_info, const Double_t weight, const std::string &systematic="NOSYS",const std::unordered_map<std::string,Double_t> &systematic_weights=std::unordered_map<std::string,Double_t>()) {
    if(systematic.compare("NOSYS") == 0) {
        std::lock_guard<std::mutex> lg(m_mutex);
        m_weight = weight;
        m_Nele = event_info.at(Cuts::Nele);
        m_Nmuon = event_info.at(Cuts::Nmuon);
        m_SS = event_info.at(Cuts::SS);
        m_SF = event_info.at(Cuts::SF);
        m_LT = event_info.at(Cuts::LT);
        m_MET = event_info.at(Cuts::MET);
        m_METsig = event_info.at(Cuts::METsig);
        m_mOSSF = event_info.at(Cuts::mOSSF);
        m_Minv_light = event_info.at(Cuts::Minv_light);
        m_Minv_light_tau = event_info.at(Cuts::Minv_light_tau);
        m_Nlight = event_info.at(Cuts::Nlight);
        m_Ntau = event_info.at(Cuts::Ntau);
        m_NBJet = event_info.at(Cuts::NBJet);
        m_LTMET = event_info.at(Cuts::LTMET);
        m_pT_leadingLep = event_info.at(Cuts::pT_leadingLep);
        m_pT_SubleadingLep = event_info.at(Cuts::pT_SubleadingLep);
        m_NJet = event_info.at(Cuts::NJet);
        m_MT = event_info.at(Cuts::MT);
        m_LTHT = event_info.at(Cuts::LTHT);
        m_Delphi_light_et = event_info.at(Cuts::Delphi_light_et);
        m_Delphi_light_light = event_info.at(Cuts::Delphi_light_light);
        m_DelR_light_et = event_info.at(Cuts::DelR_light_et);
        m_DelR_light_light = event_info.at(Cuts::DelR_light_light);
        m_pT_leadingTau = event_info.at(Cuts::pT_leadingTau);
	m_pre_weight = event_info.at(Cuts::pre_weight);
        m_Delphi_tau_et = event_info.at(Cuts::Delphi_tau_et);
        m_Delphi_tau_light = event_info.at(Cuts::Delphi_tau_light);
        m_DelR_tau_et = event_info.at(Cuts::DelR_tau_et);
        m_DelR_tau_light = event_info.at(Cuts::DelR_tau_light);
	m_fold = event_info.at(Cuts::FOLD);
        m_LT_tau = event_info.at(Cuts::LT_tau);
	m_HT = event_info.at(Cuts::HT);
	m_Minv_light_jet = event_info.at(Cuts::Minv_light_jet);
	m_Delphi_jet_et = event_info.at(Cuts::Delphi_jet_et);
	m_DelR_jet_et = event_info.at(Cuts::DelR_jet_et);
	m_pT_leadingJet = event_info.at(Cuts::pT_leadingJet);
	m_Delphi_jet_light = event_info.at(Cuts::Delphi_jet_light);
	m_DelR_jet_light = event_info.at(Cuts::DelR_jet_light);
	m_Minv_tau_jet = event_info.at(Cuts::Minv_tau_jet);
	m_Delphi_tau_jet = event_info.at(Cuts::Delphi_tau_jet);
	m_DelR_tau_jet = event_info.at(Cuts::DelR_tau_jet);
	m_Minv_jetjet = event_info.at(Cuts::Minv_jetjet);
	for(auto sys_weight : systematic_weights){
            m_syst_weights[sys_weight.first] = sys_weight.second;
	}
	m_nominal_tree->Fill();
	return;
    }
    bool is_weight_syst = vll_helpers::systematics.at(systematic);
    if(!is_weight_syst) {
	std::lock_guard<std::mutex> lg(m_mutex);
	m_weight = weight;
        m_Nele = event_info.at(Cuts::Nele);
        m_Nmuon = event_info.at(Cuts::Nmuon);
        m_SS = event_info.at(Cuts::SS);
        m_SF = event_info.at(Cuts::SF);
	m_LT = event_info.at(Cuts::LT);
	m_MET = event_info.at(Cuts::MET);
	m_METsig = event_info.at(Cuts::METsig);
	m_mOSSF = event_info.at(Cuts::mOSSF);
	m_Minv_light = event_info.at(Cuts::Minv_light);
	m_Minv_light_tau = event_info.at(Cuts::Minv_light_tau);
	m_Nlight = event_info.at(Cuts::Nlight);
	m_Ntau = event_info.at(Cuts::Ntau);
	m_NBJet = event_info.at(Cuts::NBJet);
	m_LTMET = event_info.at(Cuts::LTMET);
	m_pT_leadingLep = event_info.at(Cuts::pT_leadingLep);
	m_pT_SubleadingLep = event_info.at(Cuts::pT_SubleadingLep);
	m_NJet = event_info.at(Cuts::NJet);
	m_MT = event_info.at(Cuts::MT);
	m_LTHT = event_info.at(Cuts::LTHT);
	m_pre_weight = event_info.at(Cuts::pre_weight);
        m_fold = event_info.at(Cuts::FOLD);
	m_Delphi_light_et = event_info.at(Cuts::Delphi_light_et);
	m_Delphi_light_light = event_info.at(Cuts::Delphi_light_light);
	m_DelR_light_et = event_info.at(Cuts::DelR_light_et);
	m_DelR_light_light = event_info.at(Cuts::DelR_light_light);
	m_pT_leadingTau = event_info.at(Cuts::pT_leadingTau);
	m_Delphi_tau_et = event_info.at(Cuts::Delphi_tau_et);
	m_Delphi_tau_light = event_info.at(Cuts::Delphi_tau_light);
	m_DelR_tau_et = event_info.at(Cuts::DelR_tau_et);
	m_DelR_tau_light = event_info.at(Cuts::DelR_tau_light);
	m_LT_tau = event_info.at(Cuts::LT_tau);
	m_HT = event_info.at(Cuts::HT);
        m_Minv_light_jet = event_info.at(Cuts::Minv_light_jet);
        m_Delphi_jet_et = event_info.at(Cuts::Delphi_jet_et);
        m_DelR_jet_et = event_info.at(Cuts::DelR_jet_et);
        m_pT_leadingJet = event_info.at(Cuts::pT_leadingJet);
        m_Delphi_jet_light = event_info.at(Cuts::Delphi_jet_light);
        m_DelR_jet_light = event_info.at(Cuts::DelR_jet_light);
        m_Minv_tau_jet = event_info.at(Cuts::Minv_tau_jet);
        m_Delphi_tau_jet = event_info.at(Cuts::Delphi_tau_jet);
        m_DelR_tau_jet = event_info.at(Cuts::DelR_tau_jet);
        m_Minv_jetjet = event_info.at(Cuts::Minv_jetjet);
	m_trees.at(systematic)->Fill();
    }
}

void FinalState::Write() {
    m_nominal_tree->Write();
    for(auto t : m_trees)
        t.second->Write();
    TDirectory *pwd = TDirectory::CurrentDirectory();
    m_hist_dir->cd();
    for(auto h : m_histograms)
        h.second->Write();
    for(auto e : m_efficiencies)
        e.second->Write();
    pwd->cd();
}
