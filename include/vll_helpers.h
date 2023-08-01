#ifndef __vll_helpers_h_
#define __vll_helpers_h_

#include "TFile.h"
#include "TKey.h"
#include "TLorentzVector.h"
#include "TTree.h"
#include "TTreeReader.h"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstring>
#include <dirent.h>
#include <errno.h>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

template <class T>
class TTreeReaderValue;
template <class T>
class TTreeReaderArray;

enum Configuration { nominal,fake_lep };
enum Fake_Configuration { zero_fake,one_fake,two_fake };

namespace vll_helpers {

    // Map telling us whether each systematic variation affects event weight only (``weight systematic") or not
    static std::unordered_map<std::string,bool> systematics;
    static std::mutex systematics_mutex;

    // Map DSIDs and systematic names to lumi weight values
    std::unordered_map<Int_t,std::unordered_map<std::string,Double_t>> lumi_weights;
    // Store DSIDs of Sherpa samples
    std::vector<Int_t> sherpa_dids;
    const std::vector<std::string> tausyst = {"INVM","NUMBJETS","MET","UP","DOWN","STATUP","STATDOWN","RNN","FRACUP","FRACDOWN","INVM_40","NUMBJETS_40","MET_40","UP_40","DOWN_40","STATUP_40","STATDOWN_40","RNN_40","FRACUP_40","FRACDOWN_40"};
    const std::vector<std::string> htausyst = {"INVM","NUMBJETS","UP","DOWN","STATUP","STATDOWN","RNN","INVM_40","NUMBJETS_40","UP_40","DOWN_40","STATUP_40","STATDOWN_40","RNN_40"};
    const std::vector<std::string> elsyst = {"NUMJETS","MET","UP","DOWN"};
    const std::vector<std::string> musyst = {"JETPT","DELPHIUP","DELPHIDOWN","MET","METUP","METDOWN","UP","DOWN"};

    struct lep {
        TLorentzVector v;
        Int_t charge;
        Int_t absPdgId;
    };

    struct jet {
        TLorentzVector v;
        float pt;
	float eta;
        Int_t leading;
    };

    struct tau {
        TLorentzVector v;
        float pt;
	float eta;
        Int_t leading;
    };

    struct missinget {
        TLorentzVector v;
    };

    // Open metadata and sumweights text files, compute the lumi weight for each DSID, and store them in the lumiweights map
    void read_metadata(const std::string campaign, const enum Configuration config, const std::string argv0) {
        vll_helpers::lumi_weights[0]["NOSYS"] = 1.0;

        std::string DAOD;
        if(config == nominal) DAOD = "EXOT12";
	if(config == fake_lep) DAOD = "EXOT12";

        std::string bindir = argv0.substr(0,argv0.find_last_of("/"));
        std::string sumweightsFile_in = bindir + "/../vectorlike-leptons-data/"+DAOD+"_sumweights_MC16" + campaign + ".txt";
        std::string pyamiFile_in = bindir + "/../vectorlike-leptons-data/"+DAOD+"_metadata_MC16" + campaign + ".txt";
        std::string sherpaFile_in = bindir + "/../vectorlike-leptons-data/sherpa.txt";

        float lum = 1.0;
        if(campaign.compare("a") == 0)
            lum = 36207.66;
        else if(campaign.compare("d") == 0)
            lum = 44307.4;
        else if(campaign.compare("e") == 0)
            lum = 58450.1;
        else 
            std::cerr << "MC campaign declaration incorrect.  Events will not be normalized to correct luminosity." << std::endl;

        std::ifstream pyami_file(pyamiFile_in.c_str());
        std::ifstream sumweights_file(sumweightsFile_in.c_str());
        std::ifstream sherpa_file(sherpaFile_in.c_str());
        if(!pyami_file.is_open()) {
            std::cerr << "Failed to open " << pyamiFile_in << std::endl;
            std::exit(1);
        }
        if(!sumweights_file.is_open()) {
            std::cerr << "Failed to open " << sumweightsFile_in << std::endl;
            std::exit(1);
        }
        if(!sherpa_file.is_open()) {
            std::cerr << "Failed to open " << sherpaFile_in << std::endl;
            std::exit(1);
        }
        std::unordered_map<Int_t,Double_t> xsec_map;
        std::string line;
        while(std::getline(pyami_file,line)) {
            if(line.length() == 0 || line.at(0) == '#') continue;
            std::stringstream ss(line);
            int DSID = -1;
            double cross_section, k_factor, filt_efficiency, events;
            ss >> DSID >> cross_section >> k_factor >> filt_efficiency >> events;
            if(DSID <= 0) continue;
            xsec_map.emplace(DSID,cross_section*k_factor*lum*filt_efficiency);
        }
        while(std::getline(sumweights_file,line)) {
            if(line.length() == 0 || line.at(0) == '#') continue;
            std::stringstream ss(line);
            int DSID = -1;
            std::string systematic;
            double sum_of_weights;
            ss >> DSID >> systematic >> sum_of_weights;
            if(DSID <= 0) continue;
            if(xsec_map.find(DSID) == xsec_map.end()) {
                std::cerr << "WARNING: DSID " << DSID << " was found in "+sumweightsFile_in+" but not "+pyamiFile_in+".  MC events for this DSID will not be normalized correctly." << std::endl;
            }
            vll_helpers::lumi_weights[DSID][systematic] = xsec_map[DSID]/sum_of_weights;
        }
        while(std::getline(sherpa_file,line)) {
            if(line.length() == 0 || line.at(0) == '#') continue;
            std::stringstream ss(line);
            int DSID = -1;
            ss >> DSID;
            sherpa_dids.push_back(DSID);
        }
        std::sort(sherpa_dids.begin(),sherpa_dids.end());
    }


