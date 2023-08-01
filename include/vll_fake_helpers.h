#ifndef __vll_fake_helpers_h_
#define __vll_fake_helpers_h_

#include "TLorentzVector.h"
#include <map>
#include <string>
#include <vector>
#include <TH1.h>
#include <TH2.h>
#include <iterator>

using namespace std;

namespace vll_fake_helpers {

    const std::vector<double> elpTBinsc = {20,32,34,36,38,40,45,50,60,70,75,80,85,90,95,100,125,150,175,200,225,250,275,300,325,350,375,400,425,450,475,500,1000};
    const std::vector<double> elpTBins = {30,32,34,36,38,40,45,50,60,70,75,80,85,90,95,100,125,150,175,200,225,250,275,300,325,350,375,400,425,450,475,500,1000};
    const std::vector<double> eletaBins = {0,0.7,1.37,1.52,2.01,2.47};
    const std::vector<std::string> eletaBinsStr = {"0","0.7","1.37","1.52","2.01","2.47"};

    const std::vector<double> mupTBinsc = {20,31,32,33,34,35,36,38,40,42,46,55,70,90,1000};
    const std::vector<double> mupTBins = {30,31,32,33,34,35,36,38,40,42,46,55,70,90,1000};
    const std::vector<double> muetaBins = {0,0.7,1.37,1.52,2.01,2.5};
    const std::vector<std::string> muetaBinsStr = {"0","0.7","1.37","1.52","2.01","2.5"};

    const std::vector<double> taupTBins = {20,30,40,60,90,150,999};
    const std::vector<double> htaupTBins = {20,30,40,60,90,999};
    const std::vector<int> tauprongBins = {1,3};
    const std::vector<std::string> tauprongBinsStr = {"1p","3p"};

    const std::vector<std::string> tausyst = {"NOSYS","INVM","NUMBJETS","MET","UP","DOWN","STATUP","STATDOWN","RNN","FRACUP","FRACDOWN","INVM_40","NUMBJETS_40","MET_40","UP_40","DOWN_40","STATUP_40","STATDOWN_40","RNN_40","FRACUP_40","FRACDOWN_40"};
    const std::vector<std::string> htausyst = {"NOSYS","INVM","NUMBJETS","UP","DOWN","STATUP","STATDOWN","RNN","INVM_40","NUMBJETS_40","UP_40","DOWN_40","STATUP_40","STATDOWN_40","RNN_40"};
    const std::vector<std::string> elsyst = {"NOSYS","NUMJETS","MET","UP","DOWN"};
    const std::vector<std::string> musyst = {"NOSYS","JETPT","DELPHIUP","DELPHIDOWN","MET","METUP","METDOWN","UP","DOWN"};

    struct light_lep {
        float pt;
        float eta;
    };

    std::map<std::string,TH1D*> elffmap;
    std::map<std::string,TH1D*> muffmap;
    std::map<std::string,TH1D*> tauffmap;
    std::map<std::string,TH1D*> htauffmap;
    std::map<std::string,TH1D*> gtauffmap;

