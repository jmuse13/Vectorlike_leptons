#! /bin/bash

# "MC16X" gets replaced with "MC16a,MC16d,MC16e" and dataX gets replaced with "data1516,data17,data18"

EXOT12_signal_ntuple_inputs=(
/raid06/users/dwilbern/OLD/signal_notrigsf/MC16X/mtaup130/
/raid06/users/dwilbern/OLD/signal_notrigsf/MC16X/mtaup200/
/raid06/users/dwilbern/OLD/signal_notrigsf/MC16X/mtaup300/
/raid06/users/dwilbern/OLD/signal_notrigsf/MC16X/mtaup400/
/raid06/users/dwilbern/OLD/signal_notrigsf/MC16X/mtaup500/
/raid06/users/dwilbern/OLD/signal_notrigsf/MC16X/mtaup600/
/raid06/users/dwilbern/OLD/signal_notrigsf/MC16X/mtaup700/
/raid06/users/dwilbern/OLD/signal_notrigsf/MC16X/mtaup800/
/raid06/users/dwilbern/OLD/signal_notrigsf/MC16X/mtaup900/
/raid06/users/dwilbern/OLD/signal_notrigsf/MC16X/mtaup1000/
/raid06/users/muse/EXOT12/MC16X/mtaup1100/
/raid06/users/muse/EXOT12/MC16X/mtaup1200/
/raid06/users/muse/EXOT12/MC16X/mtaup1300/)
EXOT12_signal_train_ntuple_inputs=(
/raid06/users/dwilbern/OLD/signal_notrigsf/MC16X/mtaup800/
/raid06/users/dwilbern/OLD/signal_notrigsf/MC16X/mtaup900/
/raid06/users/dwilbern/OLD/signal_notrigsf/MC16X/mtaup1000/)
EXOT12_bkg_ntuple_inputs=(
/raid06/users/muse/EXOT12/MC16X/WZ_MC16X/
/raid06/users/muse/EXOT12/MC16X/ZZ_MC16X/
/raid06/users/muse/EXOT12/MC16X/WW_MC16X/
/raid06/users/muse/EXOT12/MC16X/multiboson_MC16X/
/raid06/users/muse/EXOT12/MC16X/W+jets_MC16X/
/raid06/users/muse/EXOT12/MC16X/Z+jets_MC16X/
/raid06/users/muse/EXOT12/MC16X/rare_top_MC16X/
/raid06/users/muse/EXOT12/MC16X/ttbar_MC16X/
/raid06/users/muse/EXOT12/MC16X/ttz_MC16X/
/raid06/users/muse/EXOT12/MC16X/ttw_MC16X/
/raid06/users/muse/EXOT12/MC16X/tth_MC16X/)
EXOT12_wz_bkg_ntuple_inputs=(
/raid06/users/muse/EXOT12/MC16X/WZ_MC16X/)
EXOT12_twz_bkg_ntuple_inputs=(
/raid06/users/muse/EXOT12/MC16X/twz_MC16X/)
EXOT12_tz_bkg_ntuple_inputs=(
/raid06/users/muse/EXOT12/MC16X/tz_MC16X/)
EXOT12_alt_ttz_bkg_ntuple_inputs=(
/raid06/users/muse/EXOT12/MC16X/ttz_alt_MC16X/)
EXOT12_noboson_bkg_ntuple_inputs=(
/raid06/users/muse/EXOT12/MC16X/ZZ_MC16X/
/raid06/users/muse/EXOT12/MC16X/WW_MC16X/
/raid06/users/muse/EXOT12/MC16X/multiboson_MC16X/
/raid06/users/muse/EXOT12/MC16X/W+jets_MC16X/
/raid06/users/muse/EXOT12/MC16X/Z+jets_MC16X/
/raid06/users/muse/EXOT12/MC16X/single_top_MC16X/
/raid06/users/muse/EXOT12/MC16X/ttbar_MC16X/
/raid06/users/muse/EXOT12/MC16X/ttz_MC16X/
/raid06/users/muse/EXOT12/MC16X/ttw_MC16X/
/raid06/users/muse/EXOT12/MC16X/tth_MC16X/
/raid06/users/muse/EXOT12/MC16X/ttt_MC16X/
/raid06/users/muse/EXOT12/MC16X/tttt_MC16X/
/raid06/users/muse/EXOT12/MC16X/ttww_MC16X/)
EXOT12_data_ntuple_inputs=(
/raid06/users/muse/EXOT12/dataX/ )


