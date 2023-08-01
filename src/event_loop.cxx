#include "vll_helpers.h"
#include "vll_fake_helpers.h"
#include "FinalStates.h"

#include "TChain.h"
#include "TDirectory.h"
#include "TEfficiency.h"
#include "TH2.h"
#include "TFile.h"
#include "TLorentzVector.h"
#include "TThread.h"
#include "ROOT/TTreeProcessorMT.hxx"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <getopt.h>

void print_help(const char *argv0) {
    std::cout << "usage: " << argv0 << " -p <campaign> -i <INPUT1> -i <INPUT2> -i ... -o <OUTPUT>\n\n" <<
        "--campaign or -p specifies the MC/data campaign for correct luminosity weighting.  Valid campaigns are: a, d, e, 1516, 17, and 18.  This argument is required.\n" <<
        "--is-data or -d if running over data (disables MC weights and scale factors).\n" <<
        "--input or -i specifies an input ROOT file or directory containing ROOT files that are recursively scanned.  At least one input is required.\n" <<
        "--output or -o specifies the output file.  If not given a default will be created as ./out.root\n" <<
        "--systematics or -s to run over systematic variations.  By default the NOSYS variation is only used.\n" <<
        "--make-fake-bkg or -f to create flat ntuples for fake backgrounds.  These ntuples contain events that are weighted with data-driven fake factors.  MC events have event weight negated.  Specify one of three arguments: ele, mu, or tau.\n" <<
        "--keep-fake-mc removes veto on events with non-truth matched objects.\n" <<
        "--disable-progress-bar to disable the progress bar for batch jobs.\n" <<
        "-N X to limit the run to X events for debugging purposes.  The output events will likely not be normalized properly.\n" <<
        "--threads X or -t X to force the program to use no more than X threads." << std::endl;
}