    void read_tau_ff(std::string campaign, std::string argv0) {
        for(int i = 0; i < vll_fake_helpers::tauprongBinsStr.size(); i++){
	    for(int j = 0; j < vll_fake_helpers::tausyst.size(); j++){
                std::string histoFF = vll_fake_helpers::tausyst.at(j)+"_FF_" + vll_fake_helpers::tauprongBinsStr.at(i);
                vll_fake_helpers::tauffmap.insert(std::pair<std::string,TH1D*>(histoFF,new TH1D(histoFF.c_str(), histoFF.c_str(), vll_fake_helpers::taupTBins.size()-1, vll_fake_helpers::taupTBins.data())));
	    }
        }
        std::string bindir = argv0.substr(0,argv0.find_last_of("/"));
        std::string ffFile_in = bindir + "/../vectorlike-leptons-data/VLL_FF_tau.root";
        TFile *inFile = new TFile(ffFile_in.c_str());
        for(int i = 0; i < vll_fake_helpers::tauprongBinsStr.size(); i++){
	    for(int j = 0; j < vll_fake_helpers::tausyst.size(); j++){
                std::string histoFF = vll_fake_helpers::tausyst.at(j)+"_FF_"+vll_fake_helpers::tauprongBinsStr.at(i);
                if(vll_fake_helpers::tausyst.at(j) == "NOSYS" || vll_fake_helpers::tausyst.at(j) == "FRACUP_40" || vll_fake_helpers::tausyst.at(j) == "FRACDOWN_40" || vll_fake_helpers::tausyst.at(j) == "FRACUP" || vll_fake_helpers::tausyst.at(j) == "FRACDOWN"){
		    std::string inhistoFF = "NOSYS_FF_"+vll_fake_helpers::tauprongBinsStr.at(i);
                    TH1D *tem = (TH1D*)inFile->Get(inhistoFF.c_str());
                    TH1D *temp = (TH1D*)tem->Clone();
                    vll_fake_helpers::tauffmap.at(histoFF)->Add(temp);
		}
                if(vll_fake_helpers::tausyst.at(j) == "RNN" || vll_fake_helpers::tausyst.at(j) == "RNN_40"){
                    std::string althistoFF = "RNN_FF_"+vll_fake_helpers::tauprongBinsStr.at(i);
                    TH1D *tem = (TH1D*)inFile->Get(althistoFF.c_str());
                    TH1D *temp = (TH1D*)tem->Clone();
                    vll_fake_helpers::tauffmap.at(histoFF)->Add(temp);
                }
                if(vll_fake_helpers::tausyst.at(j) == "STATDOWN" || vll_fake_helpers::tausyst.at(j) == "STATDOWN_40"){
                    std::string althistoFF = "STATDOWN_FF_"+vll_fake_helpers::tauprongBinsStr.at(i);
                    TH1D *tem = (TH1D*)inFile->Get(althistoFF.c_str());
                    TH1D *temp = (TH1D*)tem->Clone();
                    vll_fake_helpers::tauffmap.at(histoFF)->Add(temp);
                }
                if(vll_fake_helpers::tausyst.at(j) == "STATUP" || vll_fake_helpers::tausyst.at(j) == "STATUP_40"){
                    std::string althistoFF = "STATUP_FF_"+vll_fake_helpers::tauprongBinsStr.at(i);
                    TH1D *tem = (TH1D*)inFile->Get(althistoFF.c_str());
                    TH1D *temp = (TH1D*)tem->Clone();
                    vll_fake_helpers::tauffmap.at(histoFF)->Add(temp);
                }
                if(vll_fake_helpers::tausyst.at(j) == "DOWN" || vll_fake_helpers::tausyst.at(j) == "DOWN_40"){
                    std::string althistoFF = "DOWN_FF_"+vll_fake_helpers::tauprongBinsStr.at(i);
                    TH1D *tem = (TH1D*)inFile->Get(althistoFF.c_str());
                    TH1D *temp = (TH1D*)tem->Clone();
                    vll_fake_helpers::tauffmap.at(histoFF)->Add(temp);
                }
                if(vll_fake_helpers::tausyst.at(j) == "UP" || vll_fake_helpers::tausyst.at(j) == "UP_40"){
                    std::string althistoFF = "UP_FF_"+vll_fake_helpers::tauprongBinsStr.at(i);
                    TH1D *tem = (TH1D*)inFile->Get(althistoFF.c_str());
                    TH1D *temp = (TH1D*)tem->Clone();
                    vll_fake_helpers::tauffmap.at(histoFF)->Add(temp);
                }
                if(vll_fake_helpers::tausyst.at(j) == "MET" || vll_fake_helpers::tausyst.at(j) == "MET_40"){
                    std::string althistoFF = "MET_FF_"+vll_fake_helpers::tauprongBinsStr.at(i);
                    TH1D *tem = (TH1D*)inFile->Get(althistoFF.c_str());
                    TH1D *temp = (TH1D*)tem->Clone();
                    vll_fake_helpers::tauffmap.at(histoFF)->Add(temp);
                }
		if(vll_fake_helpers::tausyst.at(j) == "INVM" || vll_fake_helpers::tausyst.at(j) == "INVM_40"){
		    std::string althistoFF = "INVM_FF_"+vll_fake_helpers::tauprongBinsStr.at(i);
                    TH1D *tem = (TH1D*)inFile->Get(althistoFF.c_str());
                    TH1D *temp = (TH1D*)tem->Clone();
                    vll_fake_helpers::tauffmap.at(histoFF)->Add(temp);
		}
                if(vll_fake_helpers::tausyst.at(j) == "NUMBJETS" || vll_fake_helpers::tausyst.at(j) == "NUMBJETS_40"){
                    std::string althistoFF = "NUMBJETS_FF_"+vll_fake_helpers::tauprongBinsStr.at(i);
                    TH1D *tem = (TH1D*)inFile->Get(althistoFF.c_str());
                    TH1D *temp = (TH1D*)tem->Clone();
                    vll_fake_helpers::tauffmap.at(histoFF)->Add(temp);
                }
	    }
        }

        inFile->Close();
    }