    // Implementation of wrappers around TTreeReaderValue and TTreeReaderArray allowing the overloaded * and [] operators to return a specified default value in case a branch doesn't exist in the TChain (e.g. when trying to read MC weights from data sample).
    template <class T>
        class MyReaderValue {
            public:
                // If read_branch is true, a TTreeReaderValue/Array will be initialized and the overloaded * and [] operators will be passed on.  If read_branch is false, the * and [] operators will return defaultval.
                // If has_systematic_variations is true, the branch ``branchname___systematic" will be read from the input ntuple, otherwise ``branchname" will be read.
                // If has_systematic_variations is true, the value of the boolean associated with systematic in the vll_helpers::systematics map will be set to the logical AND of the previous value and affects_weight_only
                MyReaderValue(const bool read_branch,TTreeReader &reader,const std::string &branchname,const std::string &systematic,bool has_systematic_variations,bool affects_weight_only=false,const T &defaultval=T()) : m_read_branch(read_branch) {
                    std::string branchname_with_syst = branchname;
                    if(has_systematic_variations) {
                        branchname_with_syst += "___";
                        branchname_with_syst += systematic;
                        auto branches = *(reader.GetTree()->GetListOfBranches());
                        bool branch_has_this_systematic = false;
                        for(int i = 0; i < reader.GetTree()->GetNbranches(); i++) {
                            if(branchname_with_syst.compare(((TBranch *)(branches[i]))->GetName()) == 0) {
                                branch_has_this_systematic = true;
                                break;
                            }
                        }
                        if(branch_has_this_systematic && systematic.compare("NOSYS") != 0) {
                            std::lock_guard<std::mutex> lg(vll_helpers::systematics_mutex);
                            vll_helpers::systematics[systematic] &= affects_weight_only;
                        } else {
                            branchname_with_syst = branchname;
                            branchname_with_syst += "___NOSYS";
                        }
                    }
                    if(read_branch) {
                        m_reader_value = new TTreeReaderValue<T>(reader,branchname_with_syst.c_str());
                        m_defaultval = NULL;
                    } else {
                        m_defaultval = new T(defaultval);
                        m_reader_value = NULL;
                    }
                }
                ~MyReaderValue() {delete m_reader_value; delete m_defaultval;}

                virtual T &operator*() {return (m_read_branch ? **m_reader_value : *m_defaultval );}

            private:
                bool m_read_branch;

                TTreeReaderValue<T> *m_reader_value;
                T *m_defaultval;
        };