int main(int argc, char *argv[]) {
    // Argument parsing
    int include_fake_mc = 0;
    int do_systematics = 0;
    int do_fake_systematics = 0;
    int is_data = 0;
    int is_signal = 0;
    int is_WZ = 0;
    int is_WZ_u = 0;
    int is_WZ_d = 0;
    int nevents = 0;
    int disable_progress_bar = 0;
    enum Configuration config = nominal;
    enum Fake_Configuration fake_config = zero_fake;
    std::string campaign = "null";
    struct option long_options[] = {
        {"campaign",required_argument,0,'p'},
        {"input",required_argument,0,'i'},
        {"output",required_argument,0,'o'},
        {"keep-fake-mc",no_argument,&include_fake_mc,1},
        {"make-fake-bkg",required_argument,0,'f'},
        {"systematics",no_argument,&do_systematics,1},
	{"fake-systematics",no_argument,&do_fake_systematics,1},
        {"wz",no_argument,&is_WZ,1},
        {"wz-up",no_argument,&is_WZ_u,1},
        {"wz-down",no_argument,&is_WZ_d,1},
        {"is-mc",no_argument,&is_data,0},
        {"is-signal",no_argument,&is_signal,1},
        {"is-data",no_argument,&is_data,1},
        {"threads",required_argument,0,'t'},
        {"disable-progress-bar",no_argument,&disable_progress_bar,1},
        {"help",no_argument,0,'h'},
        {0,0,0,0}
    };
    int option_index = 0;
    int c = 0;
    std::vector<std::string> inputs;
    std::string output;
    int nthreads = 0;
    while((c = getopt_long(argc,argv,"p:i:o:f:sbwzqmdt:N:h",long_options,&option_index)) != -1) {
        switch(c) {
            case 0:
                break;
            case 'p':
                campaign = optarg;
                break;
            case 'i':
                inputs.push_back(optarg);
                break;
            case 'o':
                output = optarg;
                break;
            case 'f':
                if(strcmp(optarg,"lep") == 0)
                    config = fake_lep;
                else
                    std::cerr << "WARNING: invalid configuration: " << optarg << ".  Running with nominal configuration." << std::endl;
		    break;		
            case 's':
                do_systematics = 1;
                break;
            case 'b':
                do_fake_systematics = 1;
                break;
            case 'w':
                is_WZ = 1;
                break;
            case 'z':
                is_WZ_u = 1;
                break;
            case 'q':
                is_WZ_d = 1;
                break;
            case 'm':
                is_data = 0;
                break;
            case 'd':
                is_data = 1;
                break;
            case 't':
                nthreads = atoi(optarg);
                break;
            case 'N':
                nevents = atoi(optarg);
                break;
            case 'h':
                print_help(argv[0]);
                exit(0);
            default:
                print_help(argv[0]);
                exit(1);
        }
    }

    if(is_WZ || is_WZ_u || is_WZ_d ||){
        do_systematics=0;
        do_fake_systematics=0;
    }

    if(inputs.empty()) {
        std::cerr << "ERROR: No inputs specified." << std::endl;
        std::exit(1);
    }
    if(is_data) {
        if(campaign.compare("1516") == 0) campaign = "a";
        else if(campaign.compare("17") == 0) campaign = "d";
        else if(campaign.compare("18") == 0) campaign = "e";
    }
    if( (campaign.compare("a") != 0) && (campaign.compare("d") != 0) && (campaign.compare("e") != 0)) {
        std::cerr << "ERROR: Invalid campaign \"" << campaign << "\"." << std::endl;
        std::exit(1);
    }

    if(output.empty()) output = "out.root";
    if(nevents < 0) nevents = 0;
    if(nthreads <= 0) nthreads = std::thread::hardware_concurrency();

    vll_helpers::read_metadata(campaign, config, argv[0]);
    vll_fake_helpers::read_tau_ff(campaign, argv[0]);
    vll_fake_helpers::read_el_ff(campaign, argv[0]);
    vll_fake_helpers::read_mu_ff(campaign, argv[0]);
    vll_fake_helpers::read_htau_ff(campaign, argv[0]);

    std::unique_ptr<std::vector<std::string>> in_files = vll_helpers::open_root_files(inputs);
    std::unique_ptr<TChain> in_chain(new TChain("physics"));
    for(const auto &in_file : *in_files)
        in_chain->Add(in_file.c_str());

    std::cout << "*** Input ROOT files ***" << std::endl;
    for(const auto &in_file : *in_files)
        std::cout << "\t" << in_file << std::endl;
    std::unique_ptr<TFile> out_file(new TFile(output.c_str(),"RECREATE"));
    std::cout << std::endl << "*** Output ROOT file ***" << std::endl;
    std::cout << "\t" << output << std::endl;

    int nsystematics = 1;
    if(do_systematics) {
        vll_helpers::get_systematics(in_chain.get());
        nsystematics = vll_helpers::systematics.size()+1;  // Include NOSYS for counting reasons but don't count it when printing out.
        std::cout << "Running over " << nsystematics-1 << " systematics." << std::endl;
    }
    
    if(do_fake_systematics) {
        vll_helpers::add_fake_systematics();
        nsystematics = vll_helpers::systematics.size()+1;  // Include NOSYS for counting reasons but don't count it when printing out.
        std::cout << "Running over " << nsystematics-1 << " systematics." << std::endl;
    }

    std::map<std::string,std::vector<int>> fold_map;
    if(do_systematics || do_fake_systematics){
        for(auto it : vll_helpers::systematics){
            fold_map.insert(std::pair<std::string,std::vector<int>>(it.first,{1,1,1,1,1,1,1,1,1,1,1,1}));
        }
    }
    fold_map.insert(std::pair<std::string,std::vector<int>>("NOSYS",{1,1,1,1,1,1,1,1,1,1,1,1}));

    // Define final states
    std::vector<FinalState *> final_states;
#include "define_finalstates.cxx"

    std::unordered_map<std::string,TH1D *> h_cutflows;
    std::unordered_map<std::string,TH1D *> h_wcutflows;
    gROOT->ProcessLine("gErrorIgnoreLevel = kWarning;");
    if(nevents == 0 || nevents > in_chain->GetEntries()) nevents = in_chain->GetEntries();
    vll_helpers::ProgressBar progress_bar(nevents*nsystematics/10000,!disable_progress_bar);

    // Event loop
    Int_t met_idx = vll_helpers::get_met_array_index(in_chain.get());
    ROOT::EnableImplicitMT(nthreads);
    ROOT::TTreeProcessorMT processor(*in_chain);
    std::mutex mutex;
    unsigned long int total = 0;
    if(nevents == 0 || nevents > in_chain->GetEntries()) nevents = in_chain->GetEntries();
    std::cout << "Running over " << nevents << " events." << std::endl;
    progress_bar.start();
    auto event_loop = [&](TTreeReader &reader) {
        unsigned long int this_reader_start = reader.GetCurrentEntry();
        if(nevents != 0 && total > nevents*nsystematics) return;
        // We'll keep track of event weights while processing weight systematics in this vector: think of it as a 2D array where the first index is event number (for this TTreeReader) and the second index is the weight systematic's name.
        std::vector<std::unordered_map<std::string,Double_t>> weight_systematics;
        auto do_event_loop = [&](const std::string &systematic) {
            std::string readsystematic = systematic;
	        for(int i = 0; i < vll_helpers::elsyst.size(); i++){
     		    if(systematic.compare("el_"+vll_helpers::elsyst.at(i)) == 0){
        	        readsystematic = "NOSYS";
    		    }
	        }
	        for(int i = 0; i < vll_helpers::musyst.size(); i++){
     		    if(systematic.compare("mu_"+vll_helpers::musyst.at(i)) == 0){
        	        readsystematic = "NOSYS";
    		    }
	        }
	        for(int i = 0; i < vll_helpers::tausyst.size(); i++){
     		    if(systematic.compare("tau_"+vll_helpers::tausyst.at(i)) == 0){
        	        readsystematic = "NOSYS";
    		    }
	        }
            for(int i = 0; i < vll_helpers::htausyst.size(); i++){
                if(systematic.compare("htau_"+vll_helpers::htausyst.at(i)) == 0){
                    readsystematic = "NOSYS";
                }
            }
            reader.Restart();
            reader.SetEntry(this_reader_start);
#include "define_ntuple_branches.cxx"

            bool doing_nominal = systematic.compare("NOSYS") == 0;
            // The boolean associated to this systematic from the vll_helpers::systematics map was set by the constructors of MyReaderValue/MyReaderArray so it has to be retrieved after the constructors have all been called.
            bool is_weight_syst = !doing_nominal && vll_helpers::systematics.at(systematic);
            for(auto &fs : final_states)
                fs->AddTree(systematic);
            mutex.lock();
            if(!is_weight_syst && h_cutflows.find(systematic) == h_cutflows.end())
                h_cutflows.insert(std::pair<std::string,TH1D *>(systematic,new TH1D((std::string("cutflow_")+systematic).c_str(),"cutflow",2,0,2)));
                h_wcutflows.insert(std::pair<std::string,TH1D *>(systematic,new TH1D((std::string("wcutflow_")+systematic).c_str(),"wcutflow",2,0,2)));
            mutex.unlock();
            unsigned long int local_cuts_passed = 0;
            unsigned long int local_total = 0;
            std::unordered_map<Cuts::EventQuantity, Double_t> event_info;
            while(reader.Next()) {
#include "foreach_event.cxx"
            }
            int n_progressbar_updates = local_total/10000;
            mutex.lock();
            if((local_total%10000) + (total%10000) >= 10000) n_progressbar_updates++;
            total += local_total;
            mutex.unlock();
            for(int i = 0; i < n_progressbar_updates; i++)
                progress_bar.update_progress();
        };  // Done with do_event_loop
        // Process systematics first so all weight systematics are done before nominal run.
        for(auto it : vll_helpers::systematics){
	 //   std::cout << "now running: " << it.first << std::endl;
            do_event_loop(it.first);
	    }
        // By this point, the non-weight systemaics should all have their trees created and filled, the nominal trees should have branches created for each systematic weight, and the weight_systematics map should be filled.
        do_event_loop("NOSYS");
    };
    processor.Process(event_loop);
    progress_bar.stop();
    std::cout << "Processed " << total/nsystematics << " events." << std::endl;

    std::cout << "Writing output file ..." << std::endl;
    for(auto h : h_cutflows)
        h.second->Write();
    for(auto h : h_wcutflows)
        h.second->Write();
    for(auto &fs : final_states) {
        fs->Write();
        delete fs;
    }
    final_states.clear();
    sleep(5);

    out_file->Close();
    sleep(5);
    return 0;
}