    void read_htau_ff(std::string campaign, std::string argv0) {
        for(int i = 0; i < vll_fake_helpers::tauprongBinsStr.size(); i++){
            for(int j = 0; j < vll_fake_helpers::htausyst.size(); j++){
                std::string histoFF = vll_fake_helpers::htausyst.at(j)+"_FF_" + vll_fake_helpers::tauprongBinsStr.at(i);
                vll_fake_helpers::htauffmap.insert(std::pair<std::string,TH1D*>(histoFF,new TH1D(histoFF.c_str(), histoFF.c_str(), vll_fake_helpers::htaupTBins.size()-1, vll_fake_helpers::htaupTBins.data())));
            }
        }
        std::string bindir = argv0.substr(0,argv0.find_last_of("/"));
        std::string ffFile_in = bindir + "/../vectorlike-leptons-data/VLL_FF_heavytau.root";
        TFile *inFile = new TFile(ffFile_in.c_str());
        for(int i = 0; i < vll_fake_helpers::tauprongBinsStr.size(); i++){
            for(int j = 0; j < vll_fake_helpers::htausyst.size(); j++){
                std::string histoFF = vll_fake_helpers::htausyst.at(j)+"_FF_"+vll_fake_helpers::tauprongBinsStr.at(i);
                if(vll_fake_helpers::htausyst.at(j) == "NOSYS"){
                    std::string inhistoFF = "NOSYS_FF_"+vll_fake_helpers::tauprongBinsStr.at(i);
                    TH1D *tem = (TH1D*)inFile->Get(inhistoFF.c_str());
                    TH1D *temp = (TH1D*)tem->Clone();
                    vll_fake_helpers::htauffmap.at(histoFF)->Add(temp);
                }
                if(vll_fake_helpers::htausyst.at(j) == "RNN" || vll_fake_helpers::htausyst.at(j) == "RNN_40"){
                    std::string althistoFF = "RNN_FF_"+vll_fake_helpers::tauprongBinsStr.at(i);
                    TH1D *tem = (TH1D*)inFile->Get(althistoFF.c_str());
                    TH1D *temp = (TH1D*)tem->Clone();
                    vll_fake_helpers::htauffmap.at(histoFF)->Add(temp);
                }
                if(vll_fake_helpers::htausyst.at(j) == "STATDOWN" || vll_fake_helpers::htausyst.at(j) == "STATDOWN_40"){
                    std::string althistoFF = "STATDOWN_FF_"+vll_fake_helpers::tauprongBinsStr.at(i);
                    TH1D *tem = (TH1D*)inFile->Get(althistoFF.c_str());
                    TH1D *temp = (TH1D*)tem->Clone();
                    vll_fake_helpers::htauffmap.at(histoFF)->Add(temp);
                }
                if(vll_fake_helpers::htausyst.at(j) == "STATUP" || vll_fake_helpers::htausyst.at(j) == "STATUP_40"){
                    std::string althistoFF = "STATUP_FF_"+vll_fake_helpers::tauprongBinsStr.at(i);
                    TH1D *tem = (TH1D*)inFile->Get(althistoFF.c_str());
                    TH1D *temp = (TH1D*)tem->Clone();
                    vll_fake_helpers::htauffmap.at(histoFF)->Add(temp);
                }
                if(vll_fake_helpers::htausyst.at(j) == "DOWN" || vll_fake_helpers::htausyst.at(j) == "DOWN_40"){
                    std::string althistoFF = "DOWN_FF_"+vll_fake_helpers::tauprongBinsStr.at(i);
                    TH1D *tem = (TH1D*)inFile->Get(althistoFF.c_str());
                    TH1D *temp = (TH1D*)tem->Clone();
                    vll_fake_helpers::htauffmap.at(histoFF)->Add(temp);
                }
                if(vll_fake_helpers::htausyst.at(j) == "UP" || vll_fake_helpers::htausyst.at(j) == "UP_40"){
                    std::string althistoFF = "UP_FF_"+vll_fake_helpers::tauprongBinsStr.at(i);
                    TH1D *tem = (TH1D*)inFile->Get(althistoFF.c_str());
                    TH1D *temp = (TH1D*)tem->Clone();
                    vll_fake_helpers::htauffmap.at(histoFF)->Add(temp);
                }
                if(vll_fake_helpers::htausyst.at(j) == "INVM" || vll_fake_helpers::htausyst.at(j) == "INVM_40"){
                    std::string althistoFF = "INVM_FF_"+vll_fake_helpers::tauprongBinsStr.at(i);
                    TH1D *tem = (TH1D*)inFile->Get(althistoFF.c_str());
                    TH1D *temp = (TH1D*)tem->Clone();
                    vll_fake_helpers::htauffmap.at(histoFF)->Add(temp);
                }
                if(vll_fake_helpers::htausyst.at(j) == "NUMBJETS" || vll_fake_helpers::htausyst.at(j) == "NUMBJETS_40"){
                    std::string althistoFF = "NUMBJETS_FF_"+vll_fake_helpers::tauprongBinsStr.at(i);
                    TH1D *tem = (TH1D*)inFile->Get(althistoFF.c_str());
                    TH1D *temp = (TH1D*)tem->Clone();
                    vll_fake_helpers::htauffmap.at(histoFF)->Add(temp);
                }
            }
        }
        inFile->Close();
    }