    // Basically the same as MyReaderValue but for TTreeReaderArray instead of TTreeReaderValue
    template <class T>
        class MyReaderArray {
            public:
                MyReaderArray(const bool read_branch,TTreeReader &reader,const std::string &branchname,const std::string &systematic,bool has_systematic_variations,bool affects_weight_only=false,const T &defaultval=T()) : m_read_branch(read_branch) {
                    std::string branchname_with_syst = branchname;
                    if(has_systematic_variations) {
                        branchname_with_syst += "___";
                        branchname_with_syst += systematic;
                        auto branches = *(reader.GetTree()->GetListOfBranches());
                        bool branch_has_this_systematic = false;
                        for(int i = 0; i < reader.GetTree()->GetNbranches(); i++) {
                            if(branchname_with_syst.compare(((TBranch *)(branches[i]))->GetName()) == 0) {
                                branch_has_this_systematic = true;
                                break;
                            }
                        }
                        if(branch_has_this_systematic && systematic.compare("NOSYS") != 0) {
                            std::lock_guard<std::mutex> lg(vll_helpers::systematics_mutex);
                            vll_helpers::systematics[systematic] &= affects_weight_only;
                        } else {
                            branchname_with_syst = branchname;
                            branchname_with_syst += "___NOSYS";
                        }
                    }
                    if(read_branch) {
                        m_reader_array = new TTreeReaderArray<T>(reader,branchname_with_syst.c_str());
                        m_defaultval = NULL;
                    } else {
                        m_defaultval = new T(defaultval);
                        m_reader_array = NULL;
                    }
                }
                ~MyReaderArray() {delete m_reader_array; delete m_defaultval;}

                T &operator[](int i) {return (m_read_branch ? (*m_reader_array)[i] : *m_defaultval );}
                Int_t GetSize() { return m_read_branch ? m_reader_array->GetSize() : -1;}

            private:
                bool m_read_branch;

                TTreeReaderArray<T> *m_reader_array;
                T *m_defaultval;
        };

    // Internal function called by open_root_files, this is separately defined to enable a recursive call.
    namespace {
        void _open_root_files(const std::vector<std::string> &inputs,std::vector<std::string> &rootfiles) {
            std::vector<std::string> subdirs;
            for(auto input : inputs) {
                struct dirent *dirent;
                DIR *dir = opendir(input.c_str());
                if(dir == NULL && errno == ENOTDIR) {
                    if(input.substr(input.find_last_of(".")+1) == "root")
                        rootfiles.push_back(input.c_str());
                } else if(dir != NULL) {
                    while((dirent = readdir(dir)) != NULL)
                        if((dirent->d_name)[0] != '.')
                            subdirs.push_back(input+"/"+dirent->d_name);
                }
            }
            if(!subdirs.empty())
                _open_root_files(subdirs,rootfiles);
        }
    }

    // Recursively find ROOT files in the directories specified in 'inputs', the output is sorted and each element is unique
    std::unique_ptr<std::vector<std::string>> open_root_files(const std::vector<std::string> &inputs) {
        std::unique_ptr<std::vector<std::string>> ret(new std::vector<std::string>());
        _open_root_files(inputs,*ret);
        std::sort(ret->begin(),ret->end());
        ret->erase(std::unique(ret->begin(),ret->end()),ret->end());
        return std::move(ret);
    }

    // Return a list of names of TTree objects inside the TFile specified by "filename"
    std::unique_ptr<std::vector<std::string>> get_list_of_trees(const std::string &filename) {
        std::unique_ptr<std::vector<std::string>> ret(new std::vector<std::string>());
        TFile f(filename.c_str(),"READ");
        for(auto key : *(f.GetListOfKeys())) {
            if(strcmp(((TKey *)key)->ReadObj()->ClassName(),"TTree") == 0)
                ret->push_back(key->GetName());

        }

        f.Close();
        return std::move(ret);
    }

    // Get the correct array index for "final" met from the "met_name" branch
    int get_met_array_index(TTree *tree) {
        std::vector<std::string> *met_name = NULL;
        tree->SetBranchAddress("met_name",&met_name);
        tree->GetEntry(0);
        if(!met_name) {
            std::cerr << "ERROR: met_name branch does not exist.  Using first value of Missing ET arrays." << std::endl;
            return 0;
        }
        int idx = std::distance(met_name->begin(),std::find(met_name->begin(),met_name->end(),"final"))-1;
        tree->SetBranchStatus("met_name",0);
        return idx;
    }

    // Loop through the branches of "tree" and get the names of systematic variations of branches that end with "NOSYS".
    void get_systematics(TTree *tree) {
        auto branches = *(tree->GetListOfBranches());
        for(int i = 0; i < tree->GetNbranches(); i++) {
            const std::string &branchname = ((TBranch *)(branches[i]))->GetName();
            int underscore_idx = branchname.find_last_of("___");
            if(underscore_idx >= branchname.length()) continue;
            if(branchname.substr(underscore_idx+1).compare("NOSYS") == 0) {
                // This branch has systematic variations!  Let's figure out what they are and add them to the systematics map if they aren't already there
                std::string branchname_nosys = branchname.substr(0,underscore_idx);
                for(int j = 0; j < tree->GetNbranches(); j++) {
                    if(j==i) continue; // Don't include NOSYS
                    const std::string &bbranchname = ((TBranch *)(branches[j]))->GetName();
                    if(bbranchname.substr(0,underscore_idx).compare(branchname_nosys) == 0) {
                        std::string syst_name = bbranchname.substr(underscore_idx+1);
//                        if(syst_name.substr(0,4).compare("GEN_") == 0) continue;
                        std::lock_guard<std::mutex> lg(vll_helpers::systematics_mutex);
                        vll_helpers::systematics[syst_name] = true;
                    }
                }
            }
        }
    }