    void read_el_ff(std::string campaign, std::string argv0) {
        for(int i = 0; i < vll_fake_helpers::eletaBins.size()-1; i++){
            if(i == 2) continue;
	    for(int j = 0; j < vll_fake_helpers::elsyst.size(); j++){
                std::string histoFF = vll_fake_helpers::elsyst.at(j)+"_FF_fail_" + vll_fake_helpers::eletaBinsStr.at(i) + "-" + vll_fake_helpers::eletaBinsStr.at(i+1);
                vll_fake_helpers::elffmap.insert(std::pair<std::string,TH1D*>(histoFF,new TH1D(histoFF.c_str(), histoFF.c_str(), vll_fake_helpers::elpTBins.size()-1, vll_fake_helpers::elpTBins.data())));
	    }
        }   

        std::string bindir = argv0.substr(0,argv0.find_last_of("/"));
        std::string ffFile_in = bindir + "/../vectorlike-leptons-data/VLL_FF_el.root";
        TFile *inFile = new TFile(ffFile_in.c_str());
        for(int i = 0; i < vll_fake_helpers::eletaBins.size()-1; i++){
            if(i == 2) continue;
	    for(int j = 0; j < vll_fake_helpers::elsyst.size(); j++){
                std::string histoFF = vll_fake_helpers::elsyst.at(j)+"_FF_fail_" + vll_fake_helpers::eletaBinsStr.at(i) + "-" +  vll_fake_helpers::eletaBinsStr.at(i+1);
                TH1D *tem = (TH1D*)inFile->Get(histoFF.c_str());
                TH1D *temp = (TH1D*)tem->Clone();
                vll_fake_helpers::elffmap.at(histoFF)->Add(temp);    
	    }
        } 

        inFile->Close();
    }

    void read_mu_ff(std::string campaign, std::string argv0) {
        for(int i = 0; i < vll_fake_helpers::muetaBins.size()-1; i++){
	    for(int j = 0; j < vll_fake_helpers::musyst.size(); j++){
                std::string histoFF = vll_fake_helpers::musyst.at(j)+"_FF_fail_" + vll_fake_helpers::muetaBinsStr.at(i) + "-" + vll_fake_helpers::muetaBinsStr.at(i+1);
                vll_fake_helpers::muffmap.insert(std::pair<std::string,TH1D*>(histoFF,new TH1D(histoFF.c_str(), histoFF.c_str(), vll_fake_helpers::mupTBins.size()-1, vll_fake_helpers::mupTBins.data())));

            }
	}

        std::string bindir = argv0.substr(0,argv0.find_last_of("/"));
        std::string ffFile_in = bindir + "/../vectorlike-leptons-data/VLL_FF_mu.root";
        TFile *inFile = new TFile(ffFile_in.c_str());
        for(int i = 0; i < vll_fake_helpers::muetaBins.size()-1; i++){
	    for(int j = 0; j < vll_fake_helpers::musyst.size(); j++){
                std::string histoFF = vll_fake_helpers::musyst.at(j)+"_FF_fail_" + vll_fake_helpers::muetaBinsStr.at(i) + "-" + vll_fake_helpers::muetaBinsStr.at(i+1);
                TH1D *tem = (TH1D*)inFile->Get(histoFF.c_str());
                TH1D *temp = (TH1D*)tem->Clone();
                vll_fake_helpers::muffmap.at(histoFF)->Add(temp);
	    }
        }

        inFile->Close();
    }

    float get_event_ff(std::vector<float> light,std::vector<float> tau,std::vector<float> htau,float down_percentage,float up_percentage,float nominal_percentage,std::vector<float> taupt, std::string sys){
        int sizefake = light.size() + tau.size();
        float ff = 1;
        for(int i = 0; i < light.size(); i++){
            ff *= light.at(i);
        }
        for(int i = 0; i < tau.size(); i++){
	    float percentage = 1;
	    if(sys == "FRACDOWN"){
		if(taupt.at(i) >= 40){
		    percentage *= down_percentage;
		}
		if(taupt.at(i) < 40){
		    percentage *= nominal_percentage;
		}
	    }
            if(sys == "FRACDOWN_40"){
                if(taupt.at(i) < 40){
                    percentage *= down_percentage;
                }
                if(taupt.at(i) >= 40){
                    percentage *= nominal_percentage;
                }
            }
            if(sys == "FRACUP"){
                if(taupt.at(i) >= 40){
                    percentage *= up_percentage;
                }
                if(taupt.at(i) < 40){
                    percentage *= nominal_percentage;
                }
            }
            if(sys == "FRACUP_40"){
                if(taupt.at(i) < 40){
                    percentage *= up_percentage;
                }
                if(taupt.at(i) >= 40){
                    percentage *= nominal_percentage;
                }
            }
	    if(sys != "FRACUP" && sys != "FRACUP_40" && sys != "FRACDOWN" && sys != "FRACDOWN_40"){
		percentage *= nominal_percentage;
	    }
            float tempff = (percentage*tau.at(i))+((1-percentage)*htau.at(i));
            ff *= tempff;
        }
        if(((sizefake % 2) == 0) && (sizefake != 0)){
            ff *= -1;
        }
        return ff;
    }
    double get_el_ff(float pt,float eta,std::string sys) {
	float ff = 0;
        for(int j = 0; j < vll_fake_helpers::elpTBins.size()-1; j++){
            for(int k = 0; k < vll_fake_helpers::eletaBins.size()-1; k++){
		if(k == 2) continue;
                if((pt >=  vll_fake_helpers::elpTBinsc[j]) && (pt <  vll_fake_helpers::elpTBinsc[j+1]) &&
                    (fabs(eta) >=  vll_fake_helpers::eletaBins[k]) && (fabs(eta) <  vll_fake_helpers::eletaBins[k+1])) {
                    ff = vll_fake_helpers::elffmap.at(sys+"_FF_fail_"+vll_fake_helpers::eletaBinsStr[k]+"-"+vll_fake_helpers::eletaBinsStr[k+1])->GetBinContent(j+1);
		}    
            }
        }
        for(int k = 0; k < vll_fake_helpers::eletaBins.size()-1; k++){
            if(k == 2) continue;
            if((pt < vll_fake_helpers::elpTBinsc[0]) && (fabs(eta) >= vll_fake_helpers::eletaBins[k]) && 
		(fabs(eta) < vll_fake_helpers::eletaBins[k+1])) {
                ff = vll_fake_helpers::elffmap.at(sys+"_FF_fail_"+vll_fake_helpers::eletaBinsStr[k]+"-"+vll_fake_helpers::eletaBinsStr[k+1])->GetBinContent(1);
            }
        }    
        return ff;
    }