    void add_fake_systematics(){
        for(int i = 0; i < vll_helpers::tausyst.size(); i++){
	    vll_helpers::systematics.insert(std::pair<std::string,bool>("tau_"+vll_helpers::tausyst.at(i),true));
	}
        for(int i = 0; i < vll_helpers::htausyst.size(); i++){
            vll_helpers::systematics.insert(std::pair<std::string,bool>("htau_"+vll_helpers::htausyst.at(i),true));
        }
        for(int i = 0; i < vll_helpers::elsyst.size(); i++){
            vll_helpers::systematics.insert(std::pair<std::string,bool>("el_"+vll_helpers::elsyst.at(i),true));
        }
        for(int i = 0; i < vll_helpers::musyst.size(); i++){
            vll_helpers::systematics.insert(std::pair<std::string,bool>("mu_"+vll_helpers::musyst.at(i),true));
        }
    }

    class ProgressBar {
        public:
            ProgressBar(unsigned long total, bool enabled=true) : m_enabled(enabled), m_total(total),m_current(0),m_chars_written(0),m_start_time(std::chrono::system_clock::now()) {}

            void start() {
                if(!m_enabled || m_total == 0)
                    return;
                if(m_thread.joinable())
                    stop();
                m_stop = false;
                m_current = 0;
                m_chars_written = 0;
                m_start_time = std::chrono::system_clock::now();
                m_thread = std::thread(&ProgressBar::display_progress_bar,this);
            }

            void stop() {
                m_mutex.lock();
                m_stop = true;
                m_mutex.unlock();
                if(m_thread.joinable())
                    m_thread.join();
            }

            void update_progress(unsigned long progress_made=1) {
                m_mutex.lock();
                m_current += progress_made;
                m_mutex.unlock();
            }

        private:
            void display_progress_bar() {
                char backspaces[1024];
                char spaces[1024];
                memset(backspaces,'\r',1023);
                memset(spaces,' ',1023);
                backspaces[1023] = '\0';
                spaces[1023] = '\0';
                while(true) {
                    const int BAR_LENGTH = 50;
                    m_mutex.lock();
                    bool local_m_stop = m_stop;
                    unsigned long local_current = m_current;
                    m_mutex.unlock();
                    if(local_m_stop) break;
                    int elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now()-m_start_time).count();
                    int predicted_remaining_seconds = elapsed_seconds/((double)local_current/m_total) - elapsed_seconds;

                    backspaces[m_chars_written] = '\0';
                    spaces[m_chars_written] = '\0';
                    std::cout << backspaces << spaces << backspaces << std::flush;
                    backspaces[m_chars_written] = '\r';
                    spaces[m_chars_written] = ' ';
                    m_chars_written = 0;
                    std::string to_print = std::to_string(100*local_current/m_total);
                    to_print += "% done [";
                    for(int i = 0; i < BAR_LENGTH*local_current/m_total; i++) to_print += '#';
                    for(int i = 0; i < BAR_LENGTH-(BAR_LENGTH*local_current/m_total); i++) to_print += '.';
                    to_print += "] ";
                    if(elapsed_seconds >= 60)
                        to_print += std::to_string(elapsed_seconds/60)+"m ";
                    to_print += std::to_string(elapsed_seconds%60)+"s elapsed: ETA ";
                    if(predicted_remaining_seconds >= 60)
                        to_print += std::to_string(predicted_remaining_seconds/60)+"m ";
                    to_print += std::to_string(predicted_remaining_seconds%60)+"s";
                    m_chars_written = to_print.length();
                    std::cout << to_print << std::flush;
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
                std::cout << std::endl;
            }

            bool m_enabled;
            unsigned long m_total,m_current;
            bool m_stop;
            int m_chars_written;
            std::chrono::time_point<std::chrono::system_clock> m_start_time;
            std::thread m_thread;
            std::mutex m_mutex;
    };
}

#endif