    double get_mu_ff(float pt,float eta,std::string sys){
        float ff = 0;
        for(int j = 0; j < vll_fake_helpers::mupTBins.size()-1; j++){
            for(int k = 0; k < vll_fake_helpers::muetaBins.size()-1; k++){
                if((pt >= vll_fake_helpers::mupTBinsc[j]) && (pt < vll_fake_helpers::mupTBinsc[j+1]) &&
                        (fabs(eta) >= vll_fake_helpers::muetaBins[k]) && (fabs(eta) < vll_fake_helpers::muetaBins[k+1])){
                    ff = vll_fake_helpers::muffmap.at(sys+"_FF_fail_"+vll_fake_helpers::muetaBinsStr[k]+"-"+vll_fake_helpers::muetaBinsStr[k+1])->GetBinContent(j+1);
        	}
	    }
	}
        for(int k = 0; k < vll_fake_helpers::muetaBins.size()-1; k++){
            if((pt < vll_fake_helpers::mupTBinsc[0]) &&
                    (fabs(eta) >= vll_fake_helpers::muetaBins[k]) && (fabs(eta) < vll_fake_helpers::muetaBins[k+1])) {
                ff = vll_fake_helpers::muffmap.at(sys+"_FF_fail_"+vll_fake_helpers::muetaBinsStr[k]+"-"+vll_fake_helpers::muetaBinsStr[k+1])->GetBinContent(1);
            }
        }
        return ff;
    }

    double get_tau_ff(int prong,float pt,std::string sys){  
        float ff = 0;
        if(sys == "INVM_40" || sys == "NUMBJETS_40" || sys == "MET_40" || sys == "UP_40" || sys == "DOWN_40" || sys == "STATUP_40" || sys == "STATDOWN_40" || sys == "RNN_40"){
            for(int j = 0; j < vll_fake_helpers::taupTBins.size()-1; j++){
                for(int k = 0; k < vll_fake_helpers::tauprongBins.size(); k++){
		    if(pt < 40){
                        if((pt >= vll_fake_helpers::taupTBins[j]) && (pt < vll_fake_helpers::taupTBins[j+1]) && (prong == vll_fake_helpers::tauprongBins[k])) {
                            ff = vll_fake_helpers::tauffmap.at(sys+"_FF_"+vll_fake_helpers::tauprongBinsStr[k])->GetBinContent(j+1);
			}
		    }
		    if(pt >= 40){
		        if((pt >= vll_fake_helpers::taupTBins[j]) && (pt < vll_fake_helpers::taupTBins[j+1]) && (prong == vll_fake_helpers::tauprongBins[k])) {
                            ff = vll_fake_helpers::tauffmap.at("NOSYS_FF_"+vll_fake_helpers::tauprongBinsStr[k])->GetBinContent(j+1);
			}
		    }
                }
	    }
	}
        if(sys == "INVM" || sys == "NUMBJETS" || sys == "MET" || sys == "UP" || sys == "DOWN" || sys == "STATUP" || sys == "STATDOWN" || sys == "RNN"){
            for(int j = 0; j < vll_fake_helpers::taupTBins.size()-1; j++){
                for(int k = 0; k < vll_fake_helpers::tauprongBins.size(); k++){
                    if(pt >= 40){
                        if((pt >= vll_fake_helpers::taupTBins[j]) && (pt < vll_fake_helpers::taupTBins[j+1]) && (prong == vll_fake_helpers::tauprongBins[k])) {
                            ff = vll_fake_helpers::tauffmap.at(sys+"_FF_"+vll_fake_helpers::tauprongBinsStr[k])->GetBinContent(j+1);
                        }
                    }
                    if(pt < 40){
                        if((pt >= vll_fake_helpers::taupTBins[j]) && (pt < vll_fake_helpers::taupTBins[j+1]) && (prong == vll_fake_helpers::tauprongBins[k])) {
                            ff = vll_fake_helpers::tauffmap.at("NOSYS_FF_"+vll_fake_helpers::tauprongBinsStr[k])->GetBinContent(j+1);
                        }
                    }
                }
            }
        }
	if(sys == "NOSYS"|| sys == "FRACUP_40" || sys == "FRACDOWN_40" || sys == "FRACUP" || sys == "FRACDOWN"){
            for(int j = 0; j < vll_fake_helpers::taupTBins.size()-1; j++){
                for(int k = 0; k < vll_fake_helpers::tauprongBins.size(); k++){
                    if((pt >= vll_fake_helpers::taupTBins[j]) && (pt < vll_fake_helpers::taupTBins[j+1]) && (prong == vll_fake_helpers::tauprongBins[k])) {
                        ff = vll_fake_helpers::tauffmap.at("NOSYS_FF_"+vll_fake_helpers::tauprongBinsStr[k])->GetBinContent(j+1);
                    }
                }
            }
        }
        return ff;
    }
    double get_htau_ff(int prong,float pt,std::string sys){
        float ff = 0;
        if(sys == "INVM_40" || sys == "NUMBJETS_40" || sys == "UP_40" || sys == "DOWN_40" || sys == "STATUP_40" || sys == "STATDOWN_40" || sys == "RNN_40"){
            for(int j = 0; j < vll_fake_helpers::htaupTBins.size()-1; j++){
                for(int k = 0; k < vll_fake_helpers::tauprongBins.size(); k++){
                    if(pt < 40){
                        if((pt >= vll_fake_helpers::htaupTBins[j]) && (pt < vll_fake_helpers::htaupTBins[j+1]) && (prong == vll_fake_helpers::tauprongBins[k])) {
                            ff = vll_fake_helpers::htauffmap.at(sys+"_FF_"+vll_fake_helpers::tauprongBinsStr[k])->GetBinContent(j+1);
                        }
                    }
                    if(pt >= 40){ 
                        if((pt >= vll_fake_helpers::htaupTBins[j]) && (pt < vll_fake_helpers::htaupTBins[j+1]) && (prong == vll_fake_helpers::tauprongBins[k])) {
                            ff = vll_fake_helpers::htauffmap.at("NOSYS_FF_"+vll_fake_helpers::tauprongBinsStr[k])->GetBinContent(j+1);
                        }
                    }
                }
            }
        }
        if(sys == "INVM" || sys == "NUMBJETS" || sys == "UP" || sys == "DOWN" || sys == "STATUP" || sys == "STATDOWN" || sys == "RNN"){
            for(int j = 0; j < vll_fake_helpers::htaupTBins.size()-1; j++){
                for(int k = 0; k < vll_fake_helpers::tauprongBins.size(); k++){
                    if(pt >= 40){
                        if((pt >= vll_fake_helpers::htaupTBins[j]) && (pt < vll_fake_helpers::htaupTBins[j+1]) && (prong == vll_fake_helpers::tauprongBins[k])) {
                            ff = vll_fake_helpers::htauffmap.at(sys+"_FF_"+vll_fake_helpers::tauprongBinsStr[k])->GetBinContent(j+1);
                        }
                    }
                    if(pt < 40){
                        if((pt >= vll_fake_helpers::htaupTBins[j]) && (pt < vll_fake_helpers::htaupTBins[j+1]) && (prong == vll_fake_helpers::tauprongBins[k])) {
                            ff = vll_fake_helpers::htauffmap.at("NOSYS_FF_"+vll_fake_helpers::tauprongBinsStr[k])->GetBinContent(j+1);
                        }
                    }
                }
            }
        }
        if(sys == "NOSYS"){
            for(int j = 0; j < vll_fake_helpers::htaupTBins.size()-1; j++){
                for(int k = 0; k < vll_fake_helpers::tauprongBins.size(); k++){
                    if((pt >= vll_fake_helpers::htaupTBins[j]) && (pt < vll_fake_helpers::htaupTBins[j+1]) && (prong == vll_fake_helpers::tauprongBins[k])) {
                        ff = vll_fake_helpers::htauffmap.at("NOSYS_FF_"+vll_fake_helpers::tauprongBinsStr[k])->GetBinContent(j+1);
                    }
                }
            }
        }
        return ff;
    }
}

#endif
